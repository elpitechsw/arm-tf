/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include <platform_def.h>
#include <bs1000_def.h>
#include <bs1000_cmu.h>
#include <bs1000_scp_lcru.h>
#include <bs1000_coresight.h>

#define CA75_N_BASE(n)			(CA75_BASE + ((n) << 26))
#define CA75_NIC_CFG_GPV_BASE(n)	(CA75_N_BASE(n) + 0x200000)
#define CA75_NIC_CFG_CORESIGHT		0x0c

#define CORESIGHT_TSGEN_TS_BASE		(CORESIGHT_CFG_BASE + 0x20000)
#define TSGEN_CNTCR			0x0
#define TSGEN_CNTCR_EN			BIT(0)

#if (PLATFORM_CHIP_COUNT > 1)

#define CORESIGHT_TSGEN_CNT_BASE	(CORESIGHT_CFG_BASE + 0x30000)
#define TSGEN_CNTCVL			0x8
#define TSGEN_CNTCVU			0xc

#define BS1000_TSGEN_MAX_OFFSET		1
#define BS1000_TSGEN_MAX_SYNC		10

typedef struct css600_tsgen_data {
	uintptr_t base;
	int64_t offset;
	int32_t dr;
	int32_t dw;
} css600_tsgen_data_t;

static css600_tsgen_data_t tsgen_ts[PLATFORM_CHIP_COUNT];
static css600_tsgen_data_t tsgen_cnt[PLATFORM_CHIP_COUNT];

static uint64_t css600_tsgen_read(const css600_tsgen_data_t *counter)
{
	uint32_t lo, up, tmp;

	do {
		up = mmio_read_32(counter->base + TSGEN_CNTCVU);
		lo = mmio_read_32(counter->base + TSGEN_CNTCVL);
		tmp = mmio_read_32(counter->base + TSGEN_CNTCVU);
	} while (up != tmp);

	return ((uint64_t) up << 32) | lo;
}

static void css600_tsgen_write(const css600_tsgen_data_t *counter,
			       uint64_t value)
{
	/* stop counter */
	mmio_clrbits_32(counter->base + TSGEN_CNTCR, TSGEN_CNTCR_EN);
	/* set counter value */
	mmio_write_32(counter->base + TSGEN_CNTCVL, value & 0xffffffff);
	mmio_write_32(counter->base + TSGEN_CNTCVU, value >> 32);
	/* start counter */
	mmio_setbits_32(counter->base + TSGEN_CNTCR, TSGEN_CNTCR_EN);
}

static bool css600_tsgen_check(css600_tsgen_data_t *master,
			       css600_tsgen_data_t *slave)
{
	uint64_t cnt[3];

	cnt[0] = css600_tsgen_read(master);
	cnt[1] = css600_tsgen_read(slave);
	cnt[2] = css600_tsgen_read(slave);

	slave->dr = cnt[2] - cnt[1];
	slave->offset = 2 * cnt[1] - cnt[0] - cnt[2];

#ifdef DEBUG
	INFO("TSGEN(%lx): dr=%d, dw=%d, offset=%ld\n",
	     slave->base, slave->dr, slave->dw, slave->offset);
#endif

	return !!((slave->offset < 0 ? -slave->offset : slave->offset) <=
		  BS1000_TSGEN_MAX_OFFSET);
}

static void css600_tsgen_adj(css600_tsgen_data_t *master,
			     css600_tsgen_data_t *slave)
{
	css600_tsgen_write(slave, css600_tsgen_read(master) + slave->dw);
}

static void css600_tsgen_sync(css600_tsgen_data_t *master,
			      css600_tsgen_data_t *slave)
{
	uint64_t cnt[2];

	css600_tsgen_write(slave, css600_tsgen_read(master));
	cnt[0] = css600_tsgen_read(master);
	cnt[1] = css600_tsgen_read(slave);
	slave->dw = cnt[0] - cnt[1] + slave->dr;
	css600_tsgen_adj(master, slave);
}

#endif

void bs1000_coresight_init(void)
{
	unsigned int chip_idx;

	for (chip_idx = 0; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		int i;

		/* Enable non-secure access */
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_CORESIGHT_APB),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_CORESIGHT_STM),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_CA75_0),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_CA75_1_5),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_CA75_6),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_CA75_7_11),
			      NIC_GPV_REGIONSEC_NONSECURE);
		for (i = 0; i < PLATFORM_CLUSTER_COUNT; i++)
			mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, CA75_NIC_CFG_GPV_BASE(i) + CA75_NIC_CFG_CORESIGHT),
				      NIC_GPV_REGIONSEC_NONSECURE);

		/* Enable TSGEN_TS */
		mmio_setbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, CORESIGHT_TSGEN_TS_BASE + TSGEN_CNTCR),
				TSGEN_CNTCR_EN);

#if (PLATFORM_CHIP_COUNT > 1)
		/* TSGEN_TS, TSGEN_CNT interchip synchronization */
		tsgen_ts[chip_idx].base = PLATFORM_ADDR_OUT_CHIP(chip_idx, CORESIGHT_TSGEN_TS_BASE);
		tsgen_cnt[chip_idx].base = PLATFORM_ADDR_OUT_CHIP(chip_idx, CORESIGHT_TSGEN_CNT_BASE);
		if (chip_idx > 0) {
			bool is_synced;

			(void)css600_tsgen_check(&tsgen_ts[0], &tsgen_ts[chip_idx]);
			css600_tsgen_sync(&tsgen_ts[0], &tsgen_ts[chip_idx]);
			for (i = 0, is_synced = false; i < BS1000_TSGEN_MAX_SYNC; ++i) {
				is_synced = css600_tsgen_check(&tsgen_ts[0], &tsgen_ts[chip_idx]);
				if (is_synced)
					break;
				tsgen_ts[chip_idx].dw -= tsgen_ts[chip_idx].offset;
				css600_tsgen_adj(&tsgen_ts[0], &tsgen_ts[chip_idx]);
			}
			INFO("Chip%u TSGEN_TS %s synced\n", chip_idx,
				is_synced ? "" : "not");
			(void)css600_tsgen_check(&tsgen_cnt[0], &tsgen_cnt[chip_idx]);
			css600_tsgen_sync(&tsgen_cnt[0], &tsgen_cnt[chip_idx]);
			for (i = 0, is_synced = false; i < BS1000_TSGEN_MAX_SYNC; ++i) {
				is_synced = css600_tsgen_check(&tsgen_cnt[0], &tsgen_cnt[chip_idx]);
				if (is_synced)
					break;
				tsgen_cnt[chip_idx].dw -= tsgen_cnt[chip_idx].offset;
				css600_tsgen_adj(&tsgen_cnt[0], &tsgen_cnt[chip_idx]);
			}
			INFO("Chip%u TSGEN_CNT %ssynced\n", chip_idx, is_synced ? "" : "not ");
		}
#endif
	}
}

#ifdef MULTICHIP_SYSTEM_COUNTERS_RESYNC

void bs1000_resync_system_counters(void)
{
	unsigned int chip_idx;

	for (chip_idx = 1; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		css600_tsgen_adj(&tsgen_ts[0], &tsgen_ts[chip_idx]);
		css600_tsgen_adj(&tsgen_cnt[0], &tsgen_cnt[chip_idx]);
	}
}

#endif
