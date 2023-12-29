/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <libfdt.h>
#include <ndelay.h>

#include <platform_def.h>
#include <baikal_sip_svc.h>
#include <bs1000_cmu.h>
#include <bs1000_def.h>
#include <bs1000_private.h>
#include <bs1000_scp_lcru.h>

static int reg_write(uintptr_t addr, uint32_t  value);
static int reg_read (uintptr_t addr, uint32_t *value);

#define MHZ 1000000L

/* type */
#define CLK_TYPE_NO	0
#define CLK_TYPE_REFCLK	1 /* fout = 25MHz || 100MHz		*/
#define CLK_TYPE_PLL	2 /* fout = fref * NF / (NR * OD)	*/
#define CLK_TYPE_CLKCH	3 /* fout = fref / (VAL_CLKDIV + 1)	*/
#define CLK_TYPE_CLKEN	4 /* fout = fref / DIV_VAL		*/
#define CLK_TYPE_CLKDIV	5 /* fout = fref / 2^(DIV_VAL - 1)	*/

struct clk_ops {
	int64_t	(*get	    )(uintptr_t base);
	int64_t	(*set	    )(uintptr_t base, int64_t freq);
	int64_t	(*disable   )(uintptr_t base);
	int64_t	(*enable    )(uintptr_t base);
	int64_t	(*is_enabled)(uintptr_t base);
	int64_t	(*round	    )(uintptr_t base, int64_t freq);
};

struct clk_desc {
	uintptr_t		base;		/* "reg"		*/
	int			type;		/* "type"		*/
	const char		*name;		/* "clock-output-names" */
	const struct clk_ops	*ops;
	struct clk_desc		*parent;	/* "clocks"		*/
};

typedef struct {
	uint32_t rst	:1;
	uint32_t bypass	:1;
	uint32_t swen	:1;
	uint32_t swrst	:1;
	uint32_t divr	:9 - 4   + 1; /* 6, max = 63	*/
	uint32_t _	:11 - 10 + 1;
	uint32_t divf	:20 - 12 + 1; /* 9, max = 511	*/
	uint32_t __	:23 - 21 + 1;
	uint32_t divq	:26 - 24 + 1; /* 3, max = 7	*/
	uint32_t ___	:1;
	uint32_t range	:30 - 28 + 1;
	uint32_t lock	:1;
} cmu_pll_t;

#define DIVR_MAX 63
#define DIVF_MAX 511
#define DIVQ_MAX 7
#define DIVR_MIN 0
#define DIVF_MIN 0
#define DIVQ_MIN 1

typedef struct {
	uint32_t clk_en		:1;
	uint32_t swrst		:1;
	uint32_t set_clkdiv	:1;
	uint32_t _		:1;
	uint32_t val_clkdiv	:11 - 4  + 1; /* 8, max = 255 */
	uint32_t __		:29 - 12 + 1;
	uint32_t clk_rdy	:1;
	uint32_t lock_clkdiv	:1;
} cmu_clkch_t;
#define VAL_CLKDIV_MAX 255

typedef struct {
	uint32_t clk_en		:1;
	uint32_t div_val_set	:1;
	uint32_t div_val	:6 - 2  + 1; /* 5, max = 31 */
	uint32_t _		:30 - 7 + 1;
	uint32_t lock		:1;
} cmu_clkdiv_t;
#define DIV_VAL_MAX 31

#define cmu_clken_t cmu_clkdiv_t

static const char *noname = "";

static const struct clk_ops ops_clkref = {
	.get		= cmu_clkref_get_rate,
	.set		= cmu_clkref_set_rate,
	.disable	= cmu_clkref_disable,
	.enable		= cmu_clkref_enable,
	.is_enabled	= cmu_clkref_is_enabled,
	.round		= cmu_clkref_round_rate,
};

static const struct clk_ops ops_pll = {
	.get		= cmu_pll_get_rate,
	.set		= cmu_pll_set_rate,
	.disable	= cmu_pll_disable,
	.enable		= cmu_pll_enable,
	.is_enabled	= cmu_pll_is_enabled,
	.round		= cmu_pll_round_rate,
};

static const struct clk_ops ops_clkch = {
	.get		= cmu_clkch_get_rate,
	.set		= cmu_clkch_set_rate,
	.disable	= cmu_clkch_disable,
	.enable		= cmu_clkch_enable,
	.is_enabled	= cmu_clkch_is_enabled,
	.round		= cmu_clkch_round_rate,
};

