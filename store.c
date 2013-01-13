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

/* Create storage for a job */
int
store_create_container(JOB *job)
{
	STORAGE *storage;
	ASSET *container;
	int r;

	/* For the moment, we'll assign the storage manually. This should
	 * be driven by defaults or something, though.
	 */
	storage = plugin_storage("file");
	if(!storage)
	{
		return -1;
	}
	job->storage = storage;
	container = job->storage->api->create_container(job->storage, job);
	if(!container)
	{
		return -1;
	}
	r = job_set_container(job, container);
	if(r < 0)
	{
		asset_free(container);
		return -1;
	}
	return 0;
}

/* Copy source assets to destination storage */
int
store_copy_source(JOB *job)
{
	ASSET *asset;

	asset = job->storage->api->copy_asset(job->storage, job, job->asset);
	if(!asset)
	{
		return -1;
	}
	job->stored = asset;
	if(job->sidecar)
	{
		asset = job->storage->api->copy_asset(job->storage, job, job->sidecar);
		if(!asset)
		{
			return -1;
		}
		job->stored_sidecar = asset;
	}
	return 0;
}

/*
int
store_create_job_recipe(JOB *job, RECIPE *recipe)
{

}
*/

