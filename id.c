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

JOBID *
id_create_uuid(uuid_t uuid)
{
	JOBID *p;
	char *s;
	size_t c;
	
	p = (JOBID *) calloc(1, sizeof(JOBID));
	if(!p)
	{
		return NULL;
	}
	memcpy(p->uuid, uuid, sizeof(uuid_t));
	uuid_unparse_lower(p->uuid, p->formatted);
	s = p->canonical;
	for(c = 0; p->formatted[c]; c++)
	{
		if(!isxdigit(p->formatted[c]))
		{
			continue;
		}
		*s = p->formatted[c];
		s++;
	}
	*s = 0;
	return p;
}

int
id_free(JOBID *id)
{
	free(id);
	return 0;
}

int
id_assign(JOB *job)
{
	uuid_t uu;
	JOBID *p;

	uuid_generate(uu);
	p = id_create_uuid(uu);
	if(!p)
	{
		return -1;
	}
	job_set_id(job, p);
	fprintf(stderr, "%s: %s: assigned UUID is %s\n", short_program_name, job->name, job->id->formatted);
	return 0;
}

