#ifndef _QI_FILTER_H_
#define _QI_FILTER_H_

#include <stdio.h>
#include "base.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct Filter Filter;
typedef struct FilterParams FilterParams;
typedef struct FilterLoadResult FilterLoadResult;
typedef struct FilterItem FilterItem;
typedef struct FilterTable FilterTable;
typedef struct FilterResult FilterResult;
typedef struct FilterResults FilterResults;
typedef enum FilterLoadResultType FilterLoadResultType;
typedef enum FilterResultType FilterResultType;
typedef enum FilterMode FilterMode;
typedef enum FilterType FilterType;
typedef void (*filterInitCallback)(void* aListener, Filter* aFilter, double aProgress);


extern Filter* createFilter();
extern void destroyFilter(Filter* aFilter);
extern void setParamsForFilter(Filter* aFilter, uint8 aP1, uint8 aP2, uint8 aP3, uint8 aP4, int16 aP5, int16 aP6);
extern void setFilterMode(Filter* aFilter, FilterMode aFilterMode);
extern FilterLoadResult loadFilterFromFile(FILE* aFile); 
extern FilterLoadResult loadFilterFromBuffer(void* aBuffer);
extern int32 storeFilterIntoFile(Filter* aFilter, FILE* aFile);
extern void initFilter(Filter* aFilter, FILE* aFile, const FilterTable* aFilterTable, FilterParams* aFilterParms);
extern BOOL remapFilter(Filter* aFilter, const FilterTable* aFilterTable, FilterParams* aFilterParms);    
extern BOOL setWordActive(Filter* aFilter, const char* aWord, BOOL aActive);    
extern BOOL isActiveWord(Filter* aFilter, const char* aWord);
extern BOOL setAllActive(Filter* aFilter, BOOL aActive);
extern FilterResults* filter(Filter* aFilter, const InputSignal* aInputSignal, const void* aReserved);
extern BOOL addWord(Filter* aFilter, const char* aWord);
extern BOOL addWords(Filter* aFilter, const char** aWords, int32 aNumOfWords);
extern BOOL wordExisted(Filter* aFilter, const char* aWord);
extern BOOL extraWordExisted(Filter* aFilter, const char* aExtraWord);
extern BOOL removeWord(Filter* aFilter, const char* aWord);
        
extern void addFilterInitCallback(Filter* aFilter, void* aListener, filterInitCallback aFilterInitCallback);
extern void removeFilterInitCallback(Filter* aFilter);
    
extern void destroyFilterTable(FilterTable* aFilterTable);
extern BOOL isQWERTY(FilterParams aParams);


#ifdef __cplusplus
}
#endif

#endif
