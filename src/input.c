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
#else
	#include <dlfcn.h>
#endif

#include <ctype.h>
#include <sys/stat.h>

#include "q2_local.h"
#include "kp_local.h"

#include "admin.h"

// extern's, etc

#ifdef WIN32
	extern HINSTANCE hDLL;	// Handle to DLL
#else
	extern void *hDLL;
#endif

typedef game_export_t * (*LPGETGAMEAPI) (game_import_t *import);

// Extern's

extern q2_game_import_t		q2_gi;
extern q2_game_export_t		q2_globals;
extern q2_game_export_t *	q2_export;

extern kp_game_import_t		kp_gi;
extern kp_game_export_t		kp_globals;
extern kp_game_export_t *	kp_export;


extern int overflow;

extern int banner;
extern int botkick;
extern int check_bots;

extern int spectators;

extern int flood_ban;
extern int flood_time;
extern int flood_lines;
extern int flood_repeat;

extern int use_say_team;

extern int maxrate, maxping, minping, maxfps;
extern int reject_msg;
extern int strict;
extern int jitter;
extern int capkick;

extern char *MOTD;
extern char *STUFF;

extern qboolean server_locked;
extern qboolean exec_conf;

extern qboolean exec_flag;
extern char exec_file[MAX_OSPATH];

extern char admin_passwd[32];
extern char super_passwd[32];

extern char gamedir[64], dllname[64];
extern char gamedll[MAX_OSPATH];

extern char admin_dll[MAX_OSPATH];

extern char auth_file[MAX_OSPATH];

extern int have_wget;
extern int auto_download ;
extern int auto_install;
extern int auto_restart;

extern int p_limit;
extern int p_kickban;
extern float p_percent;

extern unsigned itemspawns;

extern int logging;

extern int track_connect;
extern int track_disconnect;
extern int track_renames;
extern int track_refused;

extern qboolean bans_active;
extern char bansfile[64];

extern char *VERSION;

// Proto's

void ClientBegin (edict_t *ent);
void ClientCommand (edict_t *ent);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
void ClientThink (edict_t *ent, usercmd_t *cmd);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void G_RunFrame (void);
void InitGame (void);
void ServerCommand (void);
void ShutdownGame (void);
void SpawnEntities (char *mapname, char *entities, char *spawnpoint);

void LoadNameLocks (char *fname);


/*
=================
GetFile

Reads entire file to a malloc'd buffer
Used in checksum stuff
=================
*/

static char *GetFile (char *fname, unsigned int *sz)
{
	struct stat filestats;
	char *buf,*s,line[128];
	unsigned int size;
	FILE *f;

#ifdef WIN32
	snprintf(line,sizeof(line),"%s\\%s",gamedir,fname);
#else
	snprintf(line,sizeof(line),"%s/%s",gamedir,fname);
#endif

	s = line;
	if ( !(f = fopen(s,"r")) )
	{
		s = fname;
		if ( !(f = fopen(s,"r")) )
		{
			gi.dprintf ("** Unable To Read File: %s\n",fname);
			return NULL;
		}
	}

	stat(s,&filestats);
	size = filestats.st_size;

	buf = (char *) malloc (size + 1);
	fread(buf,size,1,f);
	buf[size] = '\0';	// Ensure 0 byte terminator
	fclose(f);

	if (sz) *sz = size;

	return buf;
}


/*
=================
Read_ConfigFile

Parse the bw-admin.cfg file
=================
*/

