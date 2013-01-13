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

static void updateext(ASSET *asset);

/* Create a new asset */
ASSET *
asset_create()
{
	ASSET *p;

	p = (ASSET *) calloc(1, sizeof(ASSET));
	if(!p)
	{
		return NULL;
	}
	return p;
}

/* Free an asset */
int
asset_free(ASSET *asset)
{
	if(!asset)
	{
		return 0;
	}
	free(asset->path);
	free(asset->type);
	free(asset);
	return 0;
}

/* Reset an asset to its initial state */
int
asset_reset(ASSET *asset)
{	
	free(asset->path);
	free(asset->type);
	memset(asset, 0, sizeof(ASSET));
	return 0;
}

/* Set or reset the path of an asset */
int
asset_set_path(ASSET *asset, const char *path)
{
	char *p;

	if(path)
	{
		p = strdup(path);
		if(!p)
		{
			fprintf(stderr, "%s: failed to allocate memory for asset path\n", short_program_name);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		p = NULL;
	}
	free(asset->path);
	asset->path = p;
	fprintf(stderr, "%s: asset path is now %s\n", short_program_name, asset->path);
	updateext(asset);	
	return 0;
}

/* Set the path of an asset relative to a base directory */
int
asset_set_path_basedir(ASSET *asset, const char *basedir, size_t baselen, const char *path)
{
	char *p;

	if(!baselen)
	{
		baselen = strlen(basedir);
	}
	p = (char *) malloc(baselen + strlen(path) + 2);
	if(!p)
	{
		fprintf(stderr, "%s: failed to allocate memory for asset path\n", short_program_name);
		exit(EXIT_FAILURE);
	}
	strcpy(p, basedir);
	p[baselen] = '/';
	strcpy(&(p[baselen + 1]), path);
	free(asset->path);
	asset->path = p;
	fprintf(stderr, "%s: asset path is now %s\n", short_program_name, asset->path);
	updateext(asset);
	return 0;
}

int
asset_set_path_basedir_ext(ASSET *asset, const char *basedir, size_t baselen, const char *name, char *ext)
{
	char *p;
	size_t l;

	if(!baselen)
	{
		baselen = strlen(basedir);
	}
	l = strlen(name);
	p = (char *) malloc(baselen + l + strlen(ext) + 2);
	if(!p)
	{
		fprintf(stderr, "%s: failed to allocate memory for asset path\n", short_program_name);
		exit(EXIT_FAILURE);
	}
	strcpy(p, basedir);
	p[baselen] = '/';
	strcpy(&(p[baselen + 1]), name);
	strcpy(&(p[baselen + 1 + l]), ext);
	free(asset->path);
	asset->path = p;
	fprintf(stderr, "%s: asset path is now %s\n", short_program_name, asset->path);
	updateext(asset);
	return 0;	
}


/* Set or reset the MIME type of an asset */
int
asset_set_type(ASSET *asset, const char *type)
{
	char *p;

	if(type)
	{
		p = strdup(type);
		if(!p)
		{
			fprintf(stderr, "%s: failed to allocate memory for MIME type of %s\n", short_program_name, asset->path);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		p = NULL;
	}
	free(asset->type);
	asset->type = p;
	fprintf(stderr, "%s: MIME type of %s is %s\n", short_program_name, asset->path, asset->type);
	return 0;
}

int
asset_copy_attributes(ASSET *dest, const ASSET *src)
{
	asset_set_type(dest, src->type);
	dest->container = src->container;
	dest->sidecar = src->sidecar;
	return 0;
}

static void
updateext(ASSET *asset)
{
	char *t, *p;
	
	if(!asset->path)
	{
		asset->ext = NULL;
		return;
	}
	p = strrchr(asset->path, '/');
	t = strrchr(asset->path, '.');
	if(!t || (p && p > t))
	{
		asset->ext = strchr(asset->path, 0);
	}
	else
	{
		asset->ext = t;
	}	
}
