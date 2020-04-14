/*
 * Copyright (c) 2020 Maurizio Lombardi
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

/** @addtogroup raspifb
 * @{
 */
/**
 * @file
 */

#include <stdio.h>
#include <align.h>
#include <as.h>
#include <assert.h>
#include <errno.h>
#include <ddev_srv.h>
#include <ddev/info.h>
#include <ddf/driver.h>
#include <ipcgfx/server.h>
#include <str_error.h>
#include <ddf/log.h>
#include <ddi.h>
#include <gfx/color.h>
#include <io/pixelmap.h>

#include "raspifb.h"

#define NAME "raspifb"

//#include "mbox.h"

ddev_ops_t raspifb_ddev_ops = {
	.get_gc = NULL,
	.get_info = NULL
};

gfx_context_ops_t raspifb_gc_ops = {
	.set_color = NULL,
	.fill_rect = NULL,
	.bitmap_create = NULL,
	.bitmap_destroy = NULL,
	.bitmap_render = NULL,
	.bitmap_get_alloc = NULL,
};

static void raspifb_client_conn(ipc_call_t *icall, void *arg)
{
	raspifb_t *raspifb;
	ddev_srv_t srv;
	sysarg_t gc_id;
	gfx_context_t *gc;
	errno_t rc;

	raspifb = (raspifb_t *) ddf_dev_data_get(
			ddf_fun_get_dev((ddf_fun_t *) arg));
	gc_id = ipc_get_arg3(icall);

	if (gc_id == 0) {
		/* Set up protocol structure */
		ddev_srv_initialize(&srv);
		srv.ops = &raspifb_ddev_ops;
		srv.arg = raspifb;

		/* Handle connection */
		ddev_conn(icall, &srv);
	} else {
		assert(gc_id == 42);

		rc = gfx_context_new(&raspifb_gc_ops, raspifb, &gc);
		if (rc != EOK)
			goto error;

		/* GC connection */
		gc_conn(icall, gc);
	}

	return;

error:
	async_answer_0(icall, rc);
}

static errno_t raspifb_dev_add(ddf_dev_t *dev)
{
	assert(dev);

	ddf_fun_t *fun = ddf_fun_create(dev, fun_exposed, "a");
	if (!fun) {
		ddf_log_error("Failed to create visualizer function\n");
		return ENOMEM;
	}

	ddf_fun_set_conn_handler(fun, &raspifb_client_conn);
	return EOK;
}

static driver_ops_t raspifb_driver_ops = {
	.dev_add = raspifb_dev_add,
};

static driver_t raspifb_driver = {
	.name = NAME,
	.driver_ops = &raspifb_driver_ops
};

int main(int argc, char **argv)
{
	printf("%s: HelenOS raspberry pi fb driver\n", NAME);
	ddf_log_init(NAME);
	return ddf_driver_main(&raspifb_driver);
}

/** @}
 */

