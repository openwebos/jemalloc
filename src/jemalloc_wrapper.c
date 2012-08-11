/*-
 * Copyright (c) 2008-2012 Hewlett-Packard Development Company, L.P.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************/

#define _GNU_SOURCE 1
#include <dlfcn.h>

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>

#include "rb.h"

#if defined(HEAP_TRACKING)

// -------------------------------------------------------------------------------------------
#undef assert
#define assert(x) \
    if (!(x)) {   \
    int* assertPtr = 0;                         \
    *assertPtr = 0;                             \
    }


// -------------------------------------------------------------------------------------------

extern void* je_malloc_real(size_t size);
extern void  je_free_real(void* ptr);
extern void* je_realloc_real(void* ptr, size_t size);
extern void* je_memalign_real(size_t boundary, size_t size);
extern void* je_valloc_real(size_t size);
extern void* je_calloc_real(size_t num, size_t size);
extern int   je_posix_memalign_real(void **memptr, size_t alignment, size_t size);

// -------------------------------------------------------------------------------------------

void* malloc(size_t size) __attribute__ ((weak, alias ("je_malloc")));
void  free(void* ptr) __attribute__ ((weak, alias ("je_free")));
void* realloc(void* ptr, size_t size) __attribute__ ((weak, alias ("je_realloc")));
void* memalign(size_t boundary, size_t size) __attribute__ ((weak, alias ("je_memalign")));
void* calloc(size_t num, size_t size) __attribute__ ((weak, alias("je_calloc")));
int   posix_memalign(void **memptr, size_t alignment, size_t size) __attribute__ ((weak, alias("je_posix_memalign")));

// -------------------------------------------------------------------------------------------

static void
je_wrtmessage(int fd, const char *p1)
{
	write(fd, p1, strlen(p1));
}

static void
je_print(int fd, const char *format, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

    je_wrtmessage(fd, buf);
}

// -------------------------------------------------------------------------------------------

#define BTLEN 1

typedef struct AllocRecordStruct AllocRecord;
struct AllocRecordStruct
{
    rb_node(AllocRecord) link;
    void* addr;
    void* retAddr;
    uint32_t size;
};

typedef rb_tree(AllocRecord) Allocs;
static Allocs s_allocs;

static inline int
allocCacheCompare(AllocRecord* a, AllocRecord* b)
{
    if (a->addr < b->addr)
        return -1;
    else if (a->addr > b->addr)
        return 1;
    else
        return 0;
}

rb_wrap(static, alloc_record_, Allocs, AllocRecord,
        link, allocCacheCompare);

// -------------------------------------------------------------------------------------------

static pthread_once_t  s_threadInit = PTHREAD_ONCE_INIT;
static pthread_mutex_t s_mutex;

static void je_threadInit()
{
    alloc_record_new(&s_allocs);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    //pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&s_mutex, &attr);
}

static void je_nukeEntry(void* p)
{
    AllocRecord *r, key;
    key.addr = p;

    r = alloc_record_search(&s_allocs, &key);
    if (r)
        alloc_record_remove(&s_allocs, r);
}

/*
static inline __attribute__((always_inline))
//static __attribute__((noinline))
void je_backtrace(AllocRecord* r)
{
    int i = 0;
    for (i = 0; i < BTLEN; i++)
        r->backtrace[i] = 0;

    void* fp = __builtin_frame_address(0);
    if (fp > 0) {
        for (i = 0; i < BTLEN; i++) {
            r->backtrace[i] = (void*) (*((int*)fp));
            fp = (void*) (*(int*)(fp - 4));
            je_print(STDERR_FILENO, "%d %p %p\n", i, fp, __builtin_return_address(0));
            if (fp <= 0)
                break;
        }
    }
}
*/
// -------------------------------------------------------------------------------------------

void* je_malloc(size_t size)
{
    pthread_once(&s_threadInit, je_threadInit);

    pthread_mutex_lock(&s_mutex);
    void* p = je_malloc_real(size);

    //je_threadSpecificPrint("%s: %d, %p\n", __FUNCTION__, size, p);
    if (p != NULL) {

        AllocRecord* r = je_malloc_real(sizeof(AllocRecord));
        r->addr = p;
        r->size = size;
        r->retAddr = __builtin_frame_address(0) ? __builtin_return_address(0) : 0;
        //je_backtrace(r);

        //je_nukeEntry(p);
        alloc_record_insert(&s_allocs, r);
    }
    pthread_mutex_unlock(&s_mutex);

    return p;
}

void  je_free(void* ptr)
{
    pthread_once(&s_threadInit, je_threadInit);

    if (ptr == NULL)
        return;

    pthread_mutex_lock(&s_mutex);
    AllocRecord *r, key;
    key.addr = ptr;

    r = alloc_record_search(&s_allocs, &key);
    assert(r != NULL);
    assert(r->addr == ptr);
    alloc_record_remove(&s_allocs, r);
    je_free_real(r);

    //je_threadSpecificPrint("%s: %p\n", __FUNCTION__, ptr);

    je_free_real(ptr);
    pthread_mutex_unlock(&s_mutex);
}

