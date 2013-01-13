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
	char *pending;
	size_t pendinglen;
	char *complete;
	size_t completelen;
};

/* Source API methods */
static JOB *file_collect(SOURCE *me);
static int file_begin(SOURCE *me, JOB *job);
static int file_abort(SOURCE *me, JOB *job);
static int file_complete(SOURCE *me, JOB *job);

/* Source API method table */
static SOURCE_API file_api = {
	file_collect,
	file_begin,
	file_abort,
	file_complete
};

/* Internal utilities */
static const char *basename(const char *filepath);
static int movetodest(JOB *job, const char *destdir, size_t destlen, int updatepaths);

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
	p->aborted = strdup("failed");
	p->pending = strdup("pending");
	p->complete = strdup("complete");
	if(!p->incoming || !p->aborted || !p->pending || !p->complete)
	{
		free(p->incoming);
		free(p->aborted);
		free(p->pending);
		free(p->complete);
		free(p);
		return NULL;
	}
	p->incominglen = strlen(p->incoming);
	p->abortedlen = strlen(p->aborted);
	p->pendinglen = strlen(p->pending);
	p->completelen = strlen(p->complete);
	return p;
}

/* Collect a job by scanning a source directory */
static JOB *
file_collect(SOURCE *me)
{
	JOB *job;
	ASSET *asset;
	DIR *dir;
	struct dirent *de;
	const char *srcdir;
	size_t srclen, sl, bl;
	int r;
	
	srcdir = me->incoming;
	srclen = me->incominglen;
	dir = opendir(srcdir);
	asset = NULL;
	job = NULL;
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
		if(asset)
		{
			asset_reset(asset);
		}
		else
		{
			asset = asset_create();
			if(!asset)
			{
				return NULL;
			}
		}
		asset_set_path_basedir(asset, srcdir, srclen, de->d_name);
		r = type_identify_asset(asset);
		if(r < 0)
		{
			fprintf(stderr, "%s: failed to identify asset '%s': %s\n", short_program_name, de->d_name, strerror(errno));
			asset_free(asset);
			closedir(dir);
			return NULL;
		}
		if(r == 0)
		{
			continue;
		}
		if(asset->sidecar)
		{
			continue;
		}
		job = job_create(de->d_name, me);
		if(!job)
		{
			asset_free(asset);
			closedir(dir);
			return NULL;
		}
		job_set_source_asset(job, asset);
		/* Look for a matching sidecar */
		asset = asset_create();
		if(!asset)
		{
			return NULL;
		}
		rewinddir(dir);
		sl = strlen(job->asset->basename);
		bl = sl - strlen(job->asset->ext);
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
			if(strlen(de->d_name) > sl &&
			   !strncmp(de->d_name, job->asset->basename, sl) &&
			   de->d_name[sl] == '.')
			{
				asset_reset(asset);
				/* Candidate filename begins with the asset filename */
				asset_set_path_basedir(asset, srcdir, srclen, de->d_name);
			}
			else if(strlen(de->d_name) > bl &&
					!strncmp(de->d_name, job->asset->basename, bl) &&
					strcmp(de->d_name, job->asset->basename) &&
					de->d_name[bl] == '.')
			{
				asset_reset(asset);
				/* Candidate filename begins with the asset filename */
				asset_set_path_basedir(asset, srcdir, srclen, de->d_name);
			}
			else
			{
				continue;
			}
			r = type_identify_asset(asset);
			if(r < 0)
			{
				fprintf(stderr, "%s: failed to identify asset '%s': %s\n", short_program_name, de->d_name, strerror(errno));
				asset_free(asset);
				closedir(dir);
				return NULL;
			}
			if(r == 0)
			{
				continue;
			}
			if(asset->sidecar)
			{
				job_set_sidecar(job, asset);
				asset = NULL;
				break;
			}
		}
		break;
	}
	closedir(dir);
	asset_free(asset);
	return job;
}

/* Abort a job */
static int
file_abort(SOURCE *me, JOB *job)
{
	return movetodest(job, me->aborted, me->abortedlen, 0);
}

/* Prepare a job for processing */
static int
file_begin(SOURCE *me, JOB *job)
{
	return movetodest(job, me->pending, me->pendinglen, 1);
}

/* A job has finished processing */
static int
file_complete(SOURCE *me, JOB *job)
{
	return movetodest(job, me->complete, me->completelen, 0);
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

static int
movetodest(JOB *job, const char *destdir, size_t destlen, int updatepaths)
{
	const char *t, *st;
	char *fn;
	size_t l, sl;

	t = basename(job->asset->path);
	l = strlen(t);
	if(job->sidecar)
	{
		st = basename(job->sidecar->path);
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
	fprintf(stderr, "%s: %s: moving '%s' to '%s'\n", short_program_name, job->name, job->asset->path, fn);
	if(rename(job->asset->path, fn))
	{
		fprintf(stderr, "%s: %s: failed to move '%s' to '%s': %s\n", short_program_name, job->name, job->asset->path, fn, strerror(errno));
		free(fn);
		return -1;
	}
	if(updatepaths)
	{
		asset_set_path(job->asset, fn);
	}
/*	if(job->sidecar)
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
		if(updatepaths)
		{
			job_set_sidecar(job, fn);
		}
		} */
	free(fn);
	return 0;
}
