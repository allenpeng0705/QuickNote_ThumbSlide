#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "utf8_string.h"

/* Number of following bytes in sequence based on first byte value (for bytes above 0x7f) */
static const char kUTF8_LENGTH[128] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xa0-0xaf */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xb0-0xbf */
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0xc0-0xcf */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0xd0-0xdf */
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xe0-0xef */
    3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0  /* 0xf0-0xff */
};

/* First byte mask depending on UTF-8 sequence length */
static const unsigned char kUTF8_MASK[4] = { 0x7f, 0x1f, 0x0f, 0x07 };

/* Minimum Unicode value depending on UTF-8 sequence length */
static const uint32 kUTF8_MINVAL[4] = { 0x0, 0x80, 0x800, 0x10000 };


/**********************************************************************************/
/* utf8_width:                                                                    */
/* Returns the width of a UTF-8 character in string.                              */
/**********************************************************************************/
static int utf8_char_in_string_width(const char *s)
{
    int c = *((unsigned char *)s);
    int n = 1;

    if (c & 0x80) 
    {
        while (c & (0x80 >> n))
        {
            ++ n;
        }
    }

    return n;
}


/**********************************************************************************/
/* utf8_char_width:                                                               */
/* Returns the width of a UTF-8 character.                                        */
/**********************************************************************************/
static int32 utf8_char_width(int32 c)
{
	int32 size, bits, b;

	if ( c < 128 )
	{
		return 1;
	}

	bits = 7;
	while ( c >= ( 1 << bits ) )
	{
		bits++;
	}
	size = 2;
	b = 11;

	while ( b < bits ) 
	{
		++ size;
		b += 5;
	}

	return size;
}

/**************************************************************************************/
/* utf8_getchar_advance:															  */
/* Reads and returns a character from a UTF-8 string, advancing the pointer position. */
/**************************************************************************************/
static int32 utf8_getchar_advance( const char **s )
{
	int32 c = *((unsigned char *)((*s)++));
	int32 n, t;

	if ( c & 0x80 )
	{
		n = 1;
		while ( c & ( 0x80 >> n ) )
		{
			++ n;
		}

		c &= ( 1 << ( 8 - n ) ) - 1;

		while ( --n > 0 ) 
		{
			t = *((unsigned char *)((*s)++));

			if ((!( t & 0x80 )) || ( t & 0x40 )) 
			{
				-- ( *s );
				return '^';
			}

			c = ( c << 6 ) | ( t & 0x3F );
		}
	}
	return c;
}

/**************************************************************************************/
/* utf8_getchar:                                                                      */
/* Reads a character from a UTF-8 string, this does not advance input pointer.        */
/**************************************************************************************/
static int32 utf8_getchar( const char *s )
{
	int32 c = *((unsigned char *)(s++));
	int32 n, t;

	if ( c & 0x80 ) 
	{
		n = 1;
		while ( c & ( 0x80 >> n ) )
		{
			n++;
		}
		c &= ( 1 << ( 8 - n ) ) - 1;

		while ( --n > 0 ) 
		{
			t = *((unsigned char *)( s++ ));

			if ( ( !( t & 0x80 )) || ( t & 0x40 ) )
			{
				return '^';
			}
			c = ( c <<6 ) | ( t & 0x3F );
		}
	}

	return c;
}

/**************************************************************************************/
/* utf8_setchar:                                                                      */
/* Sets a character in a UTF-8 string.                                                */
/**************************************************************************************/
static int32 utf8_setchar(char *s, int32 c)
{
	int32 size, bits, b, i;

	if ( c < 128 ) 
	{
		*s = c;
		return 1;
	}

	bits = 7;
	while ( c >= ( 1 << bits ) )
	{
		bits++;
	}

	size = 2;
	b = 11;

	while ( b < bits )
	{
		++ size;
		b += 5;
	}

	b -= ( 7 - size );
	s[0] = c >> b;

	for ( i = 0; i < size; ++ i )
	{
		s[0] |= ( 0x80 >> i );
	}

	for ( i = 1; i < size; ++ i ) 
	{
		b -= 6;
		s[i] = 0x80 | ( ( c >> b ) & 0x3F );
	}

	return size;
}


/**************************************************************************************/
/* utf8_str_size:                                                                     */
/* Returns the size of the specified string in bytes, including the                   */
/* trailing zero.                                                                     */
/**************************************************************************************/
static int32 utf8_str_size(const char *s)
{
    const char *orig = s;
    
    if(!s)
    {
        return 0;
    }

    do 
    {
    } while (utf8_getchar_advance(&s) != 0);

    return (uint32)s - (uint32)orig;
}

