#define MODULE_NAME "q_hashtable.c"

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "q_malloc.h"
#include "q_logger.h"

#include "hashtable_private.h"

static const unsigned int primes[] = {
	53, 97, 193, 389,
	769, 1543, 3079, 6151,
	12289, 24593, 49157, 98317,
	196613, 393241, 786433, 1572869,
	3145739, 6291469, 12582917, 25165843,
	50331653, 100663319, 201326611, 402653189,
	805306457, 1610612741
};

const unsigned int prime_table_length = sizeof(primes)/sizeof(primes[0]);
const double max_load_factor = 0.65;

static unsigned int indexFor(unsigned int aTableLength, unsigned int aHashValue) 
{
    return (aHashValue % aTableLength);
}

unsigned int hash( void* aTable, void* aKey)
{
    // Aim to protect against poor hash functions by adding logic here
    // - logic taken from java 1.4 hashtable source 
	QiHashtable* table = (QiHashtable*)aTable;
	unsigned int i = (table->iHashFn)(aKey, table->iTableLength);
    i += ~(i << 9);
    i ^=  ((i >> 14) | (i << 18)); 
    i +=  (i << 4);
    i ^=  ((i >> 10) | (i << 22)); 
    return i;
}

static int hashtable_expand(QiHashtable* aTable)
{
    // Double the size of the table to accomodate more entries
    struct itementry **newtable;
    struct itementry *e;
    struct itementry **pE;
    unsigned int newsize, i, index;
    // Check we're not hitting max capacity
	if (aTable->iPrimeIndex == (prime_table_length - 1)) return 0;
	newsize = primes[++(aTable->iPrimeIndex)];

    newtable = (struct itementry **)q_malloc(sizeof(struct itementry*) * newsize);

    if (NULL != newtable) {
        memset(newtable, 0, newsize * sizeof(struct itementry *));
        // This algorithm is not 'stable'. ie. it reverses the list
        // when it transfers entries between the tables
		for (i = 0; i < aTable->iTableLength; i++) {
			while (NULL != (e = aTable->iTable[i])) {
				aTable->iTable[i] = e->iNext;
				index = indexFor(newsize,e->iHashValue);
				e->iNext = newtable[index];
                newtable[index] = e;
            }
        }
        q_free(aTable->iTable);
		aTable->iTable = newtable;
    } else {
        newtable = (struct itementry **) q_realloc(aTable->iTable, newsize * sizeof(struct itementry *));
		if (NULL == newtable) {
			(aTable->iPrimeIndex)--; return 0; 
		}

		aTable->iTable = newtable;
		memset(newtable[aTable->iTableLength], 0, newsize - aTable->iTableLength);
		for (i = 0; i < aTable->iTableLength; i++) {
            for (pE = &(newtable[i]), e = *pE; e != NULL; e = *pE) {
				index = indexFor(newsize,e->iHashValue);
                if (index == i) {
					pE = &(e->iNext);
                } else {
                    *pE = e->iNext;
                    e->iNext = newtable[index];
                    newtable[index] = e;
                }
            }
        }
    }

	aTable->iTableLength = newsize;
	aTable->iLoadLimit = (unsigned int) ceil(newsize * max_load_factor);
    return 1;
}


