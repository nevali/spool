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

/* Based on the MIME type of an asset, determine whether it's a metadata
 * sidecar or not.
 */

struct identify_struct
{
	IDENTIFY_API *api;
};

/* Identification API methods */
static int sidecar_identify(IDENTIFY *me, ASSET *asset);

/* Identification API */
static IDENTIFY_API sidecar_api = {
	sidecar_identify
};

/* Construct a new handler instance */
IDENTIFY *
sidecar_create(void)
{
	IDENTIFY *p;

	p = (IDENTIFY *) calloc(1, sizeof(IDENTIFY));
	if(!p)
	{
		return NULL;
	}
	p->api = &sidecar_api;
	return p;
}

static int
sidecar_identify(IDENTIFY *me, ASSET *asset)
{
	if(!asset->type)
	{
		/* Not identified */
		return 0;
	}
	if(!strcmp(asset->type, "application/xml"))
	{
		asset->sidecar = 1;
		return 1;
	}
	return 0;
}
