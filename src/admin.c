/*============================================================================
	BW-Admin - Interface DLL for Quake II Based Mods

	Author	: Vincent 'Zarjazz' Sweeney
	Email	: zarjazz@barrysworld.com
	WebPage	: http://www.barrysworld.com/

	Copyright © 1998-2000 BarrysWorld Ltd - All rights reserved.
	----
	$Id$

	Contains more hacks and ugly code than I thought possible
	in 1 file but it works (*cough*). This is what comes from
	a program that evolves over time with no forward planning :(

	Lets not even mention 'client(ent)->pers->connected' ...

	At least the IP and other ban code is kinda reasonable (apart
	from the IP connection count stuff).

	Fuck it, the code is only for Quake2 so who cares :)
============================================================================*/

// Include basic Win32 Files
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
	#include <windows.h>
#else
	#include <dlfcn.h>
#endif

// Project Includes

#include "q2_local.h"
#include "kp_local.h"

#include "admin.h"
#include "codec.h"
#include "strings.h"

//===================================================================
// A load of typedef's, #defines & globals
//===================================================================

#define Q2(ent)	((q2ent_t *) ent)
#define KP(ent)	((kpent_t *) ent)

extern char *VERSION;

//
// More Stuff
//

game_type_t game;

game_import_t	gi;
game_export_t	globals;

q2_game_import_t	q2_gi;
q2_game_export_t	q2_globals;
q2_game_export_t *	q2_export;

kp_game_import_t	kp_gi;
kp_game_export_t	kp_globals;
kp_game_export_t *	kp_export;

typedef struct
{
	int			framenum;
	float		time;
} level_locals_t;

level_locals_t level;

cvar_t	*maxclients;

//
// Global to use in "cprintf"
//

edict_t *last_ent;
time_t tm_cur = 0;

//
// Misc Settings
//

int overflow = 0;		// Overflow (BattleGrounds Mod) Hack
int offset = 0;

int banner;
int spectators;

int	flood_ban;
int	flood_time;
int	flood_lines;
int	flood_repeat;

int use_say_team;

int maxping, minping, maxfps, maxrate;
int reject_msg;
int capkick;

char *MOTD  = NULL;
char *STUFF = NULL;

qboolean server_locked = false;
qboolean exec_conf;

qboolean exec_flag;
char exec_file[MAX_OSPATH];

char admin_passwd[32];
char super_passwd[32];

char gamedir[64], dllname[64];
char gamedll[MAX_OSPATH];

//
// Wget
//

void Check_Versions (void);
void Download_Update (void);
void Update_DLL (int auto_restart);

int have_wget;
int auto_download ;
int auto_install;
int auto_restart;

char admin_dll[MAX_OSPATH];

//
// Auth
//

char *Is_NameLocked (char *name);
char *Find_LockedName (char *pass);

char auth_file[MAX_OSPATH];

//
// Player Admin
//

edict_t *p_target = NULL;
int p_limit, p_kickban, p_frame;
float p_percent;

/* -- */

typedef struct itemspawn_s
{
	unsigned	flag;
	char		*name;
	char		*ammo;
	char		*desc;
} itemspawn_t;

unsigned itemspawns;

#define ITEMSPAWN_SHOTGUN			(1 <<  0)
#define ITEMSPAWN_SUPER_SHOTGUN		(1 <<  1)
#define ITEMSPAWN_MACHINEGUN		(1 <<  2)
#define ITEMSPAWN_CHAINGUN			(1 <<  3)
#define ITEMSPAWN_GRENADES			(1 <<  4)
#define ITEMSPAWN_GRENADE_LAUNCHER	(1 <<  5)
#define ITEMSPAWN_ROCKET_LAUNCHER	(1 <<  6)
#define ITEMSPAWN_HYPERBLASTER		(1 <<  7)
#define ITEMSPAWN_RAILGUN			(1 <<  8)
#define ITEMSPAWN_BFG				(1 <<  9)

#define ITEMSPAWN_QUAD				(1 << 10)
#define ITEMSPAWN_INVULNERABILITY	(1 << 11)

#define ITEMSPAWN_POWER_SHIELD		(1 << 12)

itemspawn_t item_table [] =
{
	{ ITEMSPAWN_SHOTGUN,			"weapon_shotgun",			NULL,			"Shotgun"},
	{ ITEMSPAWN_SUPER_SHOTGUN,		"weapon_supershotgun",		NULL,			"Super Shotgun"},
	{ ITEMSPAWN_MACHINEGUN,			"weapon_machinegun",		NULL,			"Machinegun"},
	{ ITEMSPAWN_CHAINGUN,			"weapon_chaingun",			NULL,			"Chaingun"},
	{ ITEMSPAWN_GRENADES,			"ammo_grenades",			NULL,			"Grenades"},
	{ ITEMSPAWN_GRENADE_LAUNCHER,	"weapon_grenadelauncher",	NULL,			"Grenade Launcher"},
	{ ITEMSPAWN_ROCKET_LAUNCHER,	"weapon_rocketlauncher",	"ammo_rockets",	"Rocket Launcher"},
	{ ITEMSPAWN_HYPERBLASTER,		"weapon_hyperblaster",		NULL,			"Hyperblaster"},
	{ ITEMSPAWN_RAILGUN,			"weapon_railgun",			"ammo_slugs",	"Railgun"},
	{ ITEMSPAWN_BFG,				"weapon_bfg",				NULL,			"BFG10K"},

	{ ITEMSPAWN_QUAD,				"item_quad",				NULL,			"Quad"},
	{ ITEMSPAWN_INVULNERABILITY,	"item_invulnerability",		NULL,			"Invulnerability"},

	{ ITEMSPAWN_POWER_SHIELD,		"item_power_shield",		NULL,			"Power Shield"},

/*
	{ ITEMSPAWN_SHELLS,				"ammo_shells",				NULL,			"Shells"},
	{ ITEMSPAWN_BULLETS,			"ammo_bullets",				NULL,			"Bullets"},
	{ ITEMSPAWN_CELLS,				"ammo_cells",				NULL,			"Cells"},

	{ ITEMSPAWN_ARMOR_BODY,			"item_armor_body",			NULL,			"Body Armor"},
	{ ITEMSPAWN_ARMOR_COMBAT,		"item_armor_combat",		NULL,			"Combat Armor"},
	{ ITEMSPAWN_ARMOR_JACKET,		"item_armor_jacket",		NULL,			"Jacket Armor"},
	{ ITEMSPAWN_ARMOR_SHARD,		"item_armor_shard",			NULL,			"Armor Shard"},
	{ ITEMSPAWN_POWER_SCREEN,		"item_power_screen",		NULL,			"Power Screen"},
*/

	{ 0,						NULL,						NULL }
};

/* -- */

int logging;

int track_connect;
int track_disconnect;
int track_renames;
int track_refused;

qboolean bans_active;

char bansfile[64];

/* -- */

typedef enum
{
	BOT_CONNECT,
	BOT_IMPULSE,
	BOT_FOV,
	BOT_PING,
	BOT_RATE,
	BOT_FPS,
	BOT_ZORBOT,
	BOT_RATBOT,
	BOT_FILTER,
	BOT_JITTER,
	BOT_TIMESCALE,
	BOT_INFOFLOOD,
} bot_t;

int botkick;
int check_bots;
int strict;
int jitter;

#define NUM_BOT_CHECKS	5

static	char	botstr_cmd	[NUM_BOT_CHECKS][16];
static	char	botstr_check[NUM_BOT_CHECKS][16];

// Strings used in getting client side info

static	char str_fps[8];
static	char str_tsc[8];

#define PRIV_NONE	0
#define PRIV_SUPER	1
#define PRIV_ADMIN	2

typedef struct jitter_s
{
    float bot_anglerate;
    float bot_timer;
    short bot_angles[3];
    byte  bot_attack;
    byte  bot_count;
} jitter_t;

