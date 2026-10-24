/*
	Author	: Vincent 'Zarjazz' Sweeney
	Email	: zarjazz@barrysworld.com
	WebPage	: http://www.barrysworld.com/

	Copyright © 1998-2000 BarrysWorld Ltd - All rights reserved.
	----
	$Id$
*/

// Typedef of string structure
typedef struct string_s
{
	unsigned short	tag;
	unsigned short	crc;
	unsigned int	len;
	char *txt;
} string_t;

// The string lookup tables
extern string_t string_table[];

// Proto's
void string_codec (unsigned char *s, unsigned len, int key);

// The wrapper function to return the (converted) string
char * DECODE_STRING (string_t *table,	unsigned int str_id);

#define STRING(x) DECODE_STRING(string_table,x)
