/*
 * Copyright (c) 2006 Jakub Jermar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup sparc64mm	
 * @{
 */
/** @file
 */

#ifndef KERN_sparc64_CACHE_H_
#define KERN_sparc64_CACHE_H_

#include <mm/page.h>
#include <mm/frame.h>

#define dcache_flush_page(p) \
	dcache_flush_color(PAGE_COLOR((p)))
#define dcache_flush_frame(p, f) \
	dcache_flush_tag(PAGE_COLOR((p)), ADDR2PFN((f)));

/**
 * Enumerations to differentiate among different scopes of D-Cache
 * invalidation.
 */
typedef enum {
	DCACHE_INVL_INVALID,
	DCACHE_INVL_ALL,
	DCACHE_INVL_COLOR,
	DCACHE_INVL_FRAME
} dcache_invalidate_type_t;

/**
 * Number of messages that can be queued in the cpu_arch_t structure at a time.
 */
#define DCACHE_MSG_QUEUE_LEN	10

/** D-cache shootdown message type. */
typedef struct {
	dcache_invalidate_type_t type;
	int color;
	uintptr_t frame;
} dcache_shootdown_msg_t;

extern void dcache_flush(void);
extern void dcache_flush_color(int c);
extern void dcache_flush_tag(int c, pfn_t tag);

#ifdef CONFIG_SMP
extern void dcache_shootdown_start(dcache_invalidate_type_t type, int color,
    uintptr_t frame);
extern void dcache_shootdown_finalize(void);
extern void dcache_shootdown_ipi_recv(void); 
#else
#define dcache_shootdown_start(t, c, f)
#define dcache_shootdown_finalize()
#define dcache_shootdown_ipi_recv()
#endif /* CONFIG_SMP */

#endif

/** @}
 */