typedef struct zclient_s
{
	edict_t *	ent;

	qboolean	botcheck;
	int			bottest;
	time_t		bottime;
	jitter_t	jitter;

	qboolean	impcheck;
	int			impvalue;
	int			frame_impulse;
	time_t		imptime;

	int			capcount;
	int			retry;

	int			info_name_change;
	int			info_skin_change;

	unsigned	admin_lvl;
	qboolean	vote;

	int			frame_chk_fps;
	int			frame_chk_tsc;

	unsigned	nlines;
	time_t		mute;
	time_t		flood[32];

	int			rep_count;
	time_t		rep_time;
	char		rep_flood[96];

	char		ip[32];
	char		skin[64];
	char		netname[64];
	char		rat_name[16];
	char		auth_pass[32];
} zclient_t;

zclient_t	clients[128];

//
// Proto's
//

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

void stuffcmd(edict_t *e, char *s);

void Log_Text (const char *str, ...);
void Log_Track (char *str, ...);

void ResetNameLocks (void);

void LoadConfig		(void);
game_export_t *LoadGameDLL (game_import_t *import);

void GetGameType	(void *import, void *cprintf);

//===================================================================

#ifdef WIN32
	HINSTANCE hDLL = NULL;	// Handle to DLL
#else
	void *hDLL = NULL;
	#define __inline __inline__
#endif


void stuffcmd (edict_t *e, char *s)
{
	switch (game)
	{
	case QUAKE2:
		gi.WriteByte (11);
		break;
	case KINGPIN:
		gi.WriteByte (13);
		break;
	}

	gi.WriteString (s);
	gi.unicast (e, true);

#ifdef _DEBUG
	gi.dprintf("Stuff: %s",s);
#endif
}

zclient_t *get_client (edict_t *ent)
{
	int i;
	i = ENT_NUM(ent);
	return (clients + i - 1);
}

char *get_client_ip (edict_t *ent) { return get_client(ent)->ip; }

/* ------- Wrapper Routines ------- */

char *classname (edict_t *ent)
{
	switch (game)
	{
	case QUAKE2:
		return Q2(ent)->classname;
		break;
	case KINGPIN:
		return KP(ent)->classname;
		break;
	}

	return NULL;
}

client_t *client (edict_t *ent)
{
	switch (game)
	{
	case QUAKE2:
		return Q2(ent)->client;
		break;
	case KINGPIN:
		return KP(ent)->client;
		break;
	}

	return NULL;
}

qboolean inuse (edict_t *ent)
{
	switch (game)
	{
	case QUAKE2:
		return Q2(ent)->inuse;
		break;
	case KINGPIN:
		return KP(ent)->inuse;
		break;
	}

	return false;
}

char *netname (edict_t *ent)
{
	switch (game)
	{
	case QUAKE2:
		return Q2(ent)->client->pers.netname;
		break;
	case KINGPIN:
		return KP(ent)->client->pers.netname;
		break;
	}

	return NULL;
}

int ping (edict_t *ent)
{
	switch (game)
	{
	case QUAKE2:
		return Q2(ent)->client->ping;
		break;
	case KINGPIN:
		return KP(ent)->client->ping;
		break;
	}

	return 0;
}

int svflags (edict_t *ent)
{
	switch (game)
	{
	case QUAKE2:
		return Q2(ent)->svflags;
		break;
	case KINGPIN:
		return KP(ent)->svflags;
		break;
	}

	return 0;
}

char *userinfo (edict_t *ent)
{
	switch (game)
	{
	case QUAKE2:
		return Q2(ent)->client->pers.userinfo;
		break;
	case KINGPIN:
		return KP(ent)->client->pers.userinfo;
		break;
	}

	return 0;
}

/*
=================
KickBot

go on, take a wild guess
=================
*/

static void KickBot (edict_t *ent, bot_t type, int value)
{
	char *ip;
	zclient_t *cl;
	char *name;

	cl = get_client(ent);
	ip = cl->ip;

	name = netname(ent);

	switch (type)
	{
	case BOT_CONNECT:
//		Log_Text("player %s @ %s is a bot (attempted connection)", netname(ent), ip);
		Log_Text(STRING(STR_BOT_CONNECT), name, ip);
		break;
	case BOT_IMPULSE:
//		Log_Text("player %s @ %s is a bot (used impulse=%d)", netname(ent), ip, value);
		Log_Text(STRING(STR_BOT_IMPULSE), name, ip, value);
		break;
	case BOT_FOV:
//		Log_Text("player %s @ %s is lame (fov = %d)", netname(ent), ip, value);
		Log_Text(STRING(STR_BOT_FOV), name, ip, value);
		break;
	case BOT_PING:
//		Log_Text("player %s @ %s is faking it (zero ping)", netname(ent), ip);
		Log_Text(STRING(STR_BOT_PING), name, ip);
		break;
	case BOT_RATE:
//		Log_Text("player %s @ %s is stupid (hacked exe for 'rate')", netname(ent), ip);
		Log_Text(STRING(STR_BOT_RATE), name, ip);
		break;
	case BOT_FPS:
//		Log_Text("player %s @ %s is a llama (hacked exe for 'cl_maxfps')", netname(ent), ip);
		Log_Text(STRING(STR_BOT_FPS), name, ip);
		break;
	case BOT_ZORBOT:
//		Log_Text("player %s @ %s is determined to cheat (used zbot proxy)", netname(ent), ip);
		Log_Text(STRING(STR_BOT_ZORBOT), name, ip);
		break;
	case BOT_RATBOT:
//		Log_Text("player %s @ %s is anything but l33t (ratbot)", netname(ent), ip);
		Log_Text(STRING(STR_BOT_RATBOT), name, ip);
		break;
	case BOT_FILTER: // Rat Bot : Impulse Filter
//		Log_Text("player %s @ %s is using an impulse proxy (ratbot?)", netname(ent), ip);
		Log_Text(STRING(STR_BOT_FILTER), name, ip);
		break;
	case BOT_JITTER: // "jitter" detection
//		Log_Text("player %s @ %s is a jerk (bot like movement)", netname(ent), ip);
		Log_Text(STRING(STR_BOT_JITTER), name, ip);
		break;
	case BOT_TIMESCALE: // Hacked Timescale cheat
//		Log_Text("player %s @ %s is too speedy (timescale cheat)", netname(ent), ip);
		Log_Text(STRING(STR_BOT_TIMESCALE), name, ip);
		break;
	case BOT_INFOFLOOD: // Userinfo Flood
//		Log_Text("player %s @ %s is cpu intensive (userinfo flood)", netname(ent), ip);
		Log_Text(STRING(STR_BOT_INFOFLOOD), name, ip);
		break;
	}

	if (botkick)
	{
		gi.bprintf (PRINT_HIGH,STRING(STR_BOT_KICK));

		stuffcmd(ent, STRING(STUFF_DISCONNECT));
		stuffcmd(ent, STRING(STUFF_BAD_FPS));

		if (type = BOT_ZORBOT)
		{
			gi.WriteByte(0x00); // SVC_BAD
			gi.unicast(ent, true);
		}

		KickEnt (ent);
	}
}

/*
=================
BotCheck

Detect bot like movement
=================
*/

// ---- AdminX Bot Check ---- //

#define BOT_DETECTION_TIMER 		0.1f
#define BOT_DETECTION_COUNT 		1
#define BOT_DETECTION_ATTACK_RATE	40000.0f
#define BOT_DETECTION_RESTORE_RATE	0.0f

