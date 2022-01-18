// vsscanf
// Programmed by Giuseppe Gatta, 2011
// Inherited from PSXSDK C library

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// Uncomment to enable support for %f.
//#define ALLOW_FLOAT

char libc_vsscanf_buf[512];
char libc_vsscanf_allow[256];

enum
{
	elem_skip_space = 1,
};

int libc_vsscanf_get_element(char *dst, const char *src, int flag, int s)
{
	int i;
	const char *osrc = src;

	if(flag & elem_skip_space)
	{
		while(*src == ' ')
			src++;
	}

	for(i=0;i<s;i++)
	{
		if((flag & elem_skip_space) && *src == ' ')
			break;

		if(*src != 0)
			*(dst++) = *(src++);
		else
			break;
	}

	*dst = 0;

	return src - osrc;
}

enum
{
	scanf_s_char, scanf_s_short, scanf_s_int,
	scanf_s_long,
	scanf_s_long_long
};

int vsscanf(const char *str, const char *format, va_list ap)
{
	int fp = 0;
	int sp = 0;
	int conv = 0;
	int sz = scanf_s_int; // size for numbers defaults to 32-bit
	int i,x,y,z, h;
	int suppress = 0;
	int neg = 0;
	int fsz = 512;
	int def_fsz = 1;
	int exspace = 0;
	int alt = 0;
	int r = 0;
	int exit_loop=0;
	int sp2,sp3;
	char *ep;
	long long buf;
	double fbuf;


	while(format[fp] && str[sp] && !exit_loop)
	{
		if(conv)
		{
			switch(format[fp])
			{
				case '%': // Percent, assignment does not occur
					conv = 0;
				break;
				
				case 'h': // Halve size
					sz--;
				break;

				case 'l': // Double size
					sz++;
				break;

				case '*': // Suppress
					suppress = 1;
				break;

				case ' ': // Explicit space
					exspace = 1;
				break;

				case '#': // Alternate format
					alt = 1;
				break;

				case '0' ... '9': // '0' ... '9' is a GNU C extension!
					if(def_fsz)
					{
						def_fsz = 0;
						fsz = 0;
					}

					fsz *= 10;
					fsz+=format[fp]-'0';

					if(fsz > 512)
						fsz = 512; // 512 is the maximum.
				break;
				
				case '@': // Binary. Non-standard extension
					libc_vsscanf_get_element(libc_vsscanf_buf, &str[sp], elem_skip_space, fsz);
					buf = strtoll(libc_vsscanf_buf, &ep, 2);
					sp += ep - libc_vsscanf_buf;

					if(!suppress)
					{
						switch(sz)
						{
							case scanf_s_char: *(va_arg(ap, signed char*)) = (signed char)buf;break;
							case scanf_s_short: *(va_arg(ap, short*)) = (short)buf; break;
							case scanf_s_int: *(va_arg(ap, int*)) = (int)buf; break;
							case scanf_s_long: *(va_arg(ap, long*)) = (long)buf; break;
							case scanf_s_long_long: *(va_arg(ap, long long*)) = buf; break;
						}
						r++;
					}

					conv = 0;
				break;

				case 'D':
					sz++;
				case 'd': // Decimal
				case 'u':
					libc_vsscanf_get_element(libc_vsscanf_buf, &str[sp], elem_skip_space, fsz);
					buf = strtoll(libc_vsscanf_buf, &ep, 10);
					sp += ep - libc_vsscanf_buf;

					if(!suppress)
					{
						switch(sz)
						{
							case scanf_s_char: *(va_arg(ap, signed char*)) = (signed char)buf;break;
							case scanf_s_short: *(va_arg(ap, short*)) = (short)buf; break;
							case scanf_s_int: *(va_arg(ap, int*)) = (int)buf; break;
							case scanf_s_long: *(va_arg(ap, long*)) = (long)buf; break;
							case scanf_s_long_long: *(va_arg(ap, long long*)) = buf; break;
						}
						r++;
					}

					conv = 0;
				break;

				case 's': // String
					sp += libc_vsscanf_get_element(libc_vsscanf_buf, &str[sp], elem_skip_space, fsz);

					if(!suppress)
					{
						strcpy(va_arg(ap, char*), libc_vsscanf_buf);
						r++;
					}
					
					conv = 0;
				break;

				case 'c': 
					if(def_fsz)
						fsz = 1;

					sp += (i = libc_vsscanf_get_element(libc_vsscanf_buf, &str[sp], (exspace ? elem_skip_space : 0), fsz));
					if(!suppress)
					{
						memcpy(va_arg(ap, char*), libc_vsscanf_buf, (fsz>i)?i:fsz);
						r++;
					}
				break;

				case 'n':
					if(!suppress)
					{
						*(va_arg(ap, int*)) = sp;
						r++;
					}
				break;

				case 'p':
				case 'x':
				case 'X':
					libc_vsscanf_get_element(libc_vsscanf_buf, &str[sp], elem_skip_space, fsz);
					buf = strtoll(libc_vsscanf_buf, &ep, 16);
					sp += ep - libc_vsscanf_buf;

					if(!suppress)
					{
						switch(sz)
						{
							case scanf_s_char: *(va_arg(ap, unsigned char*)) = (unsigned char)buf; break;
							case scanf_s_short:	*(va_arg(ap, unsigned short*)) = (unsigned short)buf;	break;
							case scanf_s_int: *(va_arg(ap, unsigned int*)) = (unsigned int)buf; break;
							case scanf_s_long: *(va_arg(ap, unsigned long*)) = (unsigned long)buf; break;
							case scanf_s_long_long:	*(va_arg(ap, unsigned long long*)) = (unsigned long long)buf; break;
						}
						r++;
					}

					conv = 0;
				break;

				case 'O':
					sz++;
				case 'o': // Octal integer
					libc_vsscanf_get_element(libc_vsscanf_buf, &str[sp], elem_skip_space, fsz);
					buf = strtoll(libc_vsscanf_buf, &ep, 8);
					sp += ep - libc_vsscanf_buf;

					if(!suppress)
					{
						switch(sz)
						{
							case scanf_s_char: *(va_arg(ap, unsigned char*)) = (unsigned char)buf;break;
							case scanf_s_short: *(va_arg(ap, unsigned short*)) = (unsigned short)buf; break;
							case scanf_s_int: *(va_arg(ap, unsigned int*)) = (unsigned int)buf;break;
							case scanf_s_long: *(va_arg(ap, unsigned long*)) = (unsigned long)buf; break;
							case scanf_s_long_long: *(va_arg(ap, unsigned long long*)) = (unsigned long long)buf;break;
						}
						r++;
					}

					conv = 0;
				break;

				case 'i':
					libc_vsscanf_get_element(libc_vsscanf_buf, &str[sp], elem_skip_space, fsz);

					if(libc_vsscanf_buf[0] == '0')
					{
						if(libc_vsscanf_buf[1] == 'x' || libc_vsscanf_buf[1] == 'X')
							i = 16;
						else
							i = 8;
					}
					else
						i = 10;

					buf = strtoll(libc_vsscanf_buf, &ep, i);
					sp += ep - libc_vsscanf_buf;

					if(!suppress)
					{
						switch(sz)
						{
							case scanf_s_char: *(va_arg(ap, signed char*)) = (signed char)buf; break;
							case scanf_s_short: *(va_arg(ap, short*)) = (short)buf; break;
							case scanf_s_int: *(va_arg(ap,  int*)) = (int)buf; break;
							case scanf_s_long: *(va_arg(ap, long*)) = (long)buf; break;
							case scanf_s_long_long: *(va_arg(ap, long long*)) = (long long)buf; break;
						}
						r++;
					}

					conv = 0;
				break;

				case '[':
					i=0;
					x=0; // Exclusion?
					h=0; // Hyphen?

					fp++;
					i++;

					while(format[fp])
					{
						if(format[fp] == '^' && i==1)
						{
							memset(libc_vsscanf_allow, 1, 256);
							x = 1;
							fp++; i++; continue;
						}

						if(x)
						{
							if(format[fp] == ']' && i>=3)
								break;
						}
						else
						{
							if(format[fp] == ']' && i>=2)
								break;
						}

						if(format[fp] == '-')
						{
							if(format[fp+1] != ']')
								y = 1;
							else
								libc_vsscanf_allow['-'] = x^1;
						}
						else
						{
							if(y == 1)
							{
								if(format[fp] < format[fp-2])
									libc_vsscanf_allow[format[fp]] = x^1;
								else
									for(z = format[fp-2]; z <= format[fp]; z++)
										libc_vsscanf_allow[z] = x^1;
									
								y = 0;

								//printf("%s all chars from %c to %c\n", x?"Excluding":"Including",format[fp-2], format[fp]);
							}
							else
								libc_vsscanf_allow[format[fp]] = x^1;
						}

						fp++;
						i++;
					}

// Now as we know what our character set is, let's get data from the string
	/*				puts("Character set:");

					for(y=0;y<16;y++)
					{
						for(x=0;x<16;x++)
							if(libc_vsscanf_allow[(y*16) + x])
								putchar((y*16)+x);
							else
								putchar('*');
				
						putchar('\n');
					}
        */
					i = 0;

					while(libc_vsscanf_allow[str[sp]] && i<512)
						libc_vsscanf_buf[i++] = str[sp++];
					
					libc_vsscanf_buf[i] = 0;

					if(!suppress)
					{
						strcpy(va_arg(ap, char*), libc_vsscanf_buf);
						r++;
					}
				break;

#ifdef ALLOW_FLOAT
				case 'f': // Floating point number
					libc_vsscanf_get_element(libc_vsscanf_buf, &str[sp], elem_skip_space, fsz);
					fbuf = strtod(libc_vsscanf_buf, &ep);
					sp += ep - libc_vsscanf_buf;

					if(!suppress)
					{
						switch(sz)
						{
							case scanf_s_char:
							case scanf_s_short:
							case scanf_s_int: 
								*(va_arg(ap, float*)) = (float)fbuf;
							break;

							case scanf_s_long: 
							case scanf_s_long_long:
								*(va_arg(ap, double*)) = fbuf;
							break;
						}
						r++;
					}

					conv = 0;
				break;
#endif

			}
		}
		else
		{
			if(format[fp] == '%')
			{
				conv = 1;
				neg = 0;
				suppress = 0;
				sz = scanf_s_int;
				fsz = 512;
				def_fsz = 1;
				exspace = 0;
				alt = 0;
				bzero(libc_vsscanf_allow, 256);
				//chset = 0;
			}
			else if(format[fp] != ' ')
			{
				if(format[fp] != str[sp])
					exit_loop=1;

				sp++;
			}

		}

		fp++;
	}

	return r;
}

int sscanf(const char *str, const char *fmt, ...)
{
	int r;
	va_list ap;

	va_start(ap, fmt);
	r = vsscanf(str, fmt, ap);

	va_end(ap);
	return r;
}