/**************************************************************************************/
/* utf8_setchar_at:                                                                   */
/* Modifies the character at the specified index within the string,                   */
/* handling adjustments for variable width data. Returns how far the                  */
/* rest of the string was moved.                                                      */
/**************************************************************************************/
static int32 utf8_setchar_at(char *s, int32 index, int32 c)
{
    int32 oldw, neww;

    s += utf8_offset(s, index);

    oldw = utf8_char_in_string_width(s);
    neww = utf8_char_width(c);

    if (oldw != neww)
    {
        memmove(s + neww, s + oldw, (uint32)utf8_str_size(s + oldw));
    }
    utf8_setchar(s, c);

    return neww-oldw;
}

/**************************************************************************************/
/* utf8_mbstowcs: UTF-8 to wide char string conversion                                */
/* return -1 on dst buffer overflow, -2 on invalid input char                         */
/**************************************************************************************/
static int32 utf8_mbstowcs( const char *src, int32 srclen, wchar_t *dst, int32 dstlen )
{
    int32 len;
    uint32 res;
    const char *srcend = src + srclen;
    wchar_t *dstend    = dst + dstlen;

    if ( !dstlen ) 
    {
        return utf8_strlen( src );
    }

    while ((dst < dstend) && (src < srcend))
    {
        unsigned char ch = *src++;
        if (ch < 0x80)  /* special fast case for 7-bit ASCII */
        {
            *dst++ = ch;
            continue;
        }
        len = kUTF8_LENGTH[ch-0x80];
        if (src + len > srcend) goto bad;
        res = ch & kUTF8_MASK[len];

        switch(len)
        {
        case 3:
            if ((ch = *src ^ 0x80) >= 0x40) goto bad;
            res = (res << 6) | ch;
            src++;
        case 2:
            if ((ch = *src ^ 0x80) >= 0x40) goto bad;
            res = (res << 6) | ch;
            src++;
        case 1:
            if ((ch = *src ^ 0x80) >= 0x40) goto bad;
            res = (res << 6) | ch;
            src++;
            if (res < kUTF8_MINVAL[len]) goto bad;
            if (res > 0x10ffff) goto bad;
            if (res <= 0xffff) *dst++ = res;
            else /* we need surrogates */
            {
                if (dst == dstend - 1) return -1;  /* overflow */
                res -= 0x10000;
                *dst++ = 0xd800 | (res >> 10);
                *dst++ = 0xdc00 | (res & 0x3ff);
            }
            continue;
        }
bad:
        return -2;  /* bad char */
        /* otherwise ignore it */
    }
    if (src < srcend) return -1;  /* overflow */
    return dstlen - (dstend - dst);
}

/**************************************************************************************/
/* utf8_get_value_surrogate                                                           */
/* get the next char value taking surrogates into account                             */
/**************************************************************************************/
static uint32 utf8_get_value_surrogate( const wchar_t *src, uint32 srclen )
{
    if (src[0] >= 0xd800 && src[0] <= 0xdfff)  /* surrogate pair */
    {
        if (src[0] > 0xdbff || /* invalid high surrogate */
            srclen <= 1 ||     /* missing low surrogate */
            src[1] < 0xdc00 || src[1] > 0xdfff) /* invalid low surrogate */
        {
            return 0;
        }
        return 0x10000 + ((src[0] & 0x3ff) << 10) + (src[1] & 0x3ff);
    }
    return src[0];
}

/**************************************************************************************/
/* utf8_get_wcs_to_utf8_length                                                        */
/* query destination string length given wchar string                                 */
/**************************************************************************************/
static int32 utf8_get_wcs_to_utf8_length( const wchar_t *src, uint32 srclen )
{
    int32 len;
    uint32 val;

    for (len = 0; srclen; srclen--, src++)
    {
        if (*src < 0x80)  /* 0x00-0x7f: 1 byte */
        {
            len++;
            continue;
        }
        if (*src < 0x800)  /* 0x80-0x7ff: 2 bytes */
        {
            len += 2;
            continue;
        }
        if (!(val = utf8_get_value_surrogate( src, srclen )))
        {
            return -2;
        }
        if (val < 0x10000)  /* 0x800-0xffff: 3 bytes */
        {
            len += 3;
        }
        else   /* 0x10000-0x10ffff: 4 bytes */
        {
            len += 4;
        }
    }
    return len;
}

