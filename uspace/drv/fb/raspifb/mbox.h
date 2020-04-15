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
/** @addtogroup kernel_genarch
 * @{
 */
/**
 * @file
 * @brief BCM2835 mailbox communication routines
 */

#ifndef _BCM2835_MBOX_H_
#define _BCM2835_MBOX_H_

#include <stdint.h>

#define BCM2835_MBOX0_ADDR	0x2000B880

enum {
	MBOX_CHAN_PM		= 0,
	MBOX_CHAN_FB		= 1,
	MBOX_CHAN_UART		= 2,
	MBOX_CHAN_VCHIQ		= 3,
	MBOX_CHAN_LED		= 4,
	MBOX_CHAN_BTN		= 5,
	MBOX_CHAN_TS		= 6,
	MBOX_CHAN_PROP_A2V	= 8,
	MBOX_CHAN_PROP_V2A	= 9
};

enum {
	MBOX_TAG_GET_FW_REV		= 0x00000001,
	MBOX_TAG_GET_BOARD_MODEL	= 0x00010001,
	MBOX_TAG_GET_BOARD_REV		= 0x00010002,
	MBOX_TAG_GET_BOARD_MAC		= 0x00010003,
	MBOX_TAG_GET_BOARD_SERIAL	= 0x00010004,
	MBOX_TAG_GET_ARM_MEMORY		= 0x00010005,
	MBOX_TAG_GET_VC_MEMORY		= 0x00010006,
	MBOX_TAG_GET_CLOCKS		= 0x00010007,
	MBOX_TAG_GET_CMD_LINE		= 0x00050001,
};

enum {
	MBOX_PROP_CODE_REQ	= 0x00000000,
	MBOX_PROP_CODE_RESP_OK	= 0x80000000,
	MBOX_PROP_CODE_RESP_ERR	= 0x80000001
};

enum {
	MBOX_POWER_ID_EMMC	= 0x00000000,
	MBOX_POWER_ID_UART0	= 0x00000001,
	MBOX_POWER_ID_UART1	= 0x00000002,
	MBOX_POWER_ID_USB_HCD	= 0x00000003,
};

#define MBOX_POWER_ON		(1 << 0)
#define MBOX_POWER_OFF		(1 << 1)

#define MBOX_STATUS_FULL	(1 << 31)
#define MBOX_STATUS_EMPTY	(1 << 30)

#define MBOX_COMPOSE(chan, value) (((chan) & 0xf) | ((value) & ~0xf))
#define MBOX_MSG_CHAN(msg)	((msg) & 0xf)
#define MBOX_MSG_VALUE(msg)	((msg) & ~0xf)

#define MBOX_ADDR_ALIGN		16

typedef struct {
	uint32_t read;
	uint32_t unused[3];
	uint32_t peek;
	uint32_t sender;
	uint32_t status;
	uint32_t config;
	uint32_t write;
} bcm2835_mbox_t;

typedef struct {
	uint32_t size;
	uint32_t code;
} mbox_prop_buf_hdr_t;

typedef struct {
	uint32_t tag_id;
	uint32_t buf_size;
	uint32_t val_len;
} mbox_tag_hdr_t;

typedef struct {
	uint32_t base;
	uint32_t size;
} mbox_tag_getmem_resp_t;

typedef struct {
	mbox_prop_buf_hdr_t	buf_hdr;
	mbox_tag_hdr_t		tag_hdr;
	mbox_tag_getmem_resp_t	data;
	uint32_t		zero;
} mbox_getmem_buf_t;

typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t virt_width;
	uint32_t virt_height;
	uint32_t pitch;
	uint32_t bpp;
	uint32_t x_offset;
	uint32_t y_offset;
	uint32_t addr;
	uint32_t size;
} bcm2835_fb_desc_t;

typedef struct {
	mbox_prop_buf_hdr_t hdr;
	mbox_tag_hdr_t tag_hdr;
	union {
		struct {
			uint32_t id;
			uint32_t value;
		} req;

		struct {
			uint32_t id;
			uint32_t value;
		} resp;
	} body;
	uint32_t end_tag;
} mbox_msg_set_property_t;

typedef struct {
	mbox_prop_buf_hdr_t hdr;
	mbox_tag_hdr_t tag_hdr;
	union {
		struct {
			uint32_t id;
		} req;
		struct {
			uint32_t id;
			uint32_t value;
		} resp;
	};
	uint32_t end_tag;
} mbox_msg_get_property_t;

#endif

/**
 * @}
 */
