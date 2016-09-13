#include "q_main_punc.h"

#include "q_main_number.h"
#include "q_keyboard_private.h"

static Key keys_of_main_punc[32] = {
    // The first row of Keyboard
    {
        "[",
        NULL,
        "[",
        NULL,
        NULL,
        KEY_ID_LEFT_ZHONGKUOHAO,
        0,
        0,
        40,
        52,
        "[",
        "[",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "]",
        NULL,
        "]",
        NULL,
        NULL,
        KEY_ID_RIGHT_ZHONGKUOHAO,
        40,
        0,
        40,
        52,
        "]",
        "]",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "{",
        NULL,
        "{",
        NULL,
        NULL,
        KEY_ID_LEFT_DAKUOHAO,
        80,
        0,
        40,
        52,
        "{",
        "{",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "}",
        NULL,
        "}",
        NULL,
        NULL,
        KEY_ID_RIGHT_DAKUOHAO,
        120,
        0,
        40,
        52,
        "}",
        "}",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "+",
        NULL,
        "+",
        NULL,
        NULL,
        KEY_ID_PLUS,
        160,
        0,
        40,
        52,
        "+",
        "+",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "-",
        NULL,
        "-",
        NULL,
        NULL,
        KEY_ID_DIVIDER,
        200,
        0,
        40,
        52,
        "-",
        "-",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    	
    {
        "*",
        NULL,
        "*",
        NULL,
        NULL,
        KEY_ID_STAR,
        240,
        0,
        40,
        52,
        "*",
        "*",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },

    {
        "/",
        NULL,
        "/",
        NULL,
        NULL,
        KEY_ID_RIGHT_SLASH,
        280,
        0,
        40,
        52,
        "/",
        "/",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },

    // Second Row
    {
        "=",
        NULL,
        "=",
        NULL,
        NULL,
        KEY_ID_DENGHAO,
        0,
        52,
        40,
        52,
        "=",
        "=",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "\\",
        NULL,
        "\\",
        NULL,
        NULL,
        KEY_ID_LEFT_SLASH,
        40,
        52,
        40,
        52,
        "\\",
        "\\",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "|",
        NULL,
        "|",
        NULL,
        NULL,
        KEY_ID_LINE,
        80,
        52,
        40,
        52,
        "|",
        "|",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "\"",
        NULL,
        "\"",
        NULL,
        NULL,
        KEY_ID_DOUBLE_YINGHAO,
        120,
        52,
        40,
        52,
        "\"",
        "\"",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "\'",
        NULL,
        "\'",
        NULL,
        NULL,
        KEY_ID_SINGLE_YINGHAO,
        160,
        52,
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
        "_",
        NULL,
        "_",
        NULL,
        NULL,
        KEY_ID_UNDER_LINE,
        200,
        52,
        40,
        52,
        "_",
        "_",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },

    {
        "<",
        NULL,
        "<",
        NULL,
        NULL,
        KEY_ID_LEFT_JIANKUOHAO,
        240,
        52,
        40,
        52,
        "<",
        "<",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        ">",
        NULL,
        ">",
        NULL,
        NULL,
        KEY_ID_RIGHT_JIANKUOHAO,
        280,
        52,
        40,
        52,
        ">",
        ">",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },

    // Third row
    // The fourth row of the keyboard
    {
        "~",
        NULL,
        "~",
        NULL,
        NULL,
        KEY_ID_BOLANGHAO,
        0,
        104,
        40,
        52,
        "~",
        "~",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "￥",
        NULL,
        "￥",
        NULL,
        NULL,
        KEY_ID_YUAN,
        40,
        104,
        40,
        52,
        "￥",
        "￥",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "€",
        NULL,
        "€",
        NULL,
        NULL,
        KEY_ID_RIGHT_EUROPEAN,
        80,
        104,
        40,
        52,
        "€",
        "€",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "$",
        NULL,
        "$",
        NULL,
        NULL,
        KEY_ID_DOLLAR,
        120,
        104,
        40,
        52,
        "$",
        "$",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        ":",
        NULL,
        ":",
        NULL,
        NULL,
        KEY_ID_MAOHAO,
        160,
        104,
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
        "%",
        NULL,
        "%",
        NULL,
        NULL,
        KEY_ID_PERCENTAGE,
        200,
        104,
        40,
        52,
        "%",
        "%",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "^",
        NULL,
        "^",
        NULL,
        NULL,
        KEY_ID_TOP_ARROW,
        240,
        104,
        40,
        52,
        "^",
        "^",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "?",
        NULL,
        "?",
        NULL,
        NULL,
        KEY_ID_QUESTION_MARK,
        280,
        104,
        40,
        52,
        "?",
        "?",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },

    //FOURTH ROW
    {
        "123",
        NULL,
        "123",
        NULL,
        NULL,
        KEY_ID_123,
        0,
        156,
        40,
        52,
        NULL,
        NULL,
        NULL,
        FALSE,
        KEY_SYMBOL_123,
        KEY_NORMAL        
    },
    
    {
        "ABC",
        NULL,
        "ABC",
        NULL,
        NULL,
        KEY_ID_ABC,
        40,
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
        ",",
        NULL,
        ",",
        NULL,
        NULL,
        KEY_ID_COMMA,
        80,
        156,
        40,
        52,
        ",",
        ",",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        ".",
        NULL,
        ".",
        NULL,
        NULL,
        KEY_ID_FULL_STOP,
        120,
        156,
        40,
        52,
        ".",
        ".",
        NULL,
        FALSE,
        KEY_SYMBOL_PUNCTUATION,
        KEY_NORMAL        
    },
    
    {
        "!",
        NULL,
        "!",
        NULL,
        NULL,
        KEY_ID_GANTANHAO,
        160,
        156,
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
        ";",
        NULL,
        ";",
        NULL,
        NULL,
        KEY_ID_FENHAO,
        200,
        156,
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
        "@",
        NULL,
        "@",
        NULL,
        NULL,
        KEY_ID_AT,
        240,
        156,
        40,
        52,
        "@",
        "@",
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

static Keyboard punc_keyboard = {
    "PUNC",
    "number_symbols.png",
    0,
    0,
    320,
    208,
    32,
    keys_of_main_punc,
    -1,
    FALSE,
    FALSE
};

Keyboard* g_punc_keyboard = &punc_keyboard;

