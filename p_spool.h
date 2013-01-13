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

typedef struct job_struct JOB;
typedef struct source_struct SOURCE;
typedef struct source_api_struct SOURCE_API;
typedef struct identify_struct IDENTIFY;
typedef struct identify_api_struct IDENTIFY_API;
typedef struct storage_struct STORAGE;
typedef struct storage_api_struct STORAGE_API;

struct job_struct
{
	int refcount;
	char *name;
	SOURCE *source;
	char *path;
	char *sidecar;
	char *type;	
	uuid_t uuid;
	int aborted;
	int submitted;
	int completed;
};

struct source_api_struct
{
	JOB *(*collect)(SOURCE *me);
	int (*abort)(SOURCE *me, JOB *job);
};

# ifndef SOURCE_STRUCT_DEFINED
struct source_struct
{
	SOURCE_API *api;
};
# endif

struct identify_api_struct
{
	int (*identify)(IDENTIFY *me, JOB *job);
};

# ifndef IDENTIFY_STRUCT_DEFINED
struct identify_struct
{
	IDENTIFY_API *api;
};
# endif

struct storage_api_struct
{
	int (*create_job)(STORAGE *me, JOB *job);
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

JOB *job_create(const char *name, SOURCE *source);
int job_addref(JOB *job);
int job_free(JOB *job);
JOB *job_collect(void);
JOB *job_collect_wait(void);
int job_abort(JOB *job);
int job_submitted(JOB *job);
int job_set_type(JOB *job, const char *type);

int type_identify_job(JOB *job);
int type_is_sidecar(const char *filename);

int meta_locate(JOB *job);

int id_assign(JOB *job);

int process_job(JOB *job);

int store_create_job(JOB *job);
int store_copy_source(JOB *job);

/* Built-in sources */

SOURCE *file_create(void);

/* Built-in identifiers */

IDENTIFY *ext_create(void);

/* Built-in storage */

STORAGE *fs_create(void);

#endif /*!P_SPOOL_H_*/
