
// Simple sound queue for synchronous sound handling in SDL

// Copyright (C) 2005 Shay Green. MIT license.

#ifndef SOUND_QUEUE_H
#define SOUND_QUEUE_H

#include <iostream> 
#include <list> 
#include <iterator> 
#include <cinttypes>
#include <list>
#include "Sound_Queue_wrap.h"


class Sound_Queue {
public:
	Sound_Queue();
	~Sound_Queue();
	
	// Initialize with specified sample rate and channel count.
	// Returns NULL on success, otherwise error string.
	const char* init( long sample_rate, int chan_count = 1 );
	
	// Write samples to buffer and block until enough space is available
	void write( const sample_t*, int count );
	
private:
	bool sound_open;
	struct sound_queue_ctx *ctx;
};

#endif