/**************************************************************************************/
/* utf8_wcstombs: wide char to UTF-8 string conversion                                */
/* return -1 on dst buffer overflow, -2 on invalid input char                         */
/**************************************************************************************/
static int32 utf8_wcstombs( const wchar_t *src, int32 srclen, char *dst, int32 dstlen )
{
    int32 len;

    if (!dstlen)
    {
        return utf8_get_wcs_to_utf8_length( src, srclen );
    }

    for (len = dstlen; srclen; srclen--, src++)
    {
        wchar_t ch = *src;
        uint32 val;

        if (ch < 0x80)  /* 0x00-0x7f: 1 byte */
        {
            if (!len--) return -1;  /* overflow */
            *dst++ = (char)ch;
            continue;
        }

        if (ch < 0x800)  /* 0x80-0x7ff: 2 bytes */
        {
            if ((len -= 2) < 0) return -1;  /* overflow */
            dst[1] = 0x80 | (ch & 0x3f);
            ch >>= 6;
            dst[0] = 0xc0 | ch;
            dst += 2;
            continue;
        }

        if (!(val = utf8_get_value_surrogate( src, srclen )))
        {
            return -2;
        }

        if (val < 0x10000)  /* 0x800-0xffff: 3 bytes */
        {
            if ((len -= 3) < 0) return -1;  /* overflow */
            dst[2] = 0x80 | (val & 0x3f);
            val >>= 6;
            dst[1] = 0x80 | (val & 0x3f);
            val >>= 6;
            dst[0] = 0xe0 | val;
            dst += 3;
        }
        else   /* 0x10000-0x10ffff: 4 bytes */
        {
            if ((len -= 4) < 0) return -1;  /* overflow */
            dst[3] = 0x80 | (val & 0x3f);
            val >>= 6;
            dst[2] = 0x80 | (val & 0x3f);
            val >>= 6;
            dst[1] = 0x80 | (val & 0x3f);
            val >>= 6;
            dst[0] = 0xf0 | val;
            dst += 4;
        }
    }
    return dstlen - len;
}

/**************************************************************************************/
/* utf8_strlen:                                                                        */
/* return the number of UTF-8 encoded characters in given s.                          */
/**************************************************************************************/
uint32 utf8_strlen( const char *s )
{
	uint32 c = 0;

	while ( utf8_getchar_advance( &s ) )
	{
		c++;
	}

	return c;
}

/**************************************************************************************/
/* utf8_strcpy:                                                                        */
/* Unicode version replacement of strcpy handles UTF-8 encoding.                      */
/**************************************************************************************/
char*  utf8_strcpy( char *dest, const char *src )
{
	return utf8_strcpy_safe( dest, INT_MAX, src );
}

/**************************************************************************************/
/* utf8_strcpy_safe:                                                                   */
/* Unicode version replacement of strcpy handles UTF-8 encoding with boundary check.  */
/**************************************************************************************/
char *utf8_strcpy_safe( char *dest, int32 dest_size, const char *src )
{
	int32 pos = 0;
	int32 c;

	dest_size -= utf8_char_width( 0 );

	while (( c = utf8_getchar_advance( &src ) ) != 0 ) 
	{
		dest_size -= utf8_char_width( c );
		if ( dest_size < 0 )
		{
			break;
		}

		pos += utf8_setchar( dest + pos, c );
	}

	utf8_setchar( dest + pos, 0 );

	return dest;
}

/**************************************************************************************/
/* utf8_char_at:                                                                       */
/* Returns the n-th UTF-8 encoded characters in s. Idx is n.                          */
/**************************************************************************************/
int32 utf8_char_at( const char *s, uint32 idx )
{
	return utf8_getchar( s + utf8_offset( s, idx ) );
}

/**************************************************************************************/
/* utf8_offset:													            		  */
/* Returns the offset in bytes from the start of the string to the                    */
/* character at the specified index. If the index is negative, counts                 */
/* backward from the end of the string (-1 returns an offset to the                   */
/* last character).																      */
/**************************************************************************************/
uint32 utf8_offset( const char *s, int32 idx )
{
	const char *orig = s;
	const char *last;

	if ( idx < 0 )
	{
		idx += utf8_strlen( s );
	}

	while ( idx-- > 0 ) 
	{
		last = s;
		if ( !utf8_getchar_advance( &s ) ) 
		{
			s = last;
			break;
		}
	}

	return (uint32)s - (uint32)orig;
}

/**************************************************************************************/
/* utf8_multibytetowidechar                                                */
/* Convert given UTF-8 Multibyte or default ASCII format to wchar_string              */
/**************************************************************************************/
int32 utf8_multibytetowidechar( enum CodePage page, const char* src, uint32 srclen, wchar_t* dst, uint32 dstlen )
{
    int32 ret;

    if (!src || (!dst && dstlen))
    {
        // ERROR INVALID PARAMETER
        return 0;
    }

    if (srclen <= 0)
    {
        srclen = strlen(src) + 1;
    }

    switch(page)
    {
    case UTF8:
        ret = utf8_mbstowcs( src, srclen, dst, dstlen );
        break;

    case ASCII:
    default:
        ret = mbstowcs( dst, src, srclen );
        break;
    }

    // if ret value is -1, ERROR INSUFFICIENT BUFFER!
    // if ret value is -2, ERROR NO UNICODE TRANSLATION!
	dst[srclen - 1] = (wchar_t)0;
	return ret;
}

