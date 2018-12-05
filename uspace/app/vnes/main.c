#include <stdio.h>
#include <stdlib.h>
#include <str.h>
#include <window.h>
#include <canvas.h>
#include <surface.h>
#include <errno.h>
#include <task.h>
#include <str.h>
#include <mem.h>
#include <time.h>
#include <loc.h>
#include <device/clock_dev.h>
#include <crypto.h>
#include <getopt.h>
#include "joypad.h"
#include "cppwrap.h"
#include "cwrap.h"

#define NAME "vnes"

#define WINDOW_WIDTH	256
#define WINDOW_HEIGHT	240
#define FPS		60


static struct option const long_options[] = {
	{"help", no_argument, 0, 'h'},
	{"restore", required_argument, 0, 'r'},
};

struct save_file_header {
	uint8_t version;
	char rom_filepath[256];
	uint8_t sha1_8k[HASH_SHA1];
	uint32_t cpu_dump_size;
	uint32_t ppu_dump_size;
	uint32_t mapper_dump_size;
	uint32_t pad[16];
};

static char *rom_filename = NULL;
static fibril_timer_t *frame_timer = NULL;
canvas_t *canvas;
surface_t *surface;
uint32_t *pixels;
extern int save_req;

static int need_refresh = 0;

extern int pause;

static void frame_timer_cb(void *data);
static int save_game(void);
static int restore_game(char *restore_file, char *rom_filepath);

static void usage(void)
{
	printf("Usage: #%s compositor_server [options] nes_rom\n", NAME);
	printf("-r, --restore <path>     Restore a saved game session\n");
	printf("-h, --help               Prints this message\n");
}

static void on_keyboard_event(widget_t *widget, void *data)
{
	kbd_event_t *event = (kbd_event_t *) data;
	joypad_keyboard_event(event->key, event->type == KEY_PRESS);
}

int main(int argc, char **argv)
{
	char *sha1 = NULL;
	int c, opt_ind;
	char *restore_file = NULL;
	char *comp_serv;

	if (argc < 3) {
		usage();
		return 1;
	}

	c = optind = opt_ind = 0;
	while (c != -1) {
		c = getopt_long(argc, argv, "hr:", long_options,
				&opt_ind);
		switch (c) {
		case 'h':
			usage();
			return 0;
		case 'r':
			restore_file = optarg;
			break;
		}
	}

	argv += optind;
	comp_serv = *argv;
	argv++;
	rom_filename = *argv;

	joypad_init();
	sound_init();

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

	if (restore_file)
		restore_game(restore_file, NULL);

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

	if (!pause) {
		if (need_refresh) {
			update_canvas(canvas, surface);
			need_refresh = 0;
		}
		cpu_run_frame();
	}

	if (save_req) {
		save_game();
		save_req = 0;
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

int save_game(void)
{
	FILE *fp = NULL;
	char fname[256];
	struct tm t;
	size_t svc_cnt;
	category_id_t cat_id;
	service_id_t *svc_ids = NULL;
	service_id_t svc_id;
	char *svc_name = NULL;
	int rc;
	size_t cpu_size, ppu_size, mapper_size;
	void *mapper_data = NULL;
	void *cpu_data = NULL;
	void *ppu_data = NULL;
	struct save_file_header *hdr = NULL;
	uint8_t *sha1;

	rc = loc_category_get_id("clock", &cat_id, IPC_FLAG_BLOCKING);
	if (rc != EOK)
		goto exit;

	rc = loc_category_get_svcs(cat_id, &svc_ids, &svc_cnt);
	if (rc != EOK)
		goto exit;

	if (svc_cnt == 0) {
		/* No service in clock category */
		rc = ENOENT;
		goto exit;
	}

	rc = loc_service_get_name(svc_ids[0], &svc_name);
	if (rc != EOK)
		goto exit;

	rc = loc_service_get_id(svc_name, &svc_id, 0);
	if (rc != EOK)
		goto exit;

	async_sess_t *sess = loc_service_connect(svc_id, INTERFACE_DDF, 0);
	if (!sess)
		goto exit;

	rc = clock_dev_time_get(sess, &t);
	if (rc != EOK)
		goto exit;

	snprintf(fname, sizeof(fname), "/w/data/nes_%02d_%02d_%d__%02d_%02d_%02d",
		t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min,
		t.tm_sec);

	ppu_data = ppu_dump(&ppu_size);
	cpu_data = cpu_dump(&cpu_size);
	mapper_data = cartridge_dump(&mapper_size);

	if (!mapper_data || !cpu_data || !ppu_data) {
		rc = ENOMEM;
		goto exit;
	}

	hdr = malloc(sizeof(*hdr));
	if (!hdr) {
		rc = ENOMEM;
		goto exit;
	}

	fp = fopen(fname, "wb");
	if (!fp) {
		rc = ENOMEM;
		goto exit;
	}

	sha1 = cartridge_sha1_get();

	hdr->version = 1;
	str_ncpy(hdr->rom_filepath, 255, rom_filename, 255);
	memcpy(hdr->sha1_8k, sha1, HASH_SHA1);
	hdr->cpu_dump_size = cpu_size;
	hdr->ppu_dump_size = ppu_size;
	hdr->mapper_dump_size = mapper_size;

	fwrite(hdr, 1, sizeof(*hdr), fp);
	fwrite(cpu_data, 1, cpu_size, fp);
	fwrite(ppu_data, 1, ppu_size, fp);
	fwrite(mapper_data, 1, mapper_size, fp);

exit:
	if (fp)
		fclose(fp);
	free(hdr);
	free(svc_name);
	free(svc_ids);
	free(ppu_data);
	free(cpu_data);
	free(mapper_data);
	return rc;
}

int restore_game(char *restore_file, char *rom_filepath)
{
	int rc;
	FILE *rfp = NULL;
	void *mapper_data = NULL;
	void *cpu_data = NULL;
	void *ppu_data = NULL;
	struct save_file_header *hdr = NULL;
	uint8_t *sha1 = NULL;

	rfp = fopen(restore_file, "rb");
	if (!rfp) {
		rc = ENOENT;
		goto exit;
	}

	rc = ENOMEM;

	sha1 = malloc(HASH_SHA1);
	if (!sha1)
		goto exit;

	hdr = malloc(sizeof(struct save_file_header));
	if (!hdr)
		goto exit;

	fread(hdr, 1, sizeof(struct save_file_header), rfp);
	if (hdr->version > 1) {
		rc = ENOTSUP;
		goto exit;
	}

	if (rom_filepath)
		str_ncpy(rom_filepath, 255, hdr->rom_filepath, 255);
	memcpy(sha1, hdr->sha1_8k, HASH_SHA1);

	cpu_data = malloc(hdr->cpu_dump_size);
	if (!cpu_data)
		goto exit;

	ppu_data = malloc(hdr->ppu_dump_size);
	if (!ppu_data)
		goto exit;

	mapper_data = malloc(hdr->mapper_dump_size);
	if (!mapper_data)
		goto exit;

	fread(cpu_data, 1, hdr->cpu_dump_size, rfp);
	fread(ppu_data, 1, hdr->ppu_dump_size, rfp);
	fread(mapper_data, 1, hdr->mapper_dump_size, rfp);

	cpu_restore(cpu_data);
	ppu_restore(ppu_data);
	cartridge_restore(mapper_data);

	rc = EOK;
exit:
	if (rfp)
		fclose(rfp);
	free(hdr);
	free(cpu_data);
	free(mapper_data);
	free(ppu_data);
	free(sha1);
	return rc;
}

