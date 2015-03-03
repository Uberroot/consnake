#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <stdlib.h>

typedef struct _MemPoolObject{
    void *data;
    struct _MemPoolObject *next;
} MemPoolObject;

typedef struct _MemPool{
    MemPoolObject *freeList;
} MemPool;

MemPool* MemPool_Create(unsigned int numObjs, size_t objSize);
MemPoolObject* MemPool_Alloc(MemPool *pool);
void MemPool_Free(MemPool *pool, MemPoolObject *obj);

#endif
