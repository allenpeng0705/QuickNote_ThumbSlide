
#include "q_english.h"

static char* nameOfLexicons[2] = {
   "english.ldb",
    NULL
};

static char* nameOfMajorKeyboards[7] = {
    "ThumbMode",
    "NormalMode",
    "TraceMode",
    "TraceLeftMode",
    "NUMBER",
    "PUNC",
    NULL
};

static int32 lenOfWordInLexicon[2] = {
    0,
    0
};

static RecognizerInfo english = {
    "ENGLISH",
    nameOfLexicons,
    NULL,
    NULL,
    nameOfMajorKeyboards,
    lenOfWordInLexicon,
    6,
    1
};

RecognizerInfo* g_english = &english;
