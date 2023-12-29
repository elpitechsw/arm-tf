/*
 * Copyright (c) 2021-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>

#include <baikal_bl1_stack.h>
#include <xorshift.h>

IMPORT_SYM(uintptr_t, __STACKS_START__,	STACKS_START);
IMPORT_SYM(uintptr_t, __STACKS_END__,	STACKS_END);

void bl1_stack_area_check(void)
{
	volatile uint64_t *stack_area_ptr = (volatile uint64_t *)STACKS_START;
	uint64_t val = 0xa5a5a5a5a5a5a5a5;

	while (stack_area_ptr < (volatile uint64_t *)STACKS_END) {
		val = xorshift64(val);
		if (*stack_area_ptr != val) {
			break;
		}

		++stack_area_ptr;
	}

	if (stack_area_ptr != (volatile uint64_t *)STACKS_START) {
		const unsigned int used  = STACKS_END - (uintptr_t)stack_area_ptr;
		const unsigned int total = STACKS_END - STACKS_START;

		INFO("BL1: %u bytes (%u%%) of stack are used\n", used, used * 100 / total);
	} else {
		ERROR("BL1: stack has been overflowed\n");
		panic();
	}
}

void bl1_stack_area_fill(void)
{
	volatile uint64_t *stack_area_ptr = (volatile uint64_t *)STACKS_START;
	uint64_t val = 0xa5a5a5a5a5a5a5a5;

	/*
	 * Fill the stack area up to address of the 'stack_area_ptr' variable,
	 * but reserve some memory for the function stack frame.
	 */
	while ((uintptr_t)stack_area_ptr < (uintptr_t)(&stack_area_ptr) - 64) {
		*stack_area_ptr++ = val = xorshift64(val);
	}
}
