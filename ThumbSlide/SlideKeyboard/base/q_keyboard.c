#define MODULE_NAME "q_keyboard.c"
#include <math.h>
#include <string.h>

#include "q_logger.h"
#include "q_keyboard_private.h"
#include "utf8_string.h"
#include "q_malloc.h"
#include "q_error.h"
#include "q_filter_private.h"

#include "q_data.h"


//static double distance(double aX1, double aY1, double aX2, double aY2)
//{
//    double dx = aX1 - aX2;
//    double dy = aY1 - aY2;
//    return sqrt(dx * dx + dy * dy);
//}

Keyboard* GetKeyboard(const char* aNameOfKeyboard)
{
    return NULL;
}

Keyboard* CreateKeyboard()
{
    Keyboard* ret = (Keyboard*)q_malloc(sizeof(Keyboard));
    ret->iName = NULL;
    ret->iNameOfBackgroundImg = NULL;
    ret->iNumOfKeys = 0;
    ret->iKeys = NULL;
    ret->iIndexOfCurrentKey = -1;
    ret->iSupportRegionCorrection = FALSE;
    ret->iShifted = FALSE;
    return ret;
}

BOOL Shifted(Keyboard* aKeyboard)
{
    if (aKeyboard == NULL) return FALSE;
    return aKeyboard->iShifted;
}

void setKeyboardShiftState(Keyboard* aKeyboard, BOOL aShifted)
{
    if (aKeyboard == NULL) return;
    aKeyboard->iShifted = aShifted;
}

Key* CreateKey()
{
    Key* ret = (Key*)q_malloc(sizeof(Key)); 
    ret->iLabel = NULL;
    ret->iAltLabel = NULL;
    ret->iValue = NULL;
    ret->iNextKeyboard = NULL;
    ret->iID = -1;
    ret->iShouldMap = FALSE;
    ret->iKeyType = KEY_SYMBOL_NULL;
    ret->iKeyStatus = KEY_NORMAL;
    return ret;
}

Key* Keys(Keyboard* aKeyboard)
{
    if (aKeyboard == NULL) return NULL;
    return aKeyboard->iKeys;    
}

Key* CurrentKey(Keyboard* aKeyboard)
{
    if (aKeyboard == NULL) return NULL;
    if (aKeyboard->iIndexOfCurrentKey == -1) return NULL;
    return &(aKeyboard->iKeys[aKeyboard->iIndexOfCurrentKey]);    
}

char* NameOfKeyboard(Keyboard* aKeyboard)
{
    if (aKeyboard == NULL) return NULL;
    return aKeyboard->iName;
}

void SetNameForKeyboard(Keyboard* aKeyboard, char* aName)
{
    if ((aKeyboard == NULL) || (aName == NULL) || (utf8_strlen(aName) == 0)) return;
    q_free((char*)aKeyboard->iName);
    aKeyboard->iName = (char*) q_malloc(strlen(aName) + 1); // strlen to get the size of the buffer
    utf8_strcpy((char*)aKeyboard->iName, aName); 
}

void SetNameOfBackgroundImgForKey(Key* aKey, char* aName)
{
    if ((aKey == NULL) || (aName == NULL) || (utf8_strlen(aName) == 0)) return;
    q_free((char*)aKey->iNameOfBackgroundImg);
    aKey->iNameOfBackgroundImg = (char*) q_malloc(strlen(aName) + 1); // strlen to get the size of the buffer
    utf8_strcpy((char*)aKey->iNameOfBackgroundImg, aName); 
}

char* NameOfBackgroundImageOfKey(Key* aKey)
{
    if (aKey == NULL) return NULL;
    return aKey->iNameOfBackgroundImg;
}

int32 NumOfKeys(Keyboard* aKeyboard)
{
    if (aKeyboard == NULL) return ERR_NULL_POINTER;
    return aKeyboard->iNumOfKeys;
}

void SetNumForAllKeys(Keyboard* aKeyboard, int32 aNumber)
{
    if (aKeyboard == NULL) return;
    aKeyboard->iNumOfKeys = aNumber;
}

int32 NumOfAlphaKeys(Keyboard* aKeyboard)
{
    int32 i, n;
    int32 ret = 0;
    if (aKeyboard == NULL) return ret;
    n = NumOfKeys(aKeyboard);
    
    for (i = 0; i < n; i++) {
        if (aKeyboard->iKeys[i].iKeyType == KEY_SYMBOL_ALPHA) ret++;
    }
    
    return ret;   
}

