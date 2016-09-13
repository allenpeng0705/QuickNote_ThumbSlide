#ifndef _QI_KEYBOARD_PRIVATE_H_
#define _QI_KEYBOARD_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif
    
#include "q_keyboard.h"

struct Keyboard {
    char* iName;
    char* iNameOfBackgroundImg;
    int32 iPosX;
    int32 iPosY;
    int32 iWidth;
    int32 iHeight;
    int32 iNumOfKeys;
    Key* iKeys;
    int32 iIndexOfCurrentKey;
    BOOL iShifted;
    BOOL iSupportRegionCorrection;
};

struct Key {
    char* iLabel;
    char* iAltLabel;
    char* iShiftLabel;
    char* iShiftAltLabel;
    char* iNameOfBackgroundImg;
    int32 iID;
    int32 iPosX;
    int32 iPosY;
    int32 iWidth;
    int32 iHeight;
    char* iValue;
    char* iShiftValue;
    char* iNextKeyboard;
    BOOL iShouldMap;
    KeyType iKeyType;
    KeyStatus iKeyStatus;
};
    
struct CandidatesList {
    int32 iPosX;
    int32 iPosY;
    int32 iWidth;
    int32 iHeight;
    char* iNameOfBackgroundImg;
    char* iNameOfHighlightImg;
};
    
    
#ifdef __cplusplus
}
#endif

#endif
