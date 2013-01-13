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

static int should_terminate = 0;

const char *short_program_name = "spoold";

/* spoold runs continually, collecting jobs from one or more sources, and
 * then:--
 *
 * 1. Locate and load metadata associated with the asset. It might be inbound,
 *    or it might be contained within a sidecar. At a minimum, there must be
 *    a combination of 'kind' and 'key'.
 *
 * 2. Assign a UUID (possibly updating a database along the way)
 *
 * 3. Submit the job for processing by recipes which operate on the asset's
 *    type.
 */

int
main(int argc, char **argv)
{
	JOB *job;
	int r;

	(void) argc;
	(void) argv;

	r = config_init();
	if(r < 0)
	{
		fprintf(stderr, "%s: failed to intialise configuration: %s\n", short_program_name, strerror(errno));
		exit(EXIT_FAILURE);
	}
	/* Process arguments */
	r = config_load();
	if(r < 0)
	{
		fprintf(stderr, "%s: failed to load configuration: %s\n", short_program_name, strerror(errno));
		exit(EXIT_FAILURE);
	}
	r = plugin_load();
	if(r < 0)
	{
		fprintf(stderr, "%s: failed to initialise handlers: %s\n", short_program_name, strerror(errno));
		exit(EXIT_FAILURE);
	}
	while(!should_terminate)
	{
		job = job_collect_wait();
		if(!job)
		{
			fprintf(stderr, "%s: unexpected error while waiting for a job: %s\n", short_program_name, strerror(errno));
			exit(EXIT_FAILURE);
		}
		r = meta_locate(job);
		if(r < 0)
		{
			fprintf(stderr, "%s: %s: failed to locate metadata for job\n", short_program_name, job->name);
			job_abort(job);
			continue;
		}
		r = id_assign(job);
		if(r < 0)
		{
			fprintf(stderr, "%s: %s: failed to assign identifier for job\n", short_program_name, job->name);
			job_abort(job);
			continue;
		}
		r = process_job(job);
		if(r < 0)
		{
			fprintf(stderr, "%s: %s: failed to submit job for processing\n", short_program_name, job->name);
			job_abort(job);
			continue;
		}
		job_submitted(job);
	}
	return 0;
}