static const struct clk_ops ops_clkdiv = {
	.get		= cmu_clkdiv_get_rate,
	.set		= cmu_clkdiv_set_rate,
	.disable	= cmu_clkdiv_disable,
	.enable		= cmu_clkdiv_enable,
	.is_enabled	= cmu_clkdiv_is_enabled,
	.round		= cmu_clkdiv_round_rate,
};

static const struct clk_ops ops_clken = {
	.get		= cmu_clken_get_rate,
	.set		= cmu_clken_set_rate,
	.disable	= cmu_clken_disable,
	.enable		= cmu_clken_enable,
	.is_enabled	= cmu_clken_is_enabled,
	.round		= cmu_clken_round_rate,
};

struct clk_desc *cmu_desc_get_by_idx(int idx)
{
	static struct clk_desc clks[400 * PLATFORM_CHIP_COUNT]; /* TODO: allocate memory */

	if (idx >= ARRAY_SIZE(clks)) {
		return NULL;
	}

	return &clks[idx];
}

struct clk_desc *cmu_desc_get_by_base(uintptr_t base)
{
	struct clk_desc *clk;
	int idx;

	for (idx = 0; ; idx++) {
		clk = cmu_desc_get_by_idx(idx);
		if (!clk) {
			break;
		}

		if (clk->base == base) {
			return clk;
		}
	}

	return NULL;
}

struct clk_desc *cmu_desc_create(void *fdt, int offs, int index)
{
	static int next;
	uintptr_t base = 0;
	const fdt32_t *prop;
	int proplen;
	struct clk_desc *clk;

	/* base */
	prop = fdt_getprop(fdt, offs, "reg", &proplen);
	if (prop == NULL || proplen <= 0) {
		return NULL;
	}
	/* Avoid 'fdt64_to_cpu()' with prop pointer: it could lead to unaligned access */
	base = fdt32_to_cpu(prop[0]);
	base <<= 32;
	base |= fdt32_to_cpu(prop[1]);

	/* index */
	prop = fdt_getprop(fdt, offs, "clock-indices", &proplen);
	int cnt0 = proplen / sizeof(uint32_t);
	int cnt1 = fdt_stringlist_count(fdt, offs, "clock-output-names");
	int cnt = cnt0 > cnt1 ? cnt0 : cnt1;

	if (cnt < 0) {
		cnt = 1;
	}

	if (index > cnt - 1) {
		return NULL;
	}

	if (cnt0 > 0) {
		base += fdt32_to_cpu(prop[index]) * 0x10;
	} else if (cnt1 > 0) {
		base += index * 0x10;
	} else {
		/* base = base; */
	}

	/* already ? */
	clk = cmu_desc_get_by_base(base);
	if (clk) {
		return clk;
	}

	/* new */
	clk = cmu_desc_get_by_idx(next++);
	if (!clk) {
		return NULL;
	}
	clk->type = CLK_TYPE_NO;
	clk->name = noname;

	/* base */
	clk->base = base;

	/* type */
	prop = fdt_getprop(fdt, offs, "type", &proplen);
	if (proplen > 0) {
		clk->type = fdt32_to_cpu(prop[0]);
	}

	switch (clk->type) {
	case CLK_TYPE_REFCLK:
		clk->ops = &ops_clkref;
		break;
	case CLK_TYPE_PLL:
		clk->ops = &ops_pll;
		break;
	case CLK_TYPE_CLKCH:
		clk->ops = &ops_clkch;
		break;
	case CLK_TYPE_CLKEN:
		clk->ops = &ops_clken;
		break;
	case CLK_TYPE_CLKDIV:
		clk->ops = &ops_clkdiv;
		break;
	default:
		return NULL;
	}

	/* name */
	prop = fdt_getprop(fdt, offs, "clock-output-names", &proplen);
	if (proplen > 0) {
		clk->name = fdt_stringlist_get(fdt, offs, "clock-output-names", index, NULL);
	}

	/* parent */
	prop = fdt_getprop(fdt, offs, "clocks", &proplen);
	if (proplen > 0) {
		int phandle = fdt32_to_cpu(prop[0]);
		int idx = 0;

		if (proplen > sizeof(uint32_t)) {
			idx = fdt32_to_cpu(prop[1]);
		}

		int offs2 = fdt_node_offset_by_phandle(fdt, phandle);

		clk->parent = cmu_desc_create(fdt, offs2, idx);
		if (clk->parent == NULL) {
			return NULL;
		}
	}

	/* freq */
	prop = fdt_getprop(fdt, offs, "clock-frequency", &proplen);
	if (proplen > 0) {
		uint64_t freq = fdt32_to_cpu(*prop);

		if (freq != clk->ops->get(base)) {
			clk->ops->set(base, freq);
		}
	}

