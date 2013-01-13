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

static dictionary *overrides;
static dictionary *config;

int
config_init(void)
{
	overrides = dictionary_new(0);
	if(!overrides)
	{
		return -1;
	}
	return 0;
}

int
config_load(void)
{
	int n;

	config = iniparser_load("spoold.conf");
	if(!config)
	{
		return -1;
	}
	for(n = 0; n < overrides->n; n++)
	{
		iniparser_set(config, overrides->key[n], overrides->val[n]);
	}
	dictionary_del(overrides);
	overrides = NULL;
	return 0;
}

int
config_set(const char *key, const char *value)
{
	iniparser_set((overrides ? overrides : config), key, value);
	return 0;
}

const char *
config_get(const char *key, const char *defval)
{
	return iniparser_getstring((config ? config : overrides), key, (char *) defval);
}

