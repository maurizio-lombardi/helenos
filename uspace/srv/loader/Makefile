#
# Copyright (c) 2005 Martin Decky
# Copyright (c) 2007 Jakub Jermar
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# - Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# - The name of the author may not be used to endorse or promote products
#   derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

USPACE_PREFIX = ../..
ROOT_PATH = $(USPACE_PREFIX)/..
CONFIG_MAKEFILE = $(ROOT_PATH)/Makefile.config

include $(CONFIG_MAKEFILE)

ifeq ($(UARCH),ia64)
	# IA64 has a peculiar linker script with a fixed data segment address.
	# Because the loader is a separate nonrelocatable binary in the same
	# address space as the application, we provide a modified copy of
	# the default linker script to work around that.
	EXTRA_LDFLAGS = -T elf64_ia64_loader.x
else
	# On all other architectures, we can simply move the text segment.
	EXTRA_LDFLAGS = -Wl,-Ttext-segment=0x70000000
endif

BINARY = loader
STATIC_ONLY = y

GENERIC_SOURCES = \
	main.c

SOURCES = \
	$(GENERIC_SOURCES) \
	$(ARCH_SOURCES)

include $(USPACE_PREFIX)/Makefile.common