static qboolean BotCheck (edict_t *ent, usercmd_t *ucmd)
{
    int i;
    vec3_t change;
    byte attack;
    float rate;
	jitter_t *jit;
	zclient_t *cl;

	if (svflags(ent) & (SVF_NOCLIENT | SVF_DEADMONSTER)) return 0;

	cl = get_client(ent);
	jit = &cl->jitter;

	attack = ucmd->buttons & BUTTON_ATTACK;
	if (attack ^ jit->bot_attack)
	{
		jit->bot_attack = attack;
		
		// if the first criteria doesn't hold then skip calculations
		if (!attack && (jit->bot_anglerate < BOT_DETECTION_ATTACK_RATE))
			goto noattack;
	
		// allow RJ aliases
		if ((attack && abs(ucmd->angles[0]) == 16201) || !ucmd->msec)
		{
			jit->bot_anglerate = 0.0f;
			goto noattack;
		}
		
		for (i = 0; i < 2; i++)
		{
			// angular change since last update
			change[i] = ucmd->angles[i] - jit->bot_angles[i];
			// compensate for -180 <-> 180 rollover
			if (change[i] > 32768.0f)
				change[i] -= 65536.0f;
			else if (change[i] < -32768.0f)
				change[i] += 65536.0f;
		}
		
		// calculate a combined (pitch and yaw) angular rate of change
		rate = (change[0]*change[0] + change[1]*change[1]) / ucmd->msec;

		if (attack)
		{
			jit->bot_anglerate = rate;
			return 0;
		}		

		if (rate <= BOT_DETECTION_RESTORE_RATE) // a bot
		{
			if (level.time > jit->bot_timer)
			{
				jit->bot_timer = level.time + BOT_DETECTION_TIMER;
				jit->bot_count = 1;
			}
			else
				jit->bot_count++;
			
			// having a BOT_DETECTION_COUNT > 1 is just extra precaution
			if (jit->bot_count == BOT_DETECTION_COUNT) return 1;
		}
    }

noattack:
    // only remember angles if it's not an attack
    if (!attack) for (i = 0; i < 2; i++)
		jit->bot_angles[i] = ucmd->angles[i];

	return 0;
}

/*
=================
CheckRate

Check for bad userinfo (rate / fov) settings
=================
*/

static void CheckRate (edict_t *ent, char *userinfo)
{
	int rate,fov;
	char *s;
	qboolean connected;

	if (!ent || !inuse(ent)) return;

	switch (game)
	{
	case QUAKE2:
		if (offset) // HACK for client(ent)->pers.connected! 
			connected = *((qboolean *) ((char *) &Q2(ent)->client->pers + offset));
		else
			connected = Q2(ent)->client->pers.connected;
		break;
	case KINGPIN:
		if (offset) // HACK for client(ent)->pers.connected! 
			connected = *((qboolean *) ((char *) &KP(ent)->client->pers + offset));
		else
			connected = KP(ent)->client->pers.connected;
		break;
	}

	if (!connected)
	{
#ifdef _DEBUG
		gi.dprintf ("CheckRate: !connected\n");
#endif
		return;
	}

	if (maxrate <= 0 || maxrate >= 25000) goto skip_rate;

	// check rate
	s = Info_ValueForKey (userinfo, "rate");

	// hacked exe check (ie replaced "rate" with some other text)
	if (s && !*s) KickBot(ent,BOT_RATE,0);

	rate = atoi(s);

	if (maxrate > 0 && rate > maxrate)
	{
		char str[32];

		sprintf(str,"%d",maxrate);

		Info_SetValueForKey(userinfo, "rate", str);

		sprintf(str,"rate %d\n",maxrate);

		stuffcmd(ent,str);
		gi.cprintf(ent,PRINT_HIGH,"[BW-Admin] Rate Capped To %d\n",maxrate);
	}

skip_rate:
	// check FOV
	s = Info_ValueForKey (userinfo, "fov");
	fov = atoi(s);

	if (fov >= 180) KickBot(ent,BOT_FOV,fov);
}

/*
=================
add_client

create a new local client struct
to store info just used by the
used by the admin dll
=================
*/

static zclient_t *add_client (edict_t *ent)
{
	zclient_t *cl;

	if (!ent) return NULL;

	switch (game)
	{
	case QUAKE2:
		if (!Q2(ent)->client) return NULL;
		break;
	case KINGPIN:
		if (!KP(ent)->client) return NULL;
		break;
	}

	cl = get_client(ent);

	memset(cl,0,sizeof(*cl));

	cl->ent = ent;
	cl->admin_lvl = PRIV_NONE;

	return cl;
}

// ---- //

/*
=================
add_impcheck

start a check if the client filters impulse's
=================
*/

static void add_impcheck (edict_t *ent, zclient_t *cl)
{
	char str[64];

	// Impulse Filter Test
	cl->impcheck = true;
	cl->frame_impulse = 0;

//	for cl->impvalue = 0; !cl->impvalue; cl->impvalue = rand () & 0xff);
	cl->impvalue = 151;
	sprintf(str,"impulse %d\n",cl->impvalue);

	stuffcmd(ent,str);
	cl->imptime = time(NULL);
}

/*
=================
add_botcheck

sends one of the bot check strings
=================
*/

static void add_botcheck (edict_t *ent)
{
	char str[96];
	zclient_t *cl;
	
	if (!inuse(ent) || !client(ent))
	{
		Log_Text("DEBUG: bad client in 'botcheck'");
		return;
	}

	if (!check_bots) return;

	cl = get_client(ent);

#ifdef _DEBUG
	gi.dprintf("bottest = %d : botcheck = %c\n", cl->bottest, cl->botcheck ? 'Y' : 'N');
#endif

	cl->bottest--;
	cl->botcheck = true;
	cl->bottime = time(NULL);

	if (!cl->bottest) // 0'th bot check == qtimer test
		snprintf(str, sizeof(str),
			"cmd say %s;%s\n", botstr_cmd[cl->bottest], botstr_check[cl->bottest]);
	else
		snprintf(str, sizeof(str),
			"%s\n%s\n", botstr_cmd[cl->bottest], botstr_check[cl->bottest]);

	stuffcmd(ent, str);
}

/*
=================
generate_string

create a random string used in the bot checks
=================
*/

static void generate_string (char *s, char c)
{
	const int a = 'a', z = 'z';
	int i,n;

	// Lets generate a random string to send as bot command

//	n = irandom(4,8);
	n = 4;

	*s++ = c;
	for (i = 0; i < n; i++)
	{
		*s++ = irandom(a,z);
	}
	*s = '\0';
}

// ---- //

const char *z_stristr(const char *s1, const char *s2);
qboolean addban_ip (char *s);
qboolean addban_name (char *s);

edict_t *FindEntByName (char *name)
{
	int i;
	edict_t *ent;

	for (i = 1; i <= (int)(maxclients->value); i++)
	{
		ent = (edict_t *) ((char *) globals.edicts + (i * globals.edict_size));
		if (!inuse(ent)) continue;

		if ( z_stristr(netname(ent),name) ) return ent;
	}

	return NULL;
}

static int ResetVotes ()
{
	int i,total = 0;
	edict_t *ent;

	for (i = 1; i <= (int)(maxclients->value); i++)
	{
		ent = (edict_t *) ((char *) globals.edicts + (i * globals.edict_size));
		if (inuse(ent)) { get_client(ent)->vote = false; total++; }
	}

	return total;
}

// ---- //

static void KickName (char *name)
{
	edict_t *ent;
	ent = FindEntByName(name);
	if (ent) KickPlayer(NULL,ent,"[BW-Admin] Server Admin Request");
}

static void AddBan_IP (char *ip)
{
	if (!addban_ip(ip)) return;

	Write_Bans(bansfile);
	Reset_Bans();
	Read_Bans(bansfile);

	Log_Text("Added IP Ban: %s",ip);
}

static void AddBan_NAME (char *name)
{
	if ( !addban_name(name) ) return;

	KickName(name);

	Write_Bans(bansfile);
	Reset_Bans();
	Read_Bans(bansfile);

	Log_Text("Added NAME Ban: %s",name);
}

