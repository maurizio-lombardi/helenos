#include <stdio.h>
#include <window.h>
#include <canvas.h>
#include <surface.h>
#include <errno.h>
#include <task.h>
#include <str.h>
#include "joypad.h"
#include "cppwrap.h"
#include "cwrap.h"

#define NAME "vnes"

#define WINDOW_WIDTH	256
#define WINDOW_HEIGHT	240
#define FPS		60

static fibril_timer_t *frame_timer = NULL;
canvas_t *canvas;
surface_t *surface;
uint32_t *pixels;

static int need_refresh = 0;

extern int pause;

static void frame_timer_cb(void *data);

static void usage(void) {
	printf("Usage: $%s compositor_server nes_rom\n", NAME);
}

static void on_keyboard_event(widget_t *widget, void *data)
{
	kbd_event_t *event = (kbd_event_t *) data;
	joypad_keyboard_event(event->key, event->type == KEY_PRESS);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		usage();
		return 1;
	}

	printf("Loading image %s... ", argv[2]);
	if (cartridge_load_file(argv[2])) {
		printf("\n%s: cannot find a valid NES game image\n", NAME);
		return 1;
	}
	printf("Image loading completed\n");

	window_t *main_window = window_open(argv[1], NULL,
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
	joypad_init();
	sound_init();
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
	struct timespec prev, cur;
	usec_t us, next_frame_us;
	const int delay = 1000000 / FPS;
	static usec_t err_us = 0;
	static struct timespec target_us;
	static int enable_timefix = 0;

	getuptime(&prev);

	if (!pause) {
		if (need_refresh) {
			update_canvas(canvas, surface);
			need_refresh = 0;
		}
		cpu_run_frame();
	}
	getuptime(&cur);

	if (enable_timefix)
		err_us = ts_sub_diff(&cur, &target_us);

	us = NSEC2USEC(ts_sub_diff(&cur, &prev));

	next_frame_us = delay - (us + err_us);
	if (next_frame_us < 1) {
		next_frame_us = 1;
		enable_timefix = 0;
		err_us = 0;
	} else {
		target_us = cur;
		ts_add_diff(&target_us, next_frame_us);
		enable_timefix = 1;
	}

	fibril_timer_set(frame_timer, next_frame_us, frame_timer_cb, NULL);
}

void new_frame(uint32_t *frame)
{
	size_t nbytes = WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t);
	if (memcmp(pixels, frame, nbytes) != 0) {
		memcpy(pixels, frame, WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
		need_refresh = 1;
	} else
		need_refresh = 0;
}

