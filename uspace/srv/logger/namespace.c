/*
 * Copyright (c) 2012 Vojtech Horky
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

/** @addtogroup logger
 * @{
 */
#include <assert.h>
#include <malloc.h>
#include <str.h>
#include <stdio.h>
#include "logger.h"

/** @file
 * Logging namespaces.
 */


struct logging_namespace {
	fibril_mutex_t guard;
	size_t writers_count;
	fibril_condvar_t reader_appeared_cv;
	bool has_reader;
	FILE *logfile;
	log_level_t logfile_level;
	const char *name;
	link_t link;
	prodcons_t messages;
};

static FIBRIL_MUTEX_INITIALIZE(namespace_list_guard);
static LIST_INITIALIZE(namespace_list);

log_message_t *message_create(const char *name, log_level_t level)
{
	log_message_t *message = malloc(sizeof(log_message_t));
	if (message == NULL)
		return NULL;

	message->message = str_dup(name);
	if (message->message == NULL) {
		free(message);
		return NULL;
	}

	message->level = level;
	link_initialize(&message->link);

	return message;
}

void message_destroy(log_message_t *message)
{
	assert(message);
	free(message->message);
	free(message);
}

static logging_namespace_t *namespace_find_no_lock(const char *name)
{
	list_foreach(namespace_list, it) {
		logging_namespace_t *namespace = list_get_instance(it, logging_namespace_t, link);
		if (str_cmp(namespace->name, name) == 0) {
			return namespace;
		}
	}

	return NULL;
}

static logging_namespace_t *namespace_create_no_lock(const char *name)
{
	logging_namespace_t *existing = namespace_find_no_lock(name);
	if (existing != NULL) {
		return NULL;
	}

	logging_namespace_t *namespace = malloc(sizeof(logging_namespace_t));
	if (namespace == NULL) {
		return NULL;
	}

	namespace->name = str_dup(name);
	if (namespace->name == NULL) {
		free(namespace);
		return NULL;
	}

	char *logfilename;
	int rc = asprintf(&logfilename, "/log/%s", name);
	if (rc < 0) {
		free(namespace->name);
		free(namespace);
		return NULL;
	}
	namespace->logfile = fopen(logfilename, "a");
	free(logfilename);
	if (namespace->logfile == NULL) {
		free(namespace->name);
		free(namespace);
		return NULL;
	}

	namespace->logfile_level = get_default_logging_level();

	fibril_mutex_initialize(&namespace->guard);
	fibril_condvar_initialize(&namespace->reader_appeared_cv);
	prodcons_initialize(&namespace->messages);
	namespace->has_reader = false;
	namespace->writers_count = 0;
	link_initialize(&namespace->link);

	list_append(&namespace->link, &namespace_list);

	return namespace;
}


logging_namespace_t *namespace_create(const char *name)
{
	fibril_mutex_lock(&namespace_list_guard);
	logging_namespace_t *result = namespace_create_no_lock(name);
	fibril_mutex_unlock(&namespace_list_guard);
	return result;
}

const char *namespace_get_name(logging_namespace_t *namespace)
{
	assert(namespace);
	return namespace->name;
}

static void namespace_destroy_careful(logging_namespace_t *namespace)
{
	assert(namespace);
	fibril_mutex_lock(&namespace_list_guard);

	fibril_mutex_lock(&namespace->guard);
	if (namespace->has_reader || (namespace->writers_count > 0)) {
		fibril_mutex_unlock(&namespace->guard);
		fibril_mutex_unlock(&namespace_list_guard);
		return;
	}

	list_remove(&namespace->link);

	fibril_mutex_unlock(&namespace->guard);
	fibril_mutex_unlock(&namespace_list_guard);

	// TODO - destroy pending messages
	fclose(namespace->logfile);
	free(namespace->name);
	free(namespace);
}

void namespace_destroy(logging_namespace_t *namespace)
{
	namespace_destroy_careful(namespace);
}

logging_namespace_t *namespace_reader_attach(const char *name)
{
	logging_namespace_t *namespace = NULL;

	fibril_mutex_lock(&namespace_list_guard);

	namespace = namespace_find_no_lock(name);

	if (namespace != NULL) {
		fibril_mutex_lock(&namespace->guard);
		namespace->has_reader = true;
		fibril_condvar_broadcast(&namespace->reader_appeared_cv);
		fibril_mutex_unlock(&namespace->guard);
	}

	fibril_mutex_unlock(&namespace_list_guard);

	return namespace;
}

logging_namespace_t *namespace_writer_attach(const char *name)
{
	logging_namespace_t *namespace = NULL;

	fibril_mutex_lock(&namespace_list_guard);

	namespace = namespace_find_no_lock(name);

	if (namespace == NULL) {
		namespace = namespace_create_no_lock(name);
	}

	fibril_mutex_lock(&namespace->guard);
	namespace->writers_count++;
	fibril_mutex_unlock(&namespace->guard);

	fibril_mutex_unlock(&namespace_list_guard);

	return namespace;
}

void namespace_reader_detach(logging_namespace_t *namespace)
{
	fibril_mutex_lock(&namespace->guard);
	namespace->has_reader = false;
	fibril_condvar_broadcast(&namespace->reader_appeared_cv);
	fibril_mutex_unlock(&namespace->guard);

	namespace_destroy_careful(namespace);
}

void namespace_writer_detach(logging_namespace_t *namespace)
{
	fibril_mutex_lock(&namespace->guard);
	assert(namespace->writers_count > 0);
	namespace->writers_count--;
	fibril_mutex_unlock(&namespace->guard);

	namespace_destroy_careful(namespace);
}

bool namespace_has_reader(logging_namespace_t *namespace, log_level_t level)
{
	fibril_mutex_lock(&namespace->guard);
	bool has_reader = namespace->has_reader
	    || level <= namespace->logfile_level;
	fibril_mutex_unlock(&namespace->guard);
	return has_reader;
}

void namespace_wait_for_reader_change(logging_namespace_t *namespace, bool *has_reader_now)
{
	fibril_mutex_lock(&namespace->guard);
	bool had_reader = namespace->has_reader;
	while (had_reader == namespace->has_reader) {
		fibril_condvar_wait(&namespace->reader_appeared_cv, &namespace->guard);
	}
	*has_reader_now = namespace->has_reader;
	fibril_mutex_unlock(&namespace->guard);
}


void namespace_add_message(logging_namespace_t *namespace, const char *message, log_level_t level)
{
	if (level <= get_default_logging_level()) {
		printf("[%s %d]: %s\n", namespace->name, level, message);
	}
	if (level <= namespace->logfile_level) {
		fprintf(namespace->logfile, "[%d]: %s\n", level, message);
		fflush(namespace->logfile);
	}

	fibril_mutex_lock(&namespace->guard);
	if (namespace->has_reader) {
		log_message_t *msg = message_create(message, level);
		if (msg != NULL) {
			prodcons_produce(&namespace->messages, &msg->link);
		}
	}
	fibril_mutex_unlock(&namespace->guard);
}

log_message_t *namespace_get_next_message(logging_namespace_t *namespace)
{
	link_t *message = prodcons_consume(&namespace->messages);

	return list_get_instance(message, log_message_t, link);
}


/**
 * @}
 */
