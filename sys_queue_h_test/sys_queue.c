#include <unistd.h>
#include <sys/queue.h>
#include <pthread.h>
#include <string.h>

#if 0
/*check the difference of the following definitions:*/
struct test{
	struct hello p;
}a;
struct test{
	struct hello *p;
}a;
#endif
#if 0
/* ----------------*/
/*  LIST_HEAD(HEADNAME, TYPE); */
LIST_HEAD(storage_head, file_storage) gStroage;   ====>

struct storage_head 
{ 
    struct file_storage *lh_first; 
} gfs_slist;

/* ----------------*/
/*  LIST_ENTRY(TYPE)*/
LIST_ENTRY(struct file_storage) entry;   ==>

struct { 
    struct sruct file_storage *le_next; 
    struct sruct file_storage **le_prev; LSIT_INSERT_HEAD
} entry;

/* ---------------*/
/*       LIST_INSERT_HEAD(LIST_HEAD *head,
                       TYPE *elm, LIST_ENTRY NAME); */
LSIT_INSERT_HEAD(gfs_slist, fs, entry)   ==>
do 
{ 
    if (((fs)->entry.le_next = (gfs_slist)->lh_first) != ((void *)0)) 
        (gfs_slist)->lh_first->entry.le_prev = &(fs)->entry.le_next; 

    (gfs_slist)->lh_first = (fs); 
    (fs)->entry.le_prev = &(gfs_slist)->lh_first; 
} while ( 0);

#endif 
struct file_storage{
    const char *filename;
    pthread_mutex_t *mutex;
    SLIST_ENTRY(file_storage) entry;
};

SLIST_HEAD(storage_head, file_storage);

struct storage_head *gfs_slist = NULL;
pthread_mutex_t file_storage_mutex;
pthread_mutexattr_t file_storage_mutex_attr;


#define FILE_STORAGE_MUTEX_INIT() {                                                 \
    if (pthread_mutexattr_init(&file_storage_mutex_attr)) {                          \
        printf("\n %s %d: return fail!\n", __func__, __LINE__);                     \
        return -1;                                                                  \
    }                                                                               \
    if (pthread_mutexattr_settype(&file_storage_mutex_attr,                         \
            PTHREAD_MUTEX_RECURSIVE_NP)) {                                          \
        printf("\n %s %d: return fail!\n", __func__, __LINE__);                     \
        return -1;                                                                  \
    }                                                                               \
    if (pthread_mutex_init(&file_storage_mutex, &file_storage_mutex_attr)) {        \
        printf("\n %s %d: return fail!\n", __func__, __LINE__);                     \
    }                                                                               \
}

#define FILE_STORAGE_MUTEX_UNLINK() {                                               \
    pthread_mutexattr_destroy(&file_storage_mutex_attr);                            \
    pthread_mutex_destroy(&file_storage_mutex);                                     \
}
#define FILE_STORAGE_LOCK() {                                                       \
    pthread_mutex_lock(&file_storage_mutex);                                        \
}
#define FILE_STORAGE_UNLOCK() {                                                     \
    pthread_mutex_unlock(&file_storage_mutex);                                      \
}

int file_storage_register(const char *pathname, pthread_mutex_t *mutex)
{
    struct file_storage *fs = (struct file_storage *) malloc(sizeof(struct file_storage));
    
    if (!fs || !pathname /*|| !mutex*/) {
        printf("Failed to register file %s to data storage modules!\n", pathname);
        return -1;
    }

    fs->filename = (const char *) strdup(pathname);
    fs->mutex = mutex;
    printf("%s %d\n", __func__, __LINE__);
    FILE_STORAGE_LOCK();
    SLIST_INSERT_HEAD(gfs_slist, fs, entry);
    FILE_STORAGE_UNLOCK();
    printf("%s %d\n", __func__, __LINE__);
            printf("gfs_slist is %s\n", gfs_slist?"not null":"null");
}

int file_storage_unregister(const char *filename)
{
    struct file_storage *fs;
    
    if (!filename)
        return -1;
    FILE_STORAGE_LOCK();
    while (!SLIST_EMPTY(gfs_slist)) {
        fs = SLIST_FIRST(gfs_slist);
        if (strcmp(fs->filename, filename) == 0) {
            SLIST_REMOVE_HEAD(gfs_slist, entry);
            free(fs->filename);
            free(fs);
        }
    }
    FILE_STORAGE_UNLOCK();
}

int file_storage_unregister_all()
{
    struct file_storage *fs;

    FILE_STORAGE_LOCK();
    while (!SLIST_EMPTY(gfs_slist)) {
        fs = SLIST_FIRST(gfs_slist);
        SLIST_REMOVE_HEAD(gfs_slist, entry);
        free(fs->filename);
        free(fs);
    }
    FILE_STORAGE_UNLOCK();
}
#define LOCK(pthread_mutex)     pthread_mutex_lock(pthread_mutex)
#define UNLOCK(pthread_mutex)   pthread_mutex_unlock(pthread_mutex)

void file_save(const char *filename)
{
    printf("%s %d\n", __func__, __LINE__);
    if (access(filename, F_OK) == 0) {
        printf("tar -czvf %s.tar.gz %s ", filename, filename);
        printf("mv %s.tar.gz /storage/ ", filename);
    }

}
void *file_storage_task(void *arg)
{
    struct file_storage *fs;

    printf("%s %d\n", __func__, __LINE__);
    while(1) {
    printf("%s %d\n", __func__, __LINE__);
        sleep(1);
            printf("\n\n%s %d\n", __func__, __LINE__);

        FILE_STORAGE_LOCK();
    printf("%s %d\n", __func__, __LINE__);
        if (gfs_slist == NULL) 
            printf("gfs_slist is %s\n", gfs_slist?"not null":"null");
        SLIST_FOREACH(fs, gfs_slist, entry) {
            //LOCK(fs->mutex);
            file_save(fs->filename);
            //UNLOCK(fs->mutex);        
        }
        FILE_STORAGE_UNLOCK();
    }
    file_storage_unregister_all();
    free(gfs_slist);
    return NULL;
}

pthread_t tid_file_storage;

typedef void *(*FUNCPTR)(void *);
int file_storage_init(const char *dst_dir)
{
    int ret = 0;
    gfs_slist = (struct storage_head *) malloc(sizeof(struct storage_head));

    if (!gfs_slist)
        return NULL;
    SLIST_INIT(gfs_slist);

    FILE_STORAGE_MUTEX_INIT();
    if (pthread_create(&tid_file_storage, NULL, (FUNCPTR)file_storage_task, NULL) != 0) {
        printf("###Failed to create task for history data storage!\n");
        ret = -1;
    }
    return ret;
}
int main()
{
char *p = strdup("hello");
printf("p = %s\n", p?p:"null");

   file_storage_init("/tmp");
    file_storage_register("/home", NULL);
    sleep(1);
    file_storage_register("/home/alan", NULL);
    sleep(1);
    file_storage_register("/home/alan/tftpDir", NULL);
    sleep(1);
    file_storage_register("/home/alan/code", NULL);

    file_storage_unregister_all();
    if (SLIST_EMPTY(gfs_slist))
	    printf("list is empty\n");
    else 
    	printf("list is not empty\n");

    pthread_join(tid_file_storage, NULL);
    FILE_STORAGE_MUTEX_UNLINK();
    return 0;
}