// ---- Item Spawning ---- //

static void G_FreeEdict (edict_t *ent) // Hacked version
{
	int n;

	gi.unlinkentity (ent);

	n = ENT_NUM(ent);

	if (n <= (maxclients->value + BODY_QUEUE_SIZE))
	{
//		gi.dprintf("tried to free special edict\n");
		return;
	}

	memset (ent, 0, globals.edict_size);

	switch (game)
	{
	case QUAKE2:
		Q2(ent)->classname = "freed";
		Q2(ent)->freetime = level.time;
		Q2(ent)->inuse = false;
		break;
	case KINGPIN:
		KP(ent)->classname = "freed";
		KP(ent)->freetime = level.time;
		KP(ent)->inuse = false;
		break;
	}

}

static qboolean BlockedItem (edict_t *ent)
{
	itemspawn_t *ptr;

	for (ptr = item_table; ptr->desc; ptr++)
	{
		if (itemspawns & ptr->flag)
		{
			if ( !Q_stricmp(classname(ent), ptr->name) )
				return true;

			if (ptr->ammo && !Q_stricmp(classname(ent), ptr->ammo))
				return true;
		}
	}

	return false;
}

static void ResetItems (void)
{
	int i;
	edict_t *ent;

	for (i=0; i < globals.max_edicts; i++)
	{
		ent = (edict_t *) ((char *) globals.edicts + (i * globals.edict_size));

		if (!inuse(ent) || client(ent) || !classname(ent)) continue;

		if ( BlockedItem(ent) )
		{
			gi.dprintf("** Blocked Item : %s\n",classname(ent));
			G_FreeEdict(ent);
		}
	}
}

// ---- Time Dependant Checks ---- //

int time_checks (edict_t *ent, zclient_t *cl, char *cmd)
{
	char buf[64];

	// Check Player Voting
	if (p_target && (level.framenum >= p_frame))
	{
		int i,yes = 0,no = 0;
		edict_t *e;

		for (i = 1; i <= (int)(maxclients->value); i++)
		{
			e = (edict_t *) ((char *) globals.edicts + (i * globals.edict_size));
			if (inuse(e))
			{
				if (get_client(e)->vote)
					yes++;
				else
					no++;
			}
		}

		if ( (yes + no) > p_limit )
		{
			gi.bprintf(PRINT_HIGH,"[BW-Admin] Votes: %d YES - %d NO\n",yes,no);

			if (!no || (100.0 * (float) yes / (float) (yes + no)) > p_percent)
			{
				gi.bprintf(PRINT_HIGH,"[BW-Admin] Voting Passed \n");
				Log_Text("Players Voted To Kick: %s",netname(p_target));
				if (p_kickban)
					KickBanPlayer(NULL,p_target,NULL);
				else
					KickPlayer(NULL,p_target,NULL);
			}
			else
				gi.bprintf(PRINT_HIGH,"[BW-Admin] Voting Failed\n");
		}
		else
			gi.bprintf(PRINT_HIGH,"[BW-Admin] Voting Canceled\n");

		p_target = NULL;
	}

	// Max FPS Check
	if (cl->frame_chk_fps && level.framenum > cl->frame_chk_fps)
	{
		snprintf(buf,sizeof(buf),STRING(STR_GET_FPS),str_fps);
		stuffcmd(ent,buf);

		cl->frame_chk_fps = level.framenum + 300;
	}

	// Dynamic Timescale Cheat Check
	if (cl->frame_chk_tsc && level.framenum > cl->frame_chk_tsc)
	{
		snprintf(buf,sizeof(buf),STRING(STR_GET_TSC),str_tsc);
		stuffcmd(ent,buf);

		cl->frame_chk_tsc = level.framenum + 200;
	}

	// Impulse Filter Check
	if (cl->impcheck)
	{
		if ((tm_cur - cl->imptime) > 5)
		{
			// check if they are filtering the stuff's
			// if retry too many times, kick
			cl->retry++;
			if (cl->retry < 3)
				add_impcheck(ent,cl);
			else
				KickBot(ent,BOT_FILTER,0);
		}
	}

	// Timescale Check
	if ( !strcmp(cmd,str_tsc) )
	{
		if (gi.argc() == 1 || atoi(gi.argv(1)) != 1)
		{
			// If we get a single #timescale then client using hacked exe
			// Also, if $timescale != 1 then they've hacked exe
			KickBot(ent,BOT_TIMESCALE,0);
		}
		return 1;
	}

	// Max FPS Check
	if ( maxfps > 0 && !strcmp(cmd,str_fps) )
	{
		if (gi.argc() == 1)
		{
			// If we get a single #fps 1st then 99% sure client using hacked exe
			// (Kick the other 1% anyway :)

			// cl->time only set if we passed bot check and stuffed fps check
			if (cl->bottime && !cl->botcheck)
				KickBot(ent,BOT_FPS,0);
			else
				stuffcmd(ent,STRING(STR_SHOW_CL_FPS));
		}
		else
		{
			int fps;

			// They passed fps hacked exe checks
			cl->bottime = 0;

			fps = atoi( gi.argv(1) );
			if (fps <= 0 || fps > maxfps)
			{
				fps = maxfps;
				gi.cprintf(ent,PRINT_HIGH,"[BW-Admin] Max FPS Capped To %d\n",maxfps);

				if (cl && capkick > 0)
				{
					cl->capcount++;
					if (cl->capcount >= capkick)
						KickPlayer(NULL,ent,"Reset maxfps too many times");
				}
			}

			snprintf(buf,sizeof(buf),STRING(STR_SET_FPS),fps);
			stuffcmd(ent,buf);
		}
		return 1;
	}

	return 0;
}

// ---- Intercepted Calls ---- //

/*
===============
ClientBegin
===============
*/

void ClientBegin (edict_t *ent)
{
	char buf[64];
	zclient_t *cl;

	cl = get_client(ent);

	switch (game)
	{
	case QUAKE2:
		cl->bottest  = NUM_BOT_CHECKS;
		break;
	case KINGPIN:
		cl->bottest  = 0;
		break;
	}

	cl->botcheck = false;
	cl->impcheck = false;
	cl->info_name_change = 0;
	cl->info_skin_change = 0;
	cl->frame_chk_fps = 0;
	cl->frame_chk_tsc = 0;
	cl->capcount = 0;
	cl->mute = 0;

	memset(&cl->jitter,0,sizeof(cl->jitter));

	globals.ClientBegin(ent);

	gi.cprintf(ent, PRINT_HIGH, STRING(STR_TITLE),VERSION);

	gi.cprintf(ent, PRINT_HIGH, STRING(STR_COPYRIGHT));

	if (banner)
	{
		if (maxping > 0) gi.cprintf(ent, PRINT_MEDIUM,
									"** Server Maximum Ping:%5d       **\n",maxping);

		if (minping > 0) gi.cprintf(ent, PRINT_MEDIUM,
									"** Server Minimum Ping:%5d       **\n",minping);

		if (maxrate < 25000) gi.cprintf(ent, PRINT_MEDIUM,
									"** Max Rate Capped At :%5d       **\n",maxrate);

		if (maxfps > 0) gi.cprintf(ent, PRINT_MEDIUM,
									"** Max FPS Capped At  :%5d       **\n",maxfps);
	}

	// If voting is enabled
	if (p_limit > 0)
	{
		gi.cprintf(ent, PRINT_MEDIUM,
								"** Player Vote Kicking Enabled     **\n");
		gi.cprintf(ent, PRINT_MEDIUM,
								"** Use 'p_kick <name>' to kick     **\n");
	}

	// MOTD
	if (MOTD && MOTD[0]) gi.centerprintf(ent,MOTD);

	// Stuff Command List
	if (STUFF && STUFF[0]) stuffcmd(ent,STUFF);

	// Max FPS
	if (maxfps > 0)
	{
		snprintf(buf,sizeof(buf),STRING(STR_ALIAS_FPS),str_fps);
		stuffcmd(ent,buf);
	}

	// Ensure ratbot is listening
	stuffcmd(ent, STRING(STR_BEGIN_CHECK));
}

