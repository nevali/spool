/*
 * Copyright 2013 Mo McRoberts.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_spool.h"

static SOURCE *file_source;
static STORAGE *fs_storage;
static IDENTIFY *identify_plugins[3];

int
plugin_load(void)
{
	file_source = file_create();
	if(!file_source)
	{
		fprintf(stderr, "%s: failed to construct 'file' source: %s\n", short_program_name, strerror(errno));
		return -1;
	}
	identify_plugins[0] = ext_create();
	if(!identify_plugins[0])
	{
		fprintf(stderr, "%s: failed to construct 'ext' identification mechanism: %s\n", short_program_name, strerror(errno));
		return -1;
	}
	identify_plugins[1] = sidecar_create();
	if(!identify_plugins[0])
	{
		fprintf(stderr, "%s: failed to construct 'ext' identification mechanism: %s\n", short_program_name, strerror(errno));
		return -1;
	}
	return 0;
}

SOURCE *
plugin_source(const char *scheme)
{
	if(!strcmp(scheme, "file"))
	{
		return file_source;
	}
	errno = ENOENT;
	return NULL;
}

IDENTIFY **
plugin_identify_list(void)
{
	return identify_plugins;
}

STORAGE *
plugin_storage(const char *scheme)
{
	if(!strcmp(scheme, "file"))
	{
		return fs_storage;
	}
	errno = ENOENT;
	return NULL;
}

