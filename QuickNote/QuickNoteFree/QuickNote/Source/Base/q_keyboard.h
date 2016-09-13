#ifndef _QI_KEYBOARD_H_
#define _QI_KEYBOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"
#include "q_filter.h"

typedef struct Keyboard Keyboard;
typedef struct Key Key;
typedef struct CandidatesList CandidatesList;

extern Keyboard* GetKeyboard(const char* aNameOfKeyboard);
extern Keyboard* CreateKeyboard();
extern Key* CreateKey();    
extern char* NameOfKeyboard(Keyboard* aKeyboard);
extern BOOL Shifted(Keyboard* aKeyboard);
extern void setKeyboardShiftState(Keyboard* aKeyboard, BOOL aShifted);
extern Key* CurrentKey(Keyboard* aKeyboard);
extern Key* Keys(Keyboard* aKeyboard);
extern int32 IndexOfKey(Keyboard* aKeyboard, Key* aKey);
extern int32 IndexOfCurrentKey(Keyboard* aKeyboard);
extern void SetIndexForCurrentKey(Keyboard* aKeyboard, int32 aIndex);
extern void SetNameForKeyboard(Keyboard* aKeyboard, char* aName);
extern int32 NumOfKeys(Keyboard* aKeyboard);
extern void SetNumForAllKeys(Keyboard* aKeyboard, int32 aNumber);
extern int32 NumOfAlphaKeys(Keyboard* aKeyboard);
extern int32 NumOfMappedKeys(Keyboard* aKeyboard);
extern char* NameOfBackgroundImgOfKeyboard(Keyboard* aKeyboard);
extern char* NameOfBackgroundImageOfKey(Key* aKey);
extern void SetNameOfBackgroundImgForKey(Key* aKey, char* aName);
extern Key* KeyAtIndex(Keyboard* aKeyboard, int32 aIndex); 
extern Key* KeyAtPosition(Keyboard* aKeyboard, int32 aPosX, int32 aPosY);
extern Key* ClosestKeyAtPosition(Keyboard* aKeyboard, int32 aPosX, int32 aPosY);
extern Key* KeyWithLabel(Keyboard* aKeyboard, char* aLabel);
extern BOOL KeyContainsPosition(Key* aKey, int32 aPosX, int32 aPosY);
extern void PositionOfKeyboard(Keyboard* aKeyboard, int32* aPosX, int32* aPosY);
extern void SizeOfKeyboard(Keyboard* aKeyboard, int32* aWidth, int32* aHeight);
extern void SetPositionForKeyboard(Keyboard* aKeyboard, int32 aPosX, int32 aPosY);
extern void SetSizeForKeyboard(Keyboard* aKeyboard, int32 aWidth, int32 aHeight);    
extern void PositionOfKey(Key* aKey, int32* aPosX, int32* aPosY);
extern void SetPositionForKey(Key* aKey, int32 aPosX, int32 aPosY);
extern void SizeOfKey(Key* aKey, int32* aWidth, int32* aHeight);
extern void SetSizeForKey(Key* aKey, int32 aWidth, int32 aHeight);
extern void CentralPositionOfKey(Key* aKey, float64* aCentralX, float64* aCentralY);
extern void SetNameForBackgroundImgOfKeyboard(Keyboard* aKeyboard, char* aName);
extern BOOL SupportRegionCorrection(Keyboard* aKeyboard);
extern void SetRegionCorrectionFlag(Keyboard* aKeyboard, BOOL aFlag);    
extern char* LabelOfKey(Key* aKey);
extern void SetLabelForKey(Key* aKey, char* aLabel);
extern char* ShiftLabelOfKey(Key* aKey);
extern void SetShiftLabelForKey(Key* aKey, char* aShiftLabel);
extern char* ShiftAltLabelOfKey(Key* aKey);
extern void SetShiftAltLabelForKey(Key* aKey, char* aShiftAltLabel); 
extern char* ShiftValueOfKey(Key* aKey);
extern void SetShiftValueForKey(Key* aKey, char* aShiftValue);
extern char* AltLabelOfKey(Key* aKey);
extern void SetAltLabelForKey(Key* aKey, char* aAltLabel);
extern void SetValueForKey(Key* aKey, char* aValue);
extern char* ValueOfKey(Key* aKey);
extern BOOL ShouldMap(Key* aKey);
extern void SetMapFlagForKey(Key* aKey, BOOL aFlag);
extern int32 IDOfKey(Key* aKey);
extern void setIDForKey(Key* aKey, int32 aID);
extern char* NameOfNextKeyboard(Key* aKey);
extern void SetNameOfNextKeyboardForKey(Key* aKey, char* aName);
extern KeyType TypeOfKey(Key* aKey);
extern void SetTypeForKey(Key* aKey, KeyType aType);
extern KeyStatus StatusOfKey(Key* aKey);
extern void SetStatusForKey(Key* aKey, KeyStatus aStatus);
    
extern FilterTable* FilterTableOfKeyboard(Keyboard* aKeyboard);
extern FilterParams FilterParamsOfKeyboard(Keyboard* aKeyboard);
    


#ifdef __cplusplus
}
#endif

#endif