/*
===============
ClientCommand
===============
*/

void ClientCommand (edict_t *ent)
{
	zclient_t *cl;
	char *cmd,*args;
	int i;

	if (!inuse(ent) || !client(ent))
		return;		// not fully in game yet

	last_ent = ent;
	time( &tm_cur );

	cmd  = gi.argv(0);
	args = gi.args();

#ifdef _DEBUG
	gi.dprintf("Cmd: %s %s\n",cmd,gi.args());
#endif

	if ((maxping > 0 || minping > 0) && ping(ent) > 0)
	{
		if (maxping > 0 && ping(ent) > maxping)
		{
			KickPlayer(NULL,ent,"Ping Too High");
			Log_Text("Kicked %s with ping of %d", netname(ent), ping(ent));
		}

		if (minping > 0 && ping(ent) < minping)
		{
			KickPlayer(NULL,ent,"Ping Too Low");
			Log_Text("Kicked %s with ping of %d", netname(ent), ping(ent));
		}
	}

	cl = get_client(ent);

	// are we currently testing this client for a bot?
	if (cl->botcheck)
	{
		// missing sent bot command, kick the fuckers.
		if ( !strcmp(cmd, botstr_check[cl->bottest]) )
		{
			KickBot(ent,(cl->bottest) ? BOT_CONNECT : BOT_ZORBOT,0);

			cl->bottest = 0;
			cl->botcheck = false;

			return;
		}

		// did they reply with the sent bot command?
		// if they did they passed the test.
		if ( (cl->bottest && !strcmp(cmd,		botstr_cmd[cl->bottest])) ||
			(!cl->bottest && !strcmp(gi.argv(1),botstr_cmd[cl->bottest])) )
		{
#ifdef _DEBUG
			gi.dprintf("Passed Test: %d\n",cl->bottest);
#endif
			if (cl->bottest == 0)	// they are clean
			{
				char buf[64];
#ifdef _DEBUG
				gi.dprintf("Player Clean\n");
#endif
				cl->botcheck = false;
				cl->retry = 0;

				if (ping(ent) == 0)		// ping bot
				{
					KickBot(ent,BOT_PING,0);
					return;
				}

				CheckRate(ent, userinfo(ent));

				// Max FPS Check
				if (maxfps > 0)
				{
					snprintf(buf,sizeof(buf),STRING(STR_GET_FPS),str_fps);
					stuffcmd(ent,buf);

					cl->frame_chk_fps = level.framenum + 200;
				}

				// Check For Hacked 'timescale'
				snprintf(buf,sizeof(buf),STRING(STR_SET_TSC),irandom(2,9));
				stuffcmd(ent,buf);

				// Initiate Timescale Cheat Detection
				cl->frame_chk_tsc = level.framenum + 100;

				generate_string(cl->rat_name,'%');
				sprintf(buf,"name \"%s\"\n",cl->rat_name);
				stuffcmd(ent,buf);

				// Ratbot Check
				Info_SetValueForKey(userinfo(ent),"msg","0");

				stuffcmd(ent,STRING(STR_BEGIN_CHECK) + 4);
				gi.cprintf(ent,PRINT_CHAT,STRING(STR_RAT_DETECT0));

				// Impulse check in 100 seconds
				cl->frame_impulse = level.framenum + 100;
			}
			else
				add_botcheck(ent);	// run next test

			return;
		}

		// if more than 5 secs passed client may be lagged so test again.
		// client *must* pass botcheck to do anything.
		if ((tm_cur - cl->bottime) > 5)
		{
			// check if they are filtering the stuff's
			// if retry too many times, kick
			cl->retry++;
			if (cl->retry > 3) KickBot(ent,BOT_FILTER,0);

			cl->bottest++;
			add_botcheck(ent);

			return;
		}
	}
	else if (cl->bottest == NUM_BOT_CHECKS) // Start the bot test procedure
		add_botcheck(ent);

	// if botcheck cmd, ignore.
	for (i=0; i < NUM_BOT_CHECKS; i++)
		if ( !strcmp(cmd,botstr_check[i]) /*|| !strcmp(cmd,botstr_cmd[i])*/)
			return;

	if (cl->frame_impulse && level.framenum >= cl->frame_impulse)
	{
#ifdef _DEBUG
		gi.dprintf("Sending Impulse Check\n");
#endif
		add_impcheck(ent,cl);
	}

	// Time dependant checks
	if ( time_checks(ent,cl,cmd) ) return;

	// Check "say" commands
	if ( !Q_stricmp(cmd,"say") || !Q_stricmp(cmd,"say_team") )
	{
		char *s;

		// is source client banned from speaking?
		if (cl->mute && (tm_cur <= cl->mute)) return;

		// Ignore say_team commands ? 
		if (!use_say_team && !Q_stricmp(cmd,"say_team")) return;

		if (flood_repeat)
		{
			// repeat flood detection
			s = cl->rep_flood;

			if ( *s && !strcmp(s,args) )
			{
				if ((tm_cur - cl->rep_time) < 5)
				{
					if (++cl->rep_count >= flood_repeat)
					{
						if (cl->rep_count == flood_repeat)
							gi.cprintf(ent, PRINT_HIGH,
								"[BW-Admin] Repeat Flood Protection Enabled\n");
						cl->rep_time = tm_cur;
						return;
					}
				}
				else
					cl->rep_count = 0;

				cl->rep_time = tm_cur;
			}

			strncpy(cl->rep_flood,args,sizeof(cl->rep_flood));
			cl->rep_time = tm_cur;
		}
	}
	// Auth'd Nicks
	else if ( !Q_stricmp(cmd,"cl_nick") )
	{
		char *s,*p = args;
		char str[sizeof(cl->auth_pass)];

		// are we running the name check?
		if (cl->rat_name[0])
		{
			return;
		}
		else if ( s = Find_LockedName(p) )
		{
			snprintf(str,sizeof(str),"name \"%s\"\n",s);
			stuffcmd(ent,str);

			strncpy(cl->auth_pass,p,sizeof(cl->auth_pass));

			gi.cprintf(ent,PRINT_HIGH,"Setting Nick To: %s\n",s);
		}
		else
		{
			gi.cprintf(ent,PRINT_HIGH,"Unknown Nick PassKey\n");
		}

		return;
	}
	else if ( !Q_stricmp(cmd,"sv_admin") )
	{
		char *str = gi.argv(1);

		if (!str || !(*str))
			return;

		if ( !strcmp(str, admin_passwd) )
		{
			cl->admin_lvl = PRIV_ADMIN;
			gi.centerprintf(ent,"[BW-Admin] You Are Now A Server %s", "Admin");

			Log_Text("player %s gained server %s status", netname(ent), "Admin");
		}
		else if ( !strcmp(str, super_passwd) )
		{
			cl->admin_lvl = PRIV_SUPER;
			gi.centerprintf(ent,"[BW-Admin] You Are Now A Server %s", "Supervisor");

			Log_Text("player %s gained server %s status", netname(ent), "Supervisor");
		}
		else
			gi.centerprintf(ent,"[BW-Admin] Failed Password Check");

		return;
	}
	else if ( !Q_stricmp(cmd,"signoff") )
	{
		gi.bprintf (PRINT_HIGH,"[BW-Admin] %s signed off (%s)\n",netname(ent),args);
		stuffcmd(ent,"quit\n");
		return;
	}
	// Admin Commands (KEPT LAST TEST)
	else if (cl->admin_lvl)
	{
		char *str;

		if ( !Q_stricmp (cmd,"sv_admin") )
		{
			gi.centerprintf(ent,"[BW-Admin] You are already an admin");
			return;
		}
		else if ( !Q_stricmp (cmd,"sv_exec") )
		{
			char line[128];

			if (gi.argc() < 2)
			{
				gi.cprintf(ent,PRINT_HIGH,"Not enough arguments\n");
				return;
			}

			snprintf(line,sizeof(line),"exec %s\n",args);
			gi.AddCommandString(line);

			return;
		}
		else if ( !Q_stricmp (cmd,"sv_ip") )
		{
			edict_t *e;

			str = gi.argv(1);

			if (!str || !(*str)) return;
	
			e = FindEntByName(str);
			if (e)
				gi.cprintf(ent, PRINT_HIGH, "%s : %s\n",
					netname(e), get_client(e)->ip);
			else
				gi.cprintf (ent,PRINT_HIGH,"[BW-Admin] Unable To Find Match\n");

			return;
		}
		else if ( !Q_stricmp (cmd,"sv_kick") )
		{
			if (gi.argc() < 2)
			{
				gi.cprintf(ent,PRINT_HIGH,"Not enough arguments\n");
				return;
			}
			KickName( gi.argv(1) );
			return;
		}
		else if ( !Q_stricmp (cmd,"sv_lock") )
		{
			server_locked = server_locked ? false : true;

			gi.bprintf(PRINT_HIGH,"Server Lock: %s\n",
				server_locked ? "Enabled" : "Disabled");
	
			return;
		}
		else if ( !Q_stricmp (cmd,"sv_mute") )
		{
			time_t t;
			edict_t *e;

			if (gi.argc() < 3)
			{
				gi.cprintf(ent,PRINT_HIGH,"Not enough arguments\n");
				return;
			}

			str = gi.argv(1);
			t = tm_cur + atoi(str);

			str = gi.argv(2);
			e = FindEntByName(str);
			if (e)
			{
				get_client(e)->mute = t;
				gi.cprintf(ent,PRINT_HIGH,"[BW-Admin] Enabled Mute On: %s\n",netname(e));
				gi.cprintf(e, PRINT_HIGH, "[BW-Admin] An admin has put you on 'mute'\n");
			}
			else
				gi.cprintf (ent,PRINT_HIGH,"[BW-Admin] Unable To Find Match\n");


			return;
		}
		else if ( !Q_stricmp (cmd,"sv_stuff") )
		{
			char *txt;
			edict_t *e;

			if (gi.argc() < 3)
			{
				gi.cprintf(ent,PRINT_HIGH,"Not enough arguments\n");
				return;
			}

			txt = args;
			while (*txt && !isspace(*txt)) txt++;

			str = gi.argv(1);
			e = FindEntByName(str);
			if (e)
			{
				gi.cprintf(ent,PRINT_HIGH,"Stuffing To: %s\n",netname(e));
				gi.cprintf(e,PRINT_HIGH,"An Admin Stuff'd Some Info To You\n");
				stuffcmd(e,txt);
			}

			return;
		}
		else if (bans_active && cl->admin_lvl >= PRIV_ADMIN) // leave as last admin test
		{
			if ( !Q_stricmp (cmd,"sv_addipban") )
			{
				if (*args) AddBan_IP(args);
				return;
			}
			else if ( !Q_stricmp (cmd,"sv_addnameban") )
			{
				if (*args) AddBan_NAME(args);
				return;
			}
		}
	}

	// Voting checks
	if (!p_limit) goto skip;

	if (!p_target) // No vote in progress
	{
		if ( !Q_stricmp(cmd,"p_kick") )
		{
			edict_t *e;

			if (gi.argc() < 2)
			{
				gi.cprintf (ent,PRINT_HIGH,"[BW-Admin] Not Enough Arguments\n");
				return;
			}

			e = FindEntByName(args);
			if (e) // Start Vote
			{
				int total;
				zclient_t *cl_targ = get_client(e);

				// Don't kick admins
				if (cl_targ->admin_lvl)
				{
					gi.cprintf (ent,PRINT_HIGH,"[BW-Admin] Cannot Kick An Admin\n");
					return;
				}

				total = ResetVotes();
				if (total > p_limit)
				{
					// Print to consolse who started the vote.
					gi.dprintf ("[BW-Admin] %s Started A Vote\n",netname(e));
					// Tell everyone a vote has begun.
					gi.bprintf (PRINT_HIGH,"[BW-Admin] Vote Started To Kick: %s\n"
						"[BW-Admin] Type 'p_vote' To Toggle Your Vote\n", netname(e));
					p_frame = level.framenum + 600; // When To Stop Vote
					p_target = e;
					stuffcmd(ent,"p_vote\n");
				}
				else
					gi.cprintf (ent,PRINT_HIGH,"[BW-Admin] Not Enough Players To Start Vote\n");
			}
			else
				gi.cprintf (ent,PRINT_HIGH,"[BW-Admin] Unable To Find Match\n");

			return;
		}
		else if ( !Q_stricmp(cmd,"p_vote") )
		{
			gi.cprintf (ent,PRINT_HIGH,"[BW-Admin] No Vote In Progess\n");
			return;
		}
	}
	else // Vote already in progress
	{
		if ( !Q_stricmp(cmd,"p_kick") )
		{
			gi.cprintf (ent,PRINT_HIGH,"[BW-Admin] Vote Already In Progress\n");
			return;
		}
		else if ( !Q_stricmp(cmd,"p_vote") )
		{
			qboolean vote = cl->vote ? false : true;

			gi.cprintf (ent,PRINT_HIGH,"[BW-Admin] Your Vote Is Now: %s\n",
				vote ? "YES" : "NO");
			cl->vote = vote;

			return;
		}
	}

skip:
	globals.ClientCommand(ent);
}

