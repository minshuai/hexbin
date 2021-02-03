#ifndef _hex2bin_C_
#define	_hex2bin_C_

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define	log //printf

#define	NORMAL_ADDRESS	0
#define	SEGMENT_ADDRESS	1
#define	LINEAR_ADDRESS	2

static unsigned int start = 0xFFFFFFFF;
static unsigned int end = 0;
static unsigned int base_address = 0;
static unsigned int type = 0;
static unsigned char text_content[600];
static char file_name[64];
static FILE* in_file = (FILE*)0;
static FILE* out_file = (FILE*)0;

static void hexError(void)
{
	fclose(in_file);
	fclose(out_file);
	printf("error hex format\r\n");
	exit(-1);
}

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
static void text2Char(unsigned char* p)
{
	unsigned char in,out;
	
	in = *p;
	if(in>='0' && in<='9')
	{
		out = in - '0';
	}
	else if(in>='a' && in<='f')
	{
		out = in - 'a' + 10;
	}
	else if(in>='A' && in<='F')
	{
		out = in - 'A' + 10;
	}
	else
	{
		log("not valid data\r\n");
		hexError();
	}
	*p = out;
}
static void writeData(unsigned int offset,unsigned char count,unsigned char* p)
{
	unsigned int address1,address2;
	unsigned int count1,count2;
	unsigned char* ptr;

	//log("data getted\r\n");
	//for(count1=0;count1<count;count1++)
	//{
	//	log("%x ",p[count1]);
	//}
	//log("\r\n");
	count1 = count;
	count2 = 0;
	switch(type)
	{
		default:
			break;
		case NORMAL_ADDRESS:
			address1 = offset;
			address2 = address1+count-1;
			if(address2<address1)
			{
				address2 = 0x0000;
				count1 = address2-address1;
				count2 = count-count1;
			}
			break;
		case SEGMENT_ADDRESS:
			address1 = base_address + offset;
			address2 = base_address + ((offset+count-1) & 0xFFFFF);	
			if(address2<address1)
			{
				address2 = base_address & 0xFFFF0000;
				count1 = address2-address1;
				count2 = count-count1;
			}			
			break;
		case LINEAR_ADDRESS:
			address1 = base_address | offset;
			address2 = address1+count-1;
			if(address2<address1)
			{
				address2 = 0x00000000;
				count1 = address2-address1;
				count2 = count-count1;
			}
			break;
	}
	//log("start=%x,end=%x\r\n",start,end);
	log("address1=%x,count1=%x\r\n",address1,count1);
	log("address2=%x,count2=%x\r\n",address2,count2);
	if(count1 && address1<=end && ((start==0 && (address1+count1)>start) || (start!=0 && (address1+count1-1)>(start-1))))
	{
		if(address1<start)
		{
			if((address1+count1-1)>end)
			{
				count = end-start+1;
			}
			else
			{
				count = address1+count1-start;
			}
			ptr = p + (start-address1);
			address1 = start;
		}
		else
		{
			if((address1+count1-1)>end)
			{
				count = end-address1+1;
			}	
			ptr = p;
		}
		fseek(out_file,address1-start,0);
		fwrite(ptr,1,count,out_file);
	}
	if(count2 && address2<=end && (address2+count2)>=start)
	{
		log("swapped!\r\n");
		if(address2<start)
		{
			if((address2+count2)>(end+1))
			{
				count = end-start+1;
			}
			else
			{
				count = address2+count2-start;
			}
			ptr = p + (start-address2);
			address2 = start;
		}
		else
		{
			if((address2+count2)>(end+1))
			{
				count = end-address2+1;
			}	
			ptr = p;
		}
		fseek(out_file,address2-start,0);
		ptr += count1;
		fwrite(ptr,1,count,out_file);
	}
}
static void decode(unsigned int count, unsigned char* p)
{
	unsigned int length;
	unsigned int i,offset;

	for(i=0;i<count;i++)
	{
		log("%c",p[i]);
	}
	log("\r\n");
	text2Char(p+1);
	text2Char(p+2);

	length = (p[1]<<4) | p[2];
	
	if(count<(((length+5)<<1)+1))
	{
		log("wrong information length\r\n");
		hexError();
	}
	for(i=3;i<((length+5)<<1)+1;i++)
	{
		text2Char(p+i);
		//log("%x ",p[i]);
	}
	//log("\r\n");
	for(i=0;i<(length+5);i++)
	{
		*(p+i) = ((p[1+(i<<1)])<<4) | (p[2+(i<<1)]);
		//log("%x ",p[i]);
	}
	//log("\r\n");
	if(checkSum(length+4,p)!=(p[length+4]))
	{
		log("checksum not match\r\n");
		hexError();
	}
	switch(p[3])
	{
		default:
			log("invalid type\r\n");
			hexError();
			break;
		case 3:
		case 5:
			break;
		case 0:
			offset = p[1];
			offset = (offset<<8) | p[2];
			writeData(offset,length,p+4);
			break;
		case 1:
			fclose(in_file);
			fclose(out_file);
			exit(0);
			break;
		case 2:
			if(p[0]!=0x02 || p[1]!=0x00 || p[2]!=0x00)
			{
				log("invalid segment address data\r\n");
				hexError();
			}
			type = SEGMENT_ADDRESS;
			base_address = p[4];
			base_address = (base_address<<8) | p[5];
			base_address <<= 4;
			log("segment base_address=%x\r\n",base_address);
			break;
		case 4:
			if(p[0]!=0x02 || p[1]!=0x00 || p[2]!=0x00)
			{
				log("invalide linear address data\r\n");
				hexError();
			}
			type = LINEAR_ADDRESS;
			base_address = p[4];
			base_address = (base_address<<8) | p[5];
			base_address <<= 16;
			log("linear base_address=%x\r\n",base_address);
			break;
	}
}
static unsigned int parseString(unsigned int count, unsigned char* p)
{
	unsigned char len[2];
  unsigned int length;
	unsigned int i,start;
	
	for(i=0;i<count;i++)
	{
		if(p[i]==':')
		{
			start = i;
			if((i+2)<count)
			{
				len[0] = p[i+1];
				len[1] = p[i+2];
				text2Char(len);
				text2Char(len+1);
				length = (len[0]<<4) | len[1];
				i += ((length+5)<<1);
				if(i<count)
				{
					decode(i-start+1,p+start);
				}
				else
				{
					return (count-start);
				}
			}
			else
			{
				return (count-start);
			}
		}		
	}
	return 0;
}