static int Read_ConfigFile (char *fname)
{
	FILE *file;

	char line[128];
	char *s;
	// Find bw-admin.cfg file

	file = fopen(fname, "r");
	if (!file) return 0;

	// Parse the config

	while ( fgets(line, sizeof(line), file) )
	{
		int l;
		char s1[sizeof(line)],s2[sizeof(line)];

		s = line;
		// skip initial white space chars
		while (*s && isspace(*s)) s++;
		// skip emtpy lines and comments
		if ( !(*s) || *s == '#' || !strncmp(s,"//",2) ) continue;
		// remove trailing white space chars
		l = strlen(s);
		while (l > 0 && isspace(s[l-1])) s[--l] = '\0';

		// is this a valid line?
		if ( !(s = strchr(s,'=')) ) continue;
		*s++ = '\0';

		if ( sscanf(line," %s ",s1) < 1 )
			continue;

		if ( sscanf(s," %s ",s2) < 1 )
			continue;

#ifdef _DEBUG
//		gi.dprintf("Parsed: %s=%s\n",s1,s2);
#endif
		// Misc

		if ( !Q_stricmp(s1,"maxrate") )
			maxrate = atoi(s2);

		else if ( !Q_stricmp(s1,"auth") )
			strncpy(auth_file,s2,sizeof(auth_file));

		else if ( !Q_stricmp(s1,"exec") )
			strncpy(exec_file,s2,sizeof(exec_file));

		else if ( !Q_stricmp(s1,"banner") )
			banner = atoi(s2);

		else if ( !Q_stricmp(s1,"dll") )
#ifndef WIN32
			strncpy(dllname,s2,sizeof(dllname));
#else
			;
#endif
		else if ( !Q_stricmp(s1,"reload") )
			exec_conf = atoi(s2);

		else if ( !Q_stricmp(s1,"itemspawns") )
			itemspawns = atoi(s2);

		else if ( !Q_stricmp(s1,"spectators") )
			spectators = atoi(s2);

		// Capping

		else if ( !Q_stricmp(s1,"maxping") )
			maxping = atoi(s2);

		else if ( !Q_stricmp(s1,"minping") )
			minping = atoi(s2);

		else if ( !Q_stricmp(s1,"reject_msg") )
			reject_msg = atoi(s2);

		else if ( !Q_stricmp(s1,"maxfps") )
			maxfps = atoi(s2);

		else if ( !Q_stricmp(s1,"capkick") )
			capkick = atoi(s2);

		// Passwords

		else if ( !Q_stricmp(s1,"admin") )
			strncpy(admin_passwd,s2,sizeof(admin_passwd));

		else if ( !Q_stricmp(s1,"super") )
			strncpy(super_passwd,s2,sizeof(super_passwd));

		// MOTD

		else if ( !Q_stricmp(s1,"motd") )
		{
			if (MOTD) { free(MOTD);  MOTD  = NULL; }
			MOTD = GetFile(s2,NULL);
			if (MOTD) gi.dprintf("** Using MOTD File : %s\n", s2);
		}

		// Stuff Command File

		else if ( !Q_stricmp(s1,"stuff") )
		{
			if (STUFF) { free(STUFF); STUFF = NULL; }
			STUFF = GetFile(s2,NULL);
			if (STUFF) gi.dprintf("** Using STUFF File : %s\n", s2);
		}

		// Flood Protection

		else if ( !Q_stricmp(s1,"flood_ban") )
			flood_ban = atoi(s2);

		else if ( !Q_stricmp(s1,"flood_time") )
			flood_time = atoi(s2);

		else if ( !Q_stricmp(s1,"flood_repeat") )
			flood_repeat = atoi(s2);

		else if ( !Q_stricmp(s1,"flood_lines") )
		{
			flood_lines = atoi(s2);
			if (flood_lines > 32) flood_lines = 32;
		}

		else if ( !Q_stricmp(s1,"use_say_team") )
			use_say_team = atoi(s2);

		// Bot Stuff

		else if ( !Q_stricmp(s1,"botcheck") )
			check_bots = atoi(s2);

		else if ( !Q_stricmp(s1,"botkick") )
			botkick = atoi(s2);

		else if ( !Q_stricmp(s1,"strict") )
			strict = atoi(s2);

		else if ( !Q_stricmp(s1,"jitter") )
			jitter = atoi(s2);

		// Voting Stuff

		else if ( !Q_stricmp(s1,"p_limit") )
			p_limit = atoi(s2);

		else if ( !Q_stricmp(s1,"p_kickban") )
			p_kickban = atoi(s2);

		else if ( !Q_stricmp(s1,"p_percent") )
			p_percent = atof(s2);

		// Logging

		else if ( !Q_stricmp(s1,"logging") )
		{
			logging = atoi(s2);

			if (logging == 1) // Create new log file
			{
				FILE *f;
#ifdef WIN32
				snprintf(line,sizeof(line),"%s\\%s",gamedir,"BW-Admin.log");
#else
				snprintf(line,sizeof(line),"%s/%s",gamedir,"BW-Admin.log");
#endif
				f = fopen(line, "w");
				fclose (f);
			}
		}

		// Client Tracking

		else if ( !Q_stricmp(s1,"track_connect") )
			track_connect = atoi(s2);

		else if ( !Q_stricmp(s1,"track_disconnect") )
			track_disconnect = atoi(s2);

		else if ( !Q_stricmp(s1,"track_renames") )
			track_renames = atoi(s2);

		else if ( !Q_stricmp(s1,"track_refused") )
			track_refused = atoi(s2);

		// Auto Version Update
		else if ( !Q_stricmp(s1,"have_wget") )
			have_wget = atoi(s2);

		else if ( !Q_stricmp(s1,"auto_download") )
			auto_download = atoi(s2);

		else if ( !Q_stricmp(s1,"auto_install") )
			auto_install = atoi(s2);

		else if ( !Q_stricmp(s1,"auto_restart") )
			auto_restart = atoi(s2);

		// Overflow / Battle Ground Mod Hack

		else if ( !Q_stricmp(s1,"overflow") )
			overflow = atoi(s2);

		// Unknown option
		else
			gi.dprintf("Unknown Option: %s\n",s1);
	}

	fclose(file);

	return -1;
}


