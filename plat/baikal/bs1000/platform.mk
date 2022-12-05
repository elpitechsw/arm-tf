#
# Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libfdt/libfdt.mk
include lib/xlat_tables_v2/xlat_tables.mk

$(eval $(call add_define_val,SDK_VERSION,$(SDK_VERSION)))

HW_ASSISTED_COHERENCY	:=	1
USE_COHERENT_MEM	:=	0

PLATFORM_CHIP_COUNT		:=	1
PLATFORM_ADDR_BITS_PER_CHIP	:=	44

ifeq ($(BAIKAL_TARGET),dbs)
$(eval $(call add_define,BAIKAL_DBS))
else ifeq ($(BAIKAL_TARGET),dbs-ov)
$(eval $(call add_define,BAIKAL_DBS_OV))
else ifeq ($(BAIKAL_TARGET),mbs-1s)
$(eval $(call add_define,BAIKAL_MBS_1S))
else ifeq ($(BAIKAL_TARGET),mbs-2s)
$(eval $(call add_define,BAIKAL_MBS_2S))
PLATFORM_CHIP_COUNT		:=	2
PLATFORM_ADDR_BITS_PER_CHIP	:=	43
SYSTEM_COUNTERS_RESYNC		:=	no
else ifeq ($(BAIKAL_TARGET),elp_bs)
$(eval $(call add_define,ELPITECH))
ifneq ($(BAIKAL_DDR_CUSTOM_CLOCK_FREQ),)
$(eval $(call add_define,BAIKAL_DDR_CUSTOM_CLOCK_FREQ))
endif
else ifeq ($(BAIKAL_TARGET),qemu-s)
$(eval $(call add_define,BAIKAL_QEMU))
else
$(error "Error: unknown BAIKAL_TARGET=${BAIKAL_TARGET}")
endif

$(eval $(call CREATE_SEQ,SEQ,4))
ifneq ($(PLATFORM_CHIP_COUNT),$(filter $(PLATFORM_CHIP_COUNT),$(SEQ)))
 $(error  "Chip count for BS1000 platform should be one of $(SEQ), currently \
   set to ${PLATFORM_CHIP_COUNT}.")
endif
$(eval $(call add_define,PLATFORM_CHIP_COUNT))
$(eval $(call add_define,PLATFORM_ADDR_BITS_PER_CHIP))
ifeq ($(SYSTEM_COUNTERS_RESYNC),yes)
$(eval $(call add_define,MULTICHIP_SYSTEM_COUNTERS_RESYNC))
endif

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/aarch64	\
				-Iplat/baikal/bs1000/drivers		\
				-Iplat/baikal/bs1000/drivers/ddr	\
				-Iplat/baikal/bs1000/include		\
				-Iplat/baikal/common/include

PLAT_BL_COMMON_SOURCES	:=	drivers/arm/pl011/aarch64/pl011_console.S	\
				drivers/arm/smmu/smmu_v3.c			\
				plat/baikal/bs1000/aarch64/bs1000_helpers.S	\
				plat/baikal/bs1000/bs1000_scp_lcru.c		\
				plat/baikal/bs1000/drivers/bs1000_scp.c		\
				plat/baikal/common/aarch64/baikal_helpers.S	\
				plat/baikal/common/baikal_bootflash.c		\
				plat/baikal/common/baikal_console.c		\
				plat/baikal/common/dw_spi_flash.c		\
				${XLAT_TABLES_LIB_SRCS}

BL1_SOURCES		+=	drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				lib/cpus/aarch64/cortex_a75.S			\
				plat/baikal/bs1000/bs1000_bl1_setup.c		\
				plat/baikal/bs1000/bs1000_dimm_spd.c		\
				plat/baikal/bs1000/drivers/ddr/ddr_ctrl.c	\
				plat/baikal/bs1000/drivers/ddr/ddr_io.c		\
				plat/baikal/bs1000/drivers/ddr/ddr_main.c	\
				plat/baikal/bs1000/drivers/ddr/ddr_master.c	\
				plat/baikal/bs1000/drivers/ddr/ddr_misc.c	\
				plat/baikal/bs1000/drivers/ddr/ddr_odt_settings.c\
				plat/baikal/bs1000/drivers/ddr/phy/ddr_phy_calc.c\
				plat/baikal/bs1000/drivers/ddr/phy/ddr_phy_init.c\
				plat/baikal/bs1000/drivers/ddr/phy/ddr_phy_load.c\
				plat/baikal/bs1000/drivers/ddr/phy/ddr_phy_main.c\
				plat/baikal/bs1000/drivers/ddr/phy/ddr_phy_misc.c\
				plat/baikal/bs1000/drivers/ddr/phy/ddr_phy_msgb.c\
				plat/baikal/common/baikal_bl1_stack.c		\
				plat/baikal/common/baikal_common.c		\
				plat/baikal/common/baikal_io_storage.c		\
				plat/baikal/common/crc.c			\
				plat/baikal/common/dw_i2c.c			\
				plat/baikal/common/memtest.c			\
				plat/baikal/common/spd.c