static void hex2Bin(void)
{
	size_t readCount = 0;
	unsigned int count = 0;
	unsigned int i;
	unsigned char fillData = 0xFF;	

	for(i=end-start+1;i>0;i--)
	{
		fwrite(&fillData,1,1,out_file);
	}
	while(!feof(in_file) && !ferror(in_file))
	{
		readCount = fread(text_content+count,1,600-count,in_file);
		//log("read %d data\r\n",readCount);
		readCount += count;
		count = parseString(readCount,text_content);
		for(i=0;i<count;i++)
		{
			text_content[i] = text_content[readCount-count+i];
		}
	}
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
		if(text2Data(str+2,&data)<0 || data<=0)
		{
			return -1;
		}
		if((data-1)>(0xFFFFFFFF-start))
		{
			return -1;
		}
		end = start + data -1;		
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
		||((file_name[len-3]!='h')&&(file_name[len-3]!='H'))
		||((file_name[len-2]!='e')&&(file_name[len-3]!='E'))
		||((file_name[len-1]!='x')&&(file_name[len-3]!='X')))
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

	if(argc!=4 || getParameter(argc,argv)<0)
	{
		goto parameter_error;
	}

	if((in_file= fopen(file_name,"r"))==NULL)
	{
		goto parameter_error;
	}
	len = strlen(file_name);
	file_name[len-3] = 'b';
	file_name[len-2] = 'i';
	file_name[len-1] = 'n';
	if((out_file= fopen(file_name,"wb"))==NULL)
	{
		goto parameter_error;
	}
	hex2Bin();
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
		printf("wrong parameters or hex file not existed..\r\n");
		return -1;
}

#undef _hex2bin_C_
#endif	//_hex2bin_C_
