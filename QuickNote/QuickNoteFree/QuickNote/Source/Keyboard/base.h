#ifndef _QI_BASE_H__
#define _QI_BASE_H__

#include <stdlib.h>
#include <wchar.h>

//#define __OBJC__

#ifdef __cplusplus
extern "C"{
#endif
    
typedef char                  int8;
typedef unsigned char         uint8;
typedef short				  int16;
typedef unsigned short        uint16;
typedef int					  int32;
typedef unsigned int          uint32;
typedef long long             int64;
typedef unsigned long long    uint64;
typedef float                 float32;
typedef double                float64;
    
#ifndef __OBJC__
typedef int BOOL;
#endif
    
#ifndef FALSE
#define FALSE 0
#define TRUE (! FALSE)
#endif
    
#ifndef TRUE
#define FALSE 0
#define TRUE (! FALSE)
#endif
    
#ifndef NULL
#define NULL 0
#endif
    
enum CodePage {
    ASCII,
    UTF8
};
    
typedef enum Orientation {
    PORTRAIT,
    LANDSCAPE
} Orientation;
	
typedef enum SmartEdtingStatus {
    ON,
    OFF
} SmartEdtingStatus;
    
typedef struct SamplePoint {
    double iPosX;
    double iPosY;
    double iTimestamp;
} SamplePoint;

    
typedef struct InputSignal {
    uint32 iNumOfSamplePoints;
    SamplePoint* iSamplePoints;
} InputSignal;
    
    
#define MAX_WORD_LENGTH 100
    
#define MAX_KEYBOARD_NAME_LEN 256
    
typedef enum KeyType {
    KEY_SYMBOL_NULL,
    KEY_SYMBOL_ENTER,
    KEY_SYMBOL_SPACE,
    KEY_SYMBOL_TAB,
    KEY_SYMBOL_SHIFT,
    KEY_SYMBOL_CTRL,
    KEY_SYMBOL_ALT,
    KEY_SYMBOL_CAPS_LOCK,
    KEY_SYMBOL_ESCAPE,
    KEY_SYMBOL_MENU,
    KEY_SYMBOL_SWITCH_KEYBOARD,
    KEY_SYMBOL_ALPHA,
    KEY_SYMBOL_NUMBER,
    KEY_SYMBOL_PUNCTUATION,
    KEY_SYMBOL_FUNCTION,
    KEY_SYMBOL_BACKSPACE,
    KEY_SYMBOL_123,
    KEY_SYMBOL_ABC

} KeyType;
    
typedef enum KeyStatus {
    KEY_PRESSED,
    KEY_RELEASED,
    KEY_NORMAL
} KeyStatus;
    
#define MAX_COUNT_CANDIDATES 15
    
    
#define KEY_ID_START 10000
#define KEY_ID_A KEY_ID_START + 1
#define KEY_ID_B KEY_ID_START + 2
#define KEY_ID_C KEY_ID_START + 3
#define KEY_ID_D KEY_ID_START + 4
#define KEY_ID_E KEY_ID_START + 5
#define KEY_ID_F KEY_ID_START + 6
#define KEY_ID_G KEY_ID_START + 7
#define KEY_ID_H KEY_ID_START + 8
#define KEY_ID_I KEY_ID_START + 9
#define KEY_ID_J KEY_ID_START + 10
#define KEY_ID_K KEY_ID_START + 11
#define KEY_ID_L KEY_ID_START + 12
#define KEY_ID_M KEY_ID_START + 13
#define KEY_ID_N KEY_ID_START + 14
#define KEY_ID_O KEY_ID_START + 15
#define KEY_ID_P KEY_ID_START + 16
#define KEY_ID_Q KEY_ID_START + 17
#define KEY_ID_R KEY_ID_START + 18
#define KEY_ID_S KEY_ID_START + 19
#define KEY_ID_T KEY_ID_START + 20
#define KEY_ID_U KEY_ID_START + 21
#define KEY_ID_V KEY_ID_START + 22
#define KEY_ID_W KEY_ID_START + 23
#define KEY_ID_X KEY_ID_START + 24
#define KEY_ID_Y KEY_ID_START + 25
#define KEY_ID_Z KEY_ID_START + 26
    
#define KEY_ID_1 KEY_ID_START + 27
#define KEY_ID_2 KEY_ID_START + 28
#define KEY_ID_3 KEY_ID_START + 29
#define KEY_ID_4 KEY_ID_START + 30
#define KEY_ID_5 KEY_ID_START + 31
#define KEY_ID_6 KEY_ID_START + 32
#define KEY_ID_7 KEY_ID_START + 33
#define KEY_ID_8 KEY_ID_START + 34
#define KEY_ID_9 KEY_ID_START + 35
#define KEY_ID_0 KEY_ID_START + 36

#define KEY_ID_BACKSPACE KEY_ID_START + 37
#define KEY_ID_FULL_STOP KEY_ID_START + 38
#define KEY_ID_COMMA KEY_ID_START + 39
#define KEY_ID_QUESTION_MARK KEY_ID_START + 40
#define KEY_ID_SHIFT KEY_ID_START + 41
#define KEY_ID_ALT_LABEL KEY_ID_START + 42
#define KEY_ID_SPACE KEY_ID_START + 43
#define KEY_ID_123 KEY_ID_START + 44
#define KEY_ID_AT KEY_ID_START + 45
#define KEY_ID_LANGUAGE KEY_ID_START + 46
#define KEY_ID_MENU KEY_ID_START + 47
#define KEY_ID_ENTER KEY_ID_START + 48

#define KEY_ID_FENHAO KEY_ID_START + 49
#define KEY_ID_MAOHAO KEY_ID_START + 50
#define KEY_ID_GANTANHAO KEY_ID_START + 51
#define KEY_ID_ABC KEY_ID_START + 52
#define KEY_ID_LEFT_KUOHAO KEY_ID_START + 53
#define KEY_ID_RIGHT_KUOHAO KEY_ID_START + 54
#define KEY_ID_LEFT_DAKUOHAO KEY_ID_START + 55
#define KEY_ID_RIGHT_DAKUOHAO KEY_ID_START + 56
#define KEY_ID_STAR KEY_ID_START + 57
#define KEY_ID_ALT KEY_ID_START + 58
#define KEY_ID_LEFT_ZHONGKUOHAO KEY_ID_START + 59
#define KEY_ID_RIGHT_ZHONGKUOHAO KEY_ID_START + 60
#define KEY_ID_DIVIDER KEY_ID_START + 61
#define KEY_ID_PLUS KEY_ID_START + 62
#define KEY_ID_DENGHAO KEY_ID_START + 63
#define KEY_ID_UNDER_LINE KEY_ID_START + 64
#define KEY_ID_RIGHT_SLASH KEY_ID_START + 65
#define KEY_ID_LEFT_SLASH KEY_ID_START + 66
#define KEY_ID_LINE KEY_ID_START + 67
#define KEY_ID_DOUBLE_YINGHAO KEY_ID_START + 68
#define KEY_ID_SINGLE_YINGHAO KEY_ID_START + 69
#define KEY_ID_DOLLAR KEY_ID_START + 70
#define KEY_ID_YUAN KEY_ID_START + 71
#define KEY_ID_LEFT_JIANKUOHAO KEY_ID_START + 72
#define KEY_ID_RIGHT_JIANKUOHAO KEY_ID_START + 73
#define KEY_ID_RIGHT_EUROPEAN KEY_ID_START + 74
#define KEY_ID_PERCENTAGE KEY_ID_START + 75
#define KEY_ID_TOP_ARROW KEY_ID_START + 76
#define KEY_ID_BOLANGHAO KEY_ID_START + 77


    


    
        
#ifdef __cplusplus
}
#endif

#endif // End of _QI_BASE_H__

