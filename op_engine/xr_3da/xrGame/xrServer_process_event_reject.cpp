#include "stdafx.h"
#include "xrserver.h"
#include "xrserver_objects.h"
#include "ai_space.h"
#include "script_engine.h"

bool xrServer::Process_event_reject	(NET_Packet& P, const ClientID sender, const u32 time, const u16 id_parent, const u16 id_entity, bool send_message)
{
	// Parse message
	CSE_Abstract*		e_parent	= game->get_entity_from_eid	(id_parent);
	CSE_Abstract*		e_entity	= game->get_entity_from_eid	(id_entity);
	
#ifdef MORE_SPAM
	Msg("sv reject. id_parent %s id_entity %s [%d]",ent_name_safe(id_parent).c_str(),ent_name_safe(id_entity).c_str(), Device.dwFrame);
#endif

	if (!(e_parent && e_entity))
	{
		std::string parent=ent_name_safe(id_parent).c_str();
		std::string entity=ent_name_safe(id_entity).c_str();
		Msg("~ WARNING: invalid server entitys combination e_parent (%s) & e_entity (%s)",parent.c_str(),entity.c_str());
		return false;
		//FATAL("ENGINE Crush. See log for detail.");
	}

	game->OnDetach		(id_parent,id_entity);

	if (0xffff == e_entity->ID_Parent) 
	{
		Msg	("~ WARNING: can't detach independant object. entity[%s:%d], parent[%s:%d], section[%s]",
			e_entity->name_replace(),id_entity,e_parent->name_replace(),id_parent, *e_entity->s_name);
		ai().script_engine().print_stack();
		return			(false);
	}

	// Rebuild parentness
	R_ASSERT3				(e_entity->ID_Parent == id_parent, e_entity->name_replace(), e_parent->name_replace());
	e_entity->ID_Parent		= 0xffff;
	xr_vector<u16>& C		= e_parent->children;

	xr_vector<u16>::iterator c	= std::find	(C.begin(),C.end(),id_entity);
	R_ASSERT3				(C.end()!=c,e_entity->name_replace(),e_parent->name_replace());
	C.erase					(c);

	// Signal to everyone (including sender)
	if (send_message)
	{
		DWORD MODE		= net_flags(TRUE,TRUE, FALSE, TRUE);
		SendBroadcast	(BroadcastCID,P,MODE);
	}
	
	return				(true);
}