	/* info */
	VERBOSE("%s: base=0x%lx, name=%s, is_en=%ld, freq=%ld\n", __func__,
		clk->base,
		clk->name,
		clk->ops->is_enabled(base),
		clk->ops->get(base));

	return clk;
}

int cmu_desc_init(void)
{
	int offs = 0;
	int i;
	void *fdt = (void *)(uintptr_t)BAIKAL_NS_DTB_BASE;
	int ret = fdt_open_into(fdt, fdt, BAIKAL_DTB_MAX_SIZE);

	if (ret < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", fdt, ret);
		return -1;
	}

	for (;;) {
		offs = fdt_next_node(fdt, offs, NULL);
		if (offs < 0) {
			break;
		}

		if (!fdt_node_check_compatible(fdt, offs, "baikal,bs1000-cmu")) {
			for (i = 0; ; i++) {
				if (cmu_desc_create(fdt, offs, i) == NULL) {
					break;
				}
			}
		}
	}

	return 0;
}

static int reg_write(uintptr_t addr, uint32_t value)
{
	unsigned int chip = PLATFORM_ADDR_CHIP(addr);

	if ((addr >= PLATFORM_ADDR_OUT_CHIP(chip, SCP_LCRU_BASE)) &&
	    (addr <  PLATFORM_ADDR_OUT_CHIP(chip, SCP_LCRU_BASE) + SCP_LCRU_SIZE)) {
		int ret = scp_lcru_write(addr, value);

		if (ret < 0) {
			return ret;
		}
	} else {
		mmio_write_32(addr, value);
	}

	return 0;
}

static int reg_read(uintptr_t addr, uint32_t *value)
{
	unsigned int chip = PLATFORM_ADDR_CHIP(addr);

	if ((addr >= PLATFORM_ADDR_OUT_CHIP(chip, SCP_LCRU_BASE)) &&
	    (addr <  PLATFORM_ADDR_OUT_CHIP(chip, SCP_LCRU_BASE) + SCP_LCRU_SIZE)) {
		int ret = scp_lcru_read(addr, value);

		if (ret < 0) {
			return ret;
		}
	} else {
		*value = mmio_read_32(addr);
	}

	return 0;
}

int cmu_log2(int a)
{
	int n = 0;

	while (a >>= 1) {
		n++;
	}

	return n;
}

int cmu_calc_range(int64_t freq)
{
	if (freq > 200 * MHZ) {
		return -1;
	}

	/* TODO: is it correct? */
	if (freq > 130 * MHZ) {
		return 111; /* TODO: 111 != 7 */
	} else if (freq > 80 * MHZ) {
		return 110; /* TODO: 110 != 6 */
	} else if (freq > 50 * MHZ) {
		return 101; /* TODO: 101 != 5 */
	} else if (freq > 30 * MHZ) {
		return 100; /* TODO: 100 != 4 */
	} else if (freq > 18 * MHZ) {
		return 011; /* TODO: 011 != 3 */
	} else if (freq > 11 * MHZ) {
		return 010; /* TODO: 010 != 2 */
	} else if (freq >  7 * MHZ) {
		return 001;
	} else {
		return 000; /* bypass */
	}
}

int64_t cmu_get_ref(uintptr_t base)
{
	struct clk_desc *clk = cmu_desc_get_by_base(base);

	if (!clk) {
		return -1;
	}

	struct clk_desc *parent = clk->parent;

	if (!parent) {
		return -2;
	}

	int64_t freq = parent->ops->get(parent->base);

	return freq;
}

int64_t cmu_pll_calc(int64_t fref, void *div)
{
	/* fout = 2 * (fref * NF) / (NR * OD) */
	int NR, NF, OD;
	cmu_pll_t *pll = div;

	NF = pll->divf + 1;
	NR = pll->divr + 1;
	OD = 1 << pll->divq;
	if (!NR || !OD) {
		return -1;
	}

	return 2 * (fref * NF) / (NR * OD);
}

int64_t cmu_pll_div_set(uintptr_t base, void *div)
{
	uint32_t raw;
	cmu_pll_t *reg = (void *)&raw;
	cmu_pll_t *new = div;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}

	reg->divr = new->divr;
	reg->divf = new->divf;
	reg->divq = new->divq;
	reg->range = new->range;

	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