void* je_realloc(void* ptr, size_t size)
{
    pthread_once(&s_threadInit, je_threadInit);

    if (ptr == NULL)
        return je_malloc(size);

    pthread_mutex_lock(&s_mutex);
    void* p = je_realloc_real(ptr, size);

    AllocRecord *r, key;
    key.addr = ptr;

    r = alloc_record_search(&s_allocs, &key);
    assert(r != NULL);
    assert(r->addr == ptr);

    alloc_record_remove(&s_allocs, r);
    if (p != NULL) {
        r->addr = p;
        r->size = size;
        r->retAddr = __builtin_frame_address(0) ? __builtin_return_address(0) : 0;
        //je_backtrace(r);

        alloc_record_insert(&s_allocs, r);
    }
    else
        je_free_real(r);

    pthread_mutex_unlock(&s_mutex);

    //je_threadSpecificPrint("%s: %p %p\n", __FUNCTION__, p, ptr);

    return p;
}

void* je_memalign(size_t boundary, size_t size)
{
    pthread_once(&s_threadInit, je_threadInit);

    pthread_mutex_lock(&s_mutex);
    void* p = je_memalign_real(boundary, size);

    if (p != NULL) {
        AllocRecord* r = je_malloc_real(sizeof(AllocRecord));
        r->addr = p;
        r->size = size;
        r->retAddr = __builtin_frame_address(0) ? __builtin_return_address(0) : 0;
        //je_backtrace(r);

        je_nukeEntry(p);
        alloc_record_insert(&s_allocs, r);
    }

    pthread_mutex_unlock(&s_mutex);

    //je_threadSpecificPrint("%s: %p\n", __FUNCTION__, p);

    return p;
}

void* je_valloc(size_t size)
{
    pthread_once(&s_threadInit, je_threadInit);

    pthread_mutex_lock(&s_mutex);
    void* p = je_valloc_real(size);

    if (p != NULL) {
        AllocRecord* r = je_malloc_real(sizeof(AllocRecord));
        r->addr = p;
        r->size = size;
        r->retAddr = __builtin_frame_address(0) ? __builtin_return_address(0) : 0;
        //je_backtrace(r);

        je_nukeEntry(p);
        alloc_record_insert(&s_allocs, r);
    }

    pthread_mutex_unlock(&s_mutex);

    //je_threadSpecificPrint("%s: %p\n", __FUNCTION__, p);

    return p;
}

void* je_calloc(size_t num, size_t size)
{
    pthread_once(&s_threadInit, je_threadInit);

    if (size <= 0)
        return 0;

    pthread_mutex_lock(&s_mutex);
    void* p = je_calloc_real(num, size);

    if (p != NULL) {
        AllocRecord* r = je_malloc_real(sizeof(AllocRecord));
        r->addr = p;
        r->size = size;
        r->retAddr = __builtin_frame_address(0) ? __builtin_return_address(0) : 0;
        //je_backtrace(r);

        je_nukeEntry(p);
        alloc_record_insert(&s_allocs, r);
    }
    pthread_mutex_unlock(&s_mutex);

    //je_threadSpecificPrint("%s: %p\n", __FUNCTION__, p);

    return p;
}

int   je_posix_memalign(void **memptr, size_t alignment, size_t size)
{
    pthread_once(&s_threadInit, je_threadInit);

    if (size <= 0) {
        *memptr = 0;
        return 0;
    }

    pthread_mutex_lock(&s_mutex);
    int res = je_posix_memalign_real(memptr, alignment, size);

    if (*memptr != NULL) {
        AllocRecord* r = je_malloc_real(sizeof(AllocRecord));
        r->addr = *memptr;
        r->size = size;
        r->retAddr = __builtin_frame_address(0) ? __builtin_return_address(0) : 0;
        //je_backtrace(r);

        je_nukeEntry(*memptr);
        alloc_record_insert(&s_allocs, r);
    }
    pthread_mutex_unlock(&s_mutex);

    //je_threadSpecificPrint("%s: %p\n", __FUNCTION__, *memptr);

    return res;
}

// -------------------------------------------------------------------------------------------

void je_dumpHeap(const char* filePath)
{
    int fd = open(filePath, O_RDWR |  O_CREAT | O_TRUNC, 0644);
    Dl_info dlinfo;

    pthread_mutex_lock(&s_mutex);

    AllocRecord* r = alloc_record_first(&s_allocs);
    while (r != NULL) {

        const char* function = NULL;
        const char* object = NULL;
        if (r->retAddr && (dladdr(r->retAddr, &dlinfo) != 0)) {
            function = dlinfo.dli_sname;
            object = dlinfo.dli_fname;
        }

        je_print(fd, "%p %d %p %s %s\n",
                 r->addr, r->size, r->retAddr,
                 function ? function : "(unknown)",
                 object ? object : "(unknown)");

        r = alloc_record_next(&s_allocs, r);
    }

    pthread_mutex_unlock(&s_mutex);

    close(fd);
}

#else /* HEAP_TRACKING */

void je_dumpHeap(const char* filePath)
{
    printf("je_malloc built without HEAP_TRACKING\n");
}

#endif /* HEAP_TRACKING */
