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

#define IDENTIFY_STRUCT_DEFINED         1

#include "p_spool.h"

/* Identify assets by mapping extensions to MIME types using a
 * mime.types file, as distributed by ASF at:-
 *
 * http://svn.apache.org/repos/asf/httpd/httpd/trunk/docs/conf/mime.types
 */

struct identify_struct
{
	IDENTIFY_API *api;
	size_t ntypes;
	size_t nalloc;
	struct mimetype *types;
};

struct mimetype
{
	char *type;
	char *exts[16];
	size_t nexts;
};

/* Identification API methods */
static int ext_identify(IDENTIFY *me, ASSET *asset);

/* Identification API */
static IDENTIFY_API ext_api = {
	ext_identify
};

/* Internal utilities */
static int parseline(IDENTIFY *me, const char *line);
static int addtype(IDENTIFY *me, const struct mimetype *type);

/* Construct a new handler instance */
IDENTIFY *
ext_create(void)
{
	IDENTIFY *p;
	FILE *f;
	char *buf;
	size_t buflen;

	p = (IDENTIFY *) calloc(1, sizeof(IDENTIFY));
	if(!p)
	{
		return NULL;
	}
	buflen = 4096;
	buf = (char *) malloc(buflen);
	if(!buf)
	{
		free(p);
		return NULL;
	}
	p->api = &ext_api;
	f = fopen("mime.types", "r");
	if(!f)
	{
		fprintf(stderr, "%s: unable to open 'mime.types' for reading: %s\n", short_program_name, strerror(errno));
		free(p);
		free(buf);
		return NULL;
	}
	buf[buflen - 1] = 0;
	while(fgets(buf, buflen - 1, f))
	{
		parseline(p, buf);
	}
	fclose(f);
	free(buf);
	return p;
}

static int
ext_identify(IDENTIFY *me, ASSET *asset)
{
	size_t c, d;
	const char *t;

	if(asset->type)
	{
		/* Already identified */
		return 0;
	}
	if(!asset->ext || !asset->ext[0])
	{
		/* No file extension */
		return 0;
	}
	t = asset->ext;
	t++;
	for(c = 0; c < me->ntypes; c++)
	{
		for(d = 0; d < me->types[c].nexts; d++)
		{
			if(!strcmp(t, me->types[c].exts[d]))
			{
				asset_set_type(asset, me->types[c].type);
				return 1;
			}
		}
	}
	return 0;
}

static int
parseline(IDENTIFY *me, const char *line)
{
	struct mimetype t;
	char *buf, *p;

	while(isspace(*line))
	{
		line++;
	}
	if(!*line || *line == '#')
	{
		return 0;
	}
	buf = strdup(line);
	if(!buf)
	{
		return -1;
	}
	t.type = buf;
	t.nexts = 0;
	for(p = buf; *p; p++)
	{
		if(isspace(*p))
		{
			break;
		}
	}
	if(!*p)
	{
		free(buf);
		return 0;
	}
	*p = 0;
	p++;
	while(isspace(*p))
	{
		p++;
	}
	if(!*p)
	{
		free(buf);
		return 0;
	}
	while(*p)
	{
		t.exts[t.nexts] = p;
		t.nexts++;
		while(!isspace(*p))
		{
			p++;
		}
		if(*p)
		{
			*p = 0;
			p++;
		}
		while(isspace(*p))
		{
			p++;
		}
	}
	return addtype(me, &t);
}

static int
addtype(IDENTIFY *me, const struct mimetype *type)
{
	struct mimetype *n;

	if(me->nalloc < me->ntypes + 1)
	{
		n = realloc(me->types, (me->nalloc + 32) * sizeof(struct mimetype));
		if(!n)
		{
			return -1;
		}
		me->nalloc += 32;
		me->types = n;
	}
	memcpy(&(me->types[me->ntypes]), type, sizeof(struct mimetype));
	me->ntypes++;
	return 0;
}
