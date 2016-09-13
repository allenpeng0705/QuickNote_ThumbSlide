#ifndef _QI_FILTER_PRIVATE_H_
#define _QI_FILTER_PRIVATE_H_

#include <stdio.h>
#include "q_filter.h"

#ifdef __cplusplus
extern "C"{
#endif

enum FilterLoadResultType {
    LOAD_SUCCESS = 0,
    LOAD_ERROR_BAD_HEAD,
    LOAD_ERROR_BAD_TAIL,
    LOAD_ERROR_BAD_VERSION,
    LOAD_ERROR_DAMAGED
};

enum FilterResultType {
    ANY = 0,
    ACTIVE,
    PASSIVE
};

enum FilterMode {
    MODE1 = 0,
    MODE2,
    MODE3
};

enum FilterType {
    QWERTY = 0,
    OTHER
};

struct FilterParams {
    float32 iWidth;
    float32 iHeight;
    float32 iDivider1;
    float32 iDivider2;
    FilterType  iFilterType;
};

struct FilterLoadResult {
    Filter* iFilter;
    FilterLoadResultType iLoadResult;
};

struct FilterResult {
    char* iResult;
    double iDistance;
    FilterResultType iTypeOfResult;
};

struct FilterResults {
    FilterResult* iResults;
    int iNumOfResults;
};

struct FilterItem {
    char* iLabel;
    double iPosX;
    double iPosY;
};

struct FilterTable {
    FilterItem* iFilterItems;
    int32 iNumOfFilterItems;
    char* iIgnorChars;
    int32 iNumOfIgnorChars;
};

#ifdef __cplusplus
}
#endif

#endif

