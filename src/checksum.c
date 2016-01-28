// hex2bin.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#define	log //


static unsigned int size = 0;
static char file_name[64];
static FILE* in_file = (FILE*)0;

static void calcChecksum(void)
{
	unsigned int fileSize = 0;
	unsigned int crc = 0;
	unsigned int temp = 0;
	unsigned char b[4];

	fseek(in_file,0,SEEK_END);
	fileSize = ftell(in_file);
	if(fileSize!=size)
	{
		printf("length of file is not match..\r\n");
		return;
	}
	fseek(in_file,0,SEEK_SET);
	fileSize = size/4-1;
	while(fileSize>0)
	{
		fread(b,4,1,in_file);
		temp = (b[3]<<24) | (b[2]<<16) | (b[1]<<8) | (b[0]<<0);
		crc += temp;
		fileSize--;
	}
	crc = ~crc;
	fread(b,4,1,in_file);
	temp = (b[3]<<24) | (b[2]<<16) | (b[1]<<8) | (b[0]<<0);
	if(temp == crc)
	{
		printf("checksum match..\r\n");
	}
	else
	{
		printf("checksum not match..\r\n");
		printf("current checksum is %08x..\r\n",temp);
		printf("expected checksum is %08x..\r\n",crc);
	}
}
unsigned int strlen(char* str)
{
	unsigned int len = 0;
	while(*str!='\0')
	{
		len++;
		str++;
	}
	return len;
}

static int text2Data(char* str,unsigned int* data)
{
	unsigned int i,len,temp;
	char c;

	len = strlen(str);
	if(len>8 || len<=0)
	{
		return -1;
	}
	temp = 0;
	for(i=len;i>0;i--)
	{
		temp <<= 4;
		c = *str++;
		if(c>='0' && c<='9')
		{
			temp += c-'0';
		}
		else if(c>='a' && c<='f')
		{
			temp += c-'a'+10;
		}
		else if(c>='A' && c<='F')
		{
			temp += c-'A'+10;
		}
		else
		{
			return -1;
		}
	}

	*data = temp;
	return 0;
}

static int parseParameter(char* str)
{
	unsigned int data;
	
	if(str[0]=='-' && str[1]=='l')
	{
		if(text2Data(str+2,&data)<0 || data<=0 || data%4!=0)
		{
			return -1;
		}
		size = data;	
	}
	else
	{
		return -1;
	}
	return 0;
}

void conver_wchar2char(_TCHAR* src,char* dst)
{
	while(*src!='\0')
	{
		*dst++ = *src++;
	}
	*dst = '\0';
}

static int getParameter(int argc, _TCHAR* argv[])
{
	unsigned int len;
	int i;

	char arg1[64];
	char arg2[64];

	conver_wchar2char(argv[1],arg1);
	conver_wchar2char(argv[2],arg2);

	len = strlen(arg1);
	if(len<=4 || len>=(sizeof(file_name)))
	{
		return -1;
	}
	else
	{
		memcpy(file_name,arg1,len);
		if(file_name[len-4]!='.'
		||((file_name[len-3]!='b')&&(file_name[len-3]!='B'))
		||((file_name[len-2]!='i')&&(file_name[len-3]!='I'))
		||((file_name[len-1]!='n')&&(file_name[len-3]!='N')))
		{
			return -1;
		}
	}

	if(argc>2)
	{
		if(parseParameter(arg2)<0)
		{
			return -1;
		}
	}
	
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	unsigned int len;

	if(argc!=3 || getParameter(argc,argv)<0)
	{
		goto parameter_error;
	}

	if((fopen_s(&in_file,file_name,"rb"))!=0)
	{
		goto parameter_error;
	}
	calcChecksum();
	fclose(in_file);
	system("pause");
	return 0;

	parameter_error:
		if(in_file)
		{
			fclose(in_file);
		}
		printf("wrong parameters or hex file not existed..\r\n");
		return -1;
}

