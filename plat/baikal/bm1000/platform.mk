#
# Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libfdt/libfdt.mk
include lib/xlat_tables_v2/xlat_tables.mk

$(eval $(call add_define_val,SDK_VERSION,$(SDK_VERSION)))

USE_COHERENT_MEM	:=	1

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/aarch64	\
				-Iplat/baikal/bm1000/drivers		\
				-Iplat/baikal/bm1000/drivers/ddr	\
				-Iplat/baikal/bm1000/include		\
				-Iplat/baikal/common/include

PLAT_BL_COMMON_SOURCES	:=	drivers/mmc/mmc.c				\
				drivers/ti/uart/aarch64/16550_console.S		\
				plat/baikal/bm1000/aarch64/bm1000_helpers.S	\
				plat/baikal/bm1000/bm1000_common.c		\
				plat/baikal/bm1000/bm1000_font.c		\
				plat/baikal/bm1000/bm1000_mmavlsp.c		\
				plat/baikal/bm1000/bm1000_mmca57.c		\
				plat/baikal/bm1000/bm1000_mmxgbe.c		\
				plat/baikal/bm1000/drivers/bm1000_scp.c		\
				plat/baikal/bm1000/drivers/bm1000_scp_flash.c	\
				plat/baikal/common/aarch64/baikal_helpers.S	\
				plat/baikal/common/baikal_bootflash.c		\
				plat/baikal/common/baikal_console.c		\
				plat/baikal/common/baikal_mshc.c		\
				plat/baikal/common/dw_gpio.c			\
				plat/baikal/common/dw_spi_flash.c		\
				${XLAT_TABLES_LIB_SRCS}

ifeq ($(NEED_BL32),yes)
$(eval $(call add_define,BAIKAL_LOAD_BL32))
endif

ifeq ($(BAIKAL_TARGET),dbm10)
$(eval $(call add_define,BAIKAL_DBM10))
else ifeq ($(BAIKAL_TARGET),dbm20)
$(eval $(call add_define,BAIKAL_DBM20))
else ifeq ($(BAIKAL_TARGET),mbm10)
$(eval $(call add_define,BAIKAL_MBM10))
else ifeq ($(BAIKAL_TARGET),mbm20)
$(eval $(call add_define,BAIKAL_MBM20))
else ifeq ($(BAIKAL_TARGET),elp_bm)
$(eval $(call add_define,ELPITECH))
$(eval $(call add_define,BOARD_VER))
ifneq ($(BAIKAL_DDR_CUSTOM_CLOCK_FREQ),)
$(eval $(call add_define,BAIKAL_DDR_CUSTOM_CLOCK_FREQ))
endif
else ifeq ($(BAIKAL_TARGET),qemu-m)
$(eval $(call add_define,BAIKAL_QEMU))
else
$(error "Error: unknown BAIKAL_TARGET=${BAIKAL_TARGET}")
endif

BL1_SOURCES		+=	drivers/arm/ccn/ccn.c				\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				lib/cpus/aarch64/cortex_a57.S			\
				plat/arm/common/arm_ccn.c			\
				plat/baikal/bm1000/bm1000_bl1_setup.c		\
				plat/baikal/bm1000/bm1000_splash.c		\
				plat/baikal/bm1000/drivers/bm1000_cmu.c		\
				plat/baikal/bm1000/drivers/bm1000_smbus.c	\
				plat/baikal/bm1000/drivers/ddr/ddr_init.c	\
				plat/baikal/bm1000/drivers/ddr/ddr_lcru.c	\
				plat/baikal/bm1000/drivers/ddr/ddr_main.c	\
				plat/baikal/bm1000/drivers/ddr/ddr_master.c	\
				plat/baikal/bm1000/drivers/ddr/ddr_odt_settings.c\
				plat/baikal/bm1000/drivers/ddr/ddr_spd.c	\
				plat/baikal/bm1000/drivers/ddr/ddr_menu.c	\
				plat/baikal/common/baikal_bl1_stack.c		\
				plat/baikal/common/baikal_common.c		\
				plat/baikal/common/baikal_io_storage.c		\
				plat/baikal/common/crc.c			\
				plat/baikal/common/dw_i2c.c			\
				plat/baikal/common/memtest.c			\
				plat/baikal/common/ndelay.c

override BL1_DEFAULT_LINKER_SCRIPT_SOURCE := plat/baikal/common/bl1.ld.S

