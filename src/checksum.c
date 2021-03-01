
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	log //


static unsigned int size = 0;
static char file_name[64];
static FILE* in_file = (FILE*)0;

static unsigned int calcChecksum(void)
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
    system("pause");
		exit(-1);
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
	return crc;
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

static int getParameter(int argc, char* argv[])
{
	unsigned int len;
	int i;

	len = strlen(argv[1]);
	if(len<=4 || len>=(sizeof(file_name)))
	{
		return -1;
	}
	else
	{
		memcpy(file_name,argv[1],len);
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
		if(parseParameter(argv[2])<0)
		{
			return -1;
		}
	}
	
	return 0;
}

int main(int argc, char** argv)
{
	unsigned int len;
  unsigned int crc;
  unsigned int fileSize = 0;
  char* buf = NULL;

	if(argc!=3 || getParameter(argc,argv)<0)
	{
		goto parameter_error;
	}

	if((fopen_s(&in_file,file_name,"rb"))!=0)
	{
		printf("open file error\r\n");
    system("pause");
		exit(-1);
	}
	crc = calcChecksum();
  buf = (char*)malloc(size);
  if(NULL==buf)
  {
    printf("malloc error\r\n");
    fclose(in_file);
    system("pause");
    exit(-1);
  }
  fseek(in_file,0,SEEK_SET);
  fread(buf,size,1,in_file);
  buf[size-4] = (char)(crc&0xFF);
  buf[size-3] = (char)((crc>>8)&0xFF);
  buf[size-2] = (char)((crc>>16)&0xFF);
  buf[size-1] = (char)((crc>>24)&0xFF);
	fclose(in_file);
  
  if((fopen_s(&in_file,file_name,"wb"))!=0)
	{
    printf("open file error\r\n");
    free(buf);
    system("pause");
		exit(-1);
	}
  fwrite(buf,size,1,in_file);
	fclose(in_file);
  
  free(buf);
  
	return 0;

	parameter_error:
		if(in_file)
		{
			fclose(in_file);
		}
		printf("wrong parameters or hex file not existed..\r\n");
    system("pause");
		return -1;
}
