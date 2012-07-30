#ifndef _combineBin_C_
#define	_combineBin_C_

#include "stdio.h"
#include "string.h"

#define log //

static FILE* in_file1 = (FILE*)0;
static FILE* in_file2 = (FILE*)0;
static FILE* out_file = (FILE*)0;
static char in_file_name1[64];
static char in_file_name2[64];
static char out_file_name[64];

static void combineBin(void)
{
	size_t read;
	unsigned char buffer[256];

	while(!feof(in_file1) && !ferror(in_file1))
	{
		read = fread(buffer,1,sizeof(buffer),in_file1);
		fwrite(buffer,1,read,out_file);
	}
	while(!feof(in_file2) && !ferror(in_file2))
	{
		read = fread(buffer,1,sizeof(buffer),in_file2);
		fwrite(buffer,1,read,out_file);
	}
}

static int getParameter(int argc, char* argv[])
{
	unsigned int len;
	int i;

	len = strlen(argv[1]);
	if(len<=4 || len>=(sizeof(in_file_name1)))
	{
		return -1;
	}
	else
	{
		memcpy(in_file_name1,argv[1],len);
		if(in_file_name1[len-4]!='.'
		||((in_file_name1[len-3]!='b')&&(in_file_name1[len-3]!='B'))
		||((in_file_name1[len-2]!='i')&&(in_file_name1[len-3]!='I'))
		||((in_file_name1[len-1]!='n')&&(in_file_name1[len-3]!='N')))
		{
			return -1;
		}
	}

	len = strlen(argv[2]);
	if(len<=4 || len>=(sizeof(in_file_name2)))
	{
		return -1;
	}
	else
	{
		memcpy(in_file_name2,argv[2],len);
		if(in_file_name2[len-4]!='.'
		||((in_file_name2[len-3]!='b')&&(in_file_name2[len-3]!='B'))
		||((in_file_name2[len-2]!='i')&&(in_file_name2[len-3]!='I'))
		||((in_file_name2[len-1]!='n')&&(in_file_name2[len-3]!='N')))
		{
			return -1;
		}
	}

	len = strlen(argv[3]);
	if(len<=4 || len>=(sizeof(out_file_name)))
	{
		return -1;
	}
	else
	{
		memcpy(out_file_name,argv[3],len);
		if(out_file_name[len-4]!='.'
		||((out_file_name[len-3]!='b')&&(out_file_name[len-3]!='B'))
		||((out_file_name[len-2]!='i')&&(out_file_name[len-3]!='I'))
		||((out_file_name[len-1]!='n')&&(out_file_name[len-3]!='N')))
		{
			return -1;
		}
	}
	return 0;
}

int main(unsigned int argc, char* argv[])
{
	if(argc!=4 || getParameter(argc,argv)<0)
	{
		goto parameter_error;
	}

	if((in_file1= fopen(in_file_name1,"rb"))==NULL)
	{
		goto parameter_error;
	}
	if((in_file2= fopen(in_file_name2,"rb"))==NULL)
	{
		goto parameter_error;
	}
	if((out_file= fopen(out_file_name,"wb"))==NULL)
	{
		goto parameter_error;
	}
	combineBin();
	fclose(in_file1);
	fclose(in_file2);
	fclose(out_file);
	return 0;

	parameter_error:
		if(in_file1)
		{
			fclose(in_file1);
		}
		if(in_file2)
		{
			fclose(in_file2);
		}
		if(out_file)
		{
			fclose(out_file);
		}
		printf("wrong parameters..\r\n");
		return -1;
}

#undef _combineBin_C_
#endif	//_combineBin_C_