int32 NumOfMappedKeys(Keyboard* aKeyboard)
{
    int32 i, n;
    int32 ret = 0;
    if (aKeyboard == NULL) return ret;
    n = NumOfKeys(aKeyboard);
    
    for (i = 0; i < n; i++) {
        if (aKeyboard->iKeys[i].iShouldMap == TRUE) ret++;
    }
    
    return ret;    
}

Key* KeyAtIndex(Keyboard* aKeyboard, int32 aIndex)
{
    if (aKeyboard == NULL) return NULL;
    if ((aIndex < 0) || (aIndex >= aKeyboard->iNumOfKeys)) return NULL;
    return &(aKeyboard->iKeys[aIndex]);
}

int32 IndexOfKey(Keyboard* aKeyboard, Key* aKey)
{
    int32 i, n;
    int32 ret = -1;
    
    if ((aKeyboard == NULL) || (aKey == NULL)) return ret;
    n = NumOfKeys(aKeyboard);
    
	for (i = 0; i < n; i++) {
        if (&(aKeyboard->iKeys[i]) == aKey) {
            ret = i;
            break;
        }
    }    
    return ret;    
}

int32 IndexOfCurrentKey(Keyboard* aKeyboard)
{
    if (aKeyboard == NULL) return -1;
    return aKeyboard->iIndexOfCurrentKey;
}

void SetIndexForCurrentKey(Keyboard* aKeyboard, int32 aIndex)
{
    int32 n = 0;
    if (aKeyboard == NULL) return;
    n = NumOfKeys(aKeyboard);
    if ((aIndex >= n) || (aIndex < 0)) return;
    if ((aKeyboard->iIndexOfCurrentKey < n) || (aKeyboard->iIndexOfCurrentKey > 0)) {
        aKeyboard->iKeys[aKeyboard->iIndexOfCurrentKey].iKeyStatus = KEY_NORMAL;
    }
    aKeyboard->iIndexOfCurrentKey = aIndex;
    aKeyboard->iKeys[aKeyboard->iIndexOfCurrentKey].iKeyStatus = KEY_PRESSED;
            
}

Key* ClosestKeyAtPosition(Keyboard* aKeyboard, int32 aPosX, int32 aPosY)
{
    int32 i, n;
    Key* ret = NULL;
    
    if (aKeyboard == NULL) return NULL;
    n = NumOfKeys(aKeyboard);
    float64 minLen = 5000.0;
    
	for (i = 0; i < n; i++) {
        float64 posX, posY;
        Key* key = &(aKeyboard->iKeys[i]);
        CentralPositionOfKey(key, &posX, &posY);
        posX = abs(aPosX - posX);
        posY = abs(aPosY - posY);
        float64 len = sqrt(posX*posX + posY*posY);
        if (len < minLen) {
            minLen = len;
            ret = key;
        }
    }
    
    return ret;
}

Key* KeyAtPosition(Keyboard* aKeyboard, int32 aPosX, int32 aPosY)
{
    int32 i, n;
    Key* ret = NULL;
    
    if (aKeyboard == NULL) return NULL;
    n = NumOfKeys(aKeyboard);
    
	for (i = 0; i < n; i++) {
        if (KeyContainsPosition(&(aKeyboard->iKeys[i]), aPosX, aPosY) == TRUE) {
            ret = &(aKeyboard->iKeys[i]);
            break;
        }
    }
    
    if (ret == NULL) {
        ret = ClosestKeyAtPosition(aKeyboard, aPosX, aPosY);
    }
                                                      
    return ret;
}
                                                      
Key* KeyWithLabel(Keyboard* aKeyboard, char* aLabel)
{
    int32 i, n;
    Key* ret = NULL;
            
    if ((aKeyboard == NULL) || (aLabel == NULL)) return NULL;
    n = NumOfKeys(aKeyboard);
            
    for (i = 0; i < n; i++) {
        if (aKeyboard->iKeys[i].iLabel == NULL) continue;
        if (utf8_strcmp(aKeyboard->iKeys[i].iLabel, aLabel) == 0) {
            ret = &(aKeyboard->iKeys[i]);
            break;
        }
        if (utf8_strcmp(aKeyboard->iKeys[i].iShiftLabel, aLabel) == 0) {
            ret = &(aKeyboard->iKeys[i]);
            break;
        }        
        
    }
                                                              
    return ret;
}
                                                      
