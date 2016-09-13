#define MODULE_NAME "q_ime.c"

#include <string.h>
#include <stdlib.h>

#include "q_ime.h"
#include "q_filter_private.h"
#include "q_malloc.h"
#include "utf8_string.h"
#include "q_recognizerinfo_factory.h"
#include "q_keyboard_factory.h"
#include "q_util.h"
#include "q_lexicon.h"


static void filter_init_callback(void *aListener, Filter *aFilter, double aProgress);

struct IME {
    char* iNameOfCurrentLanguage;
    RecognizerInfo* iInfoOfRecognizer;
    Keyboard** iKeyboards;
    Filter** iFilters;
    int32 iIndexOfCurrentKeyboard;
    BOOL iInitialized;
    char** iCandidatesBuffer;
};

static void filter_init_callback(void *aListener, Filter *aFilter, double aProgress)
{
    
}


IME* createIME()
{
    int32 i = 0;
    BOOL result = FALSE;
    IME* ime = q_malloc(sizeof(IME));
    result = createKeyboardFactory();
    if (result == FALSE) {
        q_free(ime);
        return NULL;
    }
    result = initKeyboardFactory();
    if (result == FALSE) {
        q_free(ime);
        destroyKeyboardFactory();
        return NULL;
    }
    
    result = createRecognizerInfoFactory();
    if (result == FALSE) {
        q_free(ime);
        destroyKeyboardFactory();
        return NULL;
    }
    result = initRecognizerInfoFactory();
    if (result == FALSE) {
        q_free(ime);
        destroyKeyboardFactory();
        destroyRecognizerInfoFactory();
        return NULL;        
    }
    
    ime->iCandidatesBuffer = (char**)q_malloc(MAX_COUNT_CANDIDATES*sizeof(char*));
    for (i = 0; i < MAX_COUNT_CANDIDATES; i++) {
        ime->iCandidatesBuffer[i] = (char*) q_malloc(MAX_WORD_LENGTH*3);
    }
    
    ime->iFilters = NULL;
    ime->iIndexOfCurrentKeyboard = -1;
    ime->iInfoOfRecognizer = NULL;
    ime->iKeyboards = NULL;
    ime->iNameOfCurrentLanguage = NULL;
    ime->iInitialized = FALSE;
    return ime;
}


BOOL initIME(IME* aIME, char* aNameOfSupportedLanguage)
{
    FilterLoadResult result;
    int32 i = 0;
    FilterTable* filter_table = NULL;
    FilterParams filter_params;
    
    if ((aIME == NULL) || (aNameOfSupportedLanguage == NULL)
        || (utf8_strlen(aNameOfSupportedLanguage) == 0)) {
      return FALSE;  
    }
    
    q_free(aIME->iNameOfCurrentLanguage);
    i = strlen(aNameOfSupportedLanguage);
    aIME->iNameOfCurrentLanguage = (char*)q_malloc(i + 1);
    utf8_strcpy(aIME->iNameOfCurrentLanguage, aNameOfSupportedLanguage);
    aIME->iInfoOfRecognizer = recognizerInfoWithName(aNameOfSupportedLanguage);
    if (aIME->iInfoOfRecognizer == NULL) {
        q_free(aIME->iNameOfCurrentLanguage);
        return FALSE;
    }
    
    aIME->iKeyboards = (Keyboard**)q_malloc(sizeof(Keyboard*)*(aIME->iInfoOfRecognizer->iNumOfKeyboards));
    if (aIME->iKeyboards == NULL) return FALSE;
    for (i = 0; i < aIME->iInfoOfRecognizer->iNumOfKeyboards; i++) {
        aIME->iKeyboards[i] = keyboardWithName(aIME->iInfoOfRecognizer->iNameOfMajorKeyboards[i]);
    }
    aIME->iIndexOfCurrentKeyboard = 0;
    
    aIME->iFilters = (Filter**)q_malloc(sizeof(Filter*)*(aIME->iInfoOfRecognizer->iNumOfLexicons));
    if (aIME->iFilters == NULL) return FALSE;
    for (i = 0; i < aIME->iInfoOfRecognizer->iNumOfLexicons; i++) {
        result = loadFilterFromBuffer(g_english_lexicon);
        aIME->iFilters[i] = result.iFilter;
        
        filter_table = FilterTableOfKeyboard(aIME->iKeyboards[aIME->iIndexOfCurrentKeyboard]);
        filter_params = FilterParamsOfKeyboard(aIME->iKeyboards[aIME->iIndexOfCurrentKeyboard]);
        remapFilter(result.iFilter, filter_table, &filter_params);
        
        addFilterInitCallback(result.iFilter, aIME, filter_init_callback);
        
        destroyFilterTable(filter_table);
        filter_table = NULL;
        
        if (isQWERTY(filter_params)) {
            setParamsForFilter(result.iFilter, 6, 6, 80, 30, filter_params.iWidth, filter_params.iHeight);
        } else {
            setParamsForFilter(result.iFilter, 5, 5, 80, 30, filter_params.iWidth, filter_params.iHeight);
        }        
    }
    
    aIME->iInitialized = TRUE;
    return TRUE;
}

Keyboard* CurrentKeyboard(IME* aIME) 
{
    if ((aIME == NULL) || (aIME->iKeyboards == NULL)) return NULL;
    if ((aIME->iIndexOfCurrentKeyboard < 0) || 
        (aIME->iIndexOfCurrentKeyboard >= aIME->iInfoOfRecognizer->iNumOfKeyboards)) {
        return NULL;
    }
        
    return aIME->iKeyboards[aIME->iIndexOfCurrentKeyboard];
    
}