/*
=================
List_BansFiles

=================
*/

static void List_BansFiles (void)
{
	struct stat st;
	// Find server bans file
#ifdef WIN32
	snprintf(bansfile,sizeof(bansfile),"%s\\%s",gamedir,"server_bans.txt");
#else
	snprintf(bansfile,sizeof(bansfile),"%s/%s",gamedir,"server_bans.txt");
#endif

	if ( !stat("server_bans.txt",&st) )
	{
		bans_active = true;
		gi.dprintf("** Using Bans File : %s\n", "server_bans.txt");
	}

	if ( !stat(bansfile,&st) )
	{
		bans_active = true;
		gi.dprintf("** Using Bans File : %s\n", bansfile);
	}
}

/*
=================
GetGameType
=================
*/

void GetGameType (void *import, void *cprintf)
{
	struct stat st;
	q2_game_import_t *q2_import;
	kp_game_import_t *kp_import;

	if ( !stat("quake2.exe",&st) || !stat("quake2",&st) || !stat("q2ded",&st) )
	{
		game = QUAKE2;
#ifdef _DEBUG
		fprintf(stderr,"Server Type: %s\n", "Quake2");
#endif
	}
	else if ( !stat("kingpin.exe",&st) || !stat("kingpin",&st) )
	{
		game = KINGPIN;
#ifdef _DEBUG
		fprintf(stderr,"Server Type: %s\n", "KingPin");
#endif
	}
	else
	{
		fprintf(stderr,"Unable to find server executable in current directory\n");
#ifdef WIN32
		ExitProcess(1);
#else
		exit(1);
#endif
	}

	switch (game)
	{
	case QUAKE2:
		q2_import = ((q2_game_import_t *) import);
		q2_gi = *q2_import;
		// Patch in the new cprintf
		q2_import->cprintf = cprintf;
		// Create our mapping gi structure
		gi.bprintf			= q2_gi.bprintf;
		gi.dprintf			= q2_gi.dprintf;
		gi.cprintf			= q2_gi.cprintf;
		gi.centerprintf		= q2_gi.centerprintf;
		gi.error			= q2_gi.error;
		gi.unlinkentity		= q2_gi.unlinkentity;
		gi.unicast			= q2_gi.unicast;
		gi.WriteByte		= q2_gi.WriteByte;
		gi.WriteString		= q2_gi.WriteString;
		gi.cvar				= q2_gi.cvar;
		gi.argc				= q2_gi.argc;
		gi.argv				= q2_gi.argv;
		gi.args				= q2_gi.args;
		gi.AddCommandString	= q2_gi.AddCommandString;
		break;
	case KINGPIN:
		kp_import = ((kp_game_import_t *) import);
		kp_gi = *kp_import;
		// Patch in the new cprintf
		kp_import->cprintf = cprintf;
		// Create our mapping gi structure
		gi.bprintf			= kp_gi.bprintf;
		gi.dprintf			= kp_gi.dprintf;
		gi.cprintf			= kp_gi.cprintf;
		gi.centerprintf		= kp_gi.centerprintf;
		gi.error			= kp_gi.error;
		gi.unlinkentity		= kp_gi.unlinkentity;
		gi.unicast			= kp_gi.unicast;
		gi.WriteByte		= kp_gi.WriteByte;
		gi.WriteString		= kp_gi.WriteString;
		gi.cvar				= kp_gi.cvar;
		gi.argc				= kp_gi.argc;
		gi.argv				= kp_gi.argv;
		gi.args				= kp_gi.args;
		gi.AddCommandString	= kp_gi.AddCommandString;
		break;
	}
}


/*
=================
LoadConfig
=================
*/

