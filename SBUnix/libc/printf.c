/*
  Reference : http://www.osdever.net/bkerndev

*/

#include <defs.h>
#include <system.h>
#include <syscall.h>

//Hexadecimal hash
char HASH[17]="0123456789abcdef";

unsigned short *textmemptr;
int attrib = 0x0F;
int csr_x = 0, csr_y = 0;

void *memcpy(void *dest, const void *src, int count)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    for(; count != 0; count--) *dp++ = *sp++;
    return dest;
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, int count)
{
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

/* 0 means both strings are same and nonzero value means both strings are different */
int strcmp(const char *s1, const char *s2)
{
  while(*s1 && (*s1==*s2))
        s1++,s2++;
    return *(const unsigned char*)s1-*(const unsigned char*)s2;
}


int strlen(const char *str)
{
    int retval;
    for(retval = 0; *str != '\0'; str++) retval++;
    return retval;
}
void* memset(void *ptr, int c, int n)
{
  unsigned char *s=ptr;
  while(n--)
   *s++ = (unsigned char)c;

  return ptr;
}

/* Given a octal number convert into decimal number */

int oct2dec(int n)
{

  int res = 0;
  int i=1;
  while(n)
  {
    res = res + ((n%10)*i);
    i = i*8;
    n = n/10;
  }
  return res;
}

/*  convert a string to number */
int  atoi(char *str)
{
    int res = 0;
    int i;
    for (i = 0; str[i] != '\0'; ++i)
        res = res*10 + str[i] - '0';

    return res;
}
/* convert a number to string  */
void itoa(long int value, char *out, int radix)
{
    char temp[40];
    char *ptr = temp;
    long int i;
    unsigned long v;
    int sign;

    sign = (radix == 10 && value < 0);
    if (sign)
    {
      v = -value;
    }else{
      v = (unsigned)value;
    }
    while (v || ptr == temp)
    {
        i = v % radix;
        v /= radix;
        if (i < 10)
          *ptr++ = i+'0';
        else
          *ptr++ = i + 'A' - 10;
    }

    if (sign)
    {
      *out++ = '-';
    }
    while (ptr > temp)
    {
      *out++ = *--ptr;
    }
    *out++ = '\0';
}

int pow(int x, int y)
{
    int temp;
    if( y == 0)
        return 1;
    temp = pow(x, y/2);
    if (y%2 == 0)
        return temp*temp;
    else
        return x*temp*temp;
}



/* Scrolls the screen */
void scroll(void)
{
    unsigned blank, temp;

    /* A blank is defined as a space... we need to give it
    *  backcolor too */
    blank = 0x20 | (attrib << 8);

    /* Row 24 is the end, this means we need to scroll up */
    if(csr_y >= 24)
    {
        /* Move the current text chunk that makes up the screen
        *  back in the buffer by a line */
        temp = csr_y - 24 + 1;
        memcpy (textmemptr, textmemptr + temp * 80, (24 - temp) * 80 * 2);

        /* Finally, we set the chunk of memory that occupies
        *  the last line of text to our 'blank' character */
        memsetw (textmemptr + (24 - temp) * 80, blank, 80);
        csr_y = 24 - 1;
    }
}

/* Updates the hardware cursor: the little blinking line
*  on the screen under the last character pressed! */
void move_csr(void)
{
    unsigned temp;

    /* The equation for finding the index in a linear
    *  chunk of memory can be represented by:
    *  Index = [(y * width) + x] */
    temp = csr_y * 80 + csr_x;

    /* This sends a command to indicies 14 and 15 in the
    *  CRT Control Register of the VGA controller. These
    *  are the high and low bytes of the index that show
    *  where the hardware cursor is to be 'blinking'. To
    *  learn more, you should look up some VGA specific
    *  programming documents. A great start to graphics:
    *  http://www.brackeen.com/home/vga */
    outb(0x3D4, 14);
    outb(0x3D5, temp >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, temp);
}
    
/* Clears the screen */
void cls()
{
    unsigned blank;
    int i;

    /* Again, we need the 'short' that will be used to
    *  represent a space with color */
    blank = 0x20 | (attrib << 8);

    /* Sets the entire screen to spaces in our current
    *  color */
    for(i = 0; i < 24; i++)
        memsetw (textmemptr + i * 80, blank, 80);

    /* Update out virtual cursor, and then move the
    *  hardware cursor */
    csr_x = 0;
    csr_y = 0;
    move_csr();
}


/* Puts a single character on the screen */
void putch(unsigned char c)
{
  __syscall1(PUTCHAR,c);  //1 is for putchar 
}

/* Uses the above routine to output a string... */
void puts(const char *text)
{
    int i;

    for (i = 0; i < strlen(text); i++)
    {
        putch(text[i]);
    }
}

/* Sets the forecolor and backcolor that we will use */
void settextcolor(unsigned char forecolor, unsigned char backcolor)
{
    /* Top 4 bytes are the background, bottom 4 bytes
    *  are the foreground color */
    attrib = (backcolor << 4) | (forecolor & 0x0F);
}

/* * Sets our text-mode VGA pointer, then clears the screen for us */
void init_video(void)
{
    textmemptr = (unsigned short *)VIDEO_VIRTUAL_MEMORY;    
    cls();
}




void puthex(unsigned int num)
{
     if(num/16==0)
                   putch(HASH[num]);
     else {
           puthex(num/16);
           putch(HASH[num%16]);
          }
}
void putaddr(unsigned long num)
{
     if(num/16==0)
                   putch(HASH[num]);
     else {
           putaddr(num/16);
           putch(HASH[num%16]);
          }
}

void printf(const char *format, ...)
{
  const char *incr;
  va_list arguments;
  va_start(arguments,format);
  unsigned  int i;
  unsigned long a;
  char *string;
  char intbuf[16];

  for(incr = format; *incr != '\0'; incr++)
  {
    if(*incr != '%')
    {
      putch(*incr);
    } else {
      switch(*++incr)
      {
        case 'c':
          i = va_arg(arguments, int);
          putch(i);
          break;
        case 'd':
          i = va_arg(arguments, int);
          itoa(i,intbuf,10);
          puts(intbuf);
          break;
        case 'x':
          i = va_arg(arguments,unsigned int);
          puthex(i);
          break;
        case 's':
          string = va_arg(arguments, char *);
          puts(string);
          break;
        case 'p':
          a = va_arg(arguments,unsigned long );
          putaddr(a);
          break;
      }
    }
  }
  va_end(arguments);
}

void outb(uint16_t port, char data) {

    __asm__ __volatile( "outb %0, %1;"
                    : /* void */
                    : "a" (data), "d" (port));

}

unsigned char inb(unsigned short _port)
{
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(_port));
    return ret;
}














