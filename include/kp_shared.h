
#include "shared.h"

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s
{
	vec3_t	normal;
	float	dist;
	byte	type;			// for fast side tests
	byte	signbits;		// signx + (signy<<1) + (signz<<1)
	byte	pad[2];
	// Ridah
	int		spec_updateframe;	// so we only do each plane once
	int		spec_updateframe_backface;
	byte	spec_render, spec_render_back;
} cplane_t;

// a trace is returned when a box is swept through the world
typedef struct
{
	qboolean	allsolid;	// if true, plane is not valid
	qboolean	startsolid;	// if true, the initial point was in a solid area
	float		fraction;	// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;		// final position
	cplane_t	plane;		// surface normal at impact
	csurface_t	*surface;	// surface hit
	int			contents;	// contents on other side of surface hit
	struct edict_s	*ent;		// not set by CM_*() functions
} kp_trace_t;


// this structure needs to be communicated bit-accurate
// from the server to the client to guarantee that
// prediction stays in sync, so no floats are used.
// if any part of the game code modifies this struct, it
// will result in a prediction error of some degree.
typedef struct
{
	pmtype_t	pm_type;

	short		origin[3];		// 12.3
	short		velocity[3];	// 12.3
	byte		pm_flags;		// ducked, jump_held, etc
	byte		pm_time;		// each unit = 8 ms
	short		gravity;
	short		delta_angles[3];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters

	byte		runscale;		// Ridah, so we can mess with the running speed
} kp_pmove_state_t;

typedef struct
{
	// state (in / out)
	kp_pmove_state_t	s;

	// command (in)
	usercmd_t		cmd;
	qboolean		snapinitial;	// if s has been changed outside pmove

	// results (out)
	int			numtouch;
	struct kp_edict_s	*touchents[MAXTOUCH];

	vec3_t		viewangles;			// clamped
	float		viewheight;

	vec3_t		mins, maxs;			// bounding box size

	struct pk_edict_s	*groundentity;
	int			watertype;
	int			waterlevel;

	// callbacks to test the world
	kp_trace_t		(*trace) (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);
	int			(*pointcontents) (vec3_t point);

	// JOSEPH 1-SEP-98
	int			footsteptype;

	// Ridah, bike collisions
	int			wall_collision;
} kp_pmove_t;

/*
==========================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

==========================================================
*/

typedef enum
{
	TT_NONE
} talk_tone;

// Ridah, MDX
typedef struct
{
	vec3_t	mins, maxs;
} object_bounds_t;

#define	MAX_MODEL_PARTS			8	// must change network code to increase this (also savegame code)
#define	MAX_MODELPART_OBJECTS	8	// absolutely do not change, bound by "invisible_objects" bit-flags

typedef struct model_part_s
{
// Ridah, MDX, ENABLE the following line when .mdx system goes online
	int		modelindex;					// leave as 0 if blank

	byte	invisible_objects;			// bit flags that define which sub-parts NOT to display when SET
	byte	skinnum[MAX_MODELPART_OBJECTS];

	// server-only data used for collision detection, etc
	int		object_bounds[MAX_MODELPART_OBJECTS];	// read in and allocated immediately after setting the modelindex
	char	*objectbounds_filename;					// so we can restore the object bounds data when loading a savegame

	int     baseskin;
	byte	hitpoints[MAX_MODELPART_OBJECTS];
	byte	hit_scale[MAX_MODELPART_OBJECTS]; // 0-250
} model_part_t;
// done.

#define	MAX_MODEL_DIR_LIGHTS	3		// bound to 8 by network code

typedef struct
{
	int		light_indexes[MAX_MODEL_DIR_LIGHTS];	// so we can tell the client to use a certain light index, and it knows the details for that light

	vec3_t	light_vecs[MAX_MODEL_DIR_LIGHTS];		// static light directions that touch this model (dynamic are added in ref_gl)
	vec3_t	light_colors[MAX_MODEL_DIR_LIGHTS];
	float	light_intensities[MAX_MODEL_DIR_LIGHTS];	// 0.0 -> 1.0
	byte	light_styles[MAX_MODEL_DIR_LIGHTS];
	int		num_dir_lights;

	// below this doesn't get sent to client (only used at server side)
	vec3_t	light_orgs[MAX_MODEL_DIR_LIGHTS];
} model_lighting_t;

// Ridah, flamethrower
typedef struct flamejunc_s
{
	vec3_t	org, vel;
	float	start_width, end_width, start_height, end_height;	
	float	lifetime, fadein_time;
	float	start_alpha, end_alpha;

	// current values
	float	aged, alpha;
	float	width, height;

	vec3_t	unitvel;
	int		hit_wall;

	struct flamejunc_s	*next;
} flamejunc_t;

// entity_state_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way

typedef struct kp_entity_state_s
{
	int		number;			// edict index

	vec3_t	origin;
	vec3_t	angles;
	vec3_t	old_origin;		// for lerping
	int		modelindex;

	int		frame;
	int		skinnum;
	int		effects;
	int		renderfx;
	int		solid;			// for client side prediction, 8*(bits 0-4) is x/y radius
							// 8*(bits 5-9) is z down distance, 8(bits10-15) is z up
							// gi.linkentity sets this properly
	int		sound;			// for looping sounds, to guarantee shutoff
	int		event;			// impulse events -- muzzle flashes, footsteps, etc
							// events only go out for a single frame, they
							// are automatically cleared each frame
	int		renderfx2;

	// Ridah, MDX, making way for .mdx system..
	int		num_parts;
	model_part_t	model_parts[MAX_MODEL_PARTS];
	// done.

	// Ridah, new lighting data
	model_lighting_t	model_lighting;

	vec3_t	last_lighting_update_pos;		// so we only calculate when a good distance from the last checked position
	vec3_t	last_lighting_vec_update_pos;	// set when we update the vecs

	// Ridah, flamethrower (only used on client-side)
	flamejunc_t	*flamejunc_head;
	flamejunc_t	*flamejunc_head2;

	int	last_time, prev_last_time;	// time of last call to CL_FlameEffects() for this entity
	byte	broken_flag;			// set if we release the trigger, so next time we fire, we can free all current flames

	float	alpha;			// set in CL_AddPacketEntities() from entity_state->effects
	float	scale;			// ranges from 0.0 -> 2.0
} kp_entity_state_t;

//==============================================


// player_state_t is the information needed in addition to pmove_state_t
// to rendered a view.  There will only be 10 player_state_t sent each second,
// but the number of pmove_state_t changes will be reletive to client
// frame rates
typedef struct
{
	kp_pmove_state_t	pmove;		// for prediction

	// these fields do not need to be communicated bit-precise

	vec3_t		viewangles;		// for fixed views
	vec3_t		viewoffset;		// add to pmovestate->origin
	vec3_t		kick_angles;	// add to view direction to get render angles
								// set by weapon kicks, pain effects, etc
	vec3_t		gunangles;
	vec3_t		gunoffset;
	int			gunindex;
	int			gunframe;

	float		blend[4];		// rgba full screen effect
	
	float		fov;			// horizontal field of view

	int			rdflags;		// refdef flags

	short		stats[MAX_STATS];		// fast status bar updates

 	// RAFAEL
 	// weapon mdx
 	int			num_parts;
 	model_part_t	model_parts[MAX_MODEL_PARTS];

	int			weapon_usage;
} kp_player_state_t;
