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
process_job(JOB *job)
{
	int r;

	fprintf(stderr, "%s: %s: processing job\n", short_program_name, job->name);
	r = job_begin(job);
	if(r < 0)
	{
		fprintf(stderr, "%s: %s: failed to prepare job for processing: %s\n", short_program_name, job->name, strerror(errno));
		return -1;
	}
	r = store_create_container(job);
	if(r < 0)
	{
		fprintf(stderr, "%s: %s: failed to create container for job: %s\n", short_program_name, job->name, strerror(errno));
		return -1;
	}
	r = store_copy_source(job);
	if(r < 0)
	{
		fprintf(stderr, "%s: %s: failed to copy source asset to storage: %s\n", short_program_name, job->name, strerror(errno));
	}
	/* Locate suitable recipes */
	/* Build the recipe dependency graph */
	/* Begin recipe submission and processing */
	return 0;
}