BL2_SOURCES		+=	common/desc_image_load.c			\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/baikal/bm1000/bm1000_bl2_setup.c		\
				plat/baikal/bm1000/drivers/bm1000_smbus.c	\
				plat/baikal/bm1000/dt.c				\
				plat/baikal/common/baikal_bl2_mem_params_desc.c	\
				plat/baikal/common/baikal_fdt.c			\
				plat/baikal/common/baikal_image_load.c		\
				plat/baikal/common/baikal_io_storage.c		\
				plat/baikal/common/crc.c			\
				plat/baikal/common/dw_i2c.c			\
				plat/baikal/common/spd.c			\
				$(LIBFDT_SRCS)

ifeq (${SPD},opteed)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif

include drivers/arm/gic/v3/gicv3.mk

BL31_SOURCES		+=	drivers/arm/ccn/ccn.c				\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				plat/arm/common/arm_ccn.c			\
				plat/baikal/bm1000/bm1000_bl31_logo.c		\
				plat/baikal/bm1000/bm1000_bl31_setup.c		\
				plat/baikal/bm1000/bm1000_mmcoresight.c		\
				plat/baikal/bm1000/bm1000_mmmali.c		\
				plat/baikal/bm1000/bm1000_mmpcie.c		\
				plat/baikal/bm1000/bm1000_mmusb.c		\
				plat/baikal/bm1000/bm1000_mmvdec.c		\
				plat/baikal/bm1000/bm1000_pm.c			\
				plat/baikal/bm1000/bm1000_sip_svc.c		\
				plat/baikal/bm1000/bm1000_splash.c		\
				plat/baikal/bm1000/bm1000_topology.c		\
				plat/baikal/bm1000/drivers/bm1000_cmu.c		\
				plat/baikal/bm1000/drivers/bm1000_efuse.c	\
				plat/baikal/bm1000/drivers/bm1000_smmu.c	\
				plat/baikal/bm1000/drivers/mbm_bmc.c		\
				plat/baikal/bm1000/dt.c				\
				plat/baikal/common/baikal_bl31_setup.c		\
				plat/baikal/common/baikal_common.c		\
				plat/baikal/common/baikal_fdt.c			\
				plat/baikal/common/baikal_gicv3.c		\
				plat/baikal/common/baikal_pvt.c			\
				plat/baikal/common/baikal_sip_svc_flash.c	\
				plat/baikal/common/dw_i2c.c			\
				plat/common/plat_gicv3.c			\
				plat/common/plat_psci_common.c			\
				${GICV3_SOURCES}				\
				$(LIBFDT_SRCS)

ifeq (${ENABLE_PMF}, 1)
BL31_SOURCES		+=	lib/pmf/pmf_smc.c
endif

ifeq (${MEM_PROTECT}, 1)
TF_CFLAGS_aarch64	+=	-DENABLE_MEM_PROTECT
BL31_SOURCES		+=	lib/utils/mem_region.c
endif

ifeq ($(notdir $(CC)),armclang)
TF_CFLAGS_aarch64	+=	-mcpu=cortex-a57
else ifneq ($(findstring clang,$(notdir $(CC))),)
TF_CFLAGS_aarch64	+=	-mcpu=cortex-a57
else
TF_CFLAGS_aarch64	+=	-mtune=cortex-a57
endif

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call FIP_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call FIP_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif

BL32_RAM_LOCATION	:=	tdram
ifeq (${BL32_RAM_LOCATION}, tsram)
BL32_RAM_LOCATION_ID = SEC_SRAM_ID
else ifeq (${BL32_RAM_LOCATION}, tdram)
BL32_RAM_LOCATION_ID = SEC_DRAM_ID
else
$(error "Unsupported BL32_RAM_LOCATION value")
endif

# Process flags
$(eval $(call add_define,BL32_RAM_LOCATION_ID))

# Errata workarounds for Cortex-A57 r1p3:
ERRATA_A57_806969	:=	0
ERRATA_A57_813419	:=	0
ERRATA_A57_813420	:=	0
ERRATA_A57_826974	:=	0
ERRATA_A57_826977	:=	0
ERRATA_A57_828024	:=	0
ERRATA_A57_829520	:=	0
ERRATA_A57_833471	:=	0
ERRATA_A57_859972	:=	1
ERRATA_A57_1319537	:=	1

BL1_CPPFLAGS += -march=armv8-a+crc
BL2_CPPFLAGS += -march=armv8-a+crc
BL2U_CPPFLAGS += -march=armv8-a+crc
BL31_CPPFLAGS += -march=armv8-a+crc
BL32_CPPFLAGS += -march=armv8-a+crc
