#ifndef _QI_UTF8_H__
#define _QI_UTF8_H__

#include "base.h"

#ifdef __cplusplus
extern "C"{
#endif

extern uint32 utf8_strlen(const char *s);
extern char*  utf8_strcpy(char *dest, const char *src);
extern char*  utf8_strcpy_safe(char *dest, int dest_size, const char *src);
extern int32  utf8_char_at(const char *s, uint32 idx);
extern uint32 utf8_offset(const char *s, int32 idx);
extern int32  utf8_multibytetowidechar(enum CodePage page, const char* src, uint32 srclen, wchar_t* dst, uint32 dstlen);
extern int32  utf8_widechartomultibyte(enum CodePage page, const wchar_t* src, uint32 srclen, char* dst, int32 dstlen);
extern uint32 utf8_strlenw(const wchar_t* str);
extern int32  utf8_strcmp(const char *s1, const char *s2);
extern int32  utf8_stricmp(const char *s1, const char *s2);
extern int32  utf8_strncmp(const char *s1, const char *s2, int32 n);
extern char*  utf8_strchr(const char *s, int32 c);
extern char*  utf8_strtok(char *s, const char *set);
extern char*  utf8_strtok_r(char *s, const char *set, char **last);
extern char*  utf8_strstr(const char *s1, const char *s2);
extern int32  utf8_toupper(int32 c);
extern int32  utf8_tolower(int32 c);
extern void utf8_set_char_at(char *str, int32 index, int32 c);
extern int32 utf8_strnicmp(const char *s1, const char *s2, int32 n);
extern int32 utf8_isalpha(int32 c);

#ifdef __cplusplus
}
#endif

#endif // _QI_UTF8_H__
