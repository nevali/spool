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

	(void) me;
	
	dest = asset_create();
	if(!dest)
	{
		return NULL;
	}
	asset_set_path_basedir_ext(dest, job->container->path, 0, job->id->canonical, asset->ext);
	asset_copy_attributes(dest, asset);
	fprintf(stderr, "%s: %s: copying '%s' to '%s'\n", short_program_name, job->name, asset->path, dest->path);
	return dest;
}

