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

#define HIERWIDTH                       3
#define HIERDEPTH                       4
#define STORAGE_STRUCT_DEFINED          1

#include "p_spool.h"

struct storage_struct
{
	/* Common members */
	STORAGE_API *api;
	
	/* Private data */
	char *path;
	size_t pathlen;
};

/* Storage API methods */
static ASSET *fs_create_container(STORAGE *me, JOB *job);
static ASSET *fs_copy_asset(STORAGE *me, JOB *dest, ASSET *asset);

/* Storage API */
static STORAGE_API fs_api = {
	fs_create_container,
	fs_copy_asset,
};

/* Utilities */
static int copy_file(const char *srcpath, const char *destpath);
static int open_file(const char *path, int opt, int mode);
static int close_file(int filedes);
static ssize_t read_file(int filedes, char *buf, ssize_t len);
static ssize_t write_file(int filedes, const char *buf, ssize_t len);

/* Create an instance of the storage mechanism */
STORAGE *
fs_create(void)
{
	STORAGE *p;
	const char *basepath;
	char *s;
	size_t l;

	basepath = "store";
	p = (STORAGE *) calloc(1, sizeof(STORAGE));
	if(!p)
	{
		return NULL;
	}
	p->api = &fs_api;
	/* Ensure the path length includes enough space for an asset base
	 * path.
	 */
	l = strlen(basepath) + ((HIERWIDTH + 1) * HIERDEPTH) + 32 + 4;
	s = (char *) calloc(1, l);
	if(!s)
	{
		free(p);
		return NULL;
	}
	strcpy(s, basepath);
	p->path = s;
	p->pathlen = strlen(basepath);
	return p;
}

/* Create the storage area for a job */
static ASSET *
fs_create_container(STORAGE *me, JOB *job)
{
	size_t c, max, pp, start, end;
	struct stat sbuf;
	ASSET *asset;
	int r;

	asset = asset_create();
	if(!asset)
	{
		return NULL;
	}
	asset->container = 1;
	/* Ensure the base path is reset back to its original value */
	me->path[me->pathlen] = 0;
	/* Construct a base path based upon HIERWIDTH and HIERDEPTH, derived
	 * from job->id. If HIERWIDTH was 3 and HIERDEPTH was 4, the result
	 * would be:
	 *
	 * path/AAA/BBB/CCC/DDD/AAABBBCCDDDEEE...
	 */	
	pp = me->pathlen;
	max = strlen(job->id->canonical);
	for(c = 0; c < HIERDEPTH; c++)
	{
		start = c * HIERWIDTH;
		if(start > max)
		{
			break;
		}
		end = start + HIERWIDTH;
		if(end > max)
		{
			end = max;
		}
		me->path[pp] = '/';
		pp++;
		memcpy(&(me->path[pp]), &(job->id->canonical[start]), end - start);
		pp += end - start;
		me->path[pp] = 0;
		r = stat(me->path, &sbuf);
		if(!r)
		{
			/* File exists */
			if(S_ISDIR(sbuf.st_mode))
			{
				/* It's a directory */
				continue;
			}
		}
		r = mkdir(me->path, 0777);
		if(r < 0)
		{
			fprintf(stderr, "%s: %s: %s\n", short_program_name, me->path, strerror(errno));
			asset_free(asset);
			return NULL;
		}
	}
	me->path[pp] = '/';
	pp++;
	strcpy(&(me->path[pp]), job->id->canonical);
	r = mkdir(me->path, 0777);
	if(r < 0)
	{
		fprintf(stderr, "%s: %s: %s\n", short_program_name, me->path, strerror(errno));
		asset_free(asset);
		return NULL;
	}
	r = asset_set_path(asset, me->path);
	if(r < 0)
	{
		asset_free(asset);
		return NULL;
	}
	me->path[me->pathlen] = 0;	
	return asset;
}

static ASSET *
fs_copy_asset(STORAGE *me, JOB *job, ASSET *asset)
{
	ASSET *dest;
	int r;

	(void) me;
	
	dest = asset_create();
	if(!dest)
	{
		return NULL;
	}
	asset_set_path_basedir_ext(dest, job->container->path, 0, job->id->canonical, asset->ext);
	asset_copy_attributes(dest, asset);
	fprintf(stderr, "%s: %s: copying '%s' to '%s'\n", short_program_name, job->name, asset->path, dest->path);
	/* Perform a file-copy operation */
	r = copy_file(asset->path, dest->path);
	if(r < 0)
	{
		asset_free(dest);
		return NULL;
	}
	return dest;
}

static int
copy_file(const char *srcpath, const char *destpath)
{
	static char *buf;
	static ssize_t bufsize;

	int sfd, dfd, e;
	ssize_t rlen, r;

	/* Use and re-use a static buffer */
	if(!buf)
	{
		/* Use a 4MB buffer */
		bufsize = (4 * 1024 * 1024);
		buf = malloc(bufsize);
		if(!buf)
		{
			return -1;
		}
	}
	sfd = open_file(srcpath, O_RDONLY, 0);
	if(sfd < 0)
	{
		return -1;
	}
	dfd = open_file(destpath, O_WRONLY|O_CREAT, 0666);
	if(dfd < 0)
	{
		e = errno;
		close_file(sfd);
		errno = e;
		return -1;
	}
	for(;;)
	{
		rlen = read_file(sfd, buf, bufsize);
		if(rlen == -1)
		{
			/* Read failed */
			close_file(sfd);
			close_file(dfd);
			return -1;
		}
		if(!rlen)
		{
			break;
		}
		r = write_file(sfd, buf, rlen);
		if(r == -1)
		{
			/* Write failed */
			close_file(sfd);
			close_file(dfd);
			return -1;
		}
	}
	close_file(sfd);
	close_file(dfd);
	return 0;
}

static int
open_file(const char *path, int opt, int mode)
{
	int fd;

	do
	{
		fd = open(path, opt, mode);
	}
	while(fd == -1 && errno == EINTR);
	return fd;
}

static int
close_file(int filedes)
{

	int r;

	do
	{
		r = close(filedes);
	}
	while(r == -1 && errno == EINTR);
	return r;
}

static ssize_t
read_file(int filedes, char *buf, ssize_t len)
{
	ssize_t r;

	do
	{
		r = read(filedes, buf, len);
	}
	while(r == -1 && errno == EINTR);
	return r;
}

static ssize_t
write_file(int filedes, const char *buf, ssize_t len)
{
	ssize_t r;

	while(len)
	{
		do
		{
			r = write(filedes, buf, len);
		}
		while(r == -1 && errno == EINTR);
		if(r == -1)
		{
			return -1;
		}
		if(r > len)
		{
			/* Nonsensical */
			errno = EINVAL;
			return -1;
		}
		len -= r;
		buf += r;
	}
	return 0;
}
