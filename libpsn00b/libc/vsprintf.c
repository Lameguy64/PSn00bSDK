/* printf.c
 *
 * Inherited from the PSXSDK C library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Uncomment to enable support for %f.
//#define ALLOW_FLOAT

#define SPRINTF_ALT_FLAG			(1<<0)
#define SPRINTF_ZERO_FLAG			(1<<1)
#define SPRINTF_NEGFIELD_FLAG			(1<<2)
#define SPRINTF_SPACE_FLAG			(1<<3)
#define SPRINTF_SIGN_FLAG			(1<<4)

// sprintf() macros to calculate the real padding and to write it
// these were made to not repeat the code in the function
// they can only be used in sprintf()

// sprintf macros START

#define calculate_real_padding() \
	y = 1; \
	\
	for(x=0;x<=19;x++) \
	{ \
		if(x == 0) \
			pad_quantity--; \
		else \
		{ \
			if(arg / y) \
				pad_quantity--; \
		} \
	\
		y *= 10; \
	} \
	\
	if(pad_quantity < 0) pad_quantity = 0;

/*#define calculate_real_padding_hex() \
	for (x = 0; x < 8; x++) \
	{ \
		if(x == 0) \
			pad_quantity--; \
		else \
		{ \
		if((arg >> (x * 4)) & 0xf) \
			pad_quantity--; \
		} \
	}*/
	
#define calculate_real_padding_hex() \
	last = 0; \
	for (x = 0; x < 8; x++) \
		if((arg >> (x * 4)) & 0xf) \
			last = x; \
	\
	pad_quantity = (pad_quantity - 1) - last; \
	if(pad_quantity < 0) pad_quantity = 0;

#define calculate_real_padding_bin() \
	last = 0; \
	for (x = 0; x < 32; x++) \
		if((arg >> x) & 1) \
			last = x; \
	\
	pad_quantity = (pad_quantity - 1) - last; \
	if(pad_quantity < 0) pad_quantity = 0;

#define write_padding() \
	if(!(flags & SPRINTF_NEGFIELD_FLAG)) \
		for(x = 0; x < pad_quantity; x++) \
		{ \
			if(flags & SPRINTF_ZERO_FLAG) \
					put_in_string(string, ssz, '0', string_pos++); \
			else \
					put_in_string(string, ssz, ' ', string_pos++); \
		}

#define write_neg_padding() \
	if(flags & SPRINTF_NEGFIELD_FLAG) \
	{ \
		for(x = 0; x < pad_quantity; x++) \
			put_in_string(string, ssz, ' ', string_pos++);\
	}

// sprintf macros END

enum
{
	SPRINTF_SIZE_CHAR,
	SPRINTF_SIZE_SHORT,
	SPRINTF_SIZE_INT,
	SPRINTF_SIZE_LONG,
	SPRINTF_SIZE_LONG_LONG,
};

unsigned int get_arg_in_size(int size, unsigned long *arg, unsigned int check_sign)
{
	int s = 0;

	switch(size)
	{
		case SPRINTF_SIZE_CHAR:
			*arg &= 0xff;
			
			if(check_sign)
			{
				if(*arg & (1<<7))
				{
					*arg |= 0xffffff00;
					*arg = ~(*arg - 1);
					s = 1;
				}
			}
		break;
		case SPRINTF_SIZE_SHORT:
			*arg &= 0xffff;
			
			if(check_sign)
			{
				if(*arg & (1<<15))
				{
					*arg |= 0xffff0000;
					*arg = ~(*arg - 1);
					s = 1;
				}
			}
		break;

// sizeof(long) == sizeof(int) on 32bit, so this will suffice for the psx

		case SPRINTF_SIZE_INT:
		case SPRINTF_SIZE_LONG:
			*arg &= 0xffffffff;

			if(check_sign)
			{
				if(*arg & (1<<31))
				{
					//*arg |= (long long)0xffffffff00000000;
					*arg = ~(*arg - 1);
					s = 1;
				}
			}
		break;

		/*case SPRINTF_SIZE_LONG_LONG:
			if(check_sign)
			{
				if(*arg & ((long long)1<<63))
				{
					*arg = ~(*arg - 1);
					s = 1;
				}
			}
		break;*/
	}
	
	return s;
}	

