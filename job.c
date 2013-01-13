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

/* Create a new job */
JOB *
job_create(const char *name, SOURCE *source)
{
	JOB *job;
	
	job = (JOB *) calloc(1, sizeof(JOB));
	if(!job)
	{
		return NULL;
	}
	job->source = source;
	job->name = strdup(name);
	if(!job->name)
	{
		free(job);
		return NULL;
	}
	fprintf(stderr, "%s: created new job '%s'\n", short_program_name, job->name);
	job->refcount = 1;
	return job;
}

int	
job_addref(JOB *job)
{
	job->refcount++;
	return job->refcount;
}

int
job_free(JOB *job)
{
	if(!job)
	{
		return 0;
	}
	job->refcount--;
	if(job->refcount)
	{
		return job->refcount;
	}
	free(job->name);
	free(job->path);
	free(job->sidecar);
	free(job->type);
	free(job);
	return 0;
}

/* Check whether a job is available for collection, and collect it if so */
JOB *
job_collect(void)
{
	SOURCE *src;

	src = plugin_source("file");
	if(!src)
	{
		fprintf(stderr, "%s: failed to locate a source: %s\n", short_program_name, strerror(errno));
		return NULL;
	}
	return src->api->collect(src);
}

/* Wait until a job is available for collection and then do so */
JOB *
job_collect_wait(void)
{
	JOB *job;
	int serr;

	serr = errno;
	for(;;)
	{
		errno = 0;
		job = job_collect();
		if(job)
		{
			break;
		}
		if(errno)
		{
			return NULL;
		}
		sleep(1);
	}
	errno = serr;
	return job;
}

/* Abort a job */
int
job_abort(JOB *job)
{
	fprintf(stderr, "%s: %s: aborting\n", short_program_name, job->name);
	job->aborted = 1;
	job->source->api->abort(job->source, job);
	return job_free(job);
}

/* Mark a job as having been submitted for processing */
int
job_submitted(JOB *job)
{
	if(!job->aborted)
	{
		fprintf(stderr, "%s: %s: job has been submitted for processing\n", short_program_name, job->name);
		job->submitted = 1;
	}
	return job_free(job);
}

/* Set or reset the MIME type of a job's asset */
int
job_set_type(JOB *job, const char *type)
{
	char *p;

	if(type)
	{
		p = strdup(type);
		if(!p)
		{
			fprintf(stderr, "%s: %s: failed to allocate memory for MIME type\n", short_program_name, job->name);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		p = NULL;
	}
	free(job->type);
	job->type = p;
	fprintf(stderr, "%s: %s: MIME type is %s\n", short_program_name, job->name, job->type);
	return 0;
}

