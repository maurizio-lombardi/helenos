/*
 * Copyright (c) 2010 Lenka Trochtova
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

/**
 * @defgroup libdrv generic device driver support.
 * @brief HelenOS generic device driver support.
 * @{
 */

/** @file
 */

#include <assert.h>

#include "dev_iface.h"
#include "remote_hw_res.h"
#include "remote_char_dev.h"
#include "remote_nic.h"
#include "remote_usb.h"
#include "remote_usbhc.h"
#include "remote_usbhid.h"
#include "remote_pci.h"
#include "remote_audio_mixer.h"
#include "remote_audio_pcm_buffer.h"

static const iface_dipatch_table_t remote_ifaces = {
	.ifaces = {
		[AUDIO_MIXER_IFACE] = &remote_audio_mixer_iface,
		[AUDIO_PCM_BUFFER_IFACE] = &remote_audio_pcm_buffer_iface,
		[HW_RES_DEV_IFACE] = &remote_hw_res_iface,
		[CHAR_DEV_IFACE] = &remote_char_dev_iface,
		[NIC_DEV_IFACE] = &remote_nic_iface,
		[PCI_DEV_IFACE] = &remote_pci_iface,
		[USB_DEV_IFACE] = &remote_usb_iface,
		[USBHC_DEV_IFACE] = &remote_usbhc_iface,
		[USBHID_DEV_IFACE] = &remote_usbhid_iface,
	}
};

remote_iface_t *get_remote_iface(int idx)
{
	assert(is_valid_iface_idx(idx));
	return remote_ifaces.ifaces[idx];
}

remote_iface_func_ptr_t
get_remote_method(remote_iface_t *rem_iface, sysarg_t iface_method_idx)
{
	if (iface_method_idx >= rem_iface->method_count)
		return NULL;
	
	return rem_iface->methods[iface_method_idx];
}

bool is_valid_iface_idx(int idx)
{
	return (0 <= idx) && (idx < DEV_IFACE_MAX);
}

/**
 * @}
 */
