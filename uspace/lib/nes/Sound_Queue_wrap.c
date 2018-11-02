#include <hound/client.h>

#include "Sound_Queue_wrap.h"

static hound_context_t *hound;

void sound_queue_write(const sample_t *in, int count)
{
	hound_write_main_stream(hound, in, count * sizeof(sample_t));
}

void sound_queue_init(long rate, int chan)
{
	pcm_format_t format;

	format.channels = chan;
	format.sampling_rate = rate;
	format.sample_format = PCM_SAMPLE_SINT16_LE;

	hound = hound_context_create_playback("nes", format, 4096 * sizeof(sample_t));
	hound_context_connect_target(hound, HOUND_DEFAULT_TARGET);
}

