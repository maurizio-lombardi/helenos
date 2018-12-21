#include <stdio.h>
#include <stdlib.h>
#include <str.h>
#include <window.h>
#include <canvas.h>
#include <surface.h>
#include <errno.h>
#include <task.h>
#include <str.h>
#include <str_error.h>
#include <mem.h>
#include <time.h>
#include <loc.h>
#include <device/clock_dev.h>
#include <crypto.h>
#include <getopt.h>
#include "joypad.h"
#include "cppwrap.h"
#include "cwrap.h"
#include "save.h"

#define NAME "vnes"

#define WINDOW_WIDTH	256
#define WINDOW_HEIGHT	240
#define FPS		60


static struct option const long_options[] = {
	{"help", no_argument, 0, 'h'},
	{"restore", required_argument, 0, 'r'},
	{"nosound", no_argument, 0, 'n'},
};

char *rom_filename = NULL;
static fibril_timer_t *frame_timer = NULL;
canvas_t *canvas;
surface_t *surface;
uint32_t *pixels;
extern int save_req;

static int need_refresh = 0;

extern int pause;

static void frame_timer_cb(void *data);

static void usage(void)
{
	printf("Usage: #%s compositor_server [options] nes_rom\n", NAME);
	printf("-r, --restore <path>     Restore a saved game session\n");
	printf("-n, --nosound            Disable sound\n");
	printf("-h, --help               Prints this message\n");
}

static void on_keyboard_event(widget_t *widget, void *data)
{
	kbd_event_t *event = (kbd_event_t *) data;
	joypad_keyboard_event(event->key, event->type == KEY_PRESS);
}

int main(int argc, char **argv)
{
	int rc;
	char *sha1 = NULL;
	int c, opt_ind;
	char *restore_file = NULL;
	char *comp_serv;
	int nosound = 0;

	if (argc < 3) {
		usage();
		return 1;
	}

	c = optind = opt_ind = 0;
	while (c != -1) {
		c = getopt_long(argc, argv, "nhr:", long_options,
				&opt_ind);
		switch (c) {
		case 'h':
			usage();
			return 0;
		case 'r':
			restore_file = optarg;
			break;
		case 'n':
			nosound = 1;
			break;
		}
	}

	if (argc - optind < 2) {
		usage();
		return 1;
	}

	argv += optind;
	comp_serv = *argv;
	argv++;
	rom_filename = *argv;

	joypad_init();
	sound_init(nosound);

	if (restore_file) {
		sha1 = malloc(HASH_SHA1);
		if (!sha1)
			return 1;
	}

	printf("Loading image %s... ", rom_filename);
	if (cartridge_load_file(rom_filename)) {
		printf("\n%s: cannot find a valid NES game image\n", NAME);
		return 1;
	}
	printf("Image loading completed\n");

	if (restore_file) {
		rc = restore_game(restore_file, NULL);
		if (rc) {
			printf("Game reload failed with error %s\n", str_error(rc));
			return 1;
		}
	}

	window_t *main_window = window_open(comp_serv, NULL,
	    WINDOW_MAIN | WINDOW_DECORATED, NAME);
	if (!main_window) {
		printf("%s: Cannot open main window\n", NAME);
		return 2;
	}

	surface = surface_create(WINDOW_WIDTH, WINDOW_HEIGHT, NULL,
	    SURFACE_FLAG_NONE);
	if (!surface) {
		printf("%s: Cannot create the surface\n", NAME);
		return 3;
	}

	canvas = create_canvas(window_root(main_window), NULL,
	    WINDOW_WIDTH, WINDOW_HEIGHT, surface);
	if (!canvas) {
		surface_destroy(surface);
		printf("%s: Cannot create the canvas\n", NAME);
		return 4;
	}
	sig_connect(&canvas->keyboard_event, NULL, on_keyboard_event);
	window_resize(main_window, 0, 0, WINDOW_WIDTH + 8, WINDOW_HEIGHT + 28, WINDOW_PLACEMENT_CENTER);

	pixels = (uint32_t *)surface_direct_access(surface);

	window_exec(main_window);

	frame_timer = fibril_timer_create(NULL);
	fibril_timer_set(frame_timer, 1000000, frame_timer_cb, NULL);

	task_retval(0);
	async_manager();
	return 0;
}

static void frame_timer_cb(void *data)
{
	struct timespec cur;
	usec_t next_frame_us;
	const int delay = 1000000 / FPS;
	static usec_t err_us = 0;
	static struct timespec target_us;
	static int enable_timefix = 0;
	static int flash = 0;

	if (!pause) {
		if (save_req) {
			save_game();
			save_req = 0;
			flash = 30;
		}

		if (flash > 0) {
			size_t const bytes = WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t);
			if ((flash-- / 2) & 1)
				memset(pixels, 0xFF, bytes);
			need_refresh = 1;
		}

		if (need_refresh) {
			update_canvas(canvas, surface);
			need_refresh = 0;
		}
		cpu_run_frame();
	}

	getuptime(&cur);

	if (enable_timefix)
		err_us = NSEC2USEC(ts_sub_diff(&cur, &target_us));

	next_frame_us = delay - err_us;
	if (next_frame_us < 1) {
		next_frame_us = 1;
		enable_timefix = 0;
		err_us = 0;
	} else {
		target_us = cur;
		ts_add_diff(&target_us, USEC2NSEC(next_frame_us));
		enable_timefix = 1;
	}

	fibril_timer_set(frame_timer, next_frame_us, frame_timer_cb, NULL);
}

int sha1_chksum(uint8_t *data, size_t data_size, uint8_t *hash)
{
	return create_hash(data, data_size, hash, HASH_SHA1);
}

void new_frame(uint32_t *frame)
{
	const size_t nbytes = WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t);

	if (memcmp(pixels, frame, nbytes) != 0) {
		memcpy(pixels, frame, nbytes);
		need_refresh = 1;
	} else
		need_refresh = 0;
}

