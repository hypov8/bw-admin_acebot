/*
	Author	: Vincent 'Zarjazz' Sweeney
	Email	: zarjazz@barrysworld.com
	WebPage	: http://www.barrysworld.com/

	Copyright © 1998-2000 BarrysWorld Ltd - All rights reserved.
	----
	$Id$
*/

#include <ctype.h>

#include "q2_local.h"
#include "kp_local.h"

#include "admin.h"
#include "codec.h"
#include "strings.h"


char *get_client_ip (edict_t *ent);

// Typedef's

typedef struct ban_ip_s
{
	unsigned addr;
	short maxconnections;
	short nconnections;
	qboolean allow;
	short maskbits;
} ban_ip_t;

typedef struct ban_name_s
{
	char	*txt;
} ban_name_t;

typedef struct ban_alias_s
{
	ban_ip_t ban_ip;
	ban_name_t ban_name;
} ban_alias_t;

/* =============== KICKS & BANS =============== */

#define MAX_BANS	256

static ban_alias_t	alias_bans	[MAX_BANS];
static ban_name_t	filter_bans	[MAX_BANS];
static ban_ip_t		ip_bans		[MAX_BANS];
static ban_name_t	name_bans	[MAX_BANS];

static int num_alias_bans	= 0;
static int num_filter_bans	= 0;
static int num_ip_bans		= 0;
static int num_name_bans	= 0;

#define SAFE_FREE(p) if (p != NULL) free(p)

// Defs

void ClientDisconnect (edict_t *ent);
void stuffcmd(edict_t *e, char *s);

#ifndef PERS
	#define PERS(e) (e->client->pers)
#endif

const char *z_stristr(const char *s1, const char *s2)
{
	int len;

	if (s1 == s2) return s1;

	if (s1 && s2)
	{
		len = strlen(s2);
		if (!len) return NULL;

		while (*s1)
		{
			int i;
			char c1, c2;
			const char *p1, *p2;

			p1 = s1; p2 = s2;

			for (i = len; i; i--)
			{
				c2 = tolower ((*p2++) & 0x7F);
				c1 = tolower ((*p1++) & 0x7F);
				if (c1 != c2) break;
			}
			// If matched to end of string
			if (!i) return s1;

			s1++;
		}
	}

	return NULL;
}

int z_stricmp (const char *s1, const char *s2)
{
	int c1, c2;

	do
	{
		c1 = (*s1++) & 0x7F;
		c2 = (*s2++) & 0x7F;

		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;		// strings not equal
		}
	} while (c1);

	return 0;		// strings are equal
}

static qboolean atoip (ban_ip_t *addr, const char *str)
{
	int i;
	unsigned n;
	char *s, *ip, txt[64];

	if (!str || !(*str)) return false;

	ip = txt;
	strncpy(ip,str,sizeof(txt));
	ip[sizeof(txt)-1] = '\0';

	while (isspace(*ip)) ip++;

	addr->allow = false;
	switch (*ip)
	{
	case '+':
		addr->allow = true;
	case '-':
		ip++;
		break;
	default:
		break;
	}

	addr->maxconnections = 0;
	addr->nconnections = 0;
	addr->maskbits = 32;

	if (s = strchr(ip,'#'))
	{
		*s = '\0';
		addr->maxconnections = atoi(s+1);
	}

	if (s = strchr(ip,'/'))
	{
		int n = atoi(s+1);

		if (n < 0)
			return false;
		if (n < 32)
			addr->maskbits = n;
		*s = '\0';
	}

	addr->addr = 0;
	for (i=0;i<4;i++)
	{
		addr->addr <<= 8;

		if (i < 3)
		{
			s = strchr(ip,'.');
			if (!s) return false; // Bad IP Format
			*s++ = '\0';
		}
		else if (s = strchr(ip,':'))
			*s = '\0';

		if (*ip == '*') return false; // Old IP Ban Format

		n = atoi(ip) & 0xFF;
		addr->addr |= n;

		ip = s;
	}

	return true;
}

