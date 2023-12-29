/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BM1000_CMU_H
#define BM1000_CMU_H

#include <stdbool.h>
#include <stdint.h>

struct __attribute__((__packed__)) cmu_desc {
	uintptr_t	base;
	uint64_t	frefclk;
	bool		deny_pll_reconf;
};

typedef struct __attribute__((__packed__)) {
	uint16_t	clkod;
	uint16_t	clkr;
	uint64_t	clkf;
	int8_t		pgain;
	int8_t		igain;
	int8_t		ipgain;
	int8_t		iigain;
} cmu_pll_ctl_vals_t;

struct cmu_desc *cmu_desc_get_by_idx(const unsigned int idx);

/* PLL functions */
int64_t	cmu_pll_get_rate  (const uintptr_t base, uint64_t frefclk);
int	cmu_pll_set_rate  (const uintptr_t base, uint64_t frefclk, const uint64_t fpllreq);
int	cmu_pll_disable	  (const uintptr_t base);
int	cmu_pll_enable	  (const uintptr_t base);
int	cmu_pll_is_enabled(const uintptr_t base);
void	cmu_pll_on	  (const uintptr_t base, const cmu_pll_ctl_vals_t *const pllinit);
int64_t	cmu_pll_round_rate(const uintptr_t base, uint64_t frefclk, const uint64_t fpllreq);
void	cmu_pll_set_gains (const uintptr_t base,
			   const int8_t pgain,
			   const int8_t igain,
			   const int8_t ipgain,
			   const int8_t iigain);

/* Clock channel functions */
int64_t	cmu_clkch_get_rate	(const uintptr_t base, const unsigned int clkch);
int	cmu_clkch_set_rate	(const uintptr_t base, const unsigned int clkch, const uint64_t fclkchreq);
int	cmu_clkch_disable	(const uintptr_t base, const unsigned int clkch);
int	cmu_clkch_enable	(const uintptr_t base, const unsigned int clkch);
void	cmu_clkch_enable_by_base(const uintptr_t base, const unsigned int div);
int	cmu_clkch_is_enabled	(const uintptr_t base, const unsigned int clkch);
int64_t	cmu_clkch_round_rate	(const uintptr_t base, const unsigned int clkch, const uint64_t fclkchreq);

#endif /* BM1000_CMU_H */
