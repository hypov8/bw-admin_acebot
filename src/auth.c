/*
	Author	: Vincent 'Zarjazz' Sweeney
	Email	: zarjazz@barrysworld.com
	WebPage	: http://www.barrysworld.com/

	Copyright © 1998-2000 BarrysWorld Ltd - All rights reserved.
	----
	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "q2_local.h"
#include "kp_local.h"

#include "admin.h"


int z_stricmp (const char *s1, const char *s2);

typedef struct auth_s
{
	char *name;
	char *pass;
} auth_t;


static int nauth = 0;
static auth_t *auth_list;


void ResetNameLocks (void)
{
	int i;
	auth_t *a = auth_list;

	for (i = 0; i < nauth; i++, a++)
	{
		if (a->name) free (a->name);
		if (a->pass) free (a->pass);
	}

	nauth = 0;
	if (auth_list) free (auth_list);
}

void LoadNameLocks (char *fname)
{
	FILE *file;

	char line[128];
	char *s;

	ResetNameLocks();

	file = fopen(fname, "r");
	if (!file) gi.error ("[BW-Admin] '%s' File Not Found\n",fname);

	while ( fgets(line, sizeof(line), file) )
	{
		int l;
		char *p1,*p2,*n1,*n2;

		s = line;
		// skip initial white space chars
		while (*s && isspace(*s)) s++;
		// skip emtpy lines and comments
		if ( !(*s) || *s == '#' || !strncmp(s,"//",2) ) continue;
		// remove trailing white space chars
		l = strlen(s);
		while (l > 0 && isspace(s[l-1])) s[--l] = '\0';

		// Parse line
		p2 = p1 = s; while (*p2 && !isspace(*p2)) p2++;

		n1 = strchr(p2,'"'); n2 = strrchr(p2,'"');

		// Check syntax is correct
		if (!n1 || !n2 || n1 == n2) continue;

		// Allocated new entry
		auth_list = realloc(auth_list,sizeof(auth_t) * (nauth + 1));

		*p2 = *n2 = *n1++ = '\0';

		auth_list[nauth].name = strdup(n1);
		auth_list[nauth].pass = strdup(p1);

		nauth++;
	}

	gi.dprintf("** %d Name Locks Read\n",nauth);
}


char *Is_NameLocked (char *name)
{
	int i;
	auth_t *a = auth_list;

	for (i = 0; i < nauth; i++, a++)
	{
		if ( !z_stricmp(name,a->name) ) return a->pass;
	}

	return NULL;
}

char *Find_LockedName (char *pass)
{
	int i;
	auth_t *a = auth_list;

	for (i = 0; i < nauth; i++, a++)
	{
		if ( !z_stricmp(pass,a->pass) ) return a->name;
	}

	return NULL;
}
