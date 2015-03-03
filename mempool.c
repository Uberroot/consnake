#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "mempool.h"

MemPool* MemPool_Create(unsigned int numObjs, size_t objSize)
{
    //Layout:
    //[ --- pool --- MemPoolObject, MemPoolObject, MemPoolObject... --- data, data, data... --- ]

    //Build the structures
    MemPool *pool = (MemPool*)malloc(sizeof(MemPool) + numObjs * sizeof(MemPoolObject) + numObjs * objSize);
    assert(pool);

    //Init the pool
    //memset((void*)pool + sizeof(MemPool), 0, 2 * sizeof(MemPoolObject));
    pool->freeList = (void*)pool + sizeof(MemPool) + sizeof(MemPoolObject);

    //Divide the data
    void *metapos = (void*)pool + sizeof(MemPool);
    void *pos = (void*)pool + sizeof(MemPool) + numObjs * sizeof(MemPoolObject);
    int i;

    MemPoolObject *node = pool->freeList;
    for(i = 0; i < numObjs; i++)
    {
        node->data = pos;
        node->next = (MemPoolObject*)metapos;
        node = node->next;
        pos += objSize;
        metapos += sizeof(MemPoolObject);
    }
    node->next = NULL;

    return pool;
}

MemPoolObject* MemPool_Alloc(MemPool *pool)
{
    MemPoolObject *ret = pool->freeList;
    pool->freeList = pool->freeList->next;
    return ret;
}

void MemPool_Free(MemPool *pool, MemPoolObject *obj)
{
    obj->next = pool->freeList;
    pool->freeList = obj;
}