static char *iptoa (char *ip, ban_ip_t *addr)
{
	int i;
	unsigned n;
	char s[4][4], *ptr;

	n = addr->addr;
	for (i=3;i>=0;i--)
	{
		ptr = &(s[i][0]);
		sprintf(ptr,"%hd",n & 0xFF);
		n >>= 8;
	}

	sprintf(ip,"%c %s.%s.%s.%s",
		addr->allow ? '+' : '-',
		&(s[0][0]),&(s[1][0]),&(s[2][0]),&(s[3][0])
		);

	ptr = &(s[0][0]);
	if (addr->maskbits != 32)
	{
		sprintf(ptr,"/%d",(int) addr->maskbits);
		strcat(ip,ptr);
	}

	if (addr->maxconnections > 0)
	{
		sprintf(ptr," #%d",(int) addr->maxconnections);
		strcat(ip,ptr);
	}

	return ip;
}


qboolean addban_ip (char *s)
{
	ban_ip_t ip;

	if ( !atoip(&ip,s) ) return false;

	ip_bans[num_ip_bans++] = ip;

	if (num_ip_bans == MAX_BANS)
		gi.dprintf("== WARNING: MAX %s BANS REACHED ==\n","IP");

	return true;
}

qboolean addban_name (char *s)
{
	int l;

	while ( isspace(*s) ) s++; // skip initial white space chars
	if (*s == '\0') return false;

	l = strlen(s);
	while (l && isspace(s[l-1]) ) s[--l] = '\0'; // strip whitespace chars (if any)

	name_bans[num_name_bans++].txt = strdup(s);
	if (num_name_bans == MAX_BANS)
		gi.dprintf("== WARNING: MAX %s BANS REACHED ==\n","NAME");

	return true;
}

static qboolean addban_filter (char *s)
{
	int l;

	while ( isspace(*s) ) s++; // skip initial white space chars
	if (*s == '\0') return false;

	l = strlen(s);
	while (l && isspace(s[l-1]) ) s[--l] = '\0'; // strip whitespace chars (if any)

	filter_bans[num_filter_bans++].txt = strdup(s);
	if (num_filter_bans == MAX_BANS)
		gi.dprintf("== WARNING: MAX %s BANS REACHED ==\n","FILTER");

	return true;
}

static qboolean addban_alias (char *s)
{
	int l;
	ban_ip_t ip;
	char s1[64],s2[64];

	if (sscanf(s," %s %s ",s1,s2) < 2)
		return false;

	if (!atoip(&ip,s2)) return false;

	s = s1;
	while(isspace(*s)) s++; // skip initial white space chars
	if (*s == '\0') return false;

	l = strlen(s);
	while (l && isspace(s[l-1]) ) s[--l] = '\0'; // strip whitespace chars (if any)

	alias_bans[num_alias_bans].ban_ip = ip;
	alias_bans[num_alias_bans++].ban_name.txt = strdup(s);

	if (num_alias_bans == MAX_BANS)
		gi.dprintf("== WARNING: MAX %s BANS REACHED ==\n","ALIAS");

	return true;
}

/* =============== Public Functions =============== */

void Reset_Bans (void)
{
	int i;

	for (i=0;i<num_name_bans;i++)
		SAFE_FREE(name_bans[i].txt);

	for (i=0;i<num_filter_bans;i++)
		SAFE_FREE(filter_bans[i].txt);

	for (i=0;i<num_alias_bans;i++)
		SAFE_FREE(alias_bans[i].ban_name.txt);

	num_alias_bans = num_ip_bans = num_filter_bans = num_name_bans = 0;
}


