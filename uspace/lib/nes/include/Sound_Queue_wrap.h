#ifndef _SOUND_QUEUE_WRAP_
#define _SOUND_QUEUE_WRAP_

#ifdef __cplusplus
extern "C" {
#endif

typedef short sample_t;

void sound_queue_init(long rate, int chan);
void sound_queue_write(const sample_t *in, int count);

#ifdef __cplusplus
}
#endif

#endif
