
#include "q_main_number.h"
#include "q_keyboard_private.h"

static Key keys_of_main_number[22] = {
    // The first row of Keyboard
    {
        ";",
        NULL,
        ";",
        NULL,
        NULL,
        KEY_ID_FENHAO,
        0,
        0,
        40,
        52,
        ";",
        ";",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "1",
        NULL,
        "1",
        NULL,
        NULL,
        KEY_ID_1,
        40,
        0,
        80,
        52,
        "1",
        "1",
        NULL,
        FALSE,
        KEY_SYMBOL_NUMBER,
        KEY_NORMAL        
    },
    
    {
        "2",
        NULL,
        "2",
        NULL,
        NULL,
        KEY_ID_2,
        120,
        0,
        80,
        52,
        "2",
        "2",
        NULL,
        FALSE,
        KEY_SYMBOL_NUMBER,
        KEY_NORMAL        
    },
    
    {
        "3",
        NULL,
        "3",
        NULL,
        NULL,
        KEY_ID_3,
        200,
        0,
        80,
        52,
        "3",
        "3",
        NULL,
        FALSE,
        KEY_SYMBOL_NUMBER,
        KEY_NORMAL        
    },
    
    {
        "@",
        NULL,
        "@",
        NULL,
        NULL,
        KEY_ID_AT,
        280,
        0,
        40,
        52,
        "@",
        "@",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
        
    //Second row of keyboard	
    {
        ":",
        NULL,
        ":",
        NULL,
        NULL,
        KEY_ID_MAOHAO,
        0,
        52,
        40,
        52,
        ":",
        ":",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "4",
        NULL,
        "4",
        NULL,
        NULL,
        KEY_ID_4,
        40,
        52,
        80,
        52,
        "4",
        "4",
        NULL,
        FALSE,
        KEY_SYMBOL_NUMBER,
        KEY_NORMAL        
    },
    
    {
        "5",
        NULL,
        "5",
        NULL,
        NULL,
        KEY_ID_5,
        120,
        52,
        80,
        52,
        "5",
        "5",
        NULL,
        FALSE,
        KEY_SYMBOL_NUMBER,
        KEY_NORMAL        
    },
    
    {
        "6",
        NULL,
        "6",
        NULL,
        NULL,
        KEY_ID_6,
        200,
        52,
        80,
        52,
        "6",
        "6",
        NULL,
        FALSE,
        KEY_SYMBOL_NUMBER,
        KEY_NORMAL        
    },
    
    {
        "#",
        NULL,
        "#",
        NULL,
        NULL,
        KEY_ID_G,
        280,
        52,
        40,
        52,
        "#",
        "#",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
        
    // Third row of keyboard
    {
        "!",
        NULL,
        "!",
        NULL,
        NULL,
        KEY_ID_GANTANHAO,
        0,
        105,
        40,
        52,
        "!",
        "!",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "7",
        NULL,
        "7",
        NULL,
        NULL,
        KEY_ID_7,
        40,
        105,
        80,
        52,
        "7",
        "7",
        NULL,
        FALSE,
        KEY_SYMBOL_NUMBER,
        KEY_NORMAL        
    },
    
    {
        "8",
        NULL,
        "8",
        NULL,
        NULL,
        KEY_ID_8,
        120,
        105,
        80,
        52,
        "8",
        "8",
        NULL,
        FALSE,
        KEY_SYMBOL_NUMBER,
        KEY_NORMAL        
    },
    
    {
        "9",
        NULL,
        "9",
        NULL,
        NULL,
        KEY_ID_9,
        200,
        105,
        80,
        52,
        "9",
        "9",
        NULL,
        FALSE,
        KEY_SYMBOL_NUMBER,
        KEY_NORMAL        
    },
    
    {
        "&",
        NULL,
        "&",
        NULL,
        NULL,
        KEY_ID_V,
        280,
        105,
        40,
        52,
        "&",
        "&",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
        
    // The fourth row of the keyboard
    {
        "ABC",
        NULL,
        "ABC",
        NULL,
        NULL,
        KEY_ID_ABC,
        0,
        156,
        40,
        52,
        NULL,
        NULL,
        NULL,
        FALSE,
        KEY_SYMBOL_ABC,
        KEY_NORMAL        
    },
    
    {
        "*+=",
        NULL,
        "*+=",
        NULL,
        NULL,
        KEY_ID_ALT,
        40,
        156,
        40,
        52,
        NULL,
        NULL,
        NULL,
        FALSE,
        KEY_SYMBOL_ALT,
        KEY_NORMAL        
    },
    
    {
        "\'",
        NULL,
        "\'",
        NULL,
        NULL,
        KEY_ID_SINGLE_YINGHAO,
        80,
        156,
        40,
        52,
        "\'",
        "\'",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "0",
        NULL,
        "0",
        NULL,
        NULL,
        KEY_ID_0,
        120,
        156,
        80,
        52,
        "0",
        "0",
        NULL,
        FALSE,
        KEY_SYMBOL_NUMBER,
        KEY_NORMAL        
    },
    
    {
        "(",
        NULL,
        "(",
        NULL,
        NULL,
        KEY_ID_LEFT_KUOHAO,
        200,
        156,
        40,
        52,
        "(",
        "(",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        ")",
        NULL,
        ")",
        NULL,
        NULL,
        KEY_ID_RIGHT_KUOHAO,
        240,
        156,
        40,
        52,
        ")",
        ")",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },        

        
    {
        "Del",
        NULL,
        "Del",
        NULL,
        NULL,
        KEY_ID_BACKSPACE,
        280,
        156,
        40,
        52,
        NULL,
        NULL,
        NULL,
        FALSE,
        KEY_SYMBOL_BACKSPACE,
        KEY_NORMAL        
    }
};

static Keyboard number_keyboard = {
    "NUMBER",
    "number_symbols.png",
    0,
    0,
    320,
    208,
    22,
    keys_of_main_number,
    -1,
    FALSE,
    FALSE
};

Keyboard* g_number_keyboard = &number_keyboard;