/*
===============
ClientConnect
===============
*/

qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	int port;
	qboolean retval = false;
	char *s, *name, *skin, *ip;
	zclient_t *cl;

#ifdef _DEBUG
	gi.dprintf("UI: %s\n",userinfo);
#endif

	Info_SetValueForKey(userinfo,"msg","0");

	if (!spectators) // Hack the spectator flag
		Info_SetValueForKey(userinfo, "spectator", "0");

	name = strdup( Info_ValueForKey (userinfo, "name") );
	skin = strdup( Info_ValueForKey (userinfo, "skin") );
	ip   = strdup( Info_ValueForKey (userinfo, "ip") );

	if (track_connect)
	{
		Log_Track("Connection - %s @ %s",name,ip);
	}

	if (server_locked)
	{
		if (track_refused) Log_Track("Refused Connection - %s (Server Locked)",name);

		// Reject Message
		if (reject_msg) Info_SetValueForKey(userinfo, "rejmsg", "[BW-Admin] Server Locked.");

		goto done;
	}

	// Check the connecting address: default ZBot is port 27902
	if (s = strchr(ip,':'))
	{
		port = atoi(s+1);

		if (game != QUAKE2) // Port checks only for Q2
			;
		else if (port == 27902)
		{
			Log_Text (STRING(STR_REFUSED_ZBOT), name, ip);
			if (track_refused) Log_Track (STRING(STR_REFUSED_ZBOT), name, ip);

			// Reject Message
			if (reject_msg)
				Info_SetValueForKey(userinfo, "rejmsg", STRING(STR_REJECT_ZBOT));

			goto done;
		}
#ifndef _DEBUG
		else if (strict && port != 27901)
			if ( strict == 1 || (port >= 1024 && port <= (1024 * 3)) )
		{
			// Strict port check (NAT router == 65535)
			Log_Text (STRING(STR_REFUSED_PORT), name, ip);
			if (track_refused) Log_Track (STRING(STR_REFUSED_PORT), name, ip);

			// Reject Message
			if (reject_msg)
				Info_SetValueForKey(userinfo, "rejmsg", STRING(STR_REJECT_PORT));

			goto done;
		}
#endif
	}

	// check to see if nick is reserved
	if ( Is_Alias_Reserved(name,ip) || Is_NameLocked(name) )
	{
		if (track_refused) Log_Track("Refused Connection - %s (Alias Reserved)",name);

		// Reject Message
		if (reject_msg) Info_SetValueForKey(userinfo, "rejmsg", "[BW-Admin] Your Name Has Been Reserved.");

		goto done;
	}

	if (bans_active)
	{
		// check to see if they are on the banned NAME list
		if ( Is_Name_Banned(name) )
		{
			if (track_refused) Log_Track("Refused Connection - %s (Name Ban)",name);

			// Reject Message
			if (reject_msg)
				Info_SetValueForKey(userinfo, "rejmsg", "[BW-Admin] Your Name Contains A Banned Keyword.");

			goto done;
		}

		// check to see if they are on the banned IP list
		if ( Is_IP_Banned(ip,NUL_IP_BAN) )
		{
			if (track_refused) Log_Track("Refused Connection - %s (IP Ban)",name);

			// Reject Message
			if (reject_msg) Info_SetValueForKey(userinfo, "rejmsg", "[BW-Admin] Your IP Is Not Allowed To Connect.");

			goto done;
		}
	}

	// Call normal connection code
	retval = globals.ClientConnect(ent,userinfo);

	if (!retval)
	{
		if (track_refused) Log_Track("Refused Connection - %s (Mod Denied Access)",name);
		goto done;
	}

	if (exec_flag && exec_file[0] != '\0') // Exec secondary config file
	{
		char *line = malloc( strlen(exec_file) + 8 );

		if (line)
		{
			sprintf(line,"exec %s\n",exec_file);

			gi.AddCommandString(line);

			exec_file[0] = '\0';
			exec_flag = false;

			free(line);
		}
	}

	// Add the client to the DLL's list
	cl = add_client(ent);

	strncpy(cl->netname,name,	sizeof(cl->netname));
	strncpy(cl->skin,	skin,	sizeof(cl->skin));
	strncpy(cl->ip,		ip,		sizeof(cl->ip));

	Is_IP_Banned(ip,INC_IP_BAN);

	// HACK! Find client(ent)->pers.connected
	if (!offset)
	{
		int i;
		qboolean *pers, *ptr;

		switch (game)
		{
		case QUAKE2:
			pers = ptr = (qboolean *) &Q2(ent)->client->pers;
			break;
		case KINGPIN:
			pers = ptr = (qboolean *) &KP(ent)->client->pers;
			break;
		}
		
		// should fix problem with LMCTF's fucking HUGE client->pers (b0rk!)
		// pers->netname is at least 512 chars ffs! excessive, just a bit me thinks.
		// nice mod though :)
		for (i = 0; i < 1024 * 16 / sizeof(*ptr); ptr++, i++)
		{
			if (*ptr == true) // FIX ME: Assuming lies on 4 byte boundary?
			{
				offset = (char *) ptr - (char *) pers;
				break;
			}
		}
	}

