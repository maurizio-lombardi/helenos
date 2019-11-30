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

#include <errno.h>
#include <pcut/pcut.h>
#include <stdio.h>
#include <str.h>

#include "../client.h"
#include "../display.h"
#include "../seat.h"
#include "../window.h"

PCUT_INIT;

PCUT_TEST_SUITE(display);

static void test_ds_ev_pending(void *);

static ds_client_cb_t test_ds_client_cb = {
	.ev_pending = test_ds_ev_pending
};

static void test_ds_ev_pending(void *arg)
{
	bool *called_cb = (bool *) arg;
	printf("test_ds_ev_pending\n");
	*called_cb = true;

}

/** Display creation and destruction. */
PCUT_TEST(display_create_destroy)
{
	ds_display_t *disp;
	errno_t rc;

	rc = ds_display_create(NULL, &disp);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	ds_display_destroy(disp);
}

/** Basic client operation. */
PCUT_TEST(display_client)
{
	ds_display_t *disp;
	ds_client_t *client;
	ds_client_t *c0, *c1;
	errno_t rc;

	rc = ds_display_create(NULL, &disp);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_client_create(disp, &test_ds_client_cb, NULL, &client);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	c0 = ds_display_first_client(disp);
	PCUT_ASSERT_EQUALS(c0, client);

	c1 = ds_display_next_client(c0);
	PCUT_ASSERT_NULL(c1);

	ds_client_destroy(client);
	ds_display_destroy(disp);
}

/** Test ds_display_find_window(). */
PCUT_TEST(display_find_window)
{
	ds_display_t *disp;
	ds_client_t *client;
	ds_window_t *w0;
	ds_window_t *w1;
	ds_window_t *wnd;
	errno_t rc;

	rc = ds_display_create(NULL, &disp);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_client_create(disp, &test_ds_client_cb, NULL, &client);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_window_create(client, &w1);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_window_create(client, &w0);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	wnd = ds_display_first_window(disp);
	PCUT_ASSERT_EQUALS(w0, wnd);

	wnd = ds_display_next_window(wnd);
	PCUT_ASSERT_EQUALS(w1, wnd);

	wnd = ds_display_next_window(wnd);
	PCUT_ASSERT_NULL(wnd);

	wnd = ds_display_find_window(disp, w0->id);
	PCUT_ASSERT_EQUALS(w0, wnd);

	wnd = ds_display_find_window(disp, w1->id);
	PCUT_ASSERT_EQUALS(w1, wnd);

	wnd = ds_display_find_window(disp, 0);
	PCUT_ASSERT_NULL(wnd);

	wnd = ds_display_find_window(disp, w0->id + 1);
	PCUT_ASSERT_NULL(wnd);

	ds_window_destroy(w0);
	ds_window_destroy(w1);
	ds_client_destroy(client);
	ds_display_destroy(disp);
}

/** Basic seat operation. */
PCUT_TEST(display_seat)
{
	ds_display_t *disp;
	ds_seat_t *seat;
	ds_seat_t *s0, *s1;
	errno_t rc;

	rc = ds_display_create(NULL, &disp);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_seat_create(disp, &seat);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	s0 = ds_display_first_seat(disp);
	PCUT_ASSERT_EQUALS(s0, seat);

	s1 = ds_display_next_seat(s0);
	PCUT_ASSERT_NULL(s1);

	ds_seat_destroy(seat);
	ds_display_destroy(disp);
}

/** Test ds_display_post_kbd_event(). */
PCUT_TEST(display_post_kbd_event)
{
	ds_display_t *disp;
	ds_seat_t *seat;
	ds_client_t *client;
	ds_window_t *wnd;
	kbd_event_t event;
	bool called_cb = NULL;
	errno_t rc;

	rc = ds_display_create(NULL, &disp);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_seat_create(disp, &seat);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_client_create(disp, &test_ds_client_cb, &called_cb, &client);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	rc = ds_window_create(client, &wnd);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);

	ds_seat_set_focus(seat, wnd);

	event.type = KEY_PRESS;
	event.key = KC_ENTER;
	event.mods = 0;
	event.c = L'\0';

	PCUT_ASSERT_FALSE(called_cb);

	rc = ds_display_post_kbd_event(disp, &event);
	PCUT_ASSERT_ERRNO_VAL(EOK, rc);
	PCUT_ASSERT_TRUE(called_cb);

	ds_window_destroy(wnd);
	ds_client_destroy(client);
	ds_seat_destroy(seat);
	ds_display_destroy(disp);
}

PCUT_EXPORT(display);