/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <lib/psci/psci.h>
#include <lib/utils_def.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <baikal_def.h>
#include <baikal_gicv3.h>
#include <bm1000_private.h>
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
#include <mbm_bmc.h>
#elif defined(ELPITECH)
#include <dw_gpio.h>
#if (BOARD_VER == 5) || (BOARD_VER == 8)
#define BM_RESET_PIN	17
#define BM_POWER_PIN	16
#else
#define BM_RESET_PIN	9
#define BM_POWER_PIN	10
#endif
#endif
#include <platform_def.h>

#include "bm1000_mmca57.h"

/* Make composite power state parameter till power level 0 */
#if PSCI_EXTENDED_STATE_ID
#define bm1000_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type)	\
		(((lvl0_state) << PSTATE_ID_SHIFT) |		\
		 ((type) << PSTATE_TYPE_SHIFT))
#else
#define bm1000_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type)	\
		(((lvl0_state) << PSTATE_ID_SHIFT)   |		\
		 ((pwr_lvl) << PSTATE_PWR_LVL_SHIFT) |		\
		 ((type) << PSTATE_TYPE_SHIFT))
#endif /* PSCI_EXTENDED_STATE_ID */

#define bm1000_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl1_state) << PLAT_LOCAL_PSTATE_WIDTH) |		 \
		 bm1000_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type))

static int bm1000_validate_power_state(unsigned int power_state, psci_power_state_t *req_state)
{
	unsigned int i;
	unsigned int state_id;

	/*
	 *  The table storing the valid idle power states. Ensure that the
	 *  array entries are populated in ascending order of state-id to
	 *  enable us to use binary search during power state validation.
	 *  The table must be terminated by a NULL entry.
	 */
	static const unsigned int idle_states[] = {
		/* state-id - 0x000 0001 */
		bm1000_make_pwrstate_lvl1(PLAT_LOCAL_STATE_RUN,
					  PLAT_LOCAL_STATE_RET,
					  MPIDR_AFFLVL0,
					  PSTATE_TYPE_STANDBY),
		/* ending element of idle_states */
		0
	};

	assert(req_state);

	/*
	 *  Currently we are using a linear search for finding the matching
	 *  entry in the idle power state array. This can be made a binary
	 *  search if the number of entries justify the additional complexity.
	 */
	for (i = 0; idle_states[i]; ++i) {
		if (idle_states[i] == power_state) {
			break;
		}
	}

	/* Return error if entry not found in the idle state array */
	if (!idle_states[i]) {
		return PSCI_E_INVALID_PARAMS;
	}

	state_id = psci_get_pstate_id(power_state);

	/* Parse the state-id and populate the state info parameter */
	for (i = 0; state_id; state_id >>= PLAT_LOCAL_PSTATE_WIDTH) {
		req_state->pwr_domain_state[i++] = state_id & PLAT_LOCAL_PSTATE_MASK;
	}

	return PSCI_E_SUCCESS;
}

static int bm1000_validate_ns_entrypoint(uintptr_t entrypoint)
{
	unsigned int region;
	uint64_t region_descs[3][2];
	int ret;

	if (!(entrypoint >= REGION_DRAM0_BASE &&
	      entrypoint <  REGION_DRAM0_BASE + REGION_DRAM0_SIZE) &&
	    !(entrypoint >= REGION_DRAM1_BASE &&
	      entrypoint <  REGION_DRAM1_BASE + REGION_DRAM1_SIZE) &&
	    !(entrypoint >= REGION_DRAM2_BASE &&
	      entrypoint <  REGION_DRAM2_BASE + REGION_DRAM2_SIZE)) {
		ERROR("%s: 0x%lx is out of DRAM regions\n", __func__, entrypoint);
		return PSCI_E_INVALID_ADDRESS;
	}

	if (entrypoint >= SEC_DRAM_BASE &&
	    entrypoint <  SEC_DRAM_BASE + SEC_DRAM_SIZE) {
		ERROR("%s: 0x%lx is within secure DRAM region\n", __func__, entrypoint);
		return PSCI_E_INVALID_ADDRESS;
	}

	ret = fdt_memory_node_read(region_descs);
	if (ret) {
		ERROR("%s: unable to get memory map from FDT\n", __func__);
		return PSCI_E_INVALID_ADDRESS;
	}

	for (region = 0; region < ARRAY_SIZE(region_descs); ++region) {
		uint64_t region_begin = region_descs[region][0];
		uint64_t region_end   = region_descs[region][0] +
					region_descs[region][1];

		if (entrypoint >= region_begin && entrypoint < region_end) {
			return PSCI_E_SUCCESS;
		}
	}

	ERROR("%s: 0x%lx is out of non secure DRAM\n", __func__, entrypoint);
	return PSCI_E_INVALID_ADDRESS;
}

static void bm1000_cpu_standby(plat_local_state_t cpu_state)
{
	u_register_t scr;

	assert(cpu_state == PLAT_LOCAL_STATE_RET);

	scr = read_scr_el3();
	write_scr_el3(scr | SCR_FIQ_BIT);
	isb();
	dsb();
	wfi();

	write_scr_el3(scr);
}

static int bm1000_pwr_domain_on(u_register_t mpidr)
{
	volatile uint64_t *const hold_base = (uint64_t *)BAIKAL_HOLD_BASE;
	const unsigned int pos = plat_core_pos_by_mpidr(mpidr);

	if (hold_base[pos] == BAIKAL_HOLD_STATE_WAIT) {
		/* It is cold boot of a secondary core */
		hold_base[pos] = BAIKAL_HOLD_STATE_GO;
		dsb();

		/*
		 * It seems that SEV instruction does not work properly. An event
		 * is signaled locally and not signaled to other cores. So the
		 * HOLD_STATE_GO should be set before enabling of secondary core.
		 */
		mmca57_enable_core(mpidr);
		sev();
	} else {
		/* It is warm boot */
		mmca57_enable_core(mpidr);
	}

	return PSCI_E_SUCCESS;
}

static void bm1000_pwr_domain_off(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] == PLAT_LOCAL_STATE_OFF);

	baikal_gic_cpuif_disable();

	if (target_state->pwr_domain_state[MPIDR_AFFLVL1] ==
	    PLAT_LOCAL_STATE_OFF) {
		plat_arm_interconnect_exit_coherency();
	}
}

static void bm1000_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	if (target_state->pwr_domain_state[MPIDR_AFFLVL0] == PLAT_LOCAL_STATE_RET) {
		return;
	}

	/* Code shouldn't go futher till there are no idle_states[], containing 0x2 or 0x22 */
	ERROR("%s: operation not supported\n", __func__);
	panic();
}

static void bm1000_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] == PLAT_LOCAL_STATE_OFF);

	/* Enable coherency if the cluster was off */
	if (target_state->pwr_domain_state[MPIDR_AFFLVL1] == PLAT_LOCAL_STATE_OFF) {
		plat_arm_interconnect_enter_coherency();
	}
}

static void bm1000_pwr_domain_on_finish_late(const psci_power_state_t *target_state)
{
	baikal_gic_pcpu_init();
	baikal_gic_cpuif_enable();
}

static void bm1000_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	if (target_state->pwr_domain_state[MPIDR_AFFLVL0] == PLAT_LOCAL_STATE_RET) {
		return;
	}

	/* Code shouldn't go futher till there are no idle_states[], containing 0x2 or 0x22 */
	ERROR("%s: operation not supported\n", __func__);
	panic();
}

static void __dead2 bm1000_system_off(void)
{
	dsb();
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
	mbm_bmc_pwr_off();
#elif defined(ELPITECH)
	gpio_out_rst(MMAVLSP_GPIO32_BASE, BM_POWER_PIN);
	gpio_dir_set(MMAVLSP_GPIO32_BASE, BM_POWER_PIN);
	mdelay(50);
	gpio_out_rst(MMAVLSP_GPIO32_BASE, BM_RESET_PIN);
	gpio_dir_set(MMAVLSP_GPIO32_BASE, BM_RESET_PIN);
	mdelay(4000);
#endif
	ERROR("%s: operation not supported\n", __func__);
	for (;;) {
		wfi();
	}
}

static void __dead2 bm1000_system_reset(void)
{
	dsb();
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
	mbm_bmc_pwr_rst();
#elif defined(ELPITECH)
	gpio_out_rst(MMAVLSP_GPIO32_BASE, BM_RESET_PIN);
	gpio_dir_set(MMAVLSP_GPIO32_BASE, BM_RESET_PIN);
	mdelay(4000);
#endif
	ERROR("%s: operation not supported\n", __func__);
	for (;;) {
		wfi();
	}
}

#ifdef ENABLE_MEM_PROTECT
#include <lib/utils.h>

static mem_region_t bm1000_ram_ranges[] = {
	{NS_DRAM1_BASE, NS_DRAM1_SIZE},
};

int bm1000_mem_protect_chk(uintptr_t base, u_register_t length)
{
	return mem_region_in_array_chk(bm1000_ram_ranges,
				       ARRAY_SIZE(bm1000_ram_ranges),
				       base, length);
}

int bm1000_read_mem_protect(int *enabled)
{
	*enabled = 1;
	return 0;
}

int bm1000_write_mem_protect(int val)
{
	return 0;
}
#endif

int plat_setup_psci_ops(uintptr_t sec_entrypoint, const plat_psci_ops_t **psci_ops)
{
	static const plat_psci_ops_t bm1000_psci_ops = {
		.cpu_standby		   = bm1000_cpu_standby,
		.pwr_domain_on		   = bm1000_pwr_domain_on,
		.pwr_domain_off		   = bm1000_pwr_domain_off,
		.pwr_domain_suspend	   = bm1000_pwr_domain_suspend,
		.pwr_domain_on_finish	   = bm1000_pwr_domain_on_finish,
		.pwr_domain_on_finish_late = bm1000_pwr_domain_on_finish_late,
		.pwr_domain_suspend_finish = bm1000_pwr_domain_suspend_finish,
		.system_off		   = bm1000_system_off,
		.system_reset		   = bm1000_system_reset,
		.validate_power_state	   = bm1000_validate_power_state,
		.validate_ns_entrypoint	   = bm1000_validate_ns_entrypoint,
#ifdef ENABLE_MEM_PROTECT
		.mem_protect_chk	   = bm1000_mem_protect_chk,
		.read_mem_protect	   = bm1000_read_mem_protect,
		.write_mem_protect	   = bm1000_write_mem_protect,
#endif
	};

	uintptr_t *mailbox = (void *)BAIKAL_TRUSTED_MAILBOX_BASE;

	*mailbox = sec_entrypoint;
	*psci_ops = &bm1000_psci_ops;
	return 0;
}
