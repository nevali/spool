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
# ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
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
typedef struct jobid_struct JOBID;
typedef struct source_struct SOURCE;
typedef struct source_api_struct SOURCE_API;
typedef struct identify_struct IDENTIFY;
typedef struct identify_api_struct IDENTIFY_API;
typedef struct storage_struct STORAGE;
typedef struct storage_api_struct STORAGE_API;

struct asset_struct
{
	char *path;
	char *ext;
	char *type;
	int container;
	int sidecar;
};

struct jobid_struct
{
	uuid_t uuid;
	uuid_string_t formatted;
	char canonical[33];
};

struct job_struct
{
	int refcount;
	char *name;
	JOBID *id;
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
	/* Storage container */
	ASSET *container;
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
	ASSET *(*create_container)(STORAGE *me, JOB *job);
	/* Copy an asset to destination storage, returning a new asset */
	ASSET *(*copy_asset)(STORAGE *me, JOB *dest, ASSET *asset);
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
int asset_set_path_basedir_ext(ASSET *asset, const char *basedir, size_t baselen, const char *name, char *ext);
int asset_copy_attributes(ASSET *dest, const ASSET *src);


JOB *job_create(const char *name, SOURCE *source);
int job_addref(JOB *job);
int job_free(JOB *job);
JOB *job_collect(void);
JOB *job_collect_wait(void);
int job_abort(JOB *job);
int job_begin(JOB *job);
int job_submitted(JOB *job);
int job_set_source_asset(JOB *job, ASSET *asset);
int job_set_container(JOB *job, ASSET *asset);
int job_set_id(JOB *job, JOBID *id);

int type_identify_asset(ASSET *asset);

int meta_locate(JOB *job);

JOBID *id_create_uuid(uuid_t uuid);
int id_free(JOBID *id);
int id_assign(JOB *job);

int process_job(JOB *job);

int store_create_container(JOB *job);
int store_copy_source(JOB *job);

/* Built-in sources */

SOURCE *file_create(void);

/* Built-in identification mechanisms */

IDENTIFY *ext_create(void);
IDENTIFY *sidecar_create(void);

/* Built-in storage */

STORAGE *fs_create(void);

#endif /*!P_SPOOL_H_*/
