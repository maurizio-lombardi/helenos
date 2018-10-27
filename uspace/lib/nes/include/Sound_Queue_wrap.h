#ifndef _SOUND_QUEUE_WRAP_
#define _SOUND_QUEUE_WRAP_

#ifdef __cplusplus
extern "C" {
#endif

struct sound_queue_ctx;
typedef short sample_t;

void sound_queue_init(struct sound_queue_ctx **ctx, long rate, int chan);
void sound_queue_write(struct sound_queue_ctx *ctx, const sample_t *in, int count);

#ifdef __cplusplus
}
#endif

#endif
