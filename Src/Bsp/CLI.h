




#ifndef _CLI_H
#define _CLI_H
#include "config.h"
#include "stdio.h"
#include "ctype.h"

#define STKSIZE 100       /*����ջ��С*/
#define CSTKSIZE 128      /*�ַ�ջ��С*/
#define STRLENTH 100	  /*�����ַ�������*/

typedef struct dtable
{
	union
	{
		struct
		{
			unsigned char len;
			char words[7];
		}bytes;
		
		char bits[8];
	}id;
	void (*fun)();
}table;

void CLI_Init(void);
void push(int);
int pop(void);
void pushc(char);
char popc(void);
void decipher(void const * argument);
int trypush(char *str);
int popcstr(int x , char *p);
int getcsp(void);
int getsp(void);


void getid(void);
void setup(void);

#endif
