#include <common/debug.h>
#include <lib/libc/stddef.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>

#include <baikal_def.h>
#include <crc.h>

#include "ddr_spd.h"
#include "ddr_main.h"

int console_getc_nonblocked(void)
{
	int err = ERROR_NO_VALID_CONSOLE;
	console_t *console;

	for (console = console_list; console != NULL;
	     console = console->next) {
		if ((console->flags & CONSOLE_FLAG_BOOT) && (console->getc != NULL)) {
			int ret = console->getc(console);
			if (ret >= 0)
				return ret;
			if (err != ERROR_NO_PENDING_CHAR)
				err = ret;
		}
	}

        return err;
}

static const char cfg_magic[4] = ELP_DDR_CONF_MAGIC;

static void cfg_check(struct ddr_local_conf *conf, unsigned dimms)
{
	if ((memcmp(conf->magic, cfg_magic, 4) == 0) &&
	    (crc16(conf, offsetof(struct ddr_local_conf, crc), 0)) == conf->crc &&
	    conf->dimms == dimms)
		return;
	memset(conf, 0, sizeof(*conf));
	memcpy(conf->magic, cfg_magic, 4);
	conf->freq = 0;
	conf->dimms = dimms;
	conf->cl = 0;
	conf->al = 1;
	conf->rtt_wr = 1;
	conf->rtt_nom = 3;
	conf->rtt_park = 0;
	conf->odi = 0;
	conf->phy_odt = 0x7;
	conf->phy_odi_pu = 0xb;
	conf->crc = crc16(conf, offsetof(struct ddr_local_conf, crc), 0);
}

static void cfg_freq(struct ddr_local_conf *conf)
{
	int key;

	printf("Select DDR4 rate:\n"
		"0. Leave SPD default\n"
		"1. 1600\n"
		"2. 1867\n"
		"3. 2133\n"
		"4. 2400\n"
		"5. 2667\n"
		"Any other key to return\n");
	key = console_getc();
	switch (key) {
	case '0':
		conf->freq = 0;
		return;
	case '1':
		conf->freq = 1600;
		return;
	case '2':
		conf->freq = 1867;
		return;
	case '3':
		conf->freq = 2133;
		return;
	case '4':
		conf->freq = 2400;
		return;
	case '5':
		conf->freq = 2667;
		return;
	default:
		return;
	}
}

static void cfg_get_num(uint8_t *ret, int max_val)
{
	int val = 0, key;
	*ret = 0;
	printf(" - Enter number 0..%d\n", max_val);
	key = console_getc();
	if (key >= '0' && key <= '9') {
		console_putc(key);
		val = key - '0';
	} else {
		console_putc('\n');
		return;
	}
	key = console_getc();
	if (key >= '0' && key <= '9') {
		console_putc(key);
		val = val * 10 + key - '0';
	} else if (key == '\n' || key == '\r') {
		console_putc('\n');
		if (val <= max_val)
			*ret = val;
		return;
	} else {
		return;
	}
	key = console_getc();
	if (key == '\n' || key == '\r') {
                console_putc('\n');
                if (val <= max_val)
                        *ret = val;
	}
	return;
}

static void cfg_save(struct ddr_local_conf *conf, int idx)
{
	conf->crc = crc16(conf, offsetof(struct ddr_local_conf, crc), 0);
	ddr_write_conf(idx, conf, sizeof(*conf));
}

static void cfg_chan(struct ddr_local_conf *conf, int idx)
{
	int sel;
	int done = 0;

	while (!done) {
		printf("Select field to modify:\n"
			"0. DDR4 rate:  [%d]\n"
			"1. CL:         [%d]\n"
			"2. AL:         [%d]\n"
			"3. rtt_WR:     [%d]\n"
			"4. rtt_NOM:    [%d]\n"
			"5. rtt_PARK:   [%d]\n"
			"6. ODI:        [%d]\n"
			"7. phy_odt:    [%d]\n"
			"8. phy_odi_pu: [%d]\n"
			"ENTER to save, ESC to return\n",
			conf->freq, conf->cl, conf->al, conf->rtt_wr,
			conf->rtt_nom, conf->rtt_park, conf->odi, conf->phy_odt,
			conf->phy_odi_pu);
		sel = console_getc();
		switch (sel) {
		case '0':
			cfg_freq(conf);
			break;
		case '1':
			printf("select CL");
			cfg_get_num(&conf->cl, 2);
			break;
		case '2':
			printf("select AL");
			cfg_get_num(&conf->al, 3);
			break;
		case '3':
			printf("select rtt_WR");
			cfg_get_num(&conf->rtt_wr, 5);
			break;
		case '4':
			printf("select rtt_NOM");
			cfg_get_num(&conf->rtt_nom, 8);
			break;
		case '5':
			printf("select rtt_PARK");
			cfg_get_num(&conf->rtt_park, 8);
			break;
		case '6':
			printf("select ODI");
			cfg_get_num(&conf->odi, 2);
			break;
		case '7':
			printf("select phy_ODT");
			cfg_get_num(&conf->phy_odt, 16);
			break;
		case '8':
			printf("select phy_ODI_PU");
			cfg_get_num(&conf->phy_odi_pu, 16);
			break;
		case '\n':
		case '\r':
			cfg_save(conf, idx);
			done = 1;
			break;
		default:
			done = 1;
			break;
		}
	}
}

int ddr_conf(unsigned ddr_map)
{
	int key = 0;
	uint64_t timeout;
	struct ddr_local_conf *conf0 = NULL, *conf1 = NULL;
	unsigned dimms;
	extern struct spd_container spd_content;

	if (ddr_map & 1) {
		conf0 = (struct ddr_local_conf *)spd_content.content[0].user;
		if (spd_content.dual_channel[0] == 'y')
			dimms = 2;
		else
			dimms = 0;
		cfg_check(conf0, dimms);
	}
	if (ddr_map & 2) {
		conf1 = (struct ddr_local_conf *)spd_content.content[1].user;
		if (spd_content.dual_channel[1] == 'y')
			dimms = 3;
		else
			dimms = 1;
		cfg_check(conf1, dimms);
	}
	printf("Press S to enter DDR configuration...");
	timeout = timeout_init_us(3000000);
	while (!timeout_elapsed(timeout)) {
		key = console_getc_nonblocked();
		if (key > 0)
			break;
	}
	printf("\n");
	if (key <= 0 || key != 's')
		return 0;

	if (conf0) {
		printf("Configure DDR channel 0\n");
		cfg_chan(conf0, 0);
	}
	if (conf1) {
		printf("Configure DDR channel 1\n");
		cfg_chan(conf1, 1);
	}
	return 0;
}
