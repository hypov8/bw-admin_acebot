// g_local.h -- local definitions for game module

#include "q2_shared.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and q2ent_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "q2_game.h"

//==================================================================

// item spawnflags
#define ITEM_TRIGGER_SPAWN		0x00000001
#define ITEM_NO_TOUCH			0x00000002
// 6 bits reserved for editor flags
// 8 bits used as power cube id bits for coop games
#define DROPPED_ITEM			0x00010000
#define	DROPPED_PLAYER_ITEM		0x00020000
#define ITEM_TARGETS_USED		0x00040000

//============================================================================

// client data that stays across multiple level loads
typedef struct
{
	char		userinfo[MAX_INFO_STRING];
	char		netname[16];
	int			hand;

	qboolean	connected;			// a loadgame will leave valid entities that
									// just don't have a connection yet
} q2_client_persistant_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct q2_gclient_s
{
	// known to server
	q2_player_state_t	ps;		// communicated by server to clients
	int					ping;

	// private to game
	q2_client_persistant_t	pers;
};

struct q2_edict_s
{
	q2_entity_state_t	s;
	struct q2_gclient_s	*client;	// NULL if not a player

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
	q2ent_t		*owner;


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

