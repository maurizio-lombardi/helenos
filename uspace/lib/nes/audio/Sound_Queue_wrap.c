#include <fibril.h>
#include <fibril_synch.h>
#include <hound/client.h>
#include <errno.h>
#include <stdlib.h>
#include <str.h>

#include "Sound_Queue_wrap.h"

#define NRBUFS 8

struct snd_buffer {
	void *buf;
	size_t s;
};

struct sound_queue_ctx {
	fibril_mutex_t buffer_lock;
	fibril_condvar_t buffer_cv;
	fid_t audio_fid;
	hound_context_t *hound;
	unsigned idx_read, idx_write;
	unsigned nents;
	struct snd_buffer bufs[NRBUFS];
};

static errno_t sound_thread(void *argc)
{
	struct sound_queue_ctx *ctx = (struct sound_queue_ctx *) argc;
	struct snd_buffer *buf_entry;

	while (1) {
		fibril_mutex_lock(&ctx->buffer_lock);

		if (ctx->nents == 0)
			fibril_condvar_wait(&ctx->buffer_cv, &ctx->buffer_lock);

		while (ctx->nents > 0) {
			buf_entry = &ctx->bufs[ctx->idx_read];
			ctx->idx_read++;
			if (ctx->idx_read == NRBUFS)
				ctx->idx_read = 0;
			ctx->nents--;

			fibril_mutex_unlock(&ctx->buffer_lock);
			hound_write_main_stream(ctx->hound, buf_entry->buf, buf_entry->s * sizeof(sample_t));
			free(buf_entry->buf);
		}

	}
	return 0;
}

void sound_queue_write(struct sound_queue_ctx *ctx, const sample_t *in, int count)
{
	size_t n;
	struct snd_buffer *buf_entry;

	while (count > 0) {
		fibril_mutex_lock(&ctx->buffer_lock);

		if (ctx->nents == NRBUFS) {
			fibril_mutex_unlock(&ctx->buffer_lock);
			fibril_condvar_broadcast(&ctx->buffer_cv);
			continue;
		}

		if (count > 2048)
			n = 2048;
		else
			n = count;
		count -= n;

		buf_entry = &ctx->bufs[ctx->idx_write];
		ctx->idx_write++;
		if (ctx->idx_write == NRBUFS)
			ctx->idx_write = 0;
		ctx->nents++;

		buf_entry = malloc(n * sizeof(sample_t));
		memcpy(buf_entry, in, n * sizeof(sample_t));

		buf_entry->s = n;

		fibril_mutex_unlock(&ctx->buffer_lock);
		fibril_condvar_broadcast(&ctx->buffer_cv);
	}
}

void sound_queue_init(struct sound_queue_ctx **c, long rate, int chan)
{
	struct sound_queue_ctx *ctx;
	pcm_format_t format;

	*c = malloc(sizeof(struct sound_queue_ctx));

	ctx = *c;
	
	fibril_mutex_initialize(&ctx->buffer_lock);
	fibril_condvar_initialize(&ctx->buffer_cv);

	ctx->idx_read = ctx->idx_write = 0;
	ctx->nents = 0;

	format.channels = chan;
	format.sampling_rate = rate;
	format.sample_format = PCM_SAMPLE_SINT16_LE;

	ctx->hound = hound_context_create_playback("nes", format, 2048 * sizeof(sample_t));
	hound_context_connect_target(ctx->hound, HOUND_DEFAULT_TARGET);

	ctx->audio_fid = fibril_create(sound_thread, ctx);
	fibril_add_ready(ctx->audio_fid);
}