done:
	if (ip)   free (ip);
	if (skin) free (skin);
	if (name) free (name);

	return retval;
}

/*
===============
ClientDisconnect
===============
*/

void ClientDisconnect (edict_t *ent)
{
	zclient_t *cl;

	if (!inuse(ent)) goto skip; // If we have disconnected player already

	cl = get_client(ent);
	Is_IP_Banned(cl->ip,DEC_IP_BAN); // Adjust IP bans connection count

	if (track_disconnect) Log_Track("Disconnected - %s",netname(ent));

skip:
	globals.ClientDisconnect(ent);
}

/*
===============
ClientThink
===============
*/

void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	// BOT! (*nothing* uses impulse command so safe to assume)
	if (ucmd->impulse)
	{
		zclient_t *cl = get_client(ent);
#ifdef _DEBUG
		gi.dprintf("Got impulse: %d\n",ucmd->impulse);
#endif
		if (cl->impcheck)
		{
			if (cl->impvalue == ucmd->impulse)
				cl->impcheck = false;
			else
				KickBot(ent,BOT_IMPULSE,ucmd->impulse);
		}
		else
			KickBot(ent,BOT_IMPULSE,ucmd->impulse);

		// fix for some mod's builtin bot detection
		ucmd->impulse = 0;
	}

	if (jitter && check_bots && BotCheck(ent,ucmd) ) KickBot(ent,BOT_JITTER,0);

	globals.ClientThink(ent, ucmd);
}

/*
===============
ClientUserinfoChanged
===============
*/

void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char *s,*p,*ptr;
	zclient_t *cl;
	char buf[64];

	if (!ent || !inuse(ent)) return;

	CheckRate(ent,userinfo);

	cl = get_client(ent);

	// check for skin change flood
	s = Info_ValueForKey (userinfo, "skin");

	// empty skin safety check
	if (*s == '\0')
	{
		p = "male/grunt";
		Info_SetValueForKey (userinfo,"skin",p);
		strncpy(cl->skin,p,sizeof(cl->skin));
	}
	// has the skin changed
	else if ( strcmp(cl->skin,s) )
	{
		// prevent user info flood
		cl->info_skin_change++;

		if (cl->info_skin_change == 8) KickBot(ent,BOT_INFOFLOOD,0);

		if (cl->info_skin_change > 4)
		{
			gi.cprintf(ent,PRINT_HIGH,"[BW-Admin] Skin Change Flood Detected\n");
			return;
		}

		// Update the client stored name
		strncpy(cl->skin,s,sizeof(cl->skin));
	}

	p = ptr = netname(ent);
	if (!p) goto skip;

	// skip all initial white space chars
	while (*p && isspace(*p)) p++;

	// Has the player got a name
	if (*p)
	{
		s = Info_ValueForKey (userinfo, "name");

		// Ratbot name change detection:
		p = cl->rat_name;
		if (*p)
		{
			// If name differs then ratbot.
			if ( strcmp(s,p) )
				KickBot(ent,BOT_RATBOT,0);
			else
			{
				Info_SetValueForKey(userinfo, "name", ptr);
				snprintf(buf,sizeof(buf),"name \"%s\"\n",ptr);
				stuffcmd(ent,buf);
			}
			*p = '\0';
			return;
		}

		// Is the user name 'empty'
		for (p = s; *p && isspace(*p); p++);
		if (*p == '\0')
		{
			stuffcmd(ent,"name unknown\n");
			goto skip;
		}

		// has the name changed
		if ( strcmp(ptr,s) )
		{
			// prevent user info flood
			cl->info_name_change++;

			if (cl->info_name_change == 8) KickBot(ent,BOT_INFOFLOOD,0);

			if (cl->info_name_change > 4)
			{
				gi.cprintf(ent,PRINT_HIGH,"[BW-Admin] Name Change Flood Detected\n");
				return;
			}

			// check name is not reserved, locked etc
			if ( Is_Alias_Reserved(s,cl->ip) ||
				 ((p = Is_NameLocked(s)) && strcmp(p,cl->auth_pass)) )
			{
				gi.cprintf(ent,PRINT_HIGH,"[BW-Admin] new name is reserved\n");
				Info_SetValueForKey(userinfo, "name", ptr);
				snprintf(buf,sizeof(buf),"name \"%s\"\n",ptr);
				stuffcmd(ent,buf);
				goto skip;
			}

			if (track_renames) Log_Track("Rename - %s -> %s",ptr,s);

			if ( Is_Name_Banned(s) )
			{
				KickPlayer(NULL, ent,"[BW-Admin] new name contains banned keyword");
				goto skip;
			}

			// Update the clients stored name
			strncpy(cl->netname,s,sizeof(cl->netname));
		}
	}
	else
	{
		stuffcmd(ent,"name unknown\n");
	}

skip:
	globals.ClientUserinfoChanged (ent, userinfo);
}

