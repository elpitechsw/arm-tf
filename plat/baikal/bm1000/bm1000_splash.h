/*
 * Copyright (c) 2020-2021, Baikal Electronics, JSC. All rights reserved.
 *
 * Author: Pavel Parkhomenko <Pavel.Parkhomenko@baikalelectronics.ru>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BM1000_SPLASH_H
#define BM1000_SPLASH_H

#include <stdint.h>

#include "bm1000_font.h"

#define SDK_VERSION_SIZE	40

typedef enum {
	BAIKAL_LVDS_JEIDA_18,
	BAIKAL_LVDS_VESA_24
} lvds_data_mapping_t;

typedef struct {
	uint32_t clock;
	uint32_t hact;
	uint32_t hfp;
	uint32_t hsync;
	uint32_t hbp;
	uint32_t vact;
	uint32_t vfp;
	uint32_t vsync;
	uint32_t vbp;
	uint32_t hspol_inv;
	uint32_t vspol_inv;
	uint32_t depol_inv;
	int ports;				/* LVDS only */
	lvds_data_mapping_t data_mapping;	/* LVDS only */
	int gpio_pin;				/* LVDS only */
	int gpio_polarity;			/* LVDS only */
	uintptr_t cmu_base;
	uint64_t cmu_frefclk;
	int enabled;
} modeline_t;

typedef struct {
	/* 2-byte signature is omitted (placed just before file_size) */
	uint32_t file_size;
	uint32_t reserved;
	uint32_t pixel_offset;
	uint32_t dib_header_size;
	uint32_t width;
	uint32_t height;
	uint16_t planes : 16;
	uint16_t bpp : 16;
} bmp_header_t;

extern const modeline_t hdmi_video_mode;
extern const modeline_t lvds_video_mode;
extern char sdk_version[];

void hdmi_tx_init(void);
void print_sdk_version(uintptr_t fb_base, const modeline_t *mode, int w1, int h1, const char *version);
void early_splash(uintptr_t vdu_base, uintptr_t fb_base, const modeline_t *mode, int fb_cpp, const char *msg);
void lvds_early_splash(const char *msg);
void hdmi_early_splash(const char *msg);
void bmp_get_dimensions(const uint8_t *bmp_file, int *width, int *height);
int bmp_get_cpp(const uint8_t *bmp_file);
int bmp_to_fb(uintptr_t fb, const modeline_t *mode, const uint8_t *bmp_file, int dx, int dy, int clear_screen);
void vdu_init(uint64_t vdu_base, uint32_t fb_base, const modeline_t *mode);
void vdu_set_fb(uint64_t vdu_base, uint32_t fb_base, const modeline_t *mode, int fb_cpp);
int fdt_get_panel(modeline_t *modeline);
void display_logo_and_version(uintptr_t vdu_base, uintptr_t fb_base, const modeline_t *mode, const uint8_t *logo);

#endif /* BM1000_SPLASH_H */