override BL1_DEFAULT_LINKER_SCRIPT_SOURCE := plat/baikal/common/bl1.ld.S

BL2_SOURCES		+=	common/desc_image_load.c			\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/baikal/bs1000/bs1000_bl2_setup.c		\
				plat/baikal/common/baikal_bl2_mem_params_desc.c	\
				plat/baikal/common/baikal_image_load.c		\
				plat/baikal/common/baikal_io_storage.c		\
				plat/baikal/common/crc.c

GICV3_SUPPORT_GIC600	:=	1
ifneq ($(PLATFORM_CHIP_COUNT),1)
GICV3_IMPL_GIC600_MULTICHIP	:=	1
endif
include drivers/arm/gic/v3/gicv3.mk

BL31_SOURCES		+=	drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				lib/cpus/aarch64/cortex_a75.S			\
				plat/baikal/bs1000/bs1000_bl31_setup.c		\
				plat/baikal/bs1000/bs1000_ca75.c		\
				plat/baikal/bs1000/bs1000_dimm_spd.c		\
				plat/baikal/bs1000/bs1000_dt.c			\
				plat/baikal/bs1000/bs1000_pcie.c		\
				plat/baikal/bs1000/bs1000_pm.c			\
				plat/baikal/bs1000/bs1000_sip_svc.c		\
				plat/baikal/bs1000/bs1000_smmu.c		\
				plat/baikal/bs1000/bs1000_topology.c		\
				plat/baikal/bs1000/bs1000_usb.c			\
				plat/baikal/bs1000/drivers/bs1000_cmu.c		\
				plat/baikal/bs1000/drivers/bs1000_coresight.c	\
				plat/baikal/common/baikal_bl31_setup.c		\
				plat/baikal/common/baikal_common.c		\
				plat/baikal/common/baikal_fdt.c			\
				plat/baikal/common/baikal_gicv3.c		\
				plat/baikal/common/baikal_pvt.c			\
				plat/baikal/common/baikal_sip_svc_flash.c	\
				plat/baikal/common/crc.c			\
				plat/baikal/common/dw_i2c.c			\
				plat/baikal/common/dw_gpio.c			\
				plat/baikal/common/ndelay.c			\
				plat/baikal/common/spd.c			\
				plat/common/plat_gicv3.c			\
				plat/common/plat_psci_common.c			\
				${GICV3_SOURCES}				\
				$(LIBFDT_SRCS)
ifeq ($(SYSTEM_COUNTERS_RESYNC),yes)
BL31_SOURCES		+=	plat/baikal/common/baikal_interrupt_mgmt.c	\
				plat/baikal/common/dw_apb_timer.c
endif

ifeq (${ENABLE_PMF}, 1)
BL31_SOURCES		+=	lib/pmf/pmf_smc.c
endif

ifeq (${MEM_PROTECT}, 1)
TF_CFLAGS_aarch64	+=	-DENABLE_MEM_PROTECT
BL31_SOURCES		+=	lib/utils/mem_region.c
endif

ifeq ($(notdir $(CC)),armclang)
TF_CFLAGS_aarch64	+=	-mcpu=cortex-a75
else ifneq ($(findstring clang,$(notdir $(CC))),)
TF_CFLAGS_aarch64	+=	-mcpu=cortex-a75
else
TF_CFLAGS_aarch64	+=	-mtune=cortex-a75
endif

BL1_CPPFLAGS += -march=armv8-a+crc
BL2_CPPFLAGS += -march=armv8-a+crc
BL2U_CPPFLAGS += -march=armv8-a+crc
BL31_CPPFLAGS += -march=armv8-a+crc
BL32_CPPFLAGS += -march=armv8-a+crc
