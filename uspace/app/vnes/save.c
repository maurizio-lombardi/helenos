#include <stdio.h>
#include <stdlib.h>
#include <str.h>
#include <errno.h>
#include <task.h>
#include <str.h>
#include <str_error.h>
#include <mem.h>
#include <time.h>
#include <loc.h>
#include <device/clock_dev.h>
#include <crypto.h>
#include "save.h"
#include "cppwrap.h"

extern char *rom_filename;

struct save_file_header {
	uint8_t version;
	char rom_filepath[256];
	uint8_t sha1_8k[HASH_SHA1];
	uint32_t cpu_dump_size;
	uint32_t ppu_dump_size;
	uint32_t mapper_dump_size;
	uint32_t apu_dump_size;
	uint32_t pad[4];
};

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
	size_t cpu_size, ppu_size, mapper_size, apu_size;
	void *mapper_data = NULL;
	void *cpu_data = NULL;
	void *ppu_data = NULL;
	void *apu_data = NULL;
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
	apu_data = apu_dump(&apu_size);

	if (!mapper_data || !cpu_data || !ppu_data || !apu_data) {
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
	hdr->apu_dump_size = apu_size;

	fwrite(hdr, 1, sizeof(*hdr), fp);
	fwrite(cpu_data, 1, cpu_size, fp);
	fwrite(ppu_data, 1, ppu_size, fp);
	fwrite(mapper_data, 1, mapper_size, fp);
	fwrite(apu_data, 1, apu_size, fp);

exit:
	if (fp)
		fclose(fp);
	free(hdr);
	free(svc_name);
	free(svc_ids);
	free(ppu_data);
	free(cpu_data);
	free(apu_data);
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
	void *apu_data = NULL;
	struct save_file_header *hdr = NULL;
	uint8_t *sha1 = NULL;

	rfp = fopen(restore_file, "rb");
	if (!rfp) {
		rc = ENOENT;
		goto exit;
	}

	rc = ENOMEM;

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

	sha1 = cartridge_sha1_get();
	if (memcmp(sha1, hdr->sha1_8k, HASH_SHA1)) {
		printf("Snapshot's hash and ROM's hash are different\n");
		rc = ENOTSUP;
		goto exit;
	}

	cpu_data = malloc(hdr->cpu_dump_size);
	if (!cpu_data)
		goto exit;

	ppu_data = malloc(hdr->ppu_dump_size);
	if (!ppu_data)
		goto exit;

	mapper_data = malloc(hdr->mapper_dump_size);
	if (!mapper_data)
		goto exit;

	apu_data = malloc(hdr->apu_dump_size);
	if (!apu_data)
		goto exit;

	fread(cpu_data, 1, hdr->cpu_dump_size, rfp);
	fread(ppu_data, 1, hdr->ppu_dump_size, rfp);
	fread(mapper_data, 1, hdr->mapper_dump_size, rfp);
	fread(apu_data, 1, hdr->apu_dump_size, rfp);

	cpu_restore(cpu_data);
	ppu_restore(ppu_data);
	cartridge_restore(mapper_data);
	apu_restore(apu_data);

	rc = EOK;
exit:
	if (rfp)
		fclose(rfp);
	free(hdr);
	free(cpu_data);
	free(mapper_data);
	free(ppu_data);
	free(apu_data);
	return rc;
}

