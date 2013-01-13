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

int
type_is_sidecar(const char *name)
{
	const char *t;

	t = strrchr(name, '.');
	if(!t)
	{
		return 0;
	}
	if(!strcmp(t, ".xmp"))
	{
		return 1;
	}
	if(!strcmp(t, ".xml"))
	{
		return 1;
	}
	return 0;
}

int
type_identify_job(JOB *job)
{
	IDENTIFY **list;
	size_t c;
	int r;

	list = plugin_identify_list();
	if(!list)
	{
		fprintf(stderr, "%s: failed to obtain set of identification mechanisms: %s\n", short_program_name, strerror(errno));
		exit(EXIT_FAILURE);
	}
	for(c = 0; list[c]; c++)
	{
		r = list[c]->api->identify(list[c], job);
		if(r < 0)
		{			
			return -1;
		}
		else if(r > 0)
		{
			return 0;
		}
	}
	fprintf(stderr, "%s: %s: unable to identify '%s'\n", short_program_name, job->name, job->path);
	return -1;
}
