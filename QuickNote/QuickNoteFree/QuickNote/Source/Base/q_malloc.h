
#ifndef _QI_MALLOC_
#define _QI_MALLOC_

#include "base.h"

//#define MEMORY_TRACK

#define q_malloc(size) MALLOC_ALIGNED_TRACKED(0, size, __FUNCTION__, __FILE__, __LINE__) 
#define q_realloc(block, size) REALLOC_ALIGNED_TRACKED(0, block, size, __FUNCTION__, __FILE__, __LINE__) 
#define q_free(ptr) FREE_ALIGNED_TRACKED(ptr)
#define q_memory_leak() HAS_MEM_LEAK()

#ifdef MEMORY_TRACK

#ifdef __cplusplus
extern "C" {    
#endif
    
void*   MALLOC_ALIGNED_TRACKED( uint32 align, uint32 size, char const *pSubcategoryName, char const *pSourceFileName, int sourceLine);
void    FREE_ALIGNED_TRACKED( void* ptr );
void*   REALLOC_ALIGNED_TRACKED( uint32 align, void* pBlock, uint32 size, char const *pSubcategoryName, char const *pSourceFileName, int sourceLine);
void	HAS_MEM_LEAK();
    
#ifdef __cplusplus
}
#endif

#else

#define	MALLOC_ALIGNED_TRACKED(align, size, pSubcategoryName, pSourceFileName, sourceLine)			MALLOC_ALIGNED_UNTRACKED((align), (size))
#define FREE_ALIGNED_TRACKED(ptr)																			FREE_ALIGNED_UNTRACKED(ptr)
#define REALLOC_ALIGNED_TRACKED(align, pBlock, size, pSubcategoryName, pSourceFileName, sourceLine) REALLOC_ALIGNED_UNTRACKED((align),(pBlock), (size));
#define HAS_MEM_LEAK()

#endif 

#ifdef __cplusplus
extern "C" {
#endif
    
void*   MALLOC_ALIGNED_UNTRACKED(uint32 align, uint32 size);
void    FREE_ALIGNED_UNTRACKED( void* ptr );
void*   REALLOC_ALIGNED_UNTRACKED(uint32 align, void* pBlock, uint32 size);
    
#ifdef __cplusplus
}
#endif

#endif 