int put_in_string(char *string, unsigned int sz, char c, int pos)
{
	if(pos>=sz)
		return 0;
	else
		string[pos] = c;
		
	return 1;
}

int libc_ulltoa(unsigned long i, char *dst, int n)
{
	int x, y;
	unsigned long a, b;
	int empty_digit = 1;
	int sp=0;
	int n2=0;

	for(x=20;x>=0;x--)
	{
		a = 1;
		for(y = 0; y<x; y++)
			a *= 10;

		b = (i/a);
					
		if(b>=1)
			empty_digit = 0;
						
		if(empty_digit == 0 || x == 0)
		{	
			i -= b*a;
						
			//put_in_string(string, ssz, b + '0', string_pos++);
			if(n2!=n)
			{
				//printf("n2=%d\n",n2);
				dst[sp++] = b + '0';
				n2++;
			}
		}
	}

	if(n2!=n)dst[sp] = 0;

	return n2;
}

#ifdef ALLOW_FLOAT

void libc_float_to_string(float fl, char *dst, int n)
{
	unsigned int *p = (unsigned int*)&fl;
	unsigned long long i = 0;
	unsigned long long f = 0;
	int e, m, s;
	int x, y;
	unsigned long long z;
	
	s = *p >> 31;

	e = 	(*p >> 23) & 0xff;

	m = *p & 0x7fffff;

	if(e == 255 && m == 0) // Infinity
	{
		if(s) strncpy(dst, "-inf", n);
		else strncpy(dst, "inf", n);
	}else if(e == 255 && m != 0) // NaN
	{
		strncpy(dst, "nan", n);
	}
	else
	{
		e -= 127;
		m |= 1<<23;



		for(x = 23; x >= 0; x--)
		{
			if(m & (1<<x))
			{
				if(e >= 0)
				{
					z = 1;
					for(y=0;y<e;y++)
						z*=2;
			
					i+=z;
				}
				else
				{
					z = 5000000000000000000;
					for(y = 1; y < -e; y++) 
						z /= 2;

					f+=z;
				}
			}
			e--;
		}

		if(s && n)
		{
			*(dst++) = '-';
			n--;
		}

		x = libc_ulltoa(i, dst, n);
		n-=x;
		dst+=x;

		if(n)
		{
			*(dst++) = '.';
			n--;
			if(n)
			{
				x = libc_ulltoa(f, dst, n<6?n:6);
				n-=x;
				dst+=x;

				if(n)
					*dst=0;
			}
		}
	}
}

void libc_double_to_string(double fl, char *dst, int n)
{
	unsigned long long *p = (unsigned long long *)&fl;
	unsigned long long i = 0;
	unsigned long long f = 0;
	unsigned long long m, s;
	long long e;
	int x, y;
	unsigned long long z;
	
	s = *p >> 63;

	e = 	(*p >> 52) & 0x7ff;
	//printf("%d\n", e);

	m = *p & 0xfffffffffffff;

	for(x=0;x<52;x++)
		if(m&((unsigned long long)1<<(52-x))) putchar('1'); else putchar('0');	

	if(e == 255 && m == 0) // Infinity
	{
		if(s) strncpy(dst, "-inf", n);
		else strncpy(dst, "inf", n);
	}else if(e == 255 && m != 0) // NaN
	{
		strncpy(dst, "nan", n);
	}
	else
	{
		e -= 1023;
		m |= (unsigned long long)1<<52;

		for(x = 52; x >= 0; x--)
		{
			if(m & ((unsigned long long)1<<x))
			{
				if(e >= 0)
				{
					z = (long long)1<<e;
			
					i+=z;
				}
				else
				{
					z = 5000000000000000000;
					z >>= -(e + 1);

					f+=z;
				}
			}
			e--;
		}

		if(s && n)
		{
			*(dst++) = '-';
			n--;
		}

		x = libc_ulltoa(i, dst, n);
		n-=x;
		dst+=x;

		if(n)
		{
			*(dst++) = '.';
			n--;
			if(n)
				libc_ulltoa(f, dst, n<6?n:6);
		}
	}
}