void Read_Bans (const char *fname)
{
	int l;
	FILE *bans_file;
	char text[128], *s;

	if ( !(fname && (bans_file = fopen(fname, "r"))) )
		return;

	while (fgets(text, 128, bans_file))
	{
		s = text;
		// skip initial white space chars
		while (*s && isspace(*s)) s++;
		// skip emtpy lines and comments
		if ( !(*s) || *s == '#' || !strncmp(s,"//",2) ) continue;
		// remove trailing white space chars
		l = strlen(s);
		while (l > 0 && isspace(s[l-1])) s[--l] = '\0';

		if (!Q_strncasecmp(s,"IP:",3) && (num_ip_bans < MAX_BANS))
		{
#ifdef BAN_DEBUG
			gi.cprintf(NULL, PRINT_HIGH, "IP:%s",s+3);
#endif
			if (!addban_ip(s+3))
				gi.dprintf ("BAD BAN SYNTAX: %s\n",s);
		}
		else if (!Q_strncasecmp(s,"NAME:",5) && (num_name_bans < MAX_BANS))
		{
#ifdef BAN_DEBUG
			gi.cprintf(NULL, PRINT_HIGH, "NAME:%s",s+5);
#endif
			if (!addban_name(s+5))
				gi.dprintf ("BAD BAN SYNTAX: %s\n",s);
		}
		else if (!Q_strncasecmp(s,"ALIAS:",6) && (num_alias_bans < MAX_BANS))
		{
#ifdef BAN_DEBUG
			gi.cprintf(NULL, PRINT_HIGH, "ALIAS:%s",s+6);
#endif
			if (!addban_alias(s+6))
				gi.dprintf ("BAD BAN SYNTAX: %s\n",s);
		}
		else if (!Q_strncasecmp(s,"FILTER:",7) && (num_filter_bans < MAX_BANS))
		{
#ifdef BAN_DEBUG
			gi.cprintf(NULL, PRINT_HIGH, "FILTER:%s",s+6);
#endif
			if (!addban_filter(s+7))
				gi.dprintf ("BAD BAN SYNTAX: %s\n",s);
		}
//		else
//			gi.dprintf ("== Ignoring Line In Bans File: %s ==\n",line);
	}

	gi.dprintf("Bans: %d ALIAS : %d IP : %d NAME : %d FILTER\n",
		num_alias_bans,num_ip_bans,num_name_bans,num_filter_bans);

	fclose(bans_file);
}

void Show_Bans (edict_t *ent)
{
	int i;
	char txt[32];

	gi.cprintf (ent, PRINT_HIGH, "// Reserved Aliases\n");
	for (i=0;i<num_alias_bans;i++)
		gi.cprintf (ent, PRINT_HIGH, "ALIAS: %s %s\n",
			alias_bans[i].ban_name.txt,
			iptoa(txt, &(alias_bans[i].ban_ip))+2 );

	gi.cprintf (ent, PRINT_HIGH, "// IP Bans\n");
	for (i=0;i<num_ip_bans;i++)
		gi.cprintf (ent, PRINT_HIGH, "IP: %s\n",iptoa(txt,ip_bans + i));

	gi.cprintf (ent, PRINT_HIGH, "// NAME Bans\n");
	for (i=0;i<num_name_bans;i++)
		gi.cprintf (ent, PRINT_HIGH, "NAME: %s\n",name_bans[i].txt);

	gi.cprintf (ent, PRINT_HIGH, "// FILTER Bans\n");
	for (i=0;i<num_filter_bans;i++)
		gi.cprintf (ent, PRINT_HIGH, "FILTER: %s\n",filter_bans[i].txt);
}

int Write_Bans (const char *fname)
{
	int i;
	FILE *bans_file;
	char txt[32];

	if ( !(fname && (bans_file = fopen(fname, "w"))) )
		return 0;

	fprintf(bans_file,"// Reserved Aliases\n");
	for (i=0;i<num_alias_bans;i++)
		fprintf(bans_file,"ALIAS: %s %s\n",
			alias_bans[i].ban_name.txt,
			iptoa(txt, &(alias_bans[i].ban_ip))+2 );

	fprintf(bans_file,"\n// IP Bans\n");
	for (i=0;i<num_ip_bans;i++)
		fprintf(bans_file,"IP: %s\n",iptoa(txt,ip_bans + i));

	fprintf(bans_file,"\n// Name Bans\n");
	for (i=0;i<num_name_bans;i++)
		fprintf(bans_file,"NAME: %s\n",name_bans[i].txt);

	fprintf(bans_file,"\n// Filter Bans\n");
	for (i=0;i<num_filter_bans;i++)
		fprintf(bans_file,"FILTER: %s\n",filter_bans[i].txt);

	fclose(bans_file);

	return num_ip_bans + num_name_bans;
}