void LoadConfig (void)
{
	int x = 0;
	char *s,line[128];
	char *cfg = "bw-admin.cfg";

	// Defaults
	banner		= 1;
	botkick		= 1;
	check_bots	= 1;
	itemspawns	= 0;

	maxping		= 0;
	minping		= 0;
	maxfps		= 0;
	maxrate		= 25000;

	logging		= 0;
	reject_msg	= 1;
	strict		= 0;	// Tricky one this.
	jitter		= 1;

	spectators	= 1;

	capkick		= 0;

	flood_ban	 = 15;
	flood_time	 = 0;
	flood_lines	 = 0;
	flood_repeat = 2;

	use_say_team = 1;

	have_wget = 0;
	auto_download = 1;
	auto_install = 0;
	auto_restart = 0;

	track_connect = track_disconnect = track_renames = track_refused = 0;

	admin_passwd[0]	= '\0';
	super_passwd[0]	= '\0';

	auth_file[0]	= '\0';
	exec_file[0]	= '\0';

	bans_active = false;
	exec_conf = false;

	p_limit = 0;
	p_kickban = 0;
	p_percent = 50.0;

	overflow = 0;

	// Load The Config File(s)

	s = gi.cvar("bwcfg", "", 0)->string;
	if (s && *s != '\0') cfg = s;

#ifdef WIN32
	snprintf(line,sizeof(line),"%s\\%s",gamedir,cfg);
#else
	snprintf(line,sizeof(line),"%s/%s",gamedir,cfg);
#endif

	x |= Read_ConfigFile(cfg);
	x |= Read_ConfigFile(line);

	if (!x) gi.error ("[BW-Admin] '%s' File Not Found\n",cfg);

	// Fix some stuff for game specifics

	if (game != QUAKE2)
	{
		itemspawns = 0;
		overflow = 0;
		strict = 0;
	}

	// Print the settings
	s = gi.cvar("maxfps", "", 0)->string;
	if (s && *s != '\0') maxfps = atoi(s);

	s = gi.cvar("maxrate", "", 0)->string;
	if (s && *s != '\0') maxrate = atoi(s);

	if (itemspawns)  gi.dprintf("** Item Spawns: %d\n", itemspawns);
	if (maxrate > 0 && maxrate < 25000) gi.dprintf("** Server Max Rate: %d\n", maxrate);
	if (maxfps > 0)  gi.dprintf("** Server Max FPS: %d\n", maxfps);
	if (maxping > 0) gi.dprintf("** Server Max Ping: %d\n", maxping);
	if (minping > 0) gi.dprintf("** Server Min Ping: %d\n", minping);
	if (p_limit > 0) gi.dprintf("** Player Vote Kicking Enabled\n");

	// Load the auth name/password list
	if (auth_file[0]) LoadNameLocks(auth_file);

	List_BansFiles();
}


/*
=================
LoadGameDLL

=================
*/

