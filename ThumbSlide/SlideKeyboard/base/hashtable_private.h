#ifndef _QI_HASHTABLE_PRIVATE_H
#define _QI_HASHTABLE_PRIVATE_H

#include "q_hashtable.h"

struct itementry
{
    void* iKey;
	void* iValue;
    unsigned int iHashValue;
    struct itementry* iNext;
};

struct QiHashtable {
    unsigned int iTableLength;
    struct itementry** iTable;
	unsigned int iEntryCount;
    unsigned int iLoadLimit;
    unsigned int iPrimeIndex;
    unsigned int (*iHashFn) (void* aKey, int aTableSize);
    int (*iEqFn) (void* aKey1, void* akey2);
	void (*iFreeValueFn)(void* aValue);
	void (*iFreeKeyFn)(void* aKey);
};

struct QiHashtableIter
{
    QiHashtable* iTable;
    struct itementry* iEntry;
    struct itementry* iParent;
    unsigned int iIndex;
};

#endif