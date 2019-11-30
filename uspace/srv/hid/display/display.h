/*
 * Copyright (c) 2019 Jiri Svoboda
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

/** @addtogroup display
 * @{
 */
/**
 * @file Display server display
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <errno.h>
#include <gfx/context.h>
#include <io/kbd_event.h>
#include "types/display/client.h"
#include "types/display/display.h"
#include "types/display/seat.h"

extern errno_t ds_display_create(gfx_context_t *, ds_display_t **);
extern void ds_display_destroy(ds_display_t *);
extern void ds_display_add_client(ds_display_t *, ds_client_t *);
extern void ds_display_remove_client(ds_client_t *);
extern ds_client_t *ds_display_first_client(ds_display_t *);
extern ds_client_t *ds_display_next_client(ds_client_t *);
extern ds_window_t *ds_display_find_window(ds_display_t *, ds_wnd_id_t);
extern void ds_display_add_window(ds_display_t *, ds_window_t *);
extern void ds_display_remove_window(ds_window_t *);
extern ds_window_t *ds_display_first_window(ds_display_t *);
extern ds_window_t *ds_display_next_window(ds_window_t *);
extern errno_t ds_display_post_kbd_event(ds_display_t *, kbd_event_t *);
extern void ds_display_add_seat(ds_display_t *, ds_seat_t *);
extern void ds_display_remove_seat(ds_seat_t *);
extern ds_seat_t *ds_display_first_seat(ds_display_t *);
extern ds_seat_t *ds_display_next_seat(ds_seat_t *);

#endif

/** @}
 */