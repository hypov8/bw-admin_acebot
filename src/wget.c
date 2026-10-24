/*
	Author	: Vincent 'Zarjazz' Sweeney
	Email	: zarjazz@barrysworld.com
	WebPage	: http://www.barrysworld.com/

	Copyright © 1998-2000 BarrysWorld Ltd - All rights reserved.
	----
	$Id$
*/

// Include basic Win32 Files
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
	#include <windows.h>
#endif

// Project Includes

#include "shared.h"
#include "admin.h"

// extern's
extern char *VERSION;
extern char admin_dll[];

// local's
static float cur_ver;
static float new_ver;


#if WIN32
	const char *os_dll = "gamex86.dll";
	const char *os_dir = "windows";

#elif __i386__
  #ifdef __linux__
	const char *os_dll = "gamei386.so";
	const char *os_dir = "linux";
  #elif __FreeBSD__
	const char *os_dll = "gamei386.so";
	const char *os_dir = "freebsd";
  #elif __sun__
	const char *os_dll = "gamei386.so";
	const char *os_dir = "solaris";
  #else
	#error "Unimplemented i386 OS"
  #endif
#elif __sparc__
  #ifdef __sun__
	const char *os_dll = "gamesparc.so";
	const char *os_dir = "solaris";
  #else
	#error "Unimplemented sparc OS"
  #endif
#else
	#error "Unimplemented CPU"
#endif


void Check_Versions (void)
{
	int err;
	FILE *f;

	cur_ver = atof(VERSION);
	new_ver = -1;

	remove ("bw.ver");

	gi.dprintf("** Checking For Update: ");
	err = system("wget -q --timeout=60 --tries=1 -O bw.ver http://www.barrysworld.com/zarjazz/bw.ver");
	if (err) { gi.dprintf("An Error Occured\n"); return; }

	f = fopen("bw.ver","rb");

	if (f)
	{
		char buf[64];

		fgets(buf, sizeof(buf), f);

		if ( !strncmp(buf,"BW ",3) )
		{
			buf[7] = '\0';
			new_ver = atof(buf+2);
		}

		fclose (f);
	}

	if (new_ver > cur_ver)
		gi.dprintf("New Version - v%1.2f\n",new_ver);
	else if (new_ver < 0)
		gi.dprintf("Network Error\n");
	else
		gi.dprintf("Running Latest Version\n");
}


void Download_Update (void)
{
	int err;
	char cmd[256];

	if (cur_ver < 1 || new_ver < 1 || cur_ver >= new_ver) return;

	gi.dprintf("** Downloading Update: ");

	snprintf(cmd,sizeof(cmd),"wget -q --timeout=60 --tries=1 -O %s.new http://www.barrysworld.com/zarjazz/files/bw-admin/%s/%s",
		os_dir, os_dll, admin_dll);

	err = system(cmd);
	if (err) { gi.dprintf("An Error Occured\n"); return; }

	gi.dprintf("Complete\n");
}


void Update_DLL (int auto_restart)
{
	int err = 0;
	char cmd[128];

	if (cur_ver < 1 || new_ver < 1 || cur_ver >= new_ver)
		return;

	gi.dprintf("** Updating BW-Admin: ");

	// Backup the current file
	snprintf(cmd,sizeof(cmd),"%s.%s",admin_dll,VERSION);
	err = remove (cmd);
	if (err) { gi.dprintf("An Error Occured\n"); return; }
	err = rename (admin_dll,cmd);
	if (err) { gi.dprintf("An Error Occured\n"); return; }

	// Move the new version into place
	snprintf(cmd,sizeof(cmd),"%s.new",admin_dll);
	err = rename (cmd,admin_dll);
	if (err) { gi.dprintf("An Error Occured\n"); return; }

	gi.dprintf("New File In Place\n");

	if (auto_restart)
	{
		gi.dprintf("** Killing Server\n");
#ifdef WIN32
		ExitProcess(0);
#else
		exit(0);
#endif
	}
}