QiHashtable* create_hashtable(unsigned int aMinSize,
                 unsigned int (*aHashFunction) (void*, int),
                 int (*aKey_Eq_Fn) (void*,void*),
				 void (*aFreeValueFn)(void* aValue),
				 void (*aFreeKeyFn)(void* aKey))
{
	QiHashtable* table;
	unsigned int index;
	unsigned int size = primes[0];

	if (aMinSize > primes[prime_table_length - 1]) return NULL;

	// Enforce size as prime
	for (index = 0; index < prime_table_length; index++) {
		if (primes[index] > aMinSize) {
			size = primes[index]; 
			break; 
		}
	}

	table = (QiHashtable *)q_malloc(sizeof(QiHashtable));
	if (NULL == table) return NULL; 
	
	// q_malloc space for entries of hashtable
	table->iTable = (struct itementry **)q_malloc(sizeof(struct itementry*) * size);
	if (NULL == table->iTable) { 
		q_free(table); 
		return NULL; 
	}
	memset(table->iTable, 0, size * sizeof(struct itementry *));
	table->iTableLength = size;
	table->iPrimeIndex = index;
	table->iEntryCount = 0;
	table->iHashFn = aHashFunction;
	table->iEqFn = aKey_Eq_Fn;
	table->iFreeValueFn = aFreeValueFn;
	table->iFreeKeyFn = aFreeKeyFn;
	table->iLoadLimit = (unsigned int) ceil(size * max_load_factor);

	return table;
}


int hashtable_insert(QiHashtable* aTable, void* aKey, void* aValue)
{
    // This method allows duplicate keys - but they shouldn't be used
    unsigned int index;
    struct itementry* e;
	if (++(aTable->iEntryCount) > aTable->iLoadLimit) {
        // Ignore the return value. If expand fails, we should
        // still try cramming just this value into the existing table
        // -- we may not have memory for a larger table, but one more
        // element may be ok. Next time we insert, we'll try expanding again.*/
        // hashtable_expand(aTable);
		return 0;
    }

    e = (struct itementry *)q_malloc(sizeof(struct itementry));
    if (NULL == e) { 
		--(aTable->iEntryCount);
		return 0; 
	}

    e->iHashValue = hash(aTable, aKey);
	index = indexFor(aTable->iTableLength, e->iHashValue);
    e->iKey = aKey;
    e->iValue = aValue;
    e->iNext = aTable->iTable[index];
    aTable->iTable[index] = e;

    return 1;
}


unsigned int hashtable_count(QiHashtable* aTable)
{
	return aTable->iEntryCount;
}

unsigned int hashtable_length(QiHashtable* aTable)
{
	return aTable->iTableLength;
}


void* hashtable_search(QiHashtable* aTable, void* aKey)
{
	struct itementry* e;
	unsigned int hashvalue, index;
	hashvalue = hash(aTable, aKey);
	index = indexFor(aTable->iTableLength, hashvalue);
	e = aTable->iTable[index];

	while (NULL != e) {
		// Check hash value to short circuit heavier comparison
		if ((hashvalue == e->iHashValue) && (aTable->iEqFn(aKey, e->iKey))) return e->iValue;
		e = e->iNext;
	}
	return NULL;
}


void* hashtable_remove(QiHashtable* aTable, void* aKey)
{
	// TODO: consider compacting the table when the load factor drops enough,
	// or provide a 'compact' method.
	struct itementry* e;
	struct itementry** pE;
	void* v;
	unsigned int hashvalue, index;

	hashvalue = hash(aTable, aKey);
	index = indexFor(aTable->iTableLength, hash(aTable, aKey));
	pE = &(aTable->iTable[index]);
	e = *pE;

	while (NULL != e) {
		// Check hash value to short circuit heavier comparison
		if ((hashvalue == e->iHashValue) && (aTable->iEqFn(aKey, e->iKey))) {
			*pE = e->iNext;
			aTable->iEntryCount--;
			v = e->iValue;
			aTable->iFreeKeyFn(e->iKey);
			q_free(e);
			return v;
		}

		pE = &(e->iNext);
		e = e->iNext;
	}
	return NULL;
}


void hashtable_destroy(QiHashtable* aTable, int aFreeKey, int aFreeValue)
{
	unsigned int i;
	struct itementry *e, *f;
	struct itementry **table = aTable->iTable;

	for (i = 0; i < aTable->iTableLength; i++) {
		e = table[i];
		while (NULL != e) { 
			f = e; 
			e = e->iNext; 
			if (aFreeKey) aTable->iFreeKeyFn(f->iKey); 
			if (aFreeValue) aTable->iFreeValueFn(f->iValue); 
			q_free(f); 
		}
	}

	q_free(aTable->iTable);
	q_free(aTable);
}

