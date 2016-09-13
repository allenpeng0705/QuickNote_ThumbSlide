#define MODULE_NAME "q_keyboard_factory.c"

#include "q_keyboard_factory.h"
#include "utf8_string.h"
#include "q_malloc.h"
#include "q_hashtable.h"

#include "q_main_qwerty.h"
#include "q_main_number.h"
#include "q_main_punc.h"

#define HASHTABLE_SIZE 20

static QiHashtable* keyboardHashtable = NULL;

unsigned int hashNameForKeyboard(void *aKey, int aTableSize) 
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

int key_eq_fn1(void* aKey1, void* aKey2)
{
	if ((aKey1 == NULL) || (aKey2 == NULL)) return 0;
	if (utf8_stricmp((char*)aKey1, (char*)aKey2) == 0) return 1;
	return 0;
}

void free_value_fn1(void* aValue)
{
}

void free_key_fn1(void* aKey)
{
}

BOOL createKeyboardFactory()
{
    if (keyboardHashtable != NULL) return TRUE;
    keyboardHashtable = create_hashtable(HASHTABLE_SIZE, hashNameForKeyboard,
                                             key_eq_fn1, free_value_fn1, free_key_fn1);
    if (keyboardHashtable == NULL) return FALSE;
    return TRUE;
}

BOOL initKeyboardFactory()
{
    BOOL ret = FALSE;
    if (keyboardHashtable == NULL) return FALSE;
    if (hashtable_count(keyboardHashtable) > 0) return TRUE;
        
        
    ret = addKeyboard(g_qwerty_keyboard);
    ret = addKeyboard(g_qwerty_normal_keyboard);
    ret = addKeyboard(g_qwerty_trace_keyboard);
    ret = addKeyboard(g_qwerty_left_trace_keyboard);
    
    // Add other keyboards
    ret = addKeyboard(g_number_keyboard);
    ret = addKeyboard(g_punc_keyboard);
    return ret;
}

BOOL addKeyboard(Keyboard* aKeyboard)
{
    BOOL ret = FALSE;
    Keyboard* tmp = NULL;
    if ((keyboardHashtable == NULL) || (aKeyboard == NULL)) return ret;
    tmp = keyboardWithName(NameOfKeyboard(aKeyboard));
    if (tmp == aKeyboard) return TRUE;
    ret = hashtable_insert(keyboardHashtable, NameOfKeyboard(aKeyboard), aKeyboard);
    return ret;
}

BOOL removeKeyboard(Keyboard* aKeyboard)
{
    Keyboard* tmp = NULL;
    if (keyboardHashtable == NULL) return FALSE;
    if (aKeyboard == NULL) return TRUE;
    tmp = keyboardWithName(NameOfKeyboard(aKeyboard));
    if (tmp == NULL) return TRUE;
    
    tmp = (Keyboard*)hashtable_remove(keyboardHashtable, NameOfKeyboard(aKeyboard));
    if (tmp == aKeyboard) return TRUE;
    return FALSE;
}

BOOL removeKeyboardWithName(char* aName)
{
    Keyboard* tmp = NULL;
    if (keyboardHashtable == NULL) return FALSE;
    if (aName == NULL) return TRUE;
    tmp = keyboardWithName(aName);
    if (tmp == NULL) return TRUE;
    
    tmp = (Keyboard*)hashtable_remove(keyboardHashtable, aName);
    if (tmp != NULL) return TRUE;
    return FALSE;    
}

void destroyKeyboardFactory()
{
    if (keyboardHashtable != NULL) hashtable_destroy(keyboardHashtable, 0, 0);
    keyboardHashtable = NULL;
}

int32 numOfKeyboards()
{
    if (keyboardHashtable == NULL) return 0;
    return hashtable_count(keyboardHashtable);
}

Keyboard* keyboardWithName(char* aName)
{
    Keyboard* ret = NULL;
    if ((keyboardHashtable == NULL) || (aName == NULL) || (utf8_strlen(aName) == 0))return NULL;
    ret = hashtable_search(keyboardHashtable, aName);
    return ret;
}

