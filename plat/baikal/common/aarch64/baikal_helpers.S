/*
 * Copyright (c) 2018-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

#include <baikal_def.h>
#include <platform_def.h>

	.globl	plat_crash_console_flush
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_get_my_entrypoint
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_report_exception
	.globl	plat_secondary_cold_boot_setup
	.globl	platform_mem_init

	/* ---------------------------------------------
	 * void plat_crash_console_flush(void);
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	mov_imm	x0, BAIKAL_UART_BASE
#if defined(BAIKAL_CONSOLE_16550)
	b	console_16550_core_flush
#elif defined(BAIKAL_CONSOLE_PL011)
	b	console_pl011_core_flush
#else
	ret
#endif
endfunc plat_crash_console_flush

	/* ---------------------------------------------
	 * int plat_crash_console_init(void);
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm x0, BAIKAL_UART_BASE
	mov_imm x1, BAIKAL_UART_CLK_IN_HZ
	mov_imm x2, BAIKAL_UART_BAUDRATE
#if defined(BAIKAL_CONSOLE_16550)
	b	console_16550_core_init
#elif defined(BAIKAL_CONSOLE_PL011)
	b	console_pl011_core_init
#else
	ret
#endif
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c);
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm x1, BAIKAL_UART_BASE
#if defined(BAIKAL_CONSOLE_16550)
	b	console_16550_core_putc
#elif defined(BAIKAL_CONSOLE_PL011)
	b	console_pl011_core_putc
#else
	ret
#endif
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * uintptr_t plat_get_my_entrypoint(void);
	 * ---------------------------------------------
	 */
func plat_get_my_entrypoint
	mov_imm	x0, BAIKAL_TRUSTED_MAILBOX_BASE
	ldr	x0, [x0]
	ret
endfunc plat_get_my_entrypoint

	/* ---------------------------------------------
	 * unsigned plat_is_my_cpu_primary(void);
	 * ---------------------------------------------
	 */
func plat_is_my_cpu_primary
	mov	x9, x30
	bl	plat_my_core_pos
	cmp	x0, #BAIKAL_PRIMARY_CPU
	cset	x0, eq
	ret	x9
endfunc plat_is_my_cpu_primary

	/* ---------------------------------------------
	 * unsigned plat_my_core_pos(void);
	 * ---------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_baikal_calc_core_pos
endfunc plat_my_core_pos

	/* ---------------------------------------------
	 * void plat_report_exception(unsigned type);
	 * ---------------------------------------------
	 */
func plat_report_exception
	mov	x8, x30

	mrs	x2, currentel
	and	x2, x2, #0x0c

	/* Check EL1 */
	cmp	x2, #0x04
	beq	plat_report_el1

	adr	x4, plat_err_str
	bl	asm_print_str

	adr	x4, esr_el3_str
	bl	asm_print_str

	mrs	x4, esr_el3
	bl	asm_print_hex

	adr	x4, elr_el3_str
	bl	asm_print_str

	mrs	x4, elr_el3
	bl	asm_print_hex
	b	plat_report_end
plat_report_el1:
	adr	x4, plat_err_str
	bl	asm_print_str

	adr	x4, esr_el1_str
	bl	asm_print_str

	mrs	x4, esr_el1
	bl	asm_print_hex

	adr	x4, elr_el1_str
	bl	asm_print_str

	mrs	x4, elr_el1
	bl	asm_print_hex
plat_report_end:
	mov	x30, x8
	ret
endfunc plat_report_exception

	/* ---------------------------------------------
	 * void plat_secondary_cold_boot_setup(void);
	 * ---------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	/* Calculate address of our hold entry */
	bl	plat_my_core_pos
	mov_imm	x2, BAIKAL_HOLD_BASE

	/* Wait until we have a go */
poll_mailbox:
	ldr	x1, [x2, x0, LSL #3]
	cbz	x1, 1f
	mov_imm	x0, BAIKAL_TRUSTED_MAILBOX_BASE
	ldr	x1, [x0]
	br	x1
1:	wfe
	b	poll_mailbox
endfunc plat_secondary_cold_boot_setup

	/* ---------------------------------------------
	 * void platform_mem_init(void);
	 * ---------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

.section .rodata.rev_err_str, "aS"
plat_err_str:
	.asciz "\nPlatform exception reporting:"
esr_el3_str:
	.asciz "\nESR_EL3: "
elr_el3_str:
	.asciz "\nELR_EL3: "
esr_el1_str:
	.asciz "\nESR_EL1: "
elr_el1_str:
	.asciz "\nELR_EL1: "
