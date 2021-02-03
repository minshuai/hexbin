#ifndef _bin2hex_C_
#define	_bin2hex_C_

#include "stdio.h"
#include "string.h"

#define	BIN_DATA_LEN	24
#define	HEX_STR_LEN		64

static unsigned int start = 0;
static unsigned int length = 0xFFFFFFFF;
static unsigned int begin = 0;
static unsigned int offset = 0;
static char file_name[64];
static FILE* in_file = (FILE*)0;
static FILE* out_file = (FILE*)0;

static unsigned char checkSum(unsigned int count,unsigned char* p)
{
	unsigned char sum=0;
	unsigned int i;
	
	for(i=count;i>0;i--)
	{
		sum += *p++;
	}
	sum = ~sum;
	sum++;
	return sum;
}

static unsigned int encode(unsigned int count, unsigned char* in, unsigned char* out)
{
	const unsigned char intChar[16] = "0123456789ABCDEF";

	unsigned int i;
	unsigned char c;

	*out++ = ':';
	for(i=count;i>0;i--)
	{
		c = *in++;
		*out++ = intChar[c>>4];
		*out++ = intChar[c&0xF];
	}
	*out++ = '\r';
	*out++ = '\n';	

	return (1+(count<<1)+2);
}

static void writeExtendedLinearAddress(void)
{
	unsigned int count,k = 0;
	unsigned char bin_data[BIN_DATA_LEN];
	unsigned char hex_str[BIN_DATA_LEN];

	bin_data[k++] = 0x02;
	bin_data[k++] = 0x00;
	bin_data[k++] = 0x00;
	bin_data[k++] = 0x04;
	bin_data[k++] = (unsigned char)((begin>>24)&0xFF);
	bin_data[k++] = (unsigned char)((begin>>16)&0xFF);
	bin_data[k] = checkSum(k,bin_data);
	k++;
	count = encode(k,bin_data,hex_str);
	fwrite(hex_str,1,count,out_file);
}

static void writeEndOfFile(void)
{
	unsigned int count,k = 0;
	unsigned char bin_data[BIN_DATA_LEN];
	unsigned char hex_str[BIN_DATA_LEN];

	bin_data[k++] = 0x00;
	bin_data[k++] = 0x00;
	bin_data[k++] = 0x00;
	bin_data[k++] = 0x01;
	bin_data[k] = checkSum(k,bin_data);
	k++;
	count = encode(k,bin_data,hex_str);
	fwrite(hex_str,1,count,out_file);
}

static void writeDataRecord(unsigned int len,unsigned char* p)
{
	#define IGNOR_DATA	0xFF
	unsigned int count,k = 0;
	unsigned char bin_data[BIN_DATA_LEN];
	unsigned char hex_str[BIN_DATA_LEN];

	/*for(count=0;count<len;count++)
	{
		if(IGNOR_DATA!=*(p+count))
		{
			break;
		}
	}
	if(count>=len)
	{
		return;
	}*/
	
	bin_data[k++] = len;
	bin_data[k++] = (unsigned char)((offset>>8)&0xFF);
	bin_data[k++] = (unsigned char)(offset&0xFF);
	bin_data[k++] = 0x00;
	for(count=len;count>0;count--)
	{
		bin_data[k++] = *p++;
	}
	bin_data[k] = checkSum(k,bin_data);
	k++;
	count = encode(k,bin_data,hex_str);
	fwrite(hex_str,1,count,out_file);
}

static void writeData(unsigned int count,unsigned char* p)
{
	unsigned int len;
	if(count)
	{
		if((offset+count)<0x10000)
		{
			writeDataRecord(count,p);
			offset += count;
		}
		else if((offset+count)==0x10000)
		{
			writeDataRecord(count,p);
			begin += 0x10000;
			writeExtendedLinearAddress();
			offset = 0;
		}
		else
		{
			len = (0x10000-offset);
			writeDataRecord(len,p);
			begin += 0x10000;
			writeExtendedLinearAddress();
			offset = 0;
			writeDataRecord(count-len,p+len);
			offset = count-len;
		}
	}
}

static void bin2Hex(void)
{
	size_t readCount = 0;
	unsigned int count = 0;
	unsigned char data[16];

	writeExtendedLinearAddress();
	if(fseek(in_file,start,0)>=0)
	{
		while(!feof(in_file) && !ferror(in_file))
		{
			readCount = fread(data,1,16,in_file);
			if((count+readCount)>=length)
			{
				readCount = length - count;
				writeData(readCount,data);
				readCount = 0;
				count = length;
				break;
			}
			writeData(readCount,data);
			count += readCount;
		}
	}
	writeEndOfFile();
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
	
	if(str[0]=='-' && str[1]=='s')
	{
		if(text2Data(str+2,&data)<0)
		{
			return -1;
		}
		start = data;
	}
	else if(str[0]=='-' && str[1]=='l')
	{
		if(text2Data(str+2,&data)<0)
		{
			return -1;
		}
		length = data;
	}
	else if(str[0]=='-' && str[1]=='b')
	{
		if(text2Data(str+2,&data)<0)
		{
			return -1;
		}
		begin = data & 0xFFFF0000;
		offset = data & 0xFFFF;
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

	for(i=2;i<argc;i++)
	{
		if(parseParameter(argv[i])<0)
		{
			return -1;
		}
	}
	
	return 0;
}

int main(int argc, char** argv)
{
	unsigned int len;

	if(argc<=1 || argc>5 || getParameter(argc,argv)<0)
	{
		goto parameter_error;
	}

	if((in_file= fopen(file_name,"rb"))==NULL)
	{
		goto parameter_error;
	}
	len = strlen(file_name);
	file_name[len-3] = 'h';
	file_name[len-2] = 'e';
	file_name[len-1] = 'x';
	if((out_file= fopen(file_name,"w"))==NULL)
	{
		goto parameter_error;
	}
	bin2Hex();
	fclose(in_file);
	fclose(out_file);
	return 0;

	parameter_error:
		if(in_file)
		{
			fclose(in_file);
		}
		if(out_file)
		{
			fclose(out_file);
		}
		printf("wrong parameters or bin file not existed..\r\n");
		return -1;
}

#undef _bin2hex_C_
#endif	//_bin2hex_C_
