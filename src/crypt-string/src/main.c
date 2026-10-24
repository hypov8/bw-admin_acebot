/*
	Author	: Vincent 'Zarjazz' Sweeney
	Email	: zarjazz@barrysworld.com
	WebPage	: http://www.barrysworld.com/

	Copyright © 1998/99 BarrysWorld Ltd - All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "codec.h"
#include "common.h"


unsigned short CRC_Checksum (unsigned char *buf, int length);


char *str_header = 
	"/*\n"
	"\tAuthor\t: Vincent 'Zarjazz' Sweeney\n"
	"\tEmail\t: zarjazz@barrysworld.com\n"
	"\tWebPage\t: http://www.barrysworld.com/\n"
	"\n"
	"\tCopyright © 1998/99 BarrysWorld Ltd - All rights reserved.\n"
	"*/\n\n";

char * create_string (char *x, int *len)
{
	int n;
	char *s, *str;

	n = strlen(x);
	s = str = (char *) malloc (n + 1);
	if (!s) Error ("Input: StrDup Error");

	do
	{
		if (*x == '\\' && x[1])
		{
			x++; n--;
			switch (*x)
			{
			case 'n':
				*s++ = '\n';
				break;
			case 'r':
				*s++ = '\r';
				break;
			case '\\':
				*s++ = '\\';
				break;
			default:
				Error("Input: \\ Code");
				break;
			}
		}
		else
			*s++ = *x;

	} while (*x++);

	*len = n;

	return str;
}

/*
===============
main
===============
*/

int main (int argc, char **argv)
{
	int i,len;
	char *begin, *end;
	char *s,*s1,*s2;
	FILE *c_out, *h_out;

	unsigned char *	buffer;
	unsigned int	buf_len;

	if (argc < 2) Error ("Not enough arguments");

	// open C source file
	s = "strings.c";
	c_out = fopen (s,"wb");
	if (!c_out) Error("Unable to open file: %s",s);

	// open H header file
	s = "strings.h";
	h_out = fopen (s,"wb");
	if (!h_out) Error("Unable to open file: %s",s);

	// Read the input file
	buffer = ReadToBuf(argv[1],&buf_len);

	// Write the file headers
	fputs(str_header,c_out);
	fputs(str_header,h_out);

	fputs("#include \"codec.h\"\n\ntypedef unsigned char byte;\n\n",c_out);

	// Find the "begin" tag
	s = "// BEGIN";
	begin = strstr(buffer,s);
	if (!begin) Error("Input: No \"%s\" tag",s);

	// Find the "end" tag
	s = "// END";
	end = strstr(begin,s);
	if (!end) Error("Input: No \"%s\" tag",s);

#ifdef _DEBUG
	fflush(c_out);
	fflush(h_out);

	printf("Headers Complete\n");
#endif

	// Create Header File & Init String List

	i = 0; s1 = begin;
	while ( s = strstr(s1,"\n{ ") /* Find the { bracket */ )
	{
		char *t1, *t2;

		for (s += 3; isspace(*s); s++);	// skip any white space characters

		s2 = strchr(s,',');				// ',' seperator
		if (!s2) Error ("Input: Syntax Error");

		*s2 = 0;
		fprintf(h_out,"#define %s %d\n",s,i);
		// cleanup
		*s2 = ',';

		t1 = strchr(s2 + 1,'"');
		if (!t1) Error ("Input: Syntax Error");

		t2 = t1 + 1;
		while ( (t2 = strchr(t2,'"')) && (t2[-1] == '\\') );
		if (!t2) Error ("Input: Syntax Error");

		*t1 = *t2 = 0;

		// create a new string to copy and encrypt
		s = s2 = create_string(t1 + 1,&len);

		string_codec(s,len,i);

		fprintf(c_out,"static byte STR_%.5d[] = { ",i++);
		while (len--) fprintf(c_out,"0x%.2x, ",(int) *s++ & 0xFF);
		fputs("0x00 };\n",c_out);

		free(s2);

		s1 = t2 + 1;
		// cleanup
		*t1 = *t2 = '"';
	}

	// print string table header
	fputs("\nstring_t string_table[] =\n{\n",c_out);

	i = 0; s1 = begin;
	while ( s = strstr(s1,"\n{ ") /* Find the { bracket */ )
	{
		char *t1, *t2;
		unsigned short crc;

		for (s += 3; isspace(*s); s++);	// skip any white space characters

		s2 = strchr(s,',');				// ',' seperator
		if (!s2) Error ("Input: Syntax Error");
		*s2++ = 0;

		t1 = strchr(s2,'"');
		if (!t1) Error ("Input: Syntax Error");

		t2 = t1 + 1;
		while ( (t2 = strchr(t2,'"')) && (t2[-1] == '\\') );
		if (!t2) Error ("Input: Syntax Error");

		*t1++ = 0; *t2++ = 0;

		s = create_string(t1,&len);
		crc = CRC_Checksum(s,len);
		free(s);

		fprintf(c_out,"\t { 0, %5hu, %5d, STR_%.5d },\n",crc,len,i++);

		s1 = t2;
	}

	// print table tail
	fputs("};\n\n",c_out);

	// cleanup
	fclose(c_out);
	fclose(h_out);

	return 0;
}