BOOL KeyContainsPosition(Key* aKey, int32 aPosX, int32 aPosY)
{
    if ((aPosX >= aKey->iPosX) && (aPosX <= (aKey->iPosX + aKey->iWidth))
        && (aPosY >= aKey->iPosY) && (aPosY <= (aKey->iPosY + aKey->iHeight))) {
        return TRUE;
    }
    return FALSE;
}

void PositionOfKeyboard(Keyboard* aKeyboard, int32* aPosX, int32* aPosY)
{
    if (aKeyboard == NULL) return;
    *aPosX = aKeyboard->iPosX;
    *aPosY = aKeyboard->iPosY;
}
                                    
void SizeOfKeyboard(Keyboard* aKeyboard, int32* aWidth, int32* aHeight)
{
    if (aKeyboard == NULL) return;
    *aWidth = aKeyboard->iWidth;
    *aHeight = aKeyboard->iHeight;    
}
                                    
void SetPositionForKeyboard(Keyboard* aKeyboard, int32 aPosX, int32 aPosY)
{
    if (aKeyboard == NULL) return;
    aKeyboard->iPosX = aPosX;
    aKeyboard->iPosY = aPosY;
}
                                    
void SetSizeForKeyboard(Keyboard* aKeyboard, int32 aWidth, int32 aHeight)
{
    if (aKeyboard == NULL) return;
    aKeyboard->iWidth = aWidth;
    aKeyboard->iHeight = aHeight;        
}
                                    
void PositionOfKey(Key* aKey, int32* aPosX, int32* aPosY)
{
    if (aKey == NULL) return;
    *aPosX = aKey->iPosX;
    *aPosY = aKey->iPosY;
}
                                    
void SetPositionForKey(Key* aKey, int32 aPosX, int32 aPosY)
{
    if (aKey == NULL) return;
    aKey->iPosX = aPosX;
    aKey->iPosY = aPosY;
}
                                    
void SizeOfKey(Key* aKey, int32* aWidth, int32* aHeight)
{
    if (aKey == NULL) return;
    *aWidth = aKey->iWidth;
    *aHeight = aKey->iHeight;
}
                                    
void SetSizeForKey(Key* aKey, int32 aWidth, int32 aHeight)
{
    if (aKey == NULL) return;
    aKey->iWidth = aWidth;
    aKey->iHeight = aHeight;
}
                                    
void CentralPositionOfKey(Key* aKey, float64* aCentralX, float64* aCentralY)
{
    if (aKey == NULL) return;
    *aCentralX = aKey->iPosX + aKey->iWidth/2;
    *aCentralY = aKey->iPosY + aKey->iHeight/2;
}
                                    
char* NameOfBackgroundImgOfKeyboard(Keyboard* aKeyboard)
{
    if (aKeyboard == NULL) return NULL;
    return aKeyboard->iNameOfBackgroundImg;
}
                                    
void SetNameForBackgroundImgOfKeyboard(Keyboard* aKeyboard, char* aName)
{
    if ((aKeyboard == NULL) || (aName == NULL) || (utf8_strlen(aName) == 0))return;
    q_free(aKeyboard->iNameOfBackgroundImg);
    aKeyboard->iNameOfBackgroundImg = (char*) q_malloc(strlen(aName) + 1); // strlen to get the size of the buffer
    utf8_strcpy(aKeyboard->iNameOfBackgroundImg, aName);
}
                                    
BOOL SupportRegionCorrection(Keyboard* aKeyboard)
{
    if (aKeyboard == NULL) return FALSE;
    return aKeyboard->iSupportRegionCorrection;
}
                                    
void SetRegionCorrectionFlag(Keyboard* aKeyboard, BOOL aFlag)
{
    if (aKeyboard == NULL) return;
    aKeyboard->iSupportRegionCorrection = aFlag;
}
                                    
char* LabelOfKey(Key* aKey)
{
    if (aKey == NULL) return NULL;
    return aKey->iLabel;
}
                                    
void SetLabelForKey(Key* aKey, char* aLabel)
{
    if ((aKey == NULL) || (aLabel == NULL) || (utf8_strlen(aLabel) == 0)) return;
    q_free(aKey->iLabel);
    aKey->iLabel = q_malloc(strlen(aLabel) + 1); // the size of aLabel
    utf8_strcpy(aKey->iLabel, aLabel);
}