//
// Kick Stuff (often involves a ban)
//

void KickEnt (edict_t *ent)
{
	int i;
	char str[64];

	i = ENT_NUM(ent);

	sprintf(str,"kick %d\n",i - 1);
	gi.AddCommandString(str);
}

void KickPlayer (edict_t *admin, edict_t *idiot, char *reason)
{
	if (admin)
		gi.bprintf(PRINT_HIGH,"%s was KICKED by %s\n", netname(idiot), netname(admin));
	else
		gi.bprintf(PRINT_HIGH,"%s was KICKED from server\n", netname(idiot));

	if (reason)
		gi.bprintf(PRINT_HIGH,"Reason: %s.\n",reason);

	stuffcmd(idiot, STRING(STUFF_DISCONNECT));
	KickEnt(idiot);
}

void KickBanPlayer (edict_t *admin, edict_t *idiot, char *reason)
{
	char *ip;

	ip = get_client_ip(idiot);

	addban_ip(ip);
	addban_name(netname(idiot));

	gi.bprintf(PRINT_HIGH,"%s was BANNED from server\n", netname(idiot));

	stuffcmd(idiot, STRING(STUFF_DISCONNECT));
	KickEnt(idiot);
}

//
// Ban Stuff
//

static int get_mask (int bits)
{
	return -(1 << (32 - bits));
}

qboolean Reset_IP_Ban_Count (void)
{
	int i;
	qboolean found = false;

	for (i = 0; i < num_ip_bans; i++)
		if (ip_bans[i].nconnections != 0)
		{
			ip_bans[i].nconnections = 0;
			found = true;
		}

	return found;
}

//
// v1.3: added ip_flag variable, WHAT A HORRIBLE HACK :(
//

qboolean Is_IP_Banned (const char *addr, int ip_flag)
{
	int i;
	unsigned mask;
	qboolean banned;
	ban_ip_t ip, *ptr;

	// Convert IP ban string
	if (!atoip(&ip,addr)) return false;

	banned = false;
	for (i = 0; i < num_ip_bans; i++)
	{
		ptr = &ip_bans[i];
		mask = get_mask(ptr->maskbits);

		if ((ip.addr & mask) != (ptr->addr & mask)) // no match
			continue;

		if (ptr->allow)
		{
			banned = false;
			if (ptr->maxconnections)
			{
				if (ip_flag == NUL_IP_BAN && ptr->nconnections >= ptr->maxconnections)
					banned = true;
				else if (ip_flag == INC_IP_BAN)
				{
					if ( ++(ptr->nconnections) > ptr->maxconnections)
					{
						banned = true;
						ptr->nconnections = ptr->maxconnections; // Safety
					}
				}
				else if (ip_flag == DEC_IP_BAN && ptr->nconnections > 0)
					ptr->nconnections--;
			}
		}
		else
			banned = true;
	}

	return banned;
}

qboolean Is_Name_Banned (const char *name)
{
	int i;

	for (i=0; i < num_name_bans; i++)
		if ( z_stristr(name, name_bans[i].txt) ) return true;

	return false;
}

qboolean Is_Text_Banned (const char *str)
{
	int i;

	for (i=0; i < num_filter_bans; i++)
		if ( z_stristr(str, filter_bans[i].txt) ) return true;

	return false;
}

qboolean Is_Alias_Reserved (const char *str_name, const char *str_ip)
{
	int i;
	unsigned mask;
	ban_ip_t ip;
	ban_alias_t *ptr;
	qboolean reserved = false;

	// Convert IP ban string
	if ( !atoip(&ip,str_ip) ) return false;

	for (i=0; i < num_alias_bans; i++)
	{
		if ( z_stricmp(str_name, alias_bans[i].ban_name.txt) ) // no name match
			continue;

		ptr = &(alias_bans[i]);
		mask = get_mask(ptr->ban_ip.maskbits);

		if ((ip.addr & mask) == (ptr->ban_ip.addr & mask)) // match
			return false;
		else
			reserved = true;
	}

	return reserved;
}