/**************************************************************************************/
/* utf8_widechartomultibyte                                                */
/* Convert given wchar_string to UTF-8 Multibyte or default ASCII format              */
/**************************************************************************************/
int32 utf8_widechartomultibyte( enum CodePage page, const wchar_t* src, uint32 srclen, char* dst, int32 dstlen )
{
    int32 ret;

    if (!src || (!dst && dstlen))
    {
        // Error Invalid Parameter
        return 0;
    }

    if (srclen <= 0)
    {
        srclen = utf8_strlenw(src) + 1;
    }

    switch(page)
    {
    case UTF8:
        ret = utf8_wcstombs( src, srclen, dst, dstlen );
        break;

    case ASCII:
    default:
        ret = wcstombs( dst, src, srclen );
        break;
    }

    // if ret value is -1, ERROR INSUFFICIENT BUFFER!
    // if ret value is -2, ERROR NO UNICODE TRANSLATION!
    return ret;
}

/**************************************************************************************/
/* utf8_strlenw                                                            */
/* Calculate the wchar string length, this is created to compensate Linux platform    */
/**************************************************************************************/
uint32 utf8_strlenw( const wchar_t* str )
{
    const wchar_t *eos = str;

    while( *eos++ ) ;

    return( eos - str - 1 );
}


/**************************************************************************************/
/* utf8_strcmp:                                                                        */
/* Unicode-aware version of the ANSI strcmp() function.                               */
/**************************************************************************************/
int32 utf8_strcmp(const char *s1, const char *s2)
{
    int32 c1, c2;
    if ( !s1 || !s2 )
    {
        return 0;
    }

    for (;;) 
    {
        c1 = utf8_getchar_advance(&s1);
        c2 = utf8_getchar_advance(&s2);

        if (c1 != c2)
        {
            return c1 - c2;
        }
        if (!c1)
        {
            return 0;
        }
    }
}

int32  utf8_stricmp(const char *s1, const char *s2)
{
    int32 c1, c2;
	int32 c3, c4;
    if ( !s1 || !s2 )
    {
        return 0;
    }

    for (;;) 
    {
        c1 = utf8_getchar_advance(&s1);
        c2 = utf8_getchar_advance(&s2);

		c3 = utf8_tolower(c1);
		c4 = utf8_tolower(c2);

        if (c3 != c4)
        {
            return c3 - c4;
        }
        if (!c1)
        {
            return 0;
        }
    }
}

int32 utf8_strnicmp(const char *s1, const char *s2, int32 n)
{
    int32 c1, c2;
	int32 c3, c4;

    if ((!s1 || !s2) || (n <= 0))
    {
        return 0;
    }

    for (;;) 
    {
        c1 = utf8_getchar_advance(&s1);
        c2 = utf8_getchar_advance(&s2);

		c3 = utf8_tolower(c1);
		c4 = utf8_tolower(c2);		

        if (c3 != c4)
        {
            return c3 - c4;
        }
        if ((!c1) || (--n <= 0))
        {
            return 0;
        }
    }
}


/**************************************************************************************/
/* utf8_strncmp:                                                                       */
/* Unicode-aware version of the ANSI strncmp() function.                              */
/**************************************************************************************/
int32 utf8_strncmp(const char *s1, const char *s2, int32 n)
{
    int32 c1, c2;

    if ((!s1 || !s2) || (n <= 0))
    {
        return 0;
    }

    for (;;) 
    {
        c1 = utf8_getchar_advance(&s1);
        c2 = utf8_getchar_advance(&s2);

        if (c1 != c2)
        {
            return c1 - c2;
        }
        if ((!c1) || (--n <= 0))
        {
            return 0;
        }
    }
}

/**************************************************************************************/
/* utf8_strchr:                                                                        */
/* Unicode-aware version of the ANSI strchr() function.                               */
/**************************************************************************************/
char *utf8_strchr(const char *s, int32 c)
{
    int32 d;

    while ((d = utf8_getchar(s)) != 0) 
    {
        if (c == d)
            return (char *)s;

        s += utf8_char_in_string_width(s);
    }

    if (!c)
    {
        return (char *)s;
    }
    return NULL;
}

/**************************************************************************************/
/* utf8_strtok:                                                                        */
/* Unicode-aware version of the ANSI strtok() function.                               */
/**************************************************************************************/
char *utf8_strtok(char *s, const char *set)
{
    static char *last = NULL;

    return utf8_strtok_r(s, set, &last);
}

