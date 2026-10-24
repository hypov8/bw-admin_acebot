/*
	Author	: Vincent 'Zarjazz' Sweeney
	Email	: zarjazz@barrysworld.com
	WebPage	: http://www.barrysworld.com/

	Copyright © 1998-2000 BarrysWorld Ltd - All rights reserved.
	----
	$Id$
*/

#include "shared.h"
#include "admin.h"

extern int logging;
extern char gamedir[64];
extern char *VERSION;

/*
=================
local_time_str
=================
*/

static char *local_time_str (void)
{
	char *s;
	struct tm *newtime;
	time_t tm;

	time( &tm );
	newtime = localtime( &tm );
	s = asctime(newtime);
	s[strlen(s)-1] = '\0'; // remove newline

	return s;
}

/*
=================
Log_Text
=================
*/

void Log_Text (const char *str, ...)
{
	va_list argptr;
	char buf[1024];

	va_start(argptr, str);
	vsprintf(buf, str, argptr);
	va_end(argptr);

	gi.dprintf("[BW-Admin] %s\n", buf);

	if (logging)
	{
		char line[256];
		FILE *f;

#ifdef WIN32
		snprintf(line,sizeof(line),"%s\\%s",gamedir,"BW-Admin.log");
#else
		snprintf(line,sizeof(line),"%s/%s",gamedir,"BW-Admin.log");
#endif
		f = fopen(line, "a");
		if (!f) return;

		fprintf(f, "[BW-Admin %s] %s : %s\n", VERSION, local_time_str(),buf);
		fclose(f);
	}
}

/*
=================
Log_Track
=================
*/

void Log_Track (char *str, ...)
{
	va_list argptr;
	char line[256];
	FILE *f;

#ifdef WIN32
	snprintf(line,sizeof(line),"%s\\%s",gamedir,"BW-Tracking.log");
#else
	snprintf(line,sizeof(line),"%s/%s",gamedir,"BW-Tracking.log");
#endif

	f = fopen(line, "a");
	if (!f) return;

	fprintf(f, "%s : ", local_time_str());

	va_start(argptr, str);
	vfprintf(f, str, argptr);
	va_end(argptr);

	fprintf(f, "\n");

	fclose(f);
}