int64_t cmu_pll_div_get(uintptr_t base, void *div)
{
	int ret;

	ret = reg_read(base, div);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_pll_div_calc(int64_t freq, int64_t fref, void *div)
{
	/* fout = 2 * (fref * NF) / (NR * OD) */
	int ret = -1;
	int64_t fout = -1;

	if (!freq || !fref || !div) {
		return -1;
	}

	cmu_pll_t *pll = (void *)div;
	int divq; /* [1-7]  ::  {2, 4, 8, 16, 32, 64, 128} */
	int divr; /* [0-63]	*/
	int divf; /* [0-511]	*/
	int64_t err = LLONG_MAX;
	int64_t err2 = LLONG_MAX;

	for (divq = DIVQ_MIN; divq <= DIVQ_MAX; divq++) {
		for (divr = DIVR_MIN; divr <= DIVR_MAX; divr++) {
			int64_t fvco;
			int64_t fnr;
			int NR, OD, NF;

			NR = divr + 1;
			OD = 1 << divq;
			NF = (freq * NR * OD) / (fref * 2);
			divf = NF - 1;

			if (!NR || !OD || !NF || divf < DIVF_MIN || divf > DIVF_MAX) {
				continue;
			}

			fout = 2 * (fref * NF) / (NR * OD);
			fvco = 2 * (fref * NF) / (NR);
			fnr = (fref) / (NR);

			if ((fnr  < 7    * MHZ) || (fnr  > 200  * MHZ) ||
			    (fvco < 3000 * MHZ) || (fvco > 6000 * MHZ) ||
			    (fout < 23   * MHZ) || (fout > 3000 * MHZ)) {
				continue;
			}

			err = fout - freq;
			err = err > 0 ? err : -err;

			if (err < err2) {
				err2 = err;
				pll->divq = divq;
				pll->divr = divr;
				pll->divf = divf;
				pll->range = cmu_calc_range(fnr);
				ret = 0;
			}
		}
	}

	return ret;
}

int64_t cmu_clkch_calc(int64_t fref, void *div)
{
	/* fout = fref / (VAL_CLKDIV + 1) */
	cmu_clkch_t *reg = div;
	int dd = reg->val_clkdiv + 1;

	if (!dd || reg->val_clkdiv > VAL_CLKDIV_MAX) {
		return -1;
	}

	return fref / dd;
}

int64_t cmu_clkch_div_set(uintptr_t base, void *div)
{
	uint32_t raw;
	cmu_clkch_t *reg = (void *)&raw;
	cmu_clkch_t *new = div;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}

	reg->val_clkdiv = new->val_clkdiv;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_clkch_div_get(uintptr_t base, void *div)
{
	int ret;

	ret = reg_read(base, div);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_clkch_div_calc(int64_t freq, int64_t fref, void *div)
{
	/* fout = fref / (VAL_CLKDIV + 1) */
	int ret = -1;
	int64_t fout = -1;

	if (!freq || !fref || !div) {
		return -1;
	}

	cmu_clkch_t *new = div;
	int div0;
	int val_clkdiv;
	int64_t err = LLONG_MAX;
	int64_t err2 = LLONG_MAX;

	div0 = fref / freq - 1;
	for (val_clkdiv = div0 - 1; val_clkdiv <= div0 + 1; val_clkdiv++) {
		int dd = val_clkdiv + 1;

		if (!dd || dd < 0 || val_clkdiv > VAL_CLKDIV_MAX) {
			continue;
		}

		fout = fref / dd;
		err = fout - freq;
		err = err > 0 ? err : -err;

		if (err < err2) {
			err2 = err;
			new->val_clkdiv = val_clkdiv;
			ret = 0;
		}
	}
	if (ret < 0) {
		return ret;
	}
	return 0;
}

int64_t cmu_clkdiv_calc(int64_t fref, void *div)
{
	/* fout = fref / 2^(DIV_VAL - 1) */
	cmu_clkdiv_t *reg = div;
	int dd = 1 << (reg->div_val - 1);

	if (!dd || reg->div_val < 1 || reg->div_val > DIV_VAL_MAX) {
		return -1;
	}

	return fref / dd;
}

int64_t cmu_clkdiv_div_set(uintptr_t base, void *div)
{
	uint32_t raw;
	cmu_clkdiv_t *reg = (void *)&raw;
	cmu_clkdiv_t *new = div;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}

	reg->div_val = new->div_val;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_clkdiv_div_get(uintptr_t base, void *div)
{
	int ret;

	ret = reg_read(base, div);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

int64_t cmu_clkdiv_div_calc(int64_t freq, int64_t fref, void *div)
{
	/* fout = fref / 2^(DIV_VAL - 1) */
	int ret = -1;
	int64_t fout = -1;

	if (!freq || !fref || !div) {
		return -1;
	}

	cmu_clkdiv_t *new = div;
	int div0;
	int div_val;
	int64_t err = LLONG_MAX;
	int64_t err2 = LLONG_MAX;

	div0 = cmu_log2(fref / freq) + 1;
	for (div_val = div0 - 1; div_val <= div0 + 1; div_val++) {
		int dd = 1 << (div_val - 1);

		if (!dd || div_val < 1 || div_val > DIV_VAL_MAX) {
			continue;
		}

		fout = fref / dd;
		err = fout - freq;
		err = err > 0 ? err : -err;

		if (err < err2) {
			err2 = err;
			new->div_val = div_val;
			ret = 0;
		}
	}

	return ret;
}

int64_t cmu_clken_calc(int64_t fref, void *div)
{
	/* fout = fref / DIV_VAL */
	cmu_clkdiv_t *reg = div;
	int dd = reg->div_val;

	if (!dd || reg->div_val > DIV_VAL_MAX) {
		return -1;
	}

	return fref / dd;
}

int64_t cmu_clken_div_set(uintptr_t base, void *div)
{
	uint32_t raw;
	cmu_clkdiv_t *reg = (void *)&raw;
	cmu_clkdiv_t *new = div;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->div_val = new->div_val;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

int64_t cmu_clken_div_get(uintptr_t base, void *div)
{
	int ret;

	ret = reg_read(base, div);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

int64_t cmu_clken_div_calc(int64_t freq, int64_t fref, void *div)
{
	/* fout = fref / DIV_VAL */
	int ret = -1;
	int64_t fout = -1;

	if (!freq || !fref || !div) {
		return -1;
	}

	cmu_clkdiv_t *new = div;
	int div0;
	int div_val;
	int64_t err = LLONG_MAX;
	int64_t err2 = LLONG_MAX;

	div0 = fref / freq;
	for (div_val = div0 - 1; div_val <= div0 + 1; div_val++) {
		int dd = div_val;

		if (!dd || dd < 0 || div_val > DIV_VAL_MAX) {
			continue;
		}

		fout = fref / dd;
		err = fout - freq;
		err = err > 0 ? err : -err;

		if (err < err2) {
			err2 = err;
			new->div_val = div_val;
			ret = 0;
		}
	}

	return ret;
}

int64_t cmu_clkref_get_rate(uintptr_t base)
{
	int64_t freq;

#ifdef BAIKAL_QEMU
	freq = 25 * MHZ;
#else
	/*
	 * name: BK_GLOBAL_BOOT_CONTROL_REG, GPR45(RW)
	 * offset: 0x168
	 *     bit1 = 0;  ref_clk0, ref_clk1 = 25MHz
	 *     bit1 = 1;  ref_clk0, ref_clk1 = 100MHz
	 */

	/*
	 * name: BK_SCP_BOOT_INFO_REG, GPR48(RO)
	 * offset: 0x184
	 *     bit2 = 0;  ref_clk0, ref_clk1 = 25MHz
	 *     bit2 = 1;  ref_clk0, ref_clk1 = 100MHz
	 */
	uint32_t raw;
	int ret;
	unsigned int chip = PLATFORM_ADDR_CHIP(base);

	ret = reg_read(PLATFORM_ADDR_OUT_CHIP(chip, SCP_GPR_BOOT_INFO_STS),
		       &raw);
	if (ret < 0) {
		return ret;
	}

	if (raw & BIT(2)) {
		freq = 100 * MHZ;
	} else {
		freq = 25 * MHZ;
	}
#endif
	return freq;
}

int64_t cmu_clkref_set_rate(uintptr_t base, int64_t freq)
{
	return -1;
}

int64_t cmu_clkref_disable(uintptr_t base)
{
	return -1;
}

int64_t cmu_clkref_enable(uintptr_t base)
{
	return -1;
}

int64_t cmu_clkref_is_enabled(uintptr_t base)
{
	return 1;
}

int64_t cmu_clkref_round_rate(uintptr_t base, int64_t freq)
{
	return -1;
}

int64_t cmu_pll_get_rate(uintptr_t base)
{
	int64_t freq;
	cmu_pll_t div;
	int64_t fref = cmu_get_ref(base);

	if (fref < 0) {
		return -1;
	}

	if (cmu_pll_div_get(base, &div)) {
		return -1;
	}

	freq = cmu_pll_calc(fref, &div);
	return freq;
}

int64_t cmu_pll_set_rate(uintptr_t base, int64_t freq)
{
	uint32_t raw;
	cmu_pll_t *reg = (void *)&raw;
	int ret;

	/* 1. SWEN = 0 */
	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->swen = 0;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	/* 2. BYPASS = 1, RST = 1 */
	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->bypass = 1;
	reg->rst = 1;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	/* 3. wait */
	udelay(10);

	/* 4. BYPASS = 0 */
	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->bypass = 0;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	/* 5. set_div */
	cmu_pll_t div;
	int64_t fref = cmu_get_ref(base);

	if (fref < 0) {
		return -1;
	}

	if (cmu_pll_div_calc(freq, fref, &div) < 0) {
		return -1;
	}

	if (cmu_pll_div_set(base, &div)) {
		return -1;
	}

	/* 6. RST = 0 */
	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->rst = 0;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	/* 7. wait */
	/* 8. LOCK? */
#ifndef BAIKAL_QEMU
	int try = 100;

	do {
		ret = reg_read(base, &raw);
		if (ret < 0) {
			return ret;
		}
		udelay(10);
		if (!try--) {
			return -1;
		}
	} while (!reg->lock);
#endif
	/* 9. SWEN = 1 */
	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->swen = 1;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	/* 10. SWRST = 0 */
	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->swrst = 0;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_pll_disable(uintptr_t base)
{
	/* SWEN = 0 */
	uint32_t raw;
	cmu_pll_t *reg = (void *)&raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}

	reg->swen = 0;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_pll_enable(uintptr_t base)
{
	/* SWEN = 1 */
	uint32_t raw;
	cmu_pll_t *reg = (void *)&raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}

	reg->swen = 1;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_pll_is_enabled(uintptr_t base)
{
	/* return SWEN */
	uint32_t raw;
	cmu_pll_t *reg = (void *)&raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	return reg->swen;
}

int64_t cmu_pll_round_rate(uintptr_t base, int64_t freq)
{
	cmu_pll_t div;
	int64_t fref = cmu_get_ref(base);

	if (fref < 0) {
		return -1;
	}

	if (cmu_pll_div_calc(freq, fref, &div) < 0) {
		return -1;
	}

	int64_t calc = cmu_pll_calc(fref, &div);

	return calc;
}

/* CLKCH */
int64_t cmu_clkch_get_rate(uintptr_t base)
{
	int64_t freq;
	cmu_clkch_t div;
	int64_t fref = cmu_get_ref(base);

	if (fref < 0) {
		return -1;
	}

	if (cmu_clkch_div_get(base, &div)) {
		return -1;
	}

	freq = cmu_clkch_calc(fref, &div);
	return freq;
}

int64_t cmu_clkch_set_rate(uintptr_t base, int64_t freq)
{
	uint32_t raw;
	cmu_clkch_t *reg = (void *)&raw;
	int ret;

	/* 1. CLKEN = 0 */
	cmu_clkch_disable(base);

	/* 2. wait CLKRDY = 0 */
#ifndef BAIKAL_QEMU
	int try = 100;

	do {
		ret = reg_read(base, &raw);
		if (ret < 0) {
			return ret;
		}
		udelay(10);
		if (!try--) {
			return -1;
		}
	} while (reg->clk_rdy);
#endif
	/* 3. set dividers */
	cmu_clkch_t div;
	int64_t fref = cmu_get_ref(base);

	if (fref < 0) {
		return -1;
	}

	if (cmu_clkch_div_calc(freq, fref, &div) < 0) {
		return -1;
	}

	if (cmu_clkch_div_set(base, &div)) {
		return -1;
	}

	/* 4. SET_CLKDIV = 1 */
	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->set_clkdiv = 1;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	/* 5. wait LOCK_CLKDIV */
#ifndef BAIKAL_QEMU
	try = 100;
	do {
		ret = reg_read(base, &raw);
		if (ret < 0) {
			return ret;
		}
		udelay(10);
		if (!try--) {
			return -1;
		}
	} while (!reg->lock_clkdiv);
#endif
	/* 6. CLKEN = 1 */
	cmu_clkch_enable(base);

	/* 7. wait CLKRDY = 1 */
#ifndef BAIKAL_QEMU
	try = 100;
	do {
		ret = reg_read(base, &raw);
		if (ret < 0) {
			return ret;
		}
		udelay(10);
		if (!try--) {
			return -1;
		}
	} while (!reg->clk_rdy);
#endif
	/* 8. SWRST = 0 */
	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->swrst = 0;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_clkch_disable(uintptr_t base)
{
	/* CLKEN = 0 */
	uint32_t raw;
	cmu_clkch_t *reg = (void *)&raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}

	reg->clk_en = 0;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_clkch_enable(uintptr_t base)
{
	/* CLKEN = 1 */
	uint32_t raw;
	cmu_clkch_t *reg = (void *)&raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}

	reg->clk_en = 1;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_clkch_is_enabled(uintptr_t base)
{
	/* return CLKEN */
	uint32_t raw;
	cmu_clkch_t *reg = (void *)&raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}

	return reg->clk_en;
}

int64_t cmu_clkch_round_rate(uintptr_t base, int64_t freq)
{
	cmu_clkch_t div;
	int64_t fref = cmu_get_ref(base);

	if (fref < 0) {
		return -1;
	}

	if (cmu_clkch_div_calc(freq, fref, &div) < 0) {
		return -1;
	}

	int64_t calc = cmu_clkch_calc(fref, &div);

	return calc;
}

/* CLKDIV */
int64_t cmu_clkdiv_get_rate(uintptr_t base)
{
	int64_t freq;
	cmu_clkdiv_t div;
	int64_t fref = cmu_get_ref(base);

	if (fref < 0) {
		return -1;
	}

	if (cmu_clkdiv_div_get(base, &div)) {
		return -1;
	}

	freq = cmu_clkdiv_calc(fref, &div);
	return freq;
}

int64_t cmu_clkdiv_set_rate(uintptr_t base, int64_t freq)
{
	int ret;
	uint32_t raw;
	int64_t fref;
	cmu_clkdiv_t *reg = (void *)&raw;
	cmu_clkdiv_t cur;
	cmu_clkdiv_t new;

	fref = cmu_get_ref(base);
	if (fref < 0) {
		return -1;
	}

	/* calc */
	ret = cmu_clkdiv_div_calc(freq, fref, &new);
	if (ret < 0) {
		return ret;
	}
	ret = cmu_clkdiv_div_get(base, &cur);
	if (ret < 0) {
		return ret;
	}

	while (cur.div_val != new.div_val) {
		/* step */
		if (cur.div_val < new.div_val) {
			cur.div_val++;
		} else {
			cur.div_val--;
		}

		/* set */
		ret = cmu_clkdiv_div_set(base, &cur);
		if (ret < 0) {
			return ret;
		}
		ret = reg_read(base, &raw);
		if (ret < 0) {
			return ret;
		}
		reg->div_val_set = 1;
		ret = reg_write(base, raw);
		if (ret < 0) {
			return ret;
		}

#ifndef BAIKAL_QEMU
		/* wait */
		int try = 1000;

		do {
			ret = reg_read(base, &raw);
			if (ret < 0) {
				return ret;
			}
			if (!try--) {
				return -1;
			}
		} while (!reg->lock);
#endif
	}

	return 0;
}

int64_t cmu_clkdiv_disable(uintptr_t base)
{
	/* CLKEN = 0 */
	uint32_t raw;
	cmu_clkdiv_t *reg = (void *)&raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}

	reg->clk_en = 0;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_clkdiv_enable(uintptr_t base)
{
	/* CLKEN = 1 */
	uint32_t raw;
	cmu_clkdiv_t *reg = (void *)&raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}

	reg->clk_en = 1;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int64_t cmu_clkdiv_is_enabled(uintptr_t base)
{
	/* return CLKEN */
	uint32_t raw;
	cmu_clkdiv_t *reg = (void *)&raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	return reg->clk_en;
}

int64_t cmu_clkdiv_round_rate(uintptr_t base, int64_t freq)
{
	cmu_clkdiv_t div;
	int64_t fref = cmu_get_ref(base);

	if (fref < 0) {
		return -1;
	}

	if (cmu_clkdiv_div_calc(freq, fref, &div) < 0) {
		return -1;
	}

	int64_t calc = cmu_clkdiv_calc(fref, &div);

	return calc;
}

/* CLKEN */
int64_t cmu_clken_get_rate(uintptr_t base)
{
	int64_t freq = 0;
	cmu_clken_t div;
	int64_t fref = cmu_get_ref(base);

	if (fref < 0) {
		return -1;
	}

	if (cmu_clken_div_get(base, &div)) {
		return -1;
	}

	freq = cmu_clken_calc(fref, &div);
	return freq;
}

int64_t cmu_clken_set_rate(uintptr_t base, int64_t freq)
{
	uint32_t raw;
	cmu_clken_t *reg = (void *)&raw;
	int ret;

	/* 1. CLKEN = 0 */
	cmu_clken_disable(base);

	/* 2. set dividers */
	cmu_clken_t div;
	int64_t fref = cmu_get_ref(base);

	if (fref < 0) {
		return -1;
	}
	if (cmu_clken_div_calc(freq, fref, &div) < 0) {
		return -1;
	}
	if (cmu_clken_div_set(base, &div)) {
		return -1;
	}

	/* 3. SET_CLKDIV = 1 */
	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->div_val_set = 1;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}

	/* 4. wait LOCK */
#ifndef BAIKAL_QEMU
	int try;

	try = 100;
	do {
		ret = reg_read(base, &raw);
		if (ret < 0) {
			return ret;
		}
		udelay(10);
		if (!try--) {
			return -1;
		}
	} while (!reg->lock);
#endif
	/* 5. CLKEN = 1 */
	cmu_clken_enable(base);

	return 0;
}

int64_t cmu_clken_disable(uintptr_t base)
{
	/* CLKEN = 0 */
	uint32_t raw;
	cmu_clkdiv_t *reg = (void *)&raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->clk_en = 0;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

int64_t cmu_clken_enable(uintptr_t base)
{
	/* CLKEN = 1 */
	uint32_t raw;
	cmu_clkdiv_t *reg = (void *)&raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	reg->clk_en = 1;
	ret = reg_write(base, raw);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

int64_t cmu_clken_is_enabled(uintptr_t base)
{
	/* return CLKEN */
	uint32_t raw;
	int ret;

	ret = reg_read(base, &raw);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

int64_t cmu_clken_round_rate(uintptr_t base, int64_t freq)
{
	cmu_clken_t div;
	int64_t fref = cmu_get_ref(base);

	if (fref < 0) {
		return -1;
	}
	if (cmu_clken_div_calc(freq, fref, &div) < 0) {
		return -1;
	}

	int64_t calc = cmu_clken_calc(fref, &div);

	return calc;
}

int64_t baikal_smc_clk_handler(const uint32_t smc_fid,
			       const uint64_t base,
			       const uint64_t freq,
			       const uint64_t x3,
			       const uint64_t x4)
{
	struct clk_desc *clk = cmu_desc_get_by_base(base);
	/* Convert SMC FID to SMC64 to support SMC32/SMC64 */
	const uint32_t local_smc_fid = BIT(30) | smc_fid;

	if (!clk) {
		return -1;
	}

	switch (local_smc_fid) {
	case BAIKAL_SMC_CLK_GET:
		return clk->ops->get(base);
	case BAIKAL_SMC_CLK_SET:
		return clk->ops->set(base, freq);
	case BAIKAL_SMC_CLK_ROUND:
		return clk->ops->round(base, freq);
	case BAIKAL_SMC_CLK_DISABLE:
		return clk->ops->disable(base);
	case BAIKAL_SMC_CLK_ENABLE:
		return clk->ops->enable(base);
	case BAIKAL_SMC_CLK_IS_ENABLED:
		return clk->ops->is_enabled(base);
	default:
		return -1;
	}
}

int64_t baikal_smc_gmac_handler(const uint32_t smc_fid,
				const uint64_t addr,
				const uint64_t x2,
				const uint64_t x3,
				const uint64_t x4)
{
	uint32_t mask;
	uint32_t reg;
	unsigned int chip = PLATFORM_ADDR_CHIP(addr);
	/* Convert SMC FID to SMC64 to support SMC32/SMC64 */
	const uint32_t local_smc_fid = BIT(30) | smc_fid;

	switch (PLATFORM_ADDR_IN_CHIP(addr)) {
	case GMAC0_BASE:
		mask = SCP_GPR_GMAC_DIV_CTL_GMAC0_DIV2_EN;
		break;
	case GMAC1_BASE:
		mask = SCP_GPR_GMAC_DIV_CTL_GMAC1_DIV2_EN;
		break;
	default:
		return -1;
	}

	switch (local_smc_fid) {
	case BAIKAL_SMC_GMAC_DIV2_ENABLE:
		reg_read(PLATFORM_ADDR_OUT_CHIP(chip, SCP_GPR_GMAC_DIV_CTL), &reg);
		reg |= mask;
		return reg_write(PLATFORM_ADDR_OUT_CHIP(chip, SCP_GPR_GMAC_DIV_CTL), reg);
	case BAIKAL_SMC_GMAC_DIV2_DISABLE:
		reg_read(PLATFORM_ADDR_OUT_CHIP(chip, SCP_GPR_GMAC_DIV_CTL), &reg);
		reg &= ~mask;
		return reg_write(PLATFORM_ADDR_OUT_CHIP(chip, SCP_GPR_GMAC_DIV_CTL), reg);
	default:
		return -1;
	}
}