char libc_sprintf_floatbuf[64];

#endif

int vsnprintf(char *string, unsigned int size, const char *fmt, va_list ap)
{
	int string_pos,fmt_pos;
	int l;
	unsigned long arg;
	unsigned char *argcp;
	unsigned char *argcp_tmp;
	int directive_coming = 0;
	int alternate_form = 0;
	int flags = 0;
	int argsize = 2; // int
	int x, y;
	unsigned long a, b;
	int empty_digit;
	int ssz = size - 1;
	int zero_flag_imp = 0;
	int pad_quantity = 0;
	int last;

	// C11: required to check these cases and return error if detected
	if (string == NULL || fmt == NULL || size == 0) { return -1; }
	
	l = strlen(fmt);
	
	string_pos = 0;
	
	for(fmt_pos=0;fmt_pos<l;fmt_pos++)
	{
		if(directive_coming)
		{
			switch(fmt[fmt_pos])
			{
				case '%':
					put_in_string(string, ssz, '%', string_pos++);
					directive_coming = 0;
				break;
				case ' ':
					flags |= SPRINTF_SPACE_FLAG;
				break;
				case '#': // Specify alternate form
					flags |= SPRINTF_ALT_FLAG;
				break;
				case '+': // Specify sign in signed conversions
					flags |= SPRINTF_SIGN_FLAG;
				break;
				case '0': // Padding with zeros...
					if(zero_flag_imp == 0)
					{
						flags |= SPRINTF_ZERO_FLAG;
						zero_flag_imp = 1;
						//printf("Zero padding enabled!\n"); 
					}
					else
					{
						pad_quantity *= 10;
						//printf("pad_quantity = %d\n", pad_quantity);
					}	
				break;
				case '1' ... '9': // '...' cases are a GNU extension,
				                  // but they simplify a lot 
				                  
				        pad_quantity *= 10;
				        pad_quantity += fmt[fmt_pos] - '0';
				        zero_flag_imp = 1;
				        
				        //printf("pad_quantity = %d\n", pad_quantity);
				break;
				case '-': // Negative field flag
					if(flags & SPRINTF_ZERO_FLAG)
						flags &= ~SPRINTF_ZERO_FLAG;
				
					flags |= SPRINTF_NEGFIELD_FLAG;
				break;
				case 'h': // Half argument size
					if(argsize) argsize--;
				break;
				case 'l': // Double argument size
					if(argsize < 2) argsize = 2;
					//else if(argsize < SPRINTF_SIZE_LONG_LONG) argsize++;
				break;
				case 'd': // signed decimal
				case 'i':
					empty_digit = 1;
				
					//printf("argsize = %d\n", argsize);

					//if(argsize < SPRINTF_SIZE_LONG_LONG)
					arg = (unsigned long)va_arg(ap, unsigned long);
					//else
					//	arg = va_arg(ap, unsigned long);

					if(get_arg_in_size(argsize, &arg, 1))
					{
						put_in_string(string, ssz, '-', string_pos++);
						pad_quantity--;
					}
					else
					{
						if(flags & SPRINTF_SIGN_FLAG)
						{
							put_in_string(string, ssz, '+', string_pos++);
							pad_quantity--;
						}
					}
				
					/* Calculate how much padding we have to write */
					
					/*y = 1;
					
					for(x=0;x<=9;x++)
					{
						if(x == 0)
							pad_quantity--;
						else
						{
							if(arg / y)
								pad_quantity--;
						}
						
						y *= 10;
					}
					if(pad_quantity < 0) pad_quantity = 0;*/

					calculate_real_padding();
					
					//printf("Actual pad quantity = %d\n", pad_quantity);
					
					

					/*if(!(flags & SPRINTF_NEGFIELD_FLAG))
					{
						for(x = 0; x < pad_quantity; x++)
						{
							if(flags & SPRINTF_ZERO_FLAG)
								put_in_string(string, ssz, '0', string_pos++);
							else
								put_in_string(string, ssz, ' ', string_pos++);
						}
					}*/
				
					write_padding();

					for(x=19;x>=0;x--)
					{
						a = 1;
						for(y = 0; y<x; y++)
							a *= 10;
							
						b = (arg/a);
						
						if(b>=1)
							empty_digit = 0;
						
						if(empty_digit == 0 || x == 0)
						{	
							arg -= b*a;
						
							put_in_string(string, ssz, b + '0', string_pos++);
						}
					}
					
					/*if(flags & SPRINTF_NEGFIELD_FLAG)
					{
						for(x = 0; x < pad_quantity; x++)
								put_in_string(string, ssz, ' ', string_pos++);
					}*/
					write_neg_padding();
					
					directive_coming = 0;
				break;
				case 'u': // unsigned decimal
					empty_digit = 1;
				
					//if(argsize < SPRINTF_SIZE_LONG_LONG)
					arg = (unsigned long)va_arg(ap, unsigned int);
					//else
					//arg = va_arg(ap, unsigned long long);
				
					get_arg_in_size(argsize, &arg, 0);
				
					calculate_real_padding();
					write_padding();

					for(x=19;x>=0;x--)
					{
						a = 1;
						for(y = 0; y<x; y++)
							a *= 10;
						
						
							
						b = (arg/a);
						
						if(b>=1)
							empty_digit = 0;
						
						if(empty_digit == 0 || x == 0)
						{	
							arg -= b*a;
						
							put_in_string(string, ssz, b + '0', string_pos++);
						}
					}
					
					write_neg_padding();

					directive_coming = 0;
				break;		
				case 'x': // Hexadecimal
				case 'X': // Hexadecimal with big letters
				case 'p': // Hexadecimal with small letters with '0x' prefix
					empty_digit = 1;

					//if(argsize < SPRINTF_SIZE_LONG_LONG)
					arg = (unsigned long)va_arg(ap, unsigned int);
					//else
					//	arg = va_arg(ap, unsigned long int);

					get_arg_in_size(argsize, &arg, 0);
				
					if(fmt_pos == 'p')
						flags |= SPRINTF_ALT_FLAG;
				
					if(flags & SPRINTF_ALT_FLAG)
					{
						put_in_string(string, ssz, '0', string_pos++);
						
						if(fmt[fmt_pos] == 'X')
							put_in_string(string, ssz, 'X', string_pos++);
						else
							put_in_string(string, ssz, 'x', string_pos++);
					}
				
					calculate_real_padding_hex();
					write_padding();

					for(x=7;x>=0;x--)
					{
						y = arg >> (x << 2);
						y &= 0xf;
						
						if(y>=1)
							empty_digit = 0;
						
						if(empty_digit == 0 || x == 0)
						{
							if(y>=0 && y<=9)
								put_in_string(string, ssz, y + '0', string_pos++);
							else if(y>=0xA && y<=0xF)
							{
								if(fmt[fmt_pos] == 'X')
									put_in_string(string, ssz, (y - 0xa) + 'A', string_pos++);
								else
									put_in_string(string, ssz, (y - 0xa) + 'a', string_pos++);
							}
						}
					}
				
					write_neg_padding();

					directive_coming = 0;
				break;
				case 'c': // character
					arg = va_arg(ap, int);
				
					put_in_string(string, ssz, arg & 0xff, string_pos++);
					
					directive_coming = 0;
				break;
				case 's': // string
					argcp = va_arg(ap, char *);
					argcp_tmp = argcp;
				
					if(argcp == NULL)
					{
						// Non standard extension, but supported by Linux and the BSDs.
					
						put_in_string(string, ssz, '(', string_pos++);
						put_in_string(string, ssz, 'n', string_pos++);
						put_in_string(string, ssz, 'u', string_pos++);
						put_in_string(string, ssz, 'l', string_pos++);
						put_in_string(string, ssz, 'l', string_pos++);
						put_in_string(string, ssz, ')', string_pos++);
						
						directive_coming = 0;
						break;
					}
				
					while(*argcp_tmp)
					{
						if(pad_quantity > 0) pad_quantity--;
						argcp_tmp++;
					}
					
					if(!(flags & SPRINTF_NEGFIELD_FLAG))
					{
						while(pad_quantity > 0)
						{
							put_in_string(string,ssz, ' ', string_pos++);
							pad_quantity--;
						}
					}
					
					while(*argcp)
					{
						put_in_string(string, ssz, *argcp, string_pos++);
						argcp++;
					}
					
					if(flags & SPRINTF_NEGFIELD_FLAG)
					{
						while(pad_quantity > 0)
						{
							put_in_string(string,ssz, ' ', string_pos++);
							pad_quantity--;
						}
					}
						
					directive_coming = 0;
				break;
				case 'o': // Octal
					empty_digit = 1;
					
					//if(argsize < SPRINTF_SIZE_LONG_LONG)
					arg = (unsigned long)va_arg(ap, unsigned int);
					//else
					//	arg = va_arg(ap, unsigned long long);
				
					for(x=21;x>=0;x--)
					{
						y = arg >> (x * 3);
						y &= 0x7;
						
						if(y>=1)
							empty_digit = 0;
						
						if(empty_digit == 0 || x == 0)
							put_in_string(string, ssz, y + '0', string_pos++);	
					}
				
					directive_coming = 0;
				break;
				case '@': // Binary
					empty_digit = 1;
					
					//if(argsize < SPRINTF_SIZE_LONG_LONG)
					arg = (unsigned long)va_arg(ap, unsigned int);
					//else
					//	arg = va_arg(ap, unsigned long long);
				
					calculate_real_padding_bin();
					write_padding();

					for(x=31;x>=0;x--)
					{
						y = (arg >> x);
						y &= 1;
						
						if(y>=1)
							empty_digit = 0;
						
						if(empty_digit == 0 || x == 0)
							put_in_string(string, ssz, y + '0', string_pos++);	
					}
				
					write_neg_padding();
				
					directive_coming = 0;
				break;

#ifdef ALLOW_FLOAT
				case 'f':
					libc_double_to_string(va_arg(ap, double), libc_sprintf_floatbuf, 64);
					
					for(x=0;libc_sprintf_floatbuf[x]!=0;x++)
						put_in_string(string, ssz, libc_sprintf_floatbuf[x], string_pos++);

					directive_coming = 0;
				break;
#endif

				case 'n': // Number of characters written
					*(va_arg(ap,unsigned int*)) = string_pos;
					
					directive_coming = 0;
				break;
			//	default
			}
		}
		else
		{
			if(fmt[fmt_pos] == '%')
			{
				directive_coming = 1;
				flags = 0;
				argsize = 2;
				pad_quantity = 0;
				zero_flag_imp = 0;
			}
			else
				put_in_string(string, ssz, fmt[fmt_pos], string_pos++);
		}	
	}
	string[string_pos] = 0;
	return string_pos;	
}

int vsprintf(char *string, const char *fmt, va_list ap)
{
	return vsnprintf(string, 0xffffffff, fmt, ap);
}

int sprintf(char *string, const char *fmt, ...)
{
	int r;

	va_list ap;

	va_start(ap, fmt);

	r = vsprintf(string, fmt, ap);
	
	va_end(ap);
	
	return r;
}

int snprintf(char *string, unsigned int size, const char *fmt, ...)
{
	int r;

	va_list ap;

	va_start(ap, fmt);

	r = vsnprintf(string, size, fmt, ap);
	
	va_end(ap);
	
	return r;
}
