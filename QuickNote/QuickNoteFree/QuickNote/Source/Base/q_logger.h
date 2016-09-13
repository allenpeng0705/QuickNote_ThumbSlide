#ifndef _QI_LOGGER_H_
#define _QI_LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define QI_DEBUG

#define QI_LOG_LEVEL_TRACE -1
#define QI_LOG_LEVEL_DEBUG 0
#define QI_LOG_LEVEL_INFO  1
#define QI_LOG_LEVEL_WARN  2
#define QI_LOG_LEVEL_ERR 3

#define QI_LOG_LEVEL_CHECK QI_LOG_LEVEL_TRACE

#ifndef MODULE_NAME
#ifdef __MODULE__
#define MODULE_NAME __MODULE__
#else
#define MODULE_NAME __FILE__
#endif
#endif

#ifdef QI_DEBUG
/* Logging MACRO */
#define QI_LOG_LEVEL(aLevel, aFunction, aParams)                       \
    {                                                                   \
        if (log_check(aLevel, MODULE_NAME, #aFunction) == 1) {          \
            log_prefix(MODULE_NAME, #aFunction, __LINE__);              \
            log_message aParams;                                        \
            log_postfix();                                              \
        }                                                               \
    }                                                                   \

#else
#define QI_LOG_LEVEL(aLevel, aFunction, aParams) 
#endif  

#define QI_LOG_DEBUG(aFunction, aParams)                               \
        QI_LOG_LEVEL(SWI_LOG_LEVEL_DEBUG, aFunction, aParams)

#define QI_LOG(aFunction, aParams)                                     \
        QI_LOG_LEVEL(SWI_LOG_LEVEL_INFO, aFunction, aParams)

#define QI_LOG_TRACE(aFunction, aParams)                               \
        QI_LOG_LEVEL(SWI_LOG_LEVEL_TRACE, aFunction, aParams)

#define QI_LOG_WARN(aFunction, aParams)                                \
        QI_LOG_LEVEL(SWI_LOG_LEVEL_WARN, aFunction, aParams)

#define QI_LOG_ERR(aFunction, aParams)                                 \
        QI_LOG_LEVEL(SWI_LOG_LEVEL_ERR, aFunction, aParams)

#define QI_LOG_DUMP_TEXT(aBuffer)                               \
        log_printOut(aBuffer);

int  log_check(const int aLevel, const char* aModuleName, const char* aFuncName);
void log_prefix(const char* aModuleName, const char* aFuncName, int aLine);
void log_message(const char* aFormatString, ...);
void log_postfix();
void log_printOut(char* aMsg);

#ifdef __cplusplus
}
#endif

#endif
