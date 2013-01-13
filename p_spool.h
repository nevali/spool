#ifndef P_SPOOL_H_
# define P_SPOOL_H_

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <errno.h>
# include <ctype.h>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
# ifdef HAVE_DIRENT_H
#  include <dirent.h>
# endif

# if defined(HAVE_UUID_UUID_H)
#  include <uuid/uuid.h>
# elif defined(HAVE_UUID_H)
#  include <uuid.h>
# endif

# ifndef EXIT_FAILURE
#  define EXIT_FAILURE                  1
# endif

typedef struct asset_struct ASSET;
typedef struct job_struct JOB;
typedef struct source_struct SOURCE;
typedef struct source_api_struct SOURCE_API;
typedef struct identify_struct IDENTIFY;
typedef struct identify_api_struct IDENTIFY_API;
typedef struct storage_struct STORAGE;
typedef struct storage_api_struct STORAGE_API;

struct asset_struct
{
	char *path;
	char *type;
	int container;
	int sidecar;
};

struct job_struct
{
	int refcount;
	char *name;
	uuid_t uuid;
	int aborted;
	int submitted;
	int completed;
	/* Source handler */
	SOURCE *source;
	/* Storage handler */
	STORAGE *storage;
	/* Source asset */
	ASSET *asset;
	/* Sidecar */
	ASSET *sidecar;
	/* Stored asset */
	ASSET *stored;
	/* Stored sidecar */
	ASSET *stored_sidecar;
};

struct source_api_struct
{
	/* Collect a job from the source */
	JOB *(*collect)(SOURCE *me);
	/* A job has begun processing (and is now 'pending') */
	int (*begin)(SOURCE *me, JOB *job);
	/* A job has been aborted */
	int (*abort)(SOURCE *me, JOB *job);
	/* A job has been completed */
	int (*complete)(SOURCE *me, JOB *job);
};

# ifndef SOURCE_STRUCT_DEFINED
struct source_struct
{
	SOURCE_API *api;
};
# endif

struct identify_api_struct
{
	/* Identify an asset */
	int (*identify)(IDENTIFY *me, ASSET *asset);
};

# ifndef IDENTIFY_STRUCT_DEFINED
struct identify_struct
{
	IDENTIFY_API *api;
};
# endif

struct storage_api_struct
{
	/* Create storage for a job */
	int (*create_job)(STORAGE *me, JOB *job);
	/* Copy a job's assets from source to destination storage */
	int (*copy_source)(STORAGE *me, JOB *job);
};

# ifndef STORAGE_STRUCT_DEFINED
struct storage_struct
{
	STORAGE_API *api;
};
# endif

extern const char *short_program_name;

int plugin_load(void);
SOURCE *plugin_source(const char *name);
IDENTIFY **plugin_identify_list(void);
STORAGE *plugin_storage(const char *name);

ASSET *asset_create(void);
int asset_free(ASSET *asset);
int asset_reset(ASSET *asset);
int asset_set_type(ASSET *asset, const char *type);
int asset_set_path(ASSET *asset, const char *path);
int asset_set_path_basedir(ASSET *asset, const char *basedir, size_t baselen, const char *path);

JOB *job_create(const char *name, SOURCE *source);
int job_addref(JOB *job);
int job_free(JOB *job);
JOB *job_collect(void);
JOB *job_collect_wait(void);
int job_abort(JOB *job);
int job_begin(JOB *job);
int job_submitted(JOB *job);
int job_set_source_asset(JOB *job, ASSET *asset);

int type_identify_asset(ASSET *asset);

int meta_locate(JOB *job);

int id_assign(JOB *job);

int process_job(JOB *job);

int store_create_job(JOB *job);
int store_copy_source(JOB *job);

/* Built-in sources */

SOURCE *file_create(void);

/* Built-in identifiers */

IDENTIFY *ext_create(void);
IDENTIFY *sidecar_create(void);

/* Built-in storage */

STORAGE *fs_create(void);

#endif /*!P_SPOOL_H_*/
