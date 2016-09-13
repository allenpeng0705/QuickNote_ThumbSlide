#define MODULE_NAME "q_malloc.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "q_malloc.h"
#include "q_logger.h"

#ifdef MEMORY_TRACK

#define MAX_TRACK_CHAR_COUNT    128

typedef struct memtrack_node
{
    uint32                size;
    uint8                 align;
    void*                 pAddress;
    char                  szFunction[MAX_TRACK_CHAR_COUNT];
    char                  szFilename[MAX_TRACK_CHAR_COUNT];
    uint32                line;

    struct memtrack_node* pNext;
} memtrack_node_t;


memtrack_node_t* g_pMemoryTrackingNodes = NULL;


void* MALLOC_ALIGNED_TRACKED( uint32 align, uint32 size,char const *pSubcategoryName, char const *pSourceFileName, int sourceLine)
{
    memtrack_node_t* newTrackNode = NULL;
    memtrack_node_t* tempNode     = NULL;

    void* pRetAddr = MALLOC_ALIGNED_UNTRACKED( align, size, appHeap );

    if ( !pRetAddr || ( newTrackNode = MALLOC_ALIGNED_UNTRACKED( 0, sizeof( memtrack_node_t ))) == NULL )
    {
        // Out of memory allocating memory or tracking node;
        return NULL;
    }

    newTrackNode->size     = size;
    newTrackNode->align    = (uint8)align;
    newTrackNode->pAddress = pRetAddr;
    strncpy( newTrackNode->szFunction, pSubcategoryName, MAX_TRACK_CHAR_COUNT );
    strncpy( newTrackNode->szFilename, pSourceFileName, MAX_TRACK_CHAR_COUNT );
    newTrackNode->line     = sourceLine;
    newTrackNode->pNext    = NULL;

    if ( g_pMemoryTrackingNodes )
    {
        for ( tempNode = g_pMemoryTrackingNodes; tempNode->pNext; tempNode = tempNode->pNext )
            ;

        tempNode->pNext = newTrackNode;
    }
    else
    {
        g_pMemoryTrackingNodes = newTrackNode;
    }

    return pRetAddr;
}

void*   REALLOC_ALIGNED_TRACKED( uint32 align, void* pBlock, uint32 size, char const *pSubcategoryName, char const *pSourceFileName, int sourceLine)
{
	void* pvReturn;
	void* ptr = pBlock;
	size_t sizePtr = 2 * sizeof(void *);
	uint32_t oldsize = 0;

	if ( pBlock )
	{
		ptr = (void *)(((uint32_t)ptr & ~(sizePtr -1)) - sizePtr);  
		oldsize = (uint32_t)(*(((uint32_t *)ptr) + 1));
	}

	if ( (pvReturn = MALLOC_ALIGNED_TRACKED(align, size, pSubcategoryName, pSourceFileName, sourceLine)) != NULL ) 
	{
		memcpy(pvReturn, pBlock, oldsize);
		FREE_ALIGNED_TRACKED(pBlock);
	}

	return pvReturn;
}
void FREE_ALIGNED_TRACKED( void* ptr )
{
    memtrack_node_t *temp, *prev = g_pMemoryTrackingNodes;
    uint32 nodeDeleted = 0;

    if ( !ptr || !g_pMemoryTrackingNodes )
    {
        return;
    }

    if( ptr == prev->pAddress ) 
    {                                       /* are we deleting first node  */
        g_pMemoryTrackingNodes = prev->pNext;
                                            /* moves head to next node     */
        FREE_ALIGNED_UNTRACKED( prev );     /* free space occupied by node */
        nodeDeleted = 1;
    }
    else 
    {                                       /* if not the first node, then */
        temp = g_pMemoryTrackingNodes;

        while( temp )
        {
            if ( temp->pAddress == ptr )
            {
                prev->pNext = temp->pNext;          /* link previous node to next  */
                FREE_ALIGNED_UNTRACKED( temp );     /* free space occupied by node */
                nodeDeleted = 1;
                break;
            }
            prev = temp;
            temp = temp->pNext;
        }        
    }

    if ( nodeDeleted )
    {
        FREE_ALIGNED_UNTRACKED( ptr );
    }     
}


void HAS_MEM_LEAK()
{
	if (g_pMemoryTrackingNodes == NULL) {
		QI_LOG_DEBUG(HAS_MEM_LEAK, ("No MemoryLeak"));
	} else {
		memtrack_node_t *temp = g_pMemoryTrackingNodes;
        while( temp )
        {
			QI_LOG_DEBUG(HAS_MEM_LEAK, ("FileName:%s, FunctionName:%s, LineNum:%d, MemSize:%d", temp->szFilename, temp->szFunction, temp->line, temp->size));
            temp = temp->pNext;
        }        		
	}
	
}

#endif

void* MALLOC_ALIGNED_UNTRACKED( uint32 alignment, uint32 bytes)
{
#ifdef MEMORY_TRACK
	void * ptr, *retptr;
	size_t sizePtr = 2 * sizeof(void *);

	alignment = (alignment > sizePtr ? alignment : sizePtr) -1;

	if ( !(ptr = malloc(sizePtr + alignment + bytes)) )
	{
		return NULL;
	}
	memset(ptr, 0, sizePtr + alignment + bytes);

	retptr = (void *)(((uint32_t)ptr + sizePtr + alignment) & ~alignment);
	((uint32_t *)(retptr))[-2] = (uint32_t)ptr;
	((uint32_t *)(retptr))[-1] = bytes;

	return retptr;
#else
    
    void* ret = NULL;
    if ( !(ret = malloc(bytes)) ) return NULL;
	memset(ret, 0, bytes);
    return ret;
    
#endif
}

void FREE_ALIGNED_UNTRACKED( void* p )
{
#ifdef MEMORY_TRACK
	void * ptr = p;
    if (p == NULL) return;
	size_t sizePtr = 2 * sizeof(void *);

	if ( !p )
	{
		return;
	}

	ptr = (void *)(((uint32_t)ptr & ~(sizePtr -1)) - sizePtr);  
	ptr = (void *)(*((uint32_t *)ptr));
	free(ptr);
#else
    if (p == NULL) return;
    free(p);
#endif
}

void* REALLOC_ALIGNED_UNTRACKED( uint32 align, void* pBlock, uint32 newsize)
{
#ifdef MEMORY_TRACK
	void* pvReturn;
	void* ptr = pBlock;
	size_t sizePtr = 2 * sizeof(void *);
	uint32_t oldsize = 0;

	if ( pBlock )
	{
		ptr = (void *)(((uint32_t)ptr & ~(sizePtr -1)) - sizePtr);  
		oldsize = (uint32_t)(*(((uint32_t *)ptr) + 1));
	}

	if ( (pvReturn = MALLOC_ALIGNED_UNTRACKED(align, newsize)) != NULL ) 
	{
		memcpy(pvReturn, pBlock, oldsize);
		FREE_ALIGNED_UNTRACKED(pBlock);
	}

	return pvReturn;
#else
    return realloc(pBlock, newsize);
#endif
}

