#ifndef _QI_RECOGNIZERINFO_FACTORY_H_
#define _QI_RECOGNIZERINFO_FACTORY_H_

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif
   
typedef struct RecognizerInfo RecognizerInfo;    
struct RecognizerInfo {
    char* iNameOfSupportedLanguage;
    char** iNameOfLexicons;
    char** iNameOfActiveLexicons;
    char** iNameOfUserLexicons;
    char** iNameOfMajorKeyboards;
    int32* iLenOfWordInLexicon;
    int32 iNumOfKeyboards;
    int32 iNumOfLexicons;
};
    
    
extern BOOL createRecognizerInfoFactory();
extern BOOL initRecognizerInfoFactory();
extern BOOL addRecognizerInfo(RecognizerInfo* aRecognizerInfo);
extern BOOL removeRecognizerInfo(RecognizerInfo* aRecognizerInfo);
extern BOOL removeRecognizerInfoWithName(char* aName);    
extern void destroyRecognizerInfoFactory();
extern int32 numOfRecognizerInfos();
extern RecognizerInfo* recognizerInfoWithName(char* aName);
    
    
    
#ifdef __cplusplus
}
#endif

#endif