void SetAltLabelForKey(Key* aKey, char* aAltLabel)
{
    if ((aKey == NULL) || (aAltLabel == NULL) || (utf8_strlen(aAltLabel) == 0)) return;
    q_free(aKey->iAltLabel);
    aKey->iAltLabel = q_malloc(strlen(aAltLabel) + 1); // the size of aLabel
    utf8_strcpy(aKey->iAltLabel, aAltLabel);    
}
                                    
void SetValueForKey(Key* aKey, char* aValue)
{
    if ((aKey == NULL) || (aValue == NULL) || (utf8_strlen(aValue) == 0)) return;
    q_free(aKey->iValue);
    aKey->iValue = q_malloc(strlen(aValue) + 1); // the size of aLabel
    utf8_strcpy(aKey->iValue, aValue);    
}
                                    
char* AltLabelOfKey(Key* aKey)
{
    if (aKey == NULL) return NULL;
    return aKey->iAltLabel;    
}
                                    
char* ValueOfKey(Key* aKey)
{
    if (aKey == NULL) return NULL;
    return aKey->iValue;     
}
                                    
BOOL ShouldMap(Key* aKey)
{
    if (aKey == NULL) return FALSE;
    return aKey->iShouldMap;
}
                                    
void SetMapFlagForKey(Key* aKey, BOOL aFlag)
{
    if (aKey == NULL) return;
    aKey->iShouldMap = aFlag;
}
                                    
int32 IDOfKey(Key* aKey)
{
    if (aKey == NULL) return -1;
    return aKey->iID;
}
                                    
void setIDForKey(Key* aKey, int32 aID)
{
    if (aKey == NULL) return;
    aKey->iID = aID;
}
                                    
char* NameOfNextKeyboard(Key* aKey)
{
    if (aKey == NULL) return NULL;
    return aKey->iNextKeyboard;
}
                                    
void SetNameOfNextKeyboardForKey(Key* aKey, char* aName)
{
    if ((aKey == NULL) || (aName == NULL) || (utf8_strlen(aName) == 0)) return;
    q_free(aKey->iNextKeyboard);
    aKey->iNextKeyboard = q_malloc(strlen(aName) + 1);
    utf8_strcpy(aKey->iNextKeyboard, aName);
}
                                    
KeyType TypeOfKey(Key* aKey)
{
    if (aKey == NULL) return KEY_SYMBOL_NULL;
    return aKey->iKeyType;
}
                                    
void SetTypeForKey(Key* aKey, KeyType aType)
{
    if (aKey == NULL) return;
    aKey->iKeyType = aType;
}
                                    
KeyStatus StatusOfKey(Key* aKey)
{
    if (aKey == NULL) return KEY_NORMAL;
    return aKey->iKeyStatus;
}
                                    
void SetStatusForKey(Key* aKey, KeyStatus aStatus)
{
    if (aKey == NULL) return;
    aKey->iKeyStatus = aStatus;
}
                                    
char* ShiftLabelOfKey(Key* aKey)
{
    if (aKey == NULL) return NULL;
    return aKey->iShiftLabel;
}
                                    
void SetShiftLabelForKey(Key* aKey, char* aShiftLabel)
{
    if ((aKey == NULL) || (aShiftLabel == NULL) || (utf8_strlen(aShiftLabel) == 0)) return;
    q_free(aKey->iShiftLabel);
    aKey->iShiftLabel = q_malloc(strlen(aShiftLabel) + 1); // the size of aLabel
    utf8_strcpy(aKey->iShiftLabel, aShiftLabel);    
}
                                    
char* ShiftAltLabelOfKey(Key* aKey)
{
    if (aKey == NULL) return NULL;
    return aKey->iShiftAltLabel;    
}
                                    
void SetShiftAltLabelForKey(Key* aKey, char* aShiftAltLabel)
{
    if ((aKey == NULL) || (aShiftAltLabel == NULL) || (utf8_strlen(aShiftAltLabel) == 0)) return;
    q_free(aKey->iShiftAltLabel);
    aKey->iShiftAltLabel = q_malloc(strlen(aShiftAltLabel) + 1); // the size of aLabel
    utf8_strcpy(aKey->iShiftAltLabel, aShiftAltLabel);      
}
                                    