/**************************************************************************************/
/* utf8_strtok_r:                                                                      */
/* Unicode-aware version of the strtok_r() function.                                  */
/**************************************************************************************/
char *utf8_strtok_r(char *s, const char *set, char **last)
{
    char *prev_str, *tok;
    const char *setp;
    int32 c, sc;

    if (!last)
    {
        return NULL;
    }

    if (!s)
    {
        s = *last;

        if (!s)
        {
            return NULL;
        }
    }

skip_leading_delimiters:

    prev_str = s;
    c = utf8_getchar_advance((const char **)(&s));

    setp = set;

    while ((sc = utf8_getchar_advance(&setp)) != 0) 
    {
        if (c == sc)
        {
            goto skip_leading_delimiters;
        }
    }

    if (!c) 
    {
        *last = NULL;
        return NULL;
    }

    tok = prev_str;

    for (;;) 
    {
        prev_str = s;
        c = utf8_getchar_advance((const char **)(&s));

        setp = set;

        do 
        {
            sc = utf8_getchar_advance(&setp);
            if (sc == c) 
            {
                if (!c) 
                {
                    *last = NULL;
                    return tok;
                }
                else 
                {
                    s += utf8_setchar_at(prev_str, 0, 0);
                    *last = s;
                    return tok;
                }
            }
        } while (sc);
    }
}


/**************************************************************************************/
/* utf8_strstr:                                                                        */
/* Unicode-aware version of the ANSI strstr() function.                               */
/**************************************************************************************/
char *utf8_strstr(const char *s1, const char *s2)
{
    int32 len;
    if (!s1 || !s2)
    {
        return NULL;
    }

    len = utf8_strlen(s2);
    while (utf8_getchar(s1)) 
    {
        if (utf8_strncmp(s1, s2, len) == 0)
        {
            return (char *)s1;
        }
        s1 += utf8_char_in_string_width(s1);
    }

    return NULL;
}

