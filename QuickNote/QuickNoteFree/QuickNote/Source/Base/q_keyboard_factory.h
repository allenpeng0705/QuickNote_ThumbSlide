#ifndef _QI_KEYBOARD_FACTORY_H_
#define _QI_KEYBOARD_FACTORY_H_

#include "q_keyboard.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    
extern BOOL createKeyboardFactory();
extern BOOL initKeyboardFactory();
extern BOOL addKeyboard(Keyboard* aKeyboard);
extern BOOL removeKeyboard(Keyboard* aKeyboard);
extern BOOL removeKeyboardWithName(char* aName);    
extern void destroyKeyboardFactory();
extern int32 numOfKeyboards();
extern Keyboard* keyboardWithName(char* aName);



#ifdef __cplusplus
}
#endif

#endif