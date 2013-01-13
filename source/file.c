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

#define SOURCE_STRUCT_DEFINED           1

#include "p_spool.h"

struct source_struct
{
	/* Common to all source instances */
	SOURCE_API *api;
	/* Our private data */
	char *incoming;
	size_t incominglen;
	char *aborted;
	size_t abortedlen;
};

/* Source API methods */
static JOB *file_collect(SOURCE *me);
static int file_abort(SOURCE *me, JOB *job);

/* Source API method table */
static SOURCE_API file_api = {
	file_collect,
	file_abort
};

/* Internal utilities */
static const char *basename(const char *filepath);

/* Construct a new source instance for the 'file' handler */
SOURCE *
file_create(void)
{
	SOURCE *p;

	p = (SOURCE *) calloc(1, sizeof(SOURCE));
	if(!p)
	{
		return NULL;
	}
	p->api = &file_api;
	p->incoming = strdup("incoming");
	if(!p->incoming)
	{
		free(p);
		return NULL;
	}
	p->incominglen = strlen(p->incoming);
	p->aborted = strdup("failed");
	if(!p->aborted)
	{
		free(p);
		return NULL;
	}
	p->abortedlen = strlen(p->aborted);
	return p;
}

static JOB *
file_collect(SOURCE *me)
{
	JOB *job;
	DIR *dir;
	struct dirent *de;
	const char *srcdir;
	size_t srclen;

	srcdir = me->incoming;
	srclen = me->incominglen;
	job = NULL;
	dir = opendir(srcdir);
	for(;;)
	{
		de = readdir(dir);
		if(!de)
		{
			break;
		}
		if(de->d_name[0] == '.')
		{
			continue;
		}
		if(type_is_sidecar(de->d_name))
		{
			continue;
		}
		job = job_create(de->d_name, me);
		if(!job)
		{
			closedir(dir);
			return NULL;
		}
		job->path = (char *) malloc(srclen + strlen(de->d_name) + 2);
		if(!job->path)
		{
			job_free(job);
			closedir(dir);
			return NULL;
		}
		strcpy(job->path, srcdir);
		job->path[srclen] = '/';
		strcpy(&(job->path[srclen + 1]), de->d_name);
		break;
	}
	closedir(dir);
	return job;
}

static int
file_abort(SOURCE *me, JOB *job)
{
	const char *destdir, *t, *st;
	char *fn;
	size_t destlen, l, sl;
	
	destdir = me->aborted;
	destlen = me->abortedlen;
	t = basename(job->path);
	l = strlen(t);
	if(job->sidecar)
	{
		st = basename(job->sidecar);
		sl = strlen(st);
		if(sl > l)
		{
			l = sl;
		}
	}
	else
	{
		st = NULL;
	}
	fn = (char *) malloc(destlen + l + 2);
	if(!fn)
	{
		fprintf(stderr, "%s: %s: %s\n", short_program_name, job->name, strerror(errno));
		exit(EXIT_FAILURE);
	}
	strcpy(fn, destdir);
	fn[destlen] = '/';
	strcpy(&(fn[destlen + 1]), t);
	fprintf(stderr, "%s: %s: moving '%s' to '%s'\n", short_program_name, job->name, job->path, fn);
	if(rename(job->path, fn))
	{
		fprintf(stderr, "%s: %s: failed to move '%s' to '%s': %s\n", short_program_name, job->name, job->path, fn, strerror(errno));
		free(fn);
		return -1;
	}
	free(fn);
	if(job->sidecar)
	{
		strcpy(fn, destdir);
		fn[destlen] = '/';
		strcpy(&(fn[destlen + 1]), st);
		fprintf(stderr, "%s: %s: moving '%s' to '%s'\n", short_program_name, job->name, job->sidecar, fn);
		if(rename(job->sidecar, fn))
		{
			fprintf(stderr, "%s: %s: failed to move '%s' to '%s': %s\n", short_program_name, job->name, job->sidecar, fn, strerror(errno));
			free(fn);
			return -1;
		}
		free(fn);
	}
	return 0;
}

static const char *
basename(const char *filepath)
{
	const char *t;
	
	t = strrchr(filepath, '/');
	if(t)
	{
		return t + 1;
	}
	return filepath;
}
