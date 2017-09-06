#ifndef __SEM_SYNC_H__
#define __SEM_SYNC_H__
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>

char SEM_NAME[]= "/a_task_is_free1";
//char SEM_B_TASK_IS_FREE[]= "b_task_is_free";
#if 0
    #ifndef _SEM_SYNC_H_
    #define _SEM_SYNC_H_
    #include <sys/types.h>
    #include <sys/ipc.h>
    #include <sys/shm.h>
    #include <stdio.h>
    #include <semaphore.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
      
    char SEM_A_TASK_IS_FREE[]= "a_task_is_free";
    char SEM_B_TASK_IS_FREE[]= "b_task_is_free";

    #endif
#endif

#endif /* __SEM_SYNC_H__ */

