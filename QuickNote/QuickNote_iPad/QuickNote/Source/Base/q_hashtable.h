#ifndef _QI_HASHTABLE_H
#define _QI_HASHTABLE_H

#ifdef __cplusplus
extern "C"{
#endif

typedef struct QiHashtable QiHashtable;
typedef struct QiHashtableIter QiHashtableIter;

/**
 * Author: Allen Peng
 * Desc: Create one hashtable with hashing function and searching function.
 * Param: aMinSize - The minium size of the hashtable.
 *		  aHashFunction - The hashing function for the created hashtable.
 *        aKey_Eq_Fn - The searching function for the created function
 * Return: Return the created hashtable. If failed , return NULL.
 */
extern QiHashtable* create_hashtable(unsigned int aMinSize,
                 unsigned int (*aHashFunction) (void*, int),
                 int (*aKey_Eq_Fn) (void*,void*),
				 void (*aFreeValueFn)(void* aValue),
				 void (*aFreeKeyFn)(void* aKey));

/**
 * Author: Allen Peng
 * Desc: Insert one pair key/value into the hashtable.
 * Param: aTable - The table for the inserting operation.
 *		  aKey - The key for the paired element.
 *        aValue - The value for the paried element.
 * Return: 0 - failed, 1 - succeed
 */
extern int hashtable_insert(QiHashtable* aTable, void* aKey, void* aValue);

#define HASHTABLE_INSERT(fnname, keytype, valuetype) \
int fnname (QiHashtable* h, keytype *k, valuetype *v) \
{ \
    return hashtable_insert(h,k,v); \
}

/**
 * Author: Allen Peng
 * Desc: Search value in the hashtable according to the specified key.
 * Param: aTable - The table for the searching operation.
 *		  aKey - Using this key to search the element.
 * Return: return the value according to the specified hashtable and index key.
 *         no matching, return NULL.
 */
extern void* hashtable_search(QiHashtable* aTable, void* aKey);

#define HASHTABLE_SEARCH(fnname, keytype, valuetype) \
valuetype * fnname (QiHashtable* h, keytype *k) \
{ \
    return (valuetype *) (hashtable_search(h,k)); \
}

/**
 * Author: Allen Peng
 * Desc: Remove one element from the hashtable according to the specified key.
 * Param: aTable - The table for the removing operation.
 *		  aKey - Using this key to remove the element.
 * Return: return the value of removed element. 
 *         no matching, return NULL.
 */
extern void* hashtable_remove(QiHashtable* aTable, void* aKey);

#define HASHTABLE_REMOVE(fnname, keytype, valuetype) \
valuetype * fnname (QiHashtable* h, keytype *k) \
{ \
    return (valuetype *) (hashtable_remove(h,k)); \
}

/**
 * Author: Allen Peng
 * Desc: Return the count of elements in the specified hashtable.
 * Param: aTable - The table which will return the number of elements.
 * Return: return the number of entries of the specified hashtable.
 */
extern unsigned int hashtable_count(QiHashtable* aTable);

/**
 * Author: Allen Peng
 * Desc: Return the length of the specified hashtable.
 * Param: aTable - The table which will return the length.
 * Return: return the lenth of the specified hashtable.
 */
extern unsigned int hashtable_length(QiHashtable* aTable);

/**
 * Author: Allen Peng
 * Desc: Destroy the specified hashtable with 'free_value' flag.
 * Param: aTable - The table which will be destroied.
 *        aFreeValue - 1: will destroy the value. 0: won't destroy the value.
 *        aFreeKey - 1: will destroy the key. 0: won't destroy the value.
 */
extern void hashtable_destroy(QiHashtable* aTable, int aFreeKey, int aFreeValue);

/**
 * Author: Allen Peng
 * Desc: Get the iterator from the specified hashtable.
 * Param: aTable - the specified hashtable.
 * Return: return the hashtable iterator in term of the hashtable.
 */
extern QiHashtableIter* hashtable_iterator(QiHashtable* aTable);

/**
 * Author: Allen Peng
 * Desc: Return the key value of the current element at current position
 * Param: aIter - the iterator of the hashtable.
 * Return: return the key value of current element
 */
extern void* hashtable_iterator_key(QiHashtableIter* aIter);

/**
 * Author: Allen Peng
 * Desc: Return the value of the current element at current position
 * Param: aIter - the iterator of the hashtable.
 * Return: return the value of current element
 */
extern void* hashtable_iterator_value(QiHashtableIter* aIter);

/**
 * Author: Allen Peng
 * Desc: Go to next element of hashtable
 * Param: aIter - the iterator of the hashtable.
 * Return: 0 - the end of the hashtable, other value is ok.
 */
extern int hashtable_iterator_next(QiHashtableIter* aIter);

/**
 * Author: Allen Peng
 * Desc: Remove current element and advance the iterator to the next element.
 * Param: aIter - the iterator of the hashtable.
 * Return: 0 - the end of the hashtable, other value is ok.
 * Remark: If you need the value to free it, read it before
 *          removing, then free it by yourself
 */
extern int hashtable_iterator_remove(QiHashtableIter* aIter);

/**
 * Author: Allen Peng
 * Desc: Remove current element and advance the iterator to the next element.
 * Param: aIter - the iterator of the hashtable.
 * Return: 0 - the end of the hashtable, other value is ok.
 * Remark: If you need the value to free it, read it before
 *          removing, then free it by yourself
 */
extern int hashtable_iterator_search(QiHashtableIter* aIter,
                          QiHashtable* aTable, void* aKey);

#define HASHTABLE_ITERATOR_SEARCH(fnname, keytype) \
int fnname (QiHashtableIter* i, QiHashtable* h, keytype *k) \
{ \
    return (hashtable_iterator_search(i,h,k)); \
}

#ifdef __cplusplus
}
#endif

#endif