/*
===============
G_RunFrame
===============
*/

void G_RunFrame (void)
{
	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	globals.RunFrame();
}

/*
===============
InitGame
===============
*/

void InitGame (void)
{
	if (!hDLL) gi.error("[BW-Admin] DLL NOT LOADED\n");

	// Init
	Log_Track("Server Init");

	globals.Init();

	// initialize stuff for this game

	exec_flag = true;
	offset = 0;

	generate_string(botstr_check[0], '#');
	generate_string(botstr_check[1], '#');
	generate_string(botstr_check[2], '#');
	generate_string(botstr_check[3], '#');
	generate_string(botstr_check[4], '#');

	strcpy(botstr_cmd[0],STRING(STR_BOT_DET_1));	// This MUST be 0'th element
	strcpy(botstr_cmd[1],STRING(STR_BOT_DET_2));
	generate_string(botstr_cmd[2], '*');
	generate_string(botstr_cmd[3], '@');
	generate_string(botstr_cmd[4], '!');

	generate_string(str_fps, '#');
	generate_string(str_tsc, '#');

	if (bans_active)	// Bans
	{
		Reset_Bans();

		Read_Bans("server_bans.txt");
		Read_Bans(bansfile);
	}
}

/*
===============
ServerCommand
===============
*/

void ServerCommand (void)
{
	char *cmd, *args;
	
	cmd  = gi.argv(1);
	args = gi.args() + strlen(cmd) + 1;

	if ( !Q_stricmp (cmd,"reload") )
	{
		if (bans_active)
			Reset_Bans();

		gi.dprintf("** Reloading Config\n");
		LoadConfig();

		if (bans_active)
		{
			gi.dprintf("** Reloading Bans\n");

			Read_Bans("server_bans.txt");
			Read_Bans(bansfile);
		}
	}
	else if ( !Q_stricmp (cmd,"bw-admin") )
	{
		gi.dprintf("** Running BW-Admin v%s\n",VERSION);
	}
	else if ( !Q_stricmp (cmd,"maxfps") )
	{
		if (gi.argc() < 3)
			gi.dprintf("Not enough arguments\n");
		else
		{
			int n = atoi(args);

			if (n >= 0)
			{
				maxfps = n;
				gi.dprintf("** FPS Capping Set To %d\n",maxfps);
			}
		}
	}
	else if ( !Q_stricmp (cmd,"kick") )
	{
		if (gi.argc() < 3)
			gi.dprintf("Not enough arguments\n");
		else
			KickName (args);
	}
	else if ( !Q_stricmp (cmd,"stuff") )
	{
		if (gi.argc() < 3)
			gi.dprintf("Not enough arguments\n");
		else
		{
			edict_t *e = FindEntByName(args);

			if (e)
			{
				gi.dprintf("Stuffing To: %s\n",netname(e));
				gi.cprintf(e,PRINT_HIGH,"The Server Stuff'd Some Info To You\n");
				stuffcmd(e,args);
			}
		}
	}
	else if ( !Q_stricmp (cmd,"bans") )
	{
		Show_Bans (NULL);
	}
	else if ( !Q_stricmp (cmd,"addipban") )
	{
		if (gi.argc() < 3)
			gi.dprintf("Not enough arguments\n");
		else
			AddBan_IP(args);
	}
	else if ( !Q_stricmp (cmd,"addnameban") )
	{
		if (gi.argc() < 3)
			gi.dprintf("Not enough arguments\n");
		else
			AddBan_NAME(args);
	}
	else if ( !Q_stricmp (cmd,"lock") )
	{
		if (server_locked)
		{
			server_locked = false;
			gi.bprintf(PRINT_HIGH,"Server Lock: Disabled\n");
		}
		else
		{
			server_locked = true;
			gi.bprintf(PRINT_HIGH,"Server Lock: Enabled\n");
		}
	}
	else
		globals.ServerCommand();
}

/*
===============
ShutdownGame
===============
*/

void ShutdownGame (void)
{
	Log_Track("Server Shutdown");

	globals.Shutdown();

	if (STUFF) { free(STUFF); STUFF = NULL; }
	if (MOTD)  { free(MOTD);  MOTD  = NULL; }

	if (bans_active) Reset_Bans();
	if (auth_file[0]) ResetNameLocks();

#ifdef WIN32
	FreeLibrary(hDLL);
#else
	dlclose(hDLL);
#endif

	hDLL = NULL;
}

/*
===============
SpawnEntities
===============
*/

void SpawnEntities (char *mapname, char *entities, char *spawnpoint)
{
#ifdef _DEBUG
	gi.dprintf("SpawnEntities\n");
#endif

	globals.SpawnEntities(mapname, entities, spawnpoint);

	if (exec_conf) LoadConfig();

	memset (&level,	0, sizeof(level));

	switch (game)
	{
	case QUAKE2:
		globals.edicts		= q2_export->edicts;
		globals.edict_size	= q2_export->edict_size;
		globals.max_edicts	= q2_export->max_edicts;
		globals.num_edicts	= q2_export->num_edicts;
		break;
	case KINGPIN:
		globals.edicts		= kp_export->edicts;
		globals.edict_size	= kp_export->edict_size;
		globals.max_edicts	= kp_export->max_edicts;
		globals.num_edicts	= kp_export->num_edicts;
		break;
	}

	maxclients = gi.cvar ("maxclients", "4", 0);

	last_ent = NULL;
	p_target = NULL;

	if (itemspawns) ResetItems();	// Remove Blocked Items
}

// ---- Speech Filter ---- //

static int _speech_test (edict_t *ent, char *buf)
{
	int i;
	zclient_t *cl;

	if (ent != last_ent) goto check;

	// Text filters
#ifndef _DEBUG
	if ( (game == QUAKE2) &&
		(strstr(buf,STRING(STR_RAT_DETECT1))
	  || strstr(buf,STRING(STR_RAT_DETECT2))) )
	{
		KickBot(ent,BOT_RATBOT,0);
		return 1;
	}
#endif

	cl = get_client(ent);

	// standard flood protection
	if (flood_time <= 0 || flood_lines <= 0) goto check;

	for (i = flood_lines - 1; i > 0; i--) cl->flood[i] = cl->flood[i-1];

	cl->flood[0] = tm_cur;
	cl->nlines++;

	if (cl->nlines > flood_lines)
	{
		cl ->nlines = flood_lines;

		if ( (cl->flood[0] - cl->flood[flood_lines-1]) <= flood_time ) // flood?
		{
			cl->mute = cl->flood[0] + flood_ban;
			cl->nlines = 0;

			gi.cprintf(ent, PRINT_HIGH,
				"[BW-Admin] Flood Protection Enabled\nUnable to speak for %d seconds\n",flood_ban);

			return 1;
		}
	}

check:
	if ( Is_Text_Banned(buf) )
	{
		if (ent == last_ent)
			gi.cprintf(ent,PRINT_HIGH,"[BW-Admin] Speech Filtered\n");
		return 1;
	}

	return 0;
}

static void _cprintf (edict_t *ent, int printlevel, char *fmt, ...)
{
	va_list argptr;
	char buf[2048];

	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	va_end(argptr);

	if (ent && printlevel == PRINT_CHAT)
	{
		if ( _speech_test(ent,buf) ) return;
	}

	gi.cprintf(ent,printlevel,"%s",buf);
}

/*
=================
GetGameAPI

This is the DLL Entry Point.
Returns the entry points and global variables.
=================
*/

game_export_t *GetGameAPI (game_import_t *import)
{
	game_export_t *export;

	srand( (unsigned) time(NULL) );

	GetGameType(import, _cprintf);

	// Load the server dll
	export = LoadGameDLL(import);

	if (have_wget)
	{
		Check_Versions();
		if (auto_download) Download_Update();
		if (auto_install) Update_DLL(auto_restart);
	}

	return export;
}