QiHashtableIter* hashtable_iterator(QiHashtable* aTable)
{
	unsigned int i, tablelength;
	QiHashtableIter* itr = (QiHashtableIter*)	q_malloc(sizeof(QiHashtableIter));
	if (NULL == itr) return NULL;
	itr->iTable = aTable;
	itr->iEntry = NULL;
	itr->iParent = NULL;
	tablelength = aTable->iTableLength;
	itr->iIndex = tablelength;
	if (0 == aTable->iEntryCount) return itr;

	for (i = 0; i < tablelength; i++) {
		if (NULL != aTable->iTable[i]) {
			itr->iEntry = aTable->iTable[i];
			itr->iIndex = i;
			break;
		}
	}
	return itr;
}


void* hashtable_iterator_key(QiHashtableIter* aIter)
{
	return aIter->iEntry->iKey;
}

void* hashtable_iterator_value(QiHashtableIter* aIter)
{
	return aIter->iEntry->iValue;
}

int hashtable_iterator_next(QiHashtableIter* aIter)
{
	unsigned int j;
	unsigned int tablelength;
	struct itementry** table;
	struct itementry* next;
	if (NULL == aIter->iEntry) return 0;

	next = aIter->iEntry->iNext;
	if (NULL != next) {
		aIter->iParent = aIter->iEntry;
		aIter->iEntry = next;
		return 1;
	}

	tablelength = aIter->iTable->iTableLength;
	aIter->iParent = NULL;
	if (tablelength <= (j = ++(aIter->iIndex))) {
		aIter->iEntry = NULL;
		return 0; // Reach the end of hashtable
	}

	table = aIter->iTable->iTable;
	while (NULL == (next = table[j])) {
		if (++j >= tablelength) {
			aIter->iIndex = tablelength;
			aIter->iEntry = NULL;
			return 0; // Reach the end of hashtable
		}
	}

	aIter->iIndex = j;
	aIter->iEntry = next;
	return 1;
}


int hashtable_iterator_remove(QiHashtableIter* aIter)
{
	struct itementry *remember_e, *remember_parent;
	int ret;

	// Do the removal
	if (NULL == (aIter->iParent)) {
		// element is head of a chain
		aIter->iTable->iTable[aIter->iIndex] = aIter->iEntry->iNext;
	} else {
		// element is mid-chain 
		aIter->iParent->iNext = aIter->iEntry->iNext;
	}

	// aIter->iitementry is now outside the hashtable
	remember_e = aIter->iEntry;
	aIter->iTable->iEntryCount--;
	// Only delete the key
	aIter->iTable->iFreeKeyFn(remember_e->iKey);
	
	// Advance the iterator, correcting the parent
	remember_parent = aIter->iParent;
	ret = hashtable_iterator_next(aIter);
	if (aIter->iParent == remember_e) { 
		aIter->iParent = remember_parent; 
	}
	q_free(remember_e);
	return ret;
}


int hashtable_iterator_search(QiHashtableIter* aIter,
                          QiHashtable* aTable, void* aKey)
{
    struct itementry *e, *parent;
    unsigned int hashvalue, index;

    hashvalue = hash(aTable, aKey);
	index = indexFor(aTable->iTableLength, hashvalue);

    e = aTable->iTable[index];
    parent = NULL;
    while (NULL != e) {
        // Check hash value to short circuit heavier comparison
		if ((hashvalue == e->iHashValue) && (aTable->iEqFn(aKey, e->iKey))) {
            aIter->iIndex = index;
			aIter->iEntry = e;
            aIter->iParent = parent;
			aIter->iTable = aTable;
            return 1;
        }
        parent = e;
		e = e->iNext;
    }
    return 0;
}