game_export_t *LoadGameDLL (game_import_t *import)
{
	char *s, *def_gamedir;
	struct stat st;

	LPGETGAMEAPI lpGetGameAPI;
	game_export_t *    export;

#ifndef WIN32
	gi.dprintf("\n"); // Cosmetic Fix
#endif

	gi.dprintf("** BW-Admin v%s\n",VERSION);

#ifdef WIN32
	if (hDLL) gi.error("[BW-Admin] DLL Already Loaded.\n"
		"[BW-Admin] 'gamex86.dll' not in 'Quake2\\Release\\' ?\n");
#else
	if (hDLL) gi.error("[BW-Admin] DLL Already Loaded\n");
#endif

	switch (game)
	{
	case QUAKE2:
		def_gamedir = "baseq2";
		break;
	case KINGPIN:
		def_gamedir = "main";
		break;
	}

	strcpy(gamedir, gi.cvar ("game", def_gamedir, CVAR_SERVERINFO)->string);
	if (gamedir[0] == '\0') strcpy(gamedir,def_gamedir);

#ifdef WIN32
	strcpy (dllname,"gamex86.dll");
#else
	strcpy (dllname,"gamei386.so");
#endif

	// Find the admin dll location
#ifdef WIN32
	strcpy(admin_dll,"release\\gamex86.dll");
	if ( stat(admin_dll,&st) )
		snprintf(admin_dll,sizeof(admin_dll),"%s\\gamex86.dll",gamedir);
#else
// Under Unix, the dll is always in the game dir.
  #ifdef __i386__
	snprintf(admin_dll,sizeof(admin_dll),"%s/gamei386.so",gamedir);
  #elif __sparc__
	snprintf(admin_dll,sizeof(admin_dll),"%s/gamesparc.so",gamedir);
  #else
	#error "Unimplemented Unix OS"
  #endif
#endif /* WIN32 */

	if (overflow)
		s = gi.cvar("bw-admin", VERSION, 0)->string;
	else
		s = gi.cvar("bw-admin", VERSION, CVAR_SERVERINFO)->string;

	// Load the DLL .cfg file
	LoadConfig();

	// Load the game DLL
#ifdef WIN32
	snprintf(gamedll,sizeof(gamedll),"%s\\%s",gamedir,dllname);
	hDLL = LoadLibrary(gamedll);
#else
	snprintf(gamedll,sizeof(gamedll),"%s/%s",gamedir,dllname);
	hDLL = dlopen(gamedll, RTLD_LAZY); // There are some stupid people out there :/
#endif

	if (!hDLL) gi.error("[BW-Admin] Unable to load DLL: %s\n", gamedll);

	switch (game)
	{
	case QUAKE2:
		gi.dprintf("** %s Game DLL Loaded: %s\n", "Quake2", gamedll);
		break;
	case KINGPIN:
		gi.dprintf("** %s Game DLL Loaded: %s\n", "KingPin", gamedll);
		break;
	}

#ifdef WIN32
	lpGetGameAPI = (LPGETGAMEAPI) GetProcAddress(hDLL, "GetGameAPI");
#else
	lpGetGameAPI = (LPGETGAMEAPI) dlsym(hDLL, "GetGameAPI");
#endif

	if (!lpGetGameAPI)
	{
		// handle the error
#ifdef WIN32
		FreeLibrary(hDLL);
#else
		dlclose(hDLL);
#endif
		gi.error("[BW-Admin] Unable to find GetGameAPI()\n");
	}

	// call the function
	export = lpGetGameAPI (import);
	if (!export) gi.error("[BW-Admin] Error calling GetGameAPI()\n");

	// -------

	switch (game)
	{
	case QUAKE2:
		q2_export = (q2_game_export_t *) export;
		q2_globals = *q2_export;

		globals.ClientBegin				= q2_export->ClientBegin;
		globals.ClientCommand			= q2_export->ClientCommand;
		globals.ClientConnect			= q2_export->ClientConnect;
		globals.ClientDisconnect		= q2_export->ClientDisconnect;
		globals.ClientThink				= q2_export->ClientThink;
		globals.ClientUserinfoChanged	= q2_export->ClientUserinfoChanged;
		globals.RunFrame				= q2_export->RunFrame;
		globals.Init					= q2_export->Init;
		globals.ServerCommand			= q2_export->ServerCommand;
		globals.Shutdown				= q2_export->Shutdown;
		globals.SpawnEntities			= q2_export->SpawnEntities;

		q2_export->ClientBegin			= ClientBegin;
		q2_export->ClientCommand		= ClientCommand;
		q2_export->ClientConnect		= ClientConnect;
		q2_export->ClientDisconnect		= ClientDisconnect;
		q2_export->ClientThink			= ClientThink;
		q2_export->ClientUserinfoChanged= ClientUserinfoChanged;
		q2_export->RunFrame				= G_RunFrame;
		q2_export->Init					= InitGame;
		q2_export->ServerCommand		= ServerCommand;
		q2_export->Shutdown				= ShutdownGame;
		q2_export->SpawnEntities		= SpawnEntities;
		break;
	case KINGPIN:
		kp_export = (kp_game_export_t *) export;
		kp_globals = *kp_export;

		globals.ClientBegin				= kp_export->ClientBegin;
		globals.ClientCommand			= kp_export->ClientCommand;
		globals.ClientConnect			= kp_export->ClientConnect;
		globals.ClientDisconnect		= kp_export->ClientDisconnect;
		globals.ClientThink				= kp_export->ClientThink;
		globals.ClientUserinfoChanged	= kp_export->ClientUserinfoChanged;
		globals.RunFrame				= kp_export->RunFrame;
		globals.Init					= kp_export->Init;
		globals.ServerCommand			= kp_export->ServerCommand;
		globals.Shutdown				= kp_export->Shutdown;
		globals.SpawnEntities			= kp_export->SpawnEntities;

		kp_export->ClientBegin			= ClientBegin;
		kp_export->ClientCommand		= ClientCommand;
		kp_export->ClientConnect		= ClientConnect;
		kp_export->ClientDisconnect		= ClientDisconnect;
		kp_export->ClientThink			= ClientThink;
		kp_export->ClientUserinfoChanged= ClientUserinfoChanged;
		kp_export->RunFrame				= G_RunFrame;
		kp_export->Init					= InitGame;
		kp_export->ServerCommand		= ServerCommand;
		kp_export->Shutdown				= ShutdownGame;
		kp_export->SpawnEntities		= SpawnEntities;
		break;
	}

	return export;
}