char* ShiftValueOfKey(Key* aKey)
{
    if (aKey == NULL) return NULL;
    return aKey->iShiftValue;    
}
                                    
void SetShiftValueForKey(Key* aKey, char* aShiftValue)
{
    if ((aKey == NULL) || (aShiftValue == NULL) || (utf8_strlen(aShiftValue) == 0)) return;
    q_free(aKey->iShiftValue);
    aKey->iShiftValue = q_malloc(strlen(aShiftValue) + 1); // the size of aLabel
    utf8_strcpy(aKey->iShiftValue, aShiftValue); 
}

FilterTable* FilterTableOfKeyboard(Keyboard* aKeyboard)
{
    int32 i = 0;
    int32 key_count = 0;
    int32 filter_item_count = 0;
    char* filter_item_result = NULL;
    char* filter_item_shift_result = NULL;
    float64 filter_center_x, filter_center_y;
    FilterTable* ret;
    
    if (aKeyboard == NULL) return NULL;
    key_count = NumOfKeys(aKeyboard);
    
    for (i = 0; i < key_count; i++) {
        Key* key = KeyAtIndex(aKeyboard, i);
        if (ShouldMap(key)) {
            filter_item_count += utf8_strlen(ValueOfKey(key));
            filter_item_count += utf8_strlen(ShiftValueOfKey(key));
        }
    }
    
    ret = q_malloc(sizeof(FilterTable));
    ret->iNumOfFilterItems = filter_item_count;
    ret->iFilterItems = q_malloc(sizeof(FilterItem)*filter_item_count);
    ret->iIgnorChars = "'_-.";
    ret->iNumOfIgnorChars = utf8_strlen(ret->iIgnorChars);

    
    filter_item_count = 0;
    for (i = 0; i < key_count; i++) {
        int j = 0;
        Key* key = KeyAtIndex(aKeyboard, i);
        if (ShouldMap(key)) {
            filter_item_result = ValueOfKey(key);
            filter_item_shift_result = ShiftValueOfKey(key);
            
            CentralPositionOfKey(key, &filter_center_x, &filter_center_y);
            for (j = 0; j < utf8_strlen(filter_item_result); j++) {
                ret->iFilterItems[filter_item_count].iLabel = (char*)q_malloc(5);
                utf8_set_char_at(ret->iFilterItems[filter_item_count].iLabel, 
                                 0, utf8_char_at(filter_item_result, j));
                ret->iFilterItems[filter_item_count].iPosX = filter_center_x;
                ret->iFilterItems[filter_item_count].iPosY = filter_center_y;
                filter_item_count++;
            }
            
            for (j = 0; j < utf8_strlen(filter_item_shift_result); j++) {
                ret->iFilterItems[filter_item_count].iLabel = (char*)q_malloc(5);
                utf8_set_char_at(ret->iFilterItems[filter_item_count].iLabel, 
                                 0, utf8_char_at(filter_item_shift_result, j));
                ret->iFilterItems[filter_item_count].iPosX = filter_center_x;
                ret->iFilterItems[filter_item_count].iPosY = filter_center_y;
                filter_item_count++;
            }
        }
    }
    
    return ret;
}

FilterParams FilterParamsOfKeyboard(Keyboard* aKeyboard)
{
    FilterParams ret;
    Key *key_y, *key_h, *key_n;
    float64 x, y_y, y_h, y_n;
    char* name;
    key_y = KeyWithLabel(aKeyboard, "y");
    ret.iWidth = (float32) key_y->iWidth;
    ret.iHeight = (float32) key_y->iHeight;
    key_h = KeyWithLabel(aKeyboard, "h");
    key_n = KeyWithLabel(aKeyboard, "n");
    
    CentralPositionOfKey(key_y, &x, &y_y);
    CentralPositionOfKey(key_h, &x, &y_h);
    CentralPositionOfKey(key_n, &x, &y_n);
    
    ret.iDivider1 = (y_y + y_h)/2;
    ret.iDivider2 = (y_h + y_n)/2;

    name = NameOfKeyboard(aKeyboard);
    if (utf8_strstr(name, "QWERTY") != NULL) {
        ret.iFilterType = QWERTY;
    } else {
        ret.iFilterType = OTHER;
    }
    return ret;
}

