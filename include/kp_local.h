// g_local.h -- local definitions for game module

#include "kp_shared.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and kpent_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "kp_game.h"

//==================================================================

#define MELEE_DISTANCE	80

typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
} damage_t;

typedef enum 
{
	WEAPON_READY, 
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_FIRING,
// RAFAEL 01-11-99
	WEAPON_RELOADING,
	WEAPON_RELOAD_CYCLE,
	WEAPON_RELOADING_SPISTOL
} weaponstate_t;

typedef enum
{
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS,
	AMMO_MAGSLUG,
	AMMO_TRAP
} ammo_t;

// edict->movetype values
typedef enum
{
MOVETYPE_NONE,			// never moves
MOVETYPE_NOCLIP,		// origin and angles change with no interaction
MOVETYPE_PUSH,			// no clip to world, push on box contact
MOVETYPE_STOP,			// no clip to world, stops on box contact

MOVETYPE_WALK,			// gravity
MOVETYPE_STEP,			// gravity, special edge handling
MOVETYPE_FLY,
MOVETYPE_TOSS,			// gravity
MOVETYPE_FLYMISSILE,	// extra size to monsters
MOVETYPE_BOUNCE,		// added this (the comma at the end of line)
MOVETYPE_WALLBOUNCE,
MOVETYPE_TOSS_SLIDE		// Ridah, testing
} movetype_t;

typedef struct
{
	int		base_count;
	int		max_count;
	float	normal_protection;
	float	energy_protection;
	int		armor;
} gitem_armor_t;


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in kpent_t during gameplay
typedef struct
{
	// world vars
	char		*sky;
	char		*nextmap;

	int			lip;
	int			distance;
	int			height;
	char		*noise;
	float		pausetime;
	char		*item;
	char		*gravity;

	float		minyaw;
	float		maxyaw;
	float		minpitch;
	float		maxpitch;

	float		fogdensity;
	vec3_t		fogval;
	float		fogdensity2;
	vec3_t		fogval2;
} spawn_temp_t;

//============================================================================

#define MAX_WEAPONS	10


// client data that stays across multiple level loads
typedef struct
{
	char		userinfo[MAX_INFO_STRING];
	char		netname[16];
	int			hand;
	int			version;		// collected from Userinfo, used to determine

	qboolean	connected;		// a loadgame will leave valid entities that
								// just don't have a connection yet
} kp_client_persistant_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct kp_gclient_s
{
	// known to server
	kp_player_state_t	ps;		// communicated by server to clients
	int	ping;

	// private to game
	kp_client_persistant_t	pers;
};

struct kp_edict_s
{
	kp_entity_state_t	s;
	struct kp_gclient_s	*client;	// NULL if not a player
									// the server expects the first part
									// of gclient_s to be a player_state_t
									// but the rest of it is opaque
	qboolean	inuse;
	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t		area;				// linked to a division node or leaf
	
	int			num_clusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;			// unused if num_clusters != -1
	int			areanum, areanum2;

	//================================

	int			svflags;
	vec3_t		mins, maxs;
	vec3_t		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	kpent_t		*owner;

	float		voice_pitch;		// used to pitch voices up/down, 1.0 = same, 2.0 = chipmunk (double speed)

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	//================================
	int			movetype;
	int			flags;

	char		*model;
	float		freetime;			// sv.time when the object was freed
	
	//
	// only used locally in game, not by server
	//
	char		*message;
	char		*classname;
	int			spawnflags;
};
