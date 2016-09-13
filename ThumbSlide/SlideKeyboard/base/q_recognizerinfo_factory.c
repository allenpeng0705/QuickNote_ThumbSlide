#define MODULE_NAME "q_recognizerinfo_factory.c"

#include "string.h"

#include "q_recognizerinfo_factory.h"
#include "utf8_string.h"
#include "q_malloc.h"
#include "q_hashtable.h"
#include "q_english.h"

#define RECOGNIZERINFO_HASHTABLE_SIZE 50

static QiHashtable* recognizerinfoHashtable = NULL;

unsigned int hashNameForFilter(void *aKey, int aTableSize) 
{
	const char* ptr;
	int val;
    
    
	// Hash the key by performing a number of bit operations on it.
	val = 0;
	ptr = (char*)aKey;
    
	while (*ptr != '\0') {
		int tmp;
		val = (val << 4) + (*ptr);
		if ((tmp = (val & 0xf0000000))) {
			val = val ^ (tmp >> 24);
			val = val ^ tmp;
		}
		ptr++;
	}
    
	return val % aTableSize;
}

int key_eq_fn2(void* aKey1, void* aKey2)
{
	if ((aKey1 == NULL) || (aKey2 == NULL)) return 0;
	if (utf8_stricmp((char*)aKey1, (char*)aKey2) == 0) return 1;
	return 0;
}

void free_value_fn2(void* aValue)
{
}

void free_key_fn2(void* aKey)
{
}

BOOL createRecognizerInfoFactory()
{
    if (recognizerinfoHashtable != NULL) return TRUE;
    recognizerinfoHashtable = create_hashtable(RECOGNIZERINFO_HASHTABLE_SIZE, hashNameForFilter,
                                         key_eq_fn2, free_value_fn2, free_key_fn2);
    if (recognizerinfoHashtable == NULL) return FALSE;
    return TRUE;
}

BOOL initRecognizerInfoFactory()
{
    BOOL ret = FALSE;
    if (recognizerinfoHashtable == NULL) return FALSE;
    if (hashtable_count(recognizerinfoHashtable) > 0) return TRUE;
    
    
    ret = addRecognizerInfo(g_english);
    
    return ret;
}

BOOL addRecognizerInfo(RecognizerInfo* aRecognizerInfo)
{
    BOOL ret = FALSE;
    RecognizerInfo* tmp = NULL;
    if ((recognizerinfoHashtable == NULL) || (aRecognizerInfo == NULL)) return ret;
    tmp = recognizerInfoWithName(aRecognizerInfo->iNameOfSupportedLanguage);
    if (tmp == aRecognizerInfo) return TRUE;
    ret = hashtable_insert(recognizerinfoHashtable, aRecognizerInfo->iNameOfSupportedLanguage, aRecognizerInfo);
    return ret;
}

BOOL removeRecognizerInfo(RecognizerInfo* aRecognizerInfo)
{
    RecognizerInfo* tmp = NULL;
    if (recognizerinfoHashtable == NULL) return FALSE;
    if (aRecognizerInfo == NULL) return TRUE;
    tmp = recognizerInfoWithName(aRecognizerInfo->iNameOfSupportedLanguage);
    if (tmp == NULL) return TRUE;
    
    tmp = (RecognizerInfo*)hashtable_remove(recognizerinfoHashtable, aRecognizerInfo->iNameOfSupportedLanguage);
    if (tmp == aRecognizerInfo) return TRUE;
    return FALSE;
}

BOOL removeRecognizerInfoWithName(char* aName)
{
    RecognizerInfo* tmp = NULL;
    if (recognizerinfoHashtable == NULL) return FALSE;
    if (aName == NULL) return TRUE;
    tmp = recognizerInfoWithName(aName);
    if (tmp == NULL) return TRUE;
    
    tmp = (RecognizerInfo*)hashtable_remove(recognizerinfoHashtable, aName);
    if (tmp != NULL) return TRUE;
    return FALSE;    
}

void destroyRecognizerInfoFactory()
{
    if (recognizerinfoHashtable != NULL) hashtable_destroy(recognizerinfoHashtable, 0, 0);
    recognizerinfoHashtable = NULL;
}

int32 numOfaRecognizerInfos()
{
    if (recognizerinfoHashtable == NULL) return 0;
    return hashtable_count(recognizerinfoHashtable);
}

RecognizerInfo* recognizerInfoWithName(char* aName)
{
    RecognizerInfo* ret = NULL;
    if ((recognizerinfoHashtable == NULL) || (aName == NULL) || (utf8_strlen(aName) == 0))return NULL;
    ret = hashtable_search(recognizerinfoHashtable, aName);
    return ret;
}


