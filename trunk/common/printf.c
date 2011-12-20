/**
 @file printf.c

 @brief small printf() implementation

 Requires ~506B of Flash, ~18B of RAM when compiled on IAR, or about 1/3 the size of IAR-Small.

 @see http://www.sparetimelabs.com/printfrevisited/index.html

Copyright (C) 2004,2008  Kustaa Nyholm

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

YOU ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� WITHOUT WARRANTY 
OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, 
TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL TEXAS INSTRUMENTS 
OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, 
BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, 
LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY 
CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*/
#include "printf.h"

//void putchar(char c);
int putchar(int c);  //Note: Changed to match stdio.h putchar declaration

static char* bf;
static char buf[12];
static unsigned int num;
static char uc;
static char zs;

static void out(char c) {
    *bf++ = c;
    }

static void outDgt(char dgt) {
	out(dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10));
	zs=1;
    }
	
static void divOut(unsigned int div) {
    unsigned char dgt=0;
	num &= 0xffff; // just for testing the code  with 32 bit ints
	while (num>=div) {
		num -= div;
		dgt++;
		}
	if (zs || dgt>0) 
		outDgt(dgt);
    }	

void tfp_printf(char *fmt, ...)
	{
	va_list va;
	char ch;
	char* p;
	
	va_start(va,fmt);
	
	while ((ch=*(fmt++))) {
		if (ch!='%') {
			putchar(ch);
			}
		else {
			char lz=0;
			char w=0;
			ch=*(fmt++);
			if (ch=='0') {
				ch=*(fmt++);
				lz=1;
				}
			if (ch>='0' && ch<='9') {
				w=0;
				while (ch>='0' && ch<='9') {
					w=(((w<<2)+w)<<1)+ch-'0';
					ch=*fmt++;
					}
				}
			bf=buf;
			p=bf;
			zs=0;
			switch (ch) {
				case 0: 
					goto abort;
				case 'u':
				case 'd':
                                case 'i':
					num=va_arg(va, unsigned int);
					if (((ch=='d')||(ch=='i')) && (int)num<0) {
						num = -(int)num;
						out('-');
						}
					divOut(10000);
					divOut(1000);
					divOut(100);
					divOut(10);
					outDgt(num);
					break;
				case 'x': 
				case 'X' : 
				    uc= ch=='X';
					num=va_arg(va, unsigned int);
					divOut(0x1000);
					divOut(0x100);
					divOut(0x10);
					outDgt(num);
					break;
				case 'c' : 
					out((char)(va_arg(va, int)));
					break;
				case 's' : 
					p=va_arg(va, char*);
					break;
				case '%' :
					out('%');
				default:
					break;
				}
			*bf=0;
			bf=p;
			while (*bf++ && w > 0)
				w--;
			while (w-- > 0) 
				putchar(lz ? '0' : ' ');
			while ((ch= *p++))
				putchar(ch);
			}
		}
	abort:;
	va_end(va);
	}

