/*
	Author	: Vincent 'Zarjazz' Sweeney
	Email	: zarjazz@barrysworld.com
	WebPage	: http://www.barrysworld.com/

	Copyright © 1998-2000 BarrysWorld Ltd - All rights reserved.
	----
	$Id$
*/


//
// Game Type Wrapper Stuff
//

typedef void edict_t;
typedef void client_t;

typedef enum
{
	QUAKE2,
	KINGPIN,
} game_type_t;

extern game_type_t game;


char	 *	classname	(edict_t *ent);
client_t *	client		(edict_t *ent);
qboolean	inuse		(edict_t *ent);
char	 *	netname		(edict_t *ent);
int			ping		(edict_t *ent);
int			svflags		(edict_t *ent);
char	 *	userinfo	(edict_t *ent);

#define ENT_NUM(ENT) ((((char *)ENT - (char *)globals.edicts) + globals.edict_size / 2) / globals.edict_size)


//
// Ban Stuff
//

void Reset_Bans	(void);
void Read_Bans	(const char *fname);
int  Write_Bans	(const char *fname);

qboolean Reset_IP_Ban_Count (void);
void Show_Bans (void *ent);

void KickEnt (void *ent);

void KickPlayer		(void *kicker, void *player, char *reason);
void KickBanPlayer	(void *kicker, void *player, char *reason);

#define NUL_IP_BAN 0
#define INC_IP_BAN 1
#define DEC_IP_BAN 2

qboolean Is_Alias_Reserved	(const char *name, const char *ip);
qboolean Is_IP_Banned		(const char *ip, int ip_flag);
qboolean Is_Name_Banned		(const char *name);
qboolean Is_Text_Banned		(const char *str);

//
// functions provided by the main engine
//
typedef struct
{
	// special messages
	void	(*bprintf)		(int printlevel, char *fmt, ...);
	void	(*dprintf)		(char *fmt, ...);
	void	(*cprintf)		(edict_t *ent, int printlevel, char *fmt, ...);
	void	(*centerprintf)	(edict_t *ent, char *fmt, ...);

	void	(*error)		(char *fmt, ...);

	// call before removing an interactive edict
	void	(*unlinkentity) (edict_t *ent);

	// network messaging
	void	(*unicast)		(edict_t *ent, qboolean reliable);
	void	(*WriteByte)	(int c);
	void	(*WriteString)	(char *s);

	// console variable interaction
	cvar_t	*(*cvar)		(char *var_name, char *value, int flags);

	// ClientCommand and ServerCommand parameter access
	int		(*argc) (void);
	char *	(*argv) (int n);
	char *	(*args) (void);	// concatenation of all argv >= 1

	// add commands to the server console as if they were typed in
	// for map changing, etc
	void	(*AddCommandString) (char *text);
} game_import_t;

//
// functions exported by the game subsystem
//
typedef struct
{
	// the init function will only be called when a game starts,
	// not each time a level is loaded.  Persistant data for clients
	// and the server can be allocated in init
	void		(*Init)		(void);
	void		(*Shutdown) (void);

	// each new level entered will cause a call to SpawnEntities
	void		(*SpawnEntities) (char *mapname, char *entstring, char *spawnpoint);

	qboolean	(*ClientConnect)		(edict_t *ent, char *userinfo);
	void		(*ClientBegin)			(edict_t *ent);
	void		(*ClientUserinfoChanged)(edict_t *ent, char *userinfo);
	void		(*ClientDisconnect)		(edict_t *ent);
	void		(*ClientCommand)		(edict_t *ent);
	void		(*ClientThink)			(edict_t *ent, usercmd_t *cmd);

	void		(*RunFrame) (void);

	// Called when an "sv <command>" command is issued on the server console.
	void		(*ServerCommand)		(void);

	// The edict array is allocated in the game dll so it
	// can vary in size from one game to another.
	// 
	// The size will be fixed when ge->Init() is called
	edict_t		*edicts;
	int			edict_size;
	int			num_edicts;		// current number, <= max_edicts
	int			max_edicts;
} game_export_t;

extern game_import_t gi;
extern game_export_t globals;

