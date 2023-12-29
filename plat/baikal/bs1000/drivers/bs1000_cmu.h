/*
 * Copyright (c) 2021-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BS1000_CMU_H
#define BS1000_CMU_H

#include <stdint.h>

/*******************************************************************************
 * EXTERN
 ******************************************************************************/
int cmu_desc_init(void);

/*******************************************************************************
 * CLK
 ******************************************************************************/
/* PLL */
int64_t cmu_pll_get_rate     (uintptr_t base);
int64_t cmu_pll_round_rate   (uintptr_t base, int64_t req);
int64_t cmu_pll_set_rate     (uintptr_t base, int64_t req);
int64_t cmu_pll_disable      (uintptr_t base);
int64_t cmu_pll_enable       (uintptr_t base);
int64_t cmu_pll_is_enabled   (uintptr_t base);

/* CLKCH */
int64_t cmu_clkch_get_rate   (uintptr_t base);
int64_t cmu_clkch_round_rate (uintptr_t base, int64_t req);
int64_t cmu_clkch_set_rate   (uintptr_t base, int64_t req);
int64_t cmu_clkch_disable    (uintptr_t base);
int64_t cmu_clkch_enable     (uintptr_t base);
int64_t cmu_clkch_is_enabled (uintptr_t base);

/* CLKDIV */
int64_t cmu_clkdiv_get_rate  (uintptr_t base);
int64_t cmu_clkdiv_round_rate(uintptr_t base, int64_t req);
int64_t cmu_clkdiv_set_rate  (uintptr_t base, int64_t req);
int64_t cmu_clkdiv_disable   (uintptr_t base);
int64_t cmu_clkdiv_enable    (uintptr_t base);
int64_t cmu_clkdiv_is_enabled(uintptr_t base);

/* CLKEN */
int64_t cmu_clken_get_rate   (uintptr_t base);
int64_t cmu_clken_round_rate (uintptr_t base, int64_t req);
int64_t cmu_clken_set_rate   (uintptr_t base, int64_t req);
int64_t cmu_clken_disable    (uintptr_t base);
int64_t cmu_clken_enable     (uintptr_t base);
int64_t cmu_clken_is_enabled (uintptr_t base);

/* CLKEN */
int64_t cmu_clkref_get_rate  (uintptr_t base);
int64_t cmu_clkref_round_rate(uintptr_t base, int64_t req);
int64_t cmu_clkref_set_rate  (uintptr_t base, int64_t req);
int64_t cmu_clkref_disable   (uintptr_t base);
int64_t cmu_clkref_enable    (uintptr_t base);
int64_t cmu_clkref_is_enabled(uintptr_t base);

/*******************************************************************************
 * SUPPORT
 ******************************************************************************/
int64_t cmu_pll_calc         (int64_t   fref, void *div);
int64_t cmu_pll_div_set      (uintptr_t base, void *div);
int64_t cmu_pll_div_get      (uintptr_t base, void *div);
int64_t cmu_pll_div_calc     (int64_t   freq, int64_t fref, void *div);

int64_t cmu_clkch_calc       (int64_t   fref, void *div);
int64_t cmu_clkch_div_set    (uintptr_t base, void *div);
int64_t cmu_clkch_div_get    (uintptr_t base, void *div);
int64_t cmu_clkch_div_calc   (int64_t   freq, int64_t fref, void *div);

int64_t cmu_clkdiv_calc      (int64_t   fref, void *div);
int64_t cmu_clkdiv_div_set   (uintptr_t base, void *div);
int64_t cmu_clkdiv_div_get   (uintptr_t base, void *div);
int64_t cmu_clkdiv_div_calc  (int64_t   freq, int64_t fref, void *div);

int64_t cmu_clken_calc       (int64_t   fref, void *div);
int64_t cmu_clken_div_set    (uintptr_t base, void *div);
int64_t cmu_clken_div_get    (uintptr_t base, void *div);
int64_t cmu_clken_div_calc   (int64_t   freq, int64_t fref, void *div);

#endif /* BS1000_CMU_H */
