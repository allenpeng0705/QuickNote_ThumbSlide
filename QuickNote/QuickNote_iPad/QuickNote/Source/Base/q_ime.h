
#ifndef _QI_IME_H_
#define _QI_IME_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include "base.h"
#include "q_keyboard.h"
#include "q_filter.h"

typedef struct IME IME;
	
extern IME* createIME();
extern BOOL initIME(IME* aIME, char* aNameOfSupportedLanguage);
extern BOOL initialized(IME* aIME);
extern Keyboard* CurrentKeyboard(IME* aIME);
extern char* CurrentLanguage(IME* aIME);
extern BOOL changeLanguage(IME* aIME, char* aName);
extern void unInitIME(IME* aIME);
extern BOOL changeKeyboard(IME* aIME, char* aNameOfKeyboard);    
extern void destroyIME(IME* aIME);
extern const char** filterInputSignal(IME* aIME, InputSignal* aInputSignal, int32* aNum);
extern Filter* currentFilter(IME* aIME);
	
#ifdef __cplusplus
}
#endif

#endif
