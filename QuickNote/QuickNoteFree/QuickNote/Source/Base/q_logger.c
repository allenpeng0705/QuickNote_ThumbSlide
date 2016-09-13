#define MODULE_NAME "q_logger.c"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "q_logger.h"
#include "q_util.h"

#ifdef QI_DEBUG 
#define USE_EXCLUSIVE_LIST  1

#if USE_EXCLUSIVE_LIST
const static char EXCLUSIVE_MODULES[][60] =
{

    "" /* End */
};

const static char EXCLUSIVE_FUNCTION[][60] =
{

    "" /* End */
};
#endif /* USE_EXCLUSIVE_LIST */

#define MAX_MESSAGE_BUFFER_LENGTH   (512 + 1)
static char message_buffer[MAX_MESSAGE_BUFFER_LENGTH];
static unsigned int message_caret;

int log_check(const int aLevel, const char* aModuleName, const char* aFuncName)
{
#if USE_EXCLUSIVE_LIST
    int i;
    int module_count = sizeof(EXCLUSIVE_MODULES) / 60;
    int func_count = sizeof(EXCLUSIVE_FUNCTION) / 60;
#endif
    if (aLevel < QI_LOG_LEVEL_CHECK) return 0;

#if USE_EXCLUSIVE_LIST
    for (i = 0; i < module_count - 1; i++) {
        if (strcmp(aModuleName, EXCLUSIVE_MODULES[i]) == 0) return 0;
    }

    for (i = 0; i < func_count - 1; i++) {
        if (strcmp(aFuncName, EXCLUSIVE_FUNCTION[i]) == 0) return 0;
    }
#endif
    return 1;
}

void log_prefix(const char* aModuleName, const char* aFuncName, int aLine)
{
    memset(message_buffer, 0, MAX_MESSAGE_BUFFER_LENGTH);
    message_caret = 0;
    sprintf(message_buffer, "[%s]%s:", aModuleName, aFuncName);
    message_caret += strlen(aModuleName) + strlen(aFuncName) + 3;
}

void log_message(const char* aFormatString, ...)
{
    va_list ap;
    va_start(ap, aFormatString);
    vsprintf(message_buffer+message_caret, aFormatString, ap);
    va_end(ap);
}

void log_postfix()
{
    FILE* f = NULL;
//    printf(message_buffer);
//    printf("\n");
    char* path = nativePath("log.txt", FALSE);
    f = fopen(path, "a+");
	if (f == NULL) return;
    fwrite(message_buffer, strlen(message_buffer),1,f);
    fwrite("\n", 1, 1,f);
    fflush(f);
    fclose(f);
}

void log_printOut(char* aMsg)
{
    printf("%s", aMsg);
    printf("\n");
}

#else

int  log_check(const int aLevel, const char* aModuleName, const char* aFuncName){return 0;}
void log_prefix(const char* aModuleName, const char* aFuncName, int aLine){}
void log_message(const char* aFormatString, ...){}
void log_postfix(){}
void log_printOut(char* aMsg){}

#endif