/**************************************************************************************/
/* utf8_toupper:                                                                       */
/* Unicode-aware version of the ANSI toupper() function.                              */
/**************************************************************************************/
int32 utf8_toupper(int32 c)
{
	if (c == 8364) {	// handle european money symbol. The release version optimized the disassemble, it cann't handle the european mondy symbol appropriately.
		return c;
	}
    if ((c >= 97 && c <= 122) ||
        (c >= 224 && c <= 246) ||
        (c >= 248 && c <= 254) ||
        (c >= 945 && c <= 961) ||
        (c >= 963 && c <= 971) ||
        (c >= 1072 && c <= 1103))
    {
        return c + -32;
    }
    if ((c >= 598 && c <= 599))
    {
        return c + -205;
    }
    if ((c >= 650 && c <= 651))
    {
        return c + -217;
    }
    if ((c >= 941 && c <= 943))
    {
        return c + -37;
    }
    if ((c >= 973 && c <= 974))
    {
        return c + -63;
    }
    if ((c >= 1105 && c <= 1116) ||
        (c >= 1118 && c <= 1119))
    {
        return c + -80;
    }
    if ((c >= 1377 && c <= 1414))
    {
        return c + -48;
    }
    if ((c >= 7936 && c <= 7943) ||
        (c >= 7952 && c <= 7957) ||
        (c >= 7968 && c <= 7975) ||
        (c >= 7984 && c <= 7991) ||
        (c >= 8000 && c <= 8005) ||
        (c >= 8032 && c <= 8039) ||
        (c >= 8064 && c <= 8071) ||
        (c >= 8080 && c <= 8087) ||
        (c >= 8096 && c <= 8103) ||
        (c >= 8112 && c <= 8113) ||
        (c >= 8144 && c <= 8145) ||
        (c >= 8160 && c <= 8161))
    {
        return c + 8;
    }
    if ((c >= 8048 && c <= 8049))
    {
        return c + 74;
    }
    if ((c >= 8050 && c <= 8053))
    {
        return c + 86;
    }
    if ((c >= 8054 && c <= 8055))
    {
        return c + 100;
    }
    if ((c >= 8056 && c <= 8057))
    {
        return c + 128;
    }
    if ((c >= 8058 && c <= 8059))
    {
        return c + 112;
    }
    if ((c >= 8060 && c <= 8061))
    {
        return c + 126;
    }
    if ((c >= 8560 && c <= 8575))
    {
        return c + -16;
    }
    if ((c >= 9424 && c <= 9449))
    {
        return c + -26;
    }

    switch (c) {
      case 255:
          return c + 121;
      case 257:
      case 259:
      case 261:
      case 263:
      case 265:
      case 267:
      case 269:
      case 271:
      case 273:
      case 275:
      case 277:
      case 279:
      case 281:
      case 283:
      case 285:
      case 287:
      case 289:
      case 291:
      case 293:
      case 295:
      case 297:
      case 299:
      case 301:
      case 303:
      case 307:
      case 309:
      case 311:
      case 314:
      case 316:
      case 318:
      case 320:
      case 322:
      case 324:
      case 326:
      case 328:
      case 331:
      case 333:
      case 335:
      case 337:
      case 339:
      case 341:
      case 343:
      case 345:
      case 347:
      case 349:
      case 351:
      case 353:
      case 355:
      case 357:
      case 359:
      case 361:
      case 363:
      case 365:
      case 367:
      case 369:
      case 371:
      case 373:
      case 375:
      case 378:
      case 380:
      case 382:
      case 387:
      case 389:
      case 392:
      case 396:
      case 402:
      case 409:
      case 417:
      case 419:
      case 421:
      case 424:
      case 429:
      case 432:
      case 436:
      case 438:
      case 441:
      case 445:
      case 453:
      case 456:
      case 459:
      case 462:
      case 464:
      case 466:
      case 468:
      case 470:
      case 472:
      case 474:
      case 476:
      case 479:
      case 481:
      case 483:
      case 485:
      case 487:
      case 489:
      case 491:
      case 493:
      case 495:
      case 498:
      case 501:
      case 507:
      case 509:
      case 511:
      case 513:
      case 515:
      case 517:
      case 519:
      case 521:
      case 523:
      case 525:
      case 527:
      case 529:
      case 531:
      case 533:
      case 535:
      case 995:
      case 997:
      case 999:
      case 1001:
      case 1003:
      case 1005:
      case 1007:
      case 1121:
      case 1123:
      case 1125:
      case 1127:
      case 1129:
      case 1131:
      case 1133:
      case 1135:
      case 1137:
      case 1139:
      case 1141:
      case 1143:
      case 1145:
      case 1147:
      case 1149:
      case 1151:
      case 1153:
      case 1169:
      case 1171:
      case 1173:
      case 1175:
      case 1177:
      case 1179:
      case 1181:
      case 1183:
      case 1185:
      case 1187:
      case 1189:
      case 1191:
      case 1193:
      case 1195:
      case 1197:
      case 1199:
      case 1201:
      case 1203:
      case 1205:
      case 1207:
      case 1209:
      case 1211:
      case 1213:
      case 1215:
      case 1218:
      case 1220:
      case 1224:
      case 1228:
      case 1233:
      case 1235:
      case 1237:
      case 1239:
      case 1241:
      case 1243:
      case 1245:
      case 1247:
      case 1249:
      case 1251:
      case 1253:
      case 1255:
      case 1257:
      case 1259:
      case 1263:
      case 1265:
      case 1267:
      case 1269:
      case 1273:
      case 7681:
      case 7683:
      case 7685:
      case 7687:
      case 7689:
      case 7691:
      case 7693:
      case 7695:
      case 7697:
      case 7699:
      case 7701:
      case 7703:
      case 7705:
      case 7707:
      case 7709:
      case 7711:
      case 7713:
      case 7715:
      case 7717:
      case 7719:
      case 7721:
      case 7723:
      case 7725:
      case 7727:
      case 7729:
      case 7731:
      case 7733:
      case 7735:
      case 7737:
      case 7739:
      case 7741:
      case 7743:
      case 7745:
      case 7747:
      case 7749:
      case 7751:
      case 7753:
      case 7755:
      case 7757:
      case 7759:
      case 7761:
      case 7763:
      case 7765:
      case 7767:
      case 7769:
      case 7771:
      case 7773:
      case 7775:
      case 7777:
      case 7779:
      case 7781:
      case 7783:
      case 7785:
      case 7787:
      case 7789:
      case 7791:
      case 7793:
      case 7795:
      case 7797:
      case 7799:
      case 7801:
      case 7803:
      case 7805:
      case 7807:
      case 7809:
      case 7811:
      case 7813:
      case 7815:
      case 7817:
      case 7819:
      case 7821:
      case 7823:
      case 7825:
      case 7827:
      case 7829:
      case 7841:
      case 7843:
      case 7845:
      case 7847:
      case 7849:
      case 7851:
      case 7853:
      case 7855:
      case 7857:
      case 7859:
      case 7861:
      case 7863:
      case 7865:
      case 7867:
      case 7869:
      case 7871:
      case 7873:
      case 7875:
      case 7877:
      case 7879:
      case 7881:
      case 7883:
      case 7885:
      case 7887:
      case 7889:
      case 7891:
      case 7893:
      case 7895:
      case 7897:
      case 7899:
      case 7901:
      case 7903:
      case 7905:
      case 7907:
      case 7909:
      case 7911:
      case 7913:
      case 7915:
      case 7917:
      case 7919:
      case 7921:
      case 7923:
      case 7925:
      case 7927:
      case 7929:
          return c + -1;
      case 305:
          return c + -232;
      case 383:
          return c + -300;
      case 454:
      case 457:
      case 460:
      case 499:
          return c + -2;
      case 477:
      case 1010:
          return c + -79;
      case 595:
          return c + -210;
      case 596:
          return c + -206;
      case 601:
          return c + -202;
      case 603:
          return c + -203;
      case 608:
          return c + -205;
      case 611:
          return c + -207;
      case 616:
          return c + -209;
      case 617:
      case 623:
          return c + -211;
      case 626:
          return c + -213;
      case 629:
          return c + -214;
      case 640:
      case 643:
      case 648:
          return c + -218;
      case 658:
          return c + -219;
      case 837:
          return c + 84;
      case 940:
          return c + -38;
      case 962:
          return c + -31;
      case 972:
          return c + -64;
      case 976:
          return c + -62;
      case 977:
          return c + -57;
      case 981:
          return c + -47;
      case 982:
          return c + -54;
      case 1008:
          return c + -86;
      case 1009:
          return c + -80;
      case 7835:
          return c + -59;
      case 8017:
      case 8019:
      case 8021:
      case 8023:
          return c + 8;
      case 8115:
      case 8131:
      case 8179:
          return c + 9;
      case 8126:
          return c + -7205;
      case 8165:
          return c + 7;
      default:
          return c;
    }
}