char* CurrentLanguage(IME* aIME)
{
    if (aIME == NULL) return NULL;
    return aIME->iNameOfCurrentLanguage;
}

BOOL changeLanguage(IME* aIME, char* aName)
{
    if ((aIME == NULL) || (aName == NULL) || (utf8_strlen(aName) == 0)) return FALSE;
    if (utf8_strcmp(aIME->iNameOfCurrentLanguage, aName) == 0) return TRUE;
    unInitIME(aIME);
    return initIME(aIME, aName);
}

BOOL initialized(IME* aIME)
{
    if (aIME == NULL) return FALSE;
    return aIME->iInitialized;
}

void unInitIME(IME* aIME)
{
    int32 i;
    if (aIME == NULL) return;
    
    for (i = 0; i < aIME->iInfoOfRecognizer->iNumOfLexicons; i++) {        
        if (aIME->iFilters[i] != NULL) {
            removeFilterInitCallback(aIME->iFilters[i]);
            destroyFilter(aIME->iFilters[i]);		
        }        
    }

    q_free(aIME->iFilters);
    aIME->iFilters = NULL;
    q_free(aIME->iKeyboards);
    aIME->iKeyboards = NULL;
    q_free(aIME->iNameOfCurrentLanguage);
    aIME->iNameOfCurrentLanguage = NULL;
    aIME->iInfoOfRecognizer = NULL;
    aIME->iIndexOfCurrentKeyboard = 0;
    aIME->iInitialized = FALSE;
}

BOOL changeKeyboard(IME* aIME, char* aNameOfKeyboard)
{
    Keyboard* kb;
    FilterTable* table;
    FilterParams params;
    int i = 0;
    
    if ((aIME == NULL) || (aNameOfKeyboard == NULL) ||
        (utf8_strlen(aNameOfKeyboard) == 0)) {
        return FALSE;
    }
    
    kb = keyboardWithName(aNameOfKeyboard);
    
    if (SupportRegionCorrection(kb) == TRUE) {
    
        table = FilterTableOfKeyboard(kb);
        params = FilterParamsOfKeyboard(kb);
        
        for (i = 0; i < aIME->iInfoOfRecognizer->iNumOfLexicons; i++) {
            remapFilter(aIME->iFilters[i], table, &params);
            if (isQWERTY(params)) {
                setParamsForFilter(aIME->iFilters[i], 6, 6, 80, 30, params.iWidth, params.iHeight);
            } else {
                setParamsForFilter(aIME->iFilters[i], 5, 5, 80, 30, params.iWidth, params.iHeight);
            }        
        }
        
        for (i = 0; i < aIME->iInfoOfRecognizer->iNumOfKeyboards; i++) {
            if (kb == aIME->iKeyboards[i]) {
                aIME->iIndexOfCurrentKeyboard = i;
                destroyFilterTable(table);
                return TRUE;
            }
        }
        
        destroyFilterTable(table);
    } else {
        for (i = 0; i < aIME->iInfoOfRecognizer->iNumOfKeyboards; i++) {
            if (kb == aIME->iKeyboards[i]) {
                aIME->iIndexOfCurrentKeyboard = i;
                return TRUE;
            }
        }        
    }
    return FALSE;
}

void destroyIME(IME* aIME)
{
    int32 i = 0;
    if (aIME == NULL) return;
    if (aIME->iInitialized == TRUE) unInitIME(aIME);
    destroyKeyboardFactory();
    destroyRecognizerInfoFactory();
    for (i = 0; i< MAX_COUNT_CANDIDATES; i++) {
        q_free(aIME->iCandidatesBuffer[i]);
    }
    q_free(aIME->iCandidatesBuffer);
    q_free(aIME);
}

const char** filterInputSignal(IME* aIME, InputSignal* aInputSignal, int32* aNum)
{
    Filter* recognizer;
    FilterResults* results;
    int32 i, n, m;
    *aNum = 0;
    
    if ((aIME == NULL) || (aInputSignal == NULL)) return NULL;    
    m = aInputSignal->iNumOfSamplePoints;
    if (m <= 1) return NULL;    
    n = aIME->iInfoOfRecognizer->iNumOfLexicons;
    if (n <= 0) return NULL;
    if (n == 1) {
        recognizer = aIME->iFilters[0];
    } else {
        recognizer = aIME->iFilters[n - 1];
    }
    
    results = filterWord(recognizer, aInputSignal, NULL);
	
	if ((results == NULL)|| (results->iNumOfResults <= 0)) return NULL;    
    *aNum = results->iNumOfResults;
    
    for (i = 0; i < *aNum; i++) {
        memset(aIME->iCandidatesBuffer[i], 0, strlen(aIME->iCandidatesBuffer[i]));
        utf8_strcpy(aIME->iCandidatesBuffer[i], results->iResults[i].iResult);
    }
    return (const char**)aIME->iCandidatesBuffer;
}

Filter* currentFilter(IME* aIME)
{
    int n = 0;
    Filter* ret = NULL;
    if (aIME == NULL) return NULL;
    n = aIME->iInfoOfRecognizer->iNumOfLexicons;
    if (n <= 0) return NULL;
    if (n == 1) {
        ret = aIME->iFilters[0];
    } else {
        ret = aIME->iFilters[n - 1];
    }
    return ret;
}


