/*
 * string.c
 * 
 * Inherited from PSXSDK C library
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

int tolower(int chr)
{
    return (chr >='A' && chr<='Z') ? (chr + 32) : (chr);    
}

int toupper(int chr)
{
    return (chr >='a' && chr<='z') ? (chr - 32) : (chr);    
}

// Need to be replaced with MIPS assembler equivalents

void *memchr(void *s , int c , int n)
{
	while(n--)
	{
		if(*((unsigned char*)s) == (unsigned char)c)
			return s;
		
		s++;
	}
	
	return NULL;
}

char *strncpy(char *dst, const char *src, int len)
{
	char *odst=dst;

	while(*src && len)
	{
		*(dst++) = *(src++);
		len--;
	}
	
	*dst = 0;
	
	return odst;
}

char *strcpy(char *dst, const char *src)
{
	char *odst = dst;

	while(*(dst++) = *(src++));
	return odst;
}

char *strcat(char *dst, const char *src)
{
	char *o=dst;
	
	while(*dst)
		dst++;
	
	strcpy(dst, src);
	
	return o;
}

char *strncat(char *s, const char *append, int len)
{
	char *o=s;
	
	while(*s)
		s++;
	
	strncpy(s, append, len);
	
	return o;
}

int strlen(const char *str)
{
	int i = 0;
	while(*(str++))i++;
	return i;
}

char *strchr(const char *s, int c)
{
	int x;

	for(x = 0; x <= strlen(s); x++)
		if(s[x] == c) return (char*)&s[x];

	return NULL;
}

char *strrchr(const char *s, int c)
{
	int x;

	for(x = strlen(s); x>=0; x--)
		if(s[x] == c) return (char*)&s[x];

	return NULL;
}

char *strpbrk(const char *s, const char *charset)
{
	int x,y;

	for(x = 0; x < strlen(s); x++)
		for(y = 0; y < strlen(charset); y++)
			if(s[x] == charset[y]) return (char*)&s[x];

	return NULL;
}

char *strstr(const char *big, const char *little)
{
	int ls = strlen(little);
	int bs = strlen(big);
	int x;

	if(ls == 0)
		return (char*)big;
	
	if(ls > bs)
		return NULL;

	for(x = 0; x <= bs-ls; x++)
		if(memcmp(little, &big[x], ls) == 0)
			return (char*)&big[x];

	return NULL;
}

int strcmp(const char *s1, const char *s2)
{
	while((*s1) && (*s2) && (*s1 == *s2))
	{
		s1++;
		s2++;
	}

	return(*s1-*s2);
}

int strncmp(const char *s1, const char *s2, int len)
{
	int p = 0;

	while(*s1 && *s2 && (*s1 == *s2) && p<len)
	{
		p++;

		if(p<len)
		{
			s1++;
			s2++;
		}
	}

	return *s1-*s2;
}

// Requires a malloc implementation
char *strdup(const char *str)
{
	char *ns = (void*)malloc(strlen(str) + 1);

	if(ns == NULL)
		return NULL;
	
	strcpy(ns, str);
	return ns;
}

char *strndup(const char *str, int len)
{
	int n=strlen(str);
	char *ns = (void*)malloc((n+1)>len?len:(n+1));

	if(ns == NULL)
		return NULL;
	
	strncpy(ns, str, (n+1)>len?len:(n+1));
	return ns;
}
	
long long strtoll(const char *nptr, char **endptr, int base)
{
	int r = 0;
	int t = 0;
	int n = 0;
	
	if(*nptr == '-')
	{
		nptr++;
		n = 1;
	}

	if(base == 0)
		if(*nptr == '0')
			base = 8;
		else
			base = 10;

	if(!(base >= 2 && base <= 36))
		return 0;

	if(base == 16 && *nptr == '0')
	{
		if(*(nptr+1) == 'x' || *(nptr+1) == 'X')
			nptr+=2;
	}

	while(*nptr)
	{
		switch(*nptr)
		{
			case '0'...'9':
				t = *nptr - '0';
			break;
			case 'a' ... 'z':
				t = (*nptr - 'a') + 10;
			break;
			case 'A' ... 'Z':
				t = (*nptr - 'A') + 10;
			break;
			default:
				t = 1000;
			break;
		}

		if(t>=base)
			break;

		r*=base;
		r+=t;
		nptr++;
	}

	if(endptr)*endptr = (char*)nptr;
	return n?-r:r;
}

long strtol(const char *nptr, char **endptr, int base)
{
	return (long)strtoll(nptr, endptr, base);
}

double strtod(const char *nptr, char **endptr)
{
	char strbuf[64];
	int x = 0;
	int y;
	double i=0, d=0;
	int s=1;

	if(*nptr == '-')
	{
		nptr++;
		s=-1;
	}

	while(*nptr >= '0' && *nptr <= '9' && x < 18)
		strbuf[x++] = *(nptr++);

	strbuf[x] = 0;

	i = (double)strtoll(strbuf, NULL, 10);

	if(*nptr == '.')
	{
		nptr++;
		x = 0;

		while(*nptr >= '0' && *nptr <= '9' && x < 7)
			strbuf[x++] = *(nptr++);

		strbuf[x] = 0;
		
		if(endptr != NULL) *endptr = (char*)nptr;

		y=1;

		for(x=0;x<strlen(strbuf);x++)
			y*=10;

		d = (double)strtoll(strbuf, NULL, 10);
		d /= y;
	}
	else
	{
		if(endptr != NULL)
			*endptr = (char*)nptr;
	}
	
	return (i + d)*s;
}

/* implementation by Lameguy64, behaves like OpenWatcom's strtok() */
/* BIOS strtok seemed either bugged, or designed for wide chars */

static char *_strtok_curpos;
static char *_strtok_endpos;

char *strtok( char *s1, char *s2 )
{
	char *c,*t;
	
	if( s1 )
	{
		_strtok_curpos = s1;
		_strtok_endpos = s1+strlen( s1 );
	}
	else
	{
		if( _strtok_curpos >= _strtok_endpos )
			return( NULL );
	}
	
	if( !*_strtok_curpos )
		return( NULL );
	
	if( c = strstr( _strtok_curpos, s2 ) )
	{
		*c = 0;
		t = _strtok_curpos;
		_strtok_curpos = c+1;
		return( t );
	}
	else
	{
		t = _strtok_curpos;
		_strtok_curpos += strlen( t );
		return( t );
	}
	
	return( NULL );
	
} /* strtok */

long double strtold(const char *nptr, char **endptr)
{
	return (long double)strtod(nptr, endptr);
}

float strtof(const char *nptr, char **endptr)
{
	return (float)strtod(nptr, endptr);
}