/**************************************************************************************/
/* utf8_tolower:                                                                       */
/* Unicode-aware version of the ANSI tolower() function.                              */
/**************************************************************************************/
int32 utf8_tolower(int32 c)
{
	if (c == 8364) {
		return c;
	}
    if ((c >= 65 && c <= 90) ||
        (c >= 192 && c <= 214) ||
        (c >= 216 && c <= 222) ||
        (c >= 913 && c <= 929) ||
        (c >= 931 && c <= 939) ||
        (c >= 1040 && c <= 1071))
    {
        return c + 32;
    }
    if ((c >= 393 && c <= 394))
    {
        return c + 205;
    }
    if ((c >= 433 && c <= 434))
    {
        return c + 217;
    }
    if ((c >= 904 && c <= 906))
    {
        return c + 37;
    }
    if ((c >= 910 && c <= 911))
    {
        return c + 63;
    }
    if ((c >= 1025 && c <= 1036) ||
        (c >= 1038 && c <= 1039))
    {
        return c + 80;
    }
    if ((c >= 1329 && c <= 1366) ||
        (c >= 4256 && c <= 4293))
    {
        return c + 48;
    }
    if ((c >= 7944 && c <= 7951) ||
        (c >= 7960 && c <= 7965) ||
        (c >= 7976 && c <= 7983) ||
        (c >= 7992 && c <= 7999) ||
        (c >= 8008 && c <= 8013) ||
        (c >= 8040 && c <= 8047) ||
        (c >= 8072 && c <= 8079) ||
        (c >= 8088 && c <= 8095) ||
        (c >= 8104 && c <= 8111) ||
        (c >= 8120 && c <= 8121) ||
        (c >= 8152 && c <= 8153) ||
        (c >= 8168 && c <= 8169))
    {
        return c + -8;
    }
    if ((c >= 8122 && c <= 8123))
    {
        return c + -74;
    }
    if ((c >= 8136 && c <= 8139))
    {
        return c + -86;
    }
    if ((c >= 8154 && c <= 8155))
    {
        return c + -100;
    }
    if ((c >= 8170 && c <= 8171))
    {
        return c + -112;
    }
    if ((c >= 8184 && c <= 8185))
    {
        return c + -128;
    }
    if ((c >= 8186 && c <= 8187))
    {
        return c + -126;
    }
    if ((c >= 8544 && c <= 8559))
    {
        return c + 16;
    }
    if ((c >= 9398 && c <= 9423))
    {
        return c + 26;
    }

    switch (c) {
      case 256:
      case 258:
      case 260:
      case 262:
      case 264:
      case 266:
      case 268:
      case 270:
      case 272:
      case 274:
      case 276:
      case 278:
      case 280:
      case 282:
      case 284:
      case 286:
      case 288:
      case 290:
      case 292:
      case 294:
      case 296:
      case 298:
      case 300:
      case 302:
      case 306:
      case 308:
      case 310:
      case 313:
      case 315:
      case 317:
      case 319:
      case 321:
      case 323:
      case 325:
      case 327:
      case 330:
      case 332:
      case 334:
      case 336:
      case 338:
      case 340:
      case 342:
      case 344:
      case 346:
      case 348:
      case 350:
      case 352:
      case 354:
      case 356:
      case 358:
      case 360:
      case 362:
      case 364:
      case 366:
      case 368:
      case 370:
      case 372:
      case 374:
      case 377:
      case 379:
      case 381:
      case 386:
      case 388:
      case 391:
      case 395:
      case 401:
      case 408:
      case 416:
      case 418:
      case 420:
      case 423:
      case 428:
      case 431:
      case 435:
      case 437:
      case 440:
      case 444:
      case 453:
      case 456:
      case 459:
      case 461:
      case 463:
      case 465:
      case 467:
      case 469:
      case 471:
      case 473:
      case 475:
      case 478:
      case 480:
      case 482:
      case 484:
      case 486:
      case 488:
      case 490:
      case 492:
      case 494:
      case 498:
      case 500:
      case 506:
      case 508:
      case 510:
      case 512:
      case 514:
      case 516:
      case 518:
      case 520:
      case 522:
      case 524:
      case 526:
      case 528:
      case 530:
      case 532:
      case 534:
      case 994:
      case 996:
      case 998:
      case 1000:
      case 1002:
      case 1004:
      case 1006:
      case 1120:
      case 1122:
      case 1124:
      case 1126:
      case 1128:
      case 1130:
      case 1132:
      case 1134:
      case 1136:
      case 1138:
      case 1140:
      case 1142:
      case 1144:
      case 1146:
      case 1148:
      case 1150:
      case 1152:
      case 1168:
      case 1170:
      case 1172:
      case 1174:
      case 1176:
      case 1178:
      case 1180:
      case 1182:
      case 1184:
      case 1186:
      case 1188:
      case 1190:
      case 1192:
      case 1194:
      case 1196:
      case 1198:
      case 1200:
      case 1202:
      case 1204:
      case 1206:
      case 1208:
      case 1210:
      case 1212:
      case 1214:
      case 1217:
      case 1219:
      case 1223:
      case 1227:
      case 1232:
      case 1234:
      case 1236:
      case 1238:
      case 1240:
      case 1242:
      case 1244:
      case 1246:
      case 1248:
      case 1250:
      case 1252:
      case 1254:
      case 1256:
      case 1258:
      case 1262:
      case 1264:
      case 1266:
      case 1268:
      case 1272:
      case 7680:
      case 7682:
      case 7684:
      case 7686:
      case 7688:
      case 7690:
      case 7692:
      case 7694:
      case 7696:
      case 7698:
      case 7700:
      case 7702:
      case 7704:
      case 7706:
      case 7708:
      case 7710:
      case 7712:
      case 7714:
      case 7716:
      case 7718:
      case 7720:
      case 7722:
      case 7724:
      case 7726:
      case 7728:
      case 7730:
      case 7732:
      case 7734:
      case 7736:
      case 7738:
      case 7740:
      case 7742:
      case 7744:
      case 7746:
      case 7748:
      case 7750:
      case 7752:
      case 7754:
      case 7756:
      case 7758:
      case 7760:
      case 7762:
      case 7764:
      case 7766:
      case 7768:
      case 7770:
      case 7772:
      case 7774:
      case 7776:
      case 7778:
      case 7780:
      case 7782:
      case 7784:
      case 7786:
      case 7788:
      case 7790:
      case 7792:
      case 7794:
      case 7796:
      case 7798:
      case 7800:
      case 7802:
      case 7804:
      case 7806:
      case 7808:
      case 7810:
      case 7812:
      case 7814:
      case 7816:
      case 7818:
      case 7820:
      case 7822:
      case 7824:
      case 7826:
      case 7828:
      case 7840:
      case 7842:
      case 7844:
      case 7846:
      case 7848:
      case 7850:
      case 7852:
      case 7854:
      case 7856:
      case 7858:
      case 7860:
      case 7862:
      case 7864:
      case 7866:
      case 7868:
      case 7870:
      case 7872:
      case 7874:
      case 7876:
      case 7878:
      case 7880:
      case 7882:
      case 7884:
      case 7886:
      case 7888:
      case 7890:
      case 7892:
      case 7894:
      case 7896:
      case 7898:
      case 7900:
      case 7902:
      case 7904:
      case 7906:
      case 7908:
      case 7910:
      case 7912:
      case 7914:
      case 7916:
      case 7918:
      case 7920:
      case 7922:
      case 7924:
      case 7926:
      case 7928:
          return c + 1;
      case 304:
          return c + -199;
      case 376:
          return c + -121;
      case 385:
          return c + 210;
      case 390:
          return c + 206;
      case 398:
          return c + 79;
      case 399:
          return c + 202;
      case 400:
          return c + 203;
      case 403:
          return c + 205;
      case 404:
          return c + 207;
      case 406:
      case 412:
          return c + 211;
      case 407:
          return c + 209;
      case 413:
          return c + 213;
      case 415:
          return c + 214;
      case 422:
      case 425:
      case 430:
          return c + 218;
      case 439:
          return c + 219;
      case 452:
      case 455:
      case 458:
      case 497:
          return c + 2;
      case 902:
          return c + 38;
      case 908:
          return c + 64;
      case 8025:
      case 8027:
      case 8029:
      case 8031:
          return c + -8;
      case 8124:
      case 8140:
      case 8188:
          return c + -9;
      case 8172:
          return c + -7;
      default:
          return c;
    }
}

/*
 * <function>
 * <header>utf8_unicode.h</header>
 * </function>
 */
void utf8_set_char_at(char *str, int32 index, int32 c)
{
	utf8_setchar_at(str, index, c);
}

int32 utf8_isalpha(int32 c)
{
	int32 lower, upper;
	lower = utf8_tolower(c);
	upper = utf8_toupper(c);
	if(lower != upper || c == 0xA7 /*0xC2A7*/|| c == 0xDF /*0xC39F*/)
	{
		return 1;
	}
	else
		return 0;
}

