////////////////////////////////////////////////////////////////////////////
//	Module 		: xrServer_Objects_ALife_Items.cpp
//	Created 	: 19.09.2002
//  Modified 	: 04.06.2003
//	Author		: Oles Shyshkovtsov, Alexander Maksimchuk, Victor Reutskiy and Dmitriy Iassenev
//	Description : Server objects items for ALife simulator
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrMessages.h"
#include "../../xrNetServer/net_utils.h"
#include "clsid_game.h"
#include "xrServer_Objects_ALife_Items.h"
#include "clsid_game.h"

#ifndef XRGAME_EXPORTS
#	include "bone.h"
#else
#	include "..\bone.h"
#	ifdef DEBUG
#		define PHPH_DEBUG
#	endif
#endif
#ifdef PHPH_DEBUG
#include "PHDebug.h"
#endif

#pragma region CSE_ALifeInventoryItem
////////////////////////////////////////////////////////////////////////////
CSE_ALifeInventoryItem::CSE_ALifeInventoryItem(LPCSTR caSection)
{
	//������� ��������� ����
	m_fCondition				= 1.0f;

	m_fMass						= pSettings->r_float(caSection, "inv_weight");
	m_dwCost					= pSettings->r_u32(caSection, "cost");
	m_fRadiation = 0;
	if (pSettings->line_exist(caSection, "inventory_radiation"))
		m_fRadiation= pSettings->r_float(caSection, "inventory_radiation");

	if (pSettings->line_exist(caSection, "condition"))
		m_fCondition			= pSettings->r_float(caSection, "condition");

	if (pSettings->line_exist(caSection, "health_value"))
		m_iHealthValue			= pSettings->r_s32(caSection, "health_value");
	else
		m_iHealthValue			= 0;

	if (pSettings->line_exist(caSection, "food_value"))
		m_iFoodValue			= pSettings->r_s32(caSection, "food_value");
	else
		m_iFoodValue			= 0;

	m_fDeteriorationValue		= 0;

	m_last_update_time			= 0;

	State.quaternion.x			= 0.f;
	State.quaternion.y			= 0.f;
	State.quaternion.z			= 1.f;
	State.quaternion.w			= 0.f;

	State.angular_vel.set		(0.f,0.f,0.f);
	State.linear_vel.set		(0.f,0.f,0.f);
}

CSE_Abstract *CSE_ALifeInventoryItem::init	()
{
	m_self						= smart_cast<CSE_ALifeObject*>(this);
	R_ASSERT					(m_self);
//	m_self->m_flags.set			(CSE_ALifeObject::flSwitchOffline,TRUE);
	return						(base());
}

CSE_ALifeInventoryItem::~CSE_ALifeInventoryItem	()
{
}

void CSE_ALifeInventoryItem::STATE_Write	(NET_Packet &tNetPacket)
{
	tNetPacket.w_float			(m_fCondition);
	tNetPacket.w_float(m_fMass);
	tNetPacket.w_u32(m_dwCost);
	tNetPacket.w_float(m_fRadiation);
	State.position				= base()->o_Position;
}

void CSE_ALifeInventoryItem::STATE_Read		(NET_Packet &tNetPacket, u16 size)
{
	u16 m_wVersion = base()->m_wVersion;
	if (m_wVersion > 52)
		tNetPacket.r_float		(m_fCondition);
	if (m_wVersion > 120)
	{
		tNetPacket.r_float(m_fMass);
		tNetPacket.r_u32(m_dwCost);
	}
	if (m_wVersion > 121)
		tNetPacket.r_float(m_fRadiation);
	State.position				= base()->o_Position;
}

static inline bool check (const u8 &mask, const u8 &test)
{
	return							(!!(mask & test));
}

void CSE_ALifeInventoryItem::writeWoodooMagic(NET_Packet& tNetPacket)
{
#pragma region woodoo magic
	if (!m_u8NumItems) {
		tNetPacket.w_u8(0);
		return;
	}

	mask_num_items					num_items;
	num_items.mask = 0;
	num_items.num_items = m_u8NumItems;

	R_ASSERT2(
		num_items.num_items < (u8(1) << 5),
		make_string("%d", num_items.num_items)
	);

	if (State.enabled)									num_items.mask |= inventory_item_state_enabled;
	if (fis_zero(State.angular_vel.square_magnitude()))	num_items.mask |= inventory_item_angular_null;
	if (fis_zero(State.linear_vel.square_magnitude()))	num_items.mask |= inventory_item_linear_null;

	tNetPacket.w_u8(num_items.common);

	tNetPacket.w_vec3(State.position);

	tNetPacket.w_float_q8(State.quaternion.x, 0.f, 1.f);
	tNetPacket.w_float_q8(State.quaternion.y, 0.f, 1.f);
	tNetPacket.w_float_q8(State.quaternion.z, 0.f, 1.f);
	tNetPacket.w_float_q8(State.quaternion.w, 0.f, 1.f);

	if (!check(num_items.mask, inventory_item_angular_null)) {
		tNetPacket.w_float_q8(State.angular_vel.x, 0.f, 10 * PI_MUL_2);
		tNetPacket.w_float_q8(State.angular_vel.y, 0.f, 10 * PI_MUL_2);
		tNetPacket.w_float_q8(State.angular_vel.z, 0.f, 10 * PI_MUL_2);
	}

	if (!check(num_items.mask, inventory_item_linear_null)) {
		tNetPacket.w_float_q8(State.linear_vel.x, -32.f, 32.f);
		tNetPacket.w_float_q8(State.linear_vel.y, -32.f, 32.f);
		tNetPacket.w_float_q8(State.linear_vel.z, -32.f, 32.f);
	}
#pragma endregion
}

void CSE_ALifeInventoryItem::UPDATE_Write(NET_Packet &tNetPacket)
{
	writeWoodooMagic(tNetPacket);
}

void CSE_ALifeInventoryItem::readWoodooMagic(NET_Packet& tNetPacket)
{
#pragma region woodoo magic
	tNetPacket.r_u8(m_u8NumItems);
	if (!m_u8NumItems) {
		return;
	}

	mask_num_items					num_items;
	num_items.common = m_u8NumItems;
	m_u8NumItems = num_items.num_items;

	R_ASSERT2(
		m_u8NumItems < (u8(1) << 5),
		make_string("%d", m_u8NumItems)
	);

	tNetPacket.r_vec3(State.position);

	tNetPacket.r_float_q8(State.quaternion.x, 0.f, 1.f);
	tNetPacket.r_float_q8(State.quaternion.y, 0.f, 1.f);
	tNetPacket.r_float_q8(State.quaternion.z, 0.f, 1.f);
	tNetPacket.r_float_q8(State.quaternion.w, 0.f, 1.f);

	State.enabled = check(num_items.mask, inventory_item_state_enabled);

	if (!check(num_items.mask, inventory_item_angular_null)) {
		tNetPacket.r_float_q8(State.angular_vel.x, 0.f, 10 * PI_MUL_2);
		tNetPacket.r_float_q8(State.angular_vel.y, 0.f, 10 * PI_MUL_2);
		tNetPacket.r_float_q8(State.angular_vel.z, 0.f, 10 * PI_MUL_2);
	}
	else
		State.angular_vel.set(0.f, 0.f, 0.f);

	if (!check(num_items.mask, inventory_item_linear_null)) {
		tNetPacket.r_float_q8(State.linear_vel.x, -32.f, 32.f);
		tNetPacket.r_float_q8(State.linear_vel.y, -32.f, 32.f);
		tNetPacket.r_float_q8(State.linear_vel.z, -32.f, 32.f);
	}
	else
		State.linear_vel.set(0.f, 0.f, 0.f);
#pragma endregion
}
void CSE_ALifeInventoryItem::UPDATE_Read	(NET_Packet &tNetPacket)
{
	readWoodooMagic(tNetPacket);
};

void CSE_ALifeInventoryItem::FillProps		(LPCSTR pref, PropItemVec& values)
{
	PHelper().CreateFloat			(values, PrepareKey(pref, *base()->s_name, "Item condition"), 		&m_fCondition, 			0.f, 1.f);
	CSE_ALifeObject					*alife_object = smart_cast<CSE_ALifeObject*>(base());
	R_ASSERT						(alife_object);
	PHelper().CreateFlag32			(values, PrepareKey(pref, *base()->s_name,"ALife\\Useful for AI"),	&alife_object->m_flags,	CSE_ALifeObject::flUsefulForAI);
	PHelper().CreateFlag32			(values, PrepareKey(pref, *base()->s_name,"ALife\\Visible for AI"),	&alife_object->m_flags,	CSE_ALifeObject::flVisibleForAI);
}

bool CSE_ALifeInventoryItem::bfUseful		()
{
	return						(true);
}

u32 CSE_ALifeInventoryItem::update_rate		() const
{
	return						(1000);
}
#pragma endregion

#pragma region CSE_ALifeItem
CSE_ALifeItem::CSE_ALifeItem				(LPCSTR caSection) : CSE_ALifeDynamicObjectVisual(caSection), CSE_ALifeInventoryItem(caSection)
{
	m_physics_disabled			= false;
}

CSE_ALifeItem::~CSE_ALifeItem				()
{
}

CSE_Abstract *CSE_ALifeItem::init			()
{
	inherited1::init			();
	inherited2::init			();
	return						(base());
}

CSE_Abstract *CSE_ALifeItem::base			()
{
	return						(inherited1::base());
}

const CSE_Abstract *CSE_ALifeItem::base		() const
{
	return						(inherited1::base());
}

void CSE_ALifeItem::STATE_Write				(NET_Packet &tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	inherited2::STATE_Write		(tNetPacket);
}

void CSE_ALifeItem::STATE_Read				(NET_Packet &tNetPacket, u16 size)
{
	inherited1::STATE_Read		(tNetPacket, size);
	if ((m_tClassID == CLSID_OBJECT_W_BINOCULAR) && (m_wVersion < 37)) {
		tNetPacket.r_u16		();
		tNetPacket.r_u16		();
		tNetPacket.r_u8			();
	}
	inherited2::STATE_Read		(tNetPacket, size);
}

void CSE_ALifeItem::UPDATE_Write			(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
	inherited2::UPDATE_Write	(tNetPacket);

#ifdef XRGAME_EXPORTS
	m_last_update_time			= Device.dwTimeGlobal;
#endif // XRGAME_EXPORTS
};

void CSE_ALifeItem::UPDATE_Read				(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
	inherited2::UPDATE_Read		(tNetPacket);

	m_physics_disabled			= false;
};

void CSE_ALifeItem::FillProps				(LPCSTR pref, PropItemVec& values)
{
	inherited1::FillProps		(pref,	 values);
	inherited2::FillProps		(pref,	 values);
}

BOOL CSE_ALifeItem::Net_Relevant			()
{
	if (attached())
		return					(false);

	if (!m_physics_disabled && !fis_zero(State.linear_vel.square_magnitude(),EPS_L))
		return					(true);

#ifdef XRGAME_EXPORTS
	if (Device.dwTimeGlobal < (m_last_update_time + update_rate()))
		return					(false);
#endif // XRGAME_EXPORTS

	return						(true);
}

void CSE_ALifeItem::OnEvent					(NET_Packet &tNetPacket, u16 type, u32 time, ClientID sender )
{
	inherited1::OnEvent			(tNetPacket,type,time,sender);

	if (type != GE_FREEZE_OBJECT)
		return;

//	R_ASSERT					(!m_physics_disabled);
	m_physics_disabled			= true;
}
#pragma endregion

#pragma region CSE_ALifeItemTorch
CSE_ALifeItemTorch::CSE_ALifeItemTorch		(LPCSTR caSection) : CSE_ALifeItem(caSection)
{
	m_active					= false;
	m_nightvision_active		= false;
	m_attached					= false;
}

CSE_ALifeItemTorch::~CSE_ALifeItemTorch		()
{
}

BOOL	CSE_ALifeItemTorch::Net_Relevant			()
{
	if (m_attached) return true;
	return inherited::Net_Relevant();
}


void CSE_ALifeItemTorch::STATE_Read			(NET_Packet	&tNetPacket, u16 size)
{
	if (m_wVersion > 20)
		inherited::STATE_Read	(tNetPacket,size);

}

void CSE_ALifeItemTorch::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeItemTorch::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
	
	BYTE F = tNetPacket.r_u8();
	m_active					= !!(F & eTorchActive);
	m_nightvision_active		= !!(F & eNightVisionActive);
	m_attached					= !!(F & eAttached);
}

void CSE_ALifeItemTorch::UPDATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);

	BYTE F = 0;
	F |= (m_active ? eTorchActive : 0);
	F |= (m_nightvision_active ? eNightVisionActive : 0);
	F |= (m_attached ? eAttached : 0);
	tNetPacket.w_u8(F);
}

void CSE_ALifeItemTorch::FillProps			(LPCSTR pref, PropItemVec& values)
{
	inherited::FillProps			(pref,	 values);
}
#pragma endregion

#pragma region CSE_ALifeItemNVDevice
CSE_ALifeItemNVDevice::CSE_ALifeItemNVDevice(LPCSTR caSection) : CSE_ALifeItem(caSection)
{
	m_active= false;	
	m_enabled= true;
	m_attached=false;
}

CSE_ALifeItemNVDevice::~CSE_ALifeItemNVDevice() {}

BOOL	CSE_ALifeItemNVDevice::Net_Relevant			()
{
	if (m_attached) return true;
	return inherited::Net_Relevant();
}

void CSE_ALifeItemNVDevice::STATE_Read			(NET_Packet	&tNetPacket, u16 size)
{
	if (m_wVersion > 20)
		inherited::STATE_Read	(tNetPacket,size);
}

void CSE_ALifeItemNVDevice::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeItemNVDevice::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
#pragma todo("maybe remove second part from NG")
	if (m_wVersion > 119 || (m_wVersion<119 && !tNetPacket.r_eof())) //second part of if - old object loaded, but new version update updated 
	{
		BYTE F = tNetPacket.r_u8();	
		m_active					= !!(F & eActive);
		m_enabled					= !!(F & eEnabled);
		m_attached					= !!(F & eAttached);
	} 
}

void CSE_ALifeItemNVDevice::UPDATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);

	BYTE F = 0;

	F |= (m_active   ?  eActive : 0);
	F |= (m_enabled  ? eEnabled : 0);
	F |= (m_attached  ? eAttached : 0);
	tNetPacket.w_u8(F);
}

void CSE_ALifeItemNVDevice::FillProps			(LPCSTR pref, PropItemVec& values)
{
	inherited::FillProps			(pref,	 values);
}
#pragma endregion

#pragma region CSE_ALifeItemGameBox
CSE_ALifeItemGameBox::CSE_ALifeItemGameBox(LPCSTR caSection) : CSE_ALifeItem(caSection)
{
	m_user_data.write_start();
	m_user_data.read_start();
}

CSE_ALifeItemGameBox::~CSE_ALifeItemGameBox() {}

BOOL	CSE_ALifeItemGameBox::Net_Relevant()
{
	return inherited::Net_Relevant();
}

void CSE_ALifeItemGameBox::set_user_data(NET_Packet* data)
{
	m_user_data.write_start();
	if (data->B.count > 0)
	{
		data->r_seek(0);
		while (!data->r_eof())
			m_user_data.w_u8(data->r_u8());
		m_user_data.r_seek(0);
	}
}

void CSE_ALifeItemGameBox::clear_user_data()
{
	ZeroMemory(m_user_data.B.data, m_user_data.B.count);
	m_user_data.write_start();
	m_user_data.r_seek(0);
}

void CSE_ALifeItemGameBox::get_user_data(NET_Packet* data)
{
	data->write_start();
	if (m_user_data.B.count > 0)
	{
		m_user_data.r_seek(0);
		while (!m_user_data.r_eof())
			data->w_u8(m_user_data.r_u8());
		data->r_seek(0);
	}
}

void CSE_ALifeItemGameBox::STATE_Read(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read(tNetPacket, size);
	if (m_wVersion>122)
	{
		u32 uds= tNetPacket.r_u32();
		if (uds>0 && uds < NET_PacketSizeLimit)
		{
			m_user_data.write_start();
			while (uds != 0)
			{
				m_user_data.w_u8(tNetPacket.r_u8());
				uds--;
			}
			m_user_data.r_seek(0);
		}
	}
}

void CSE_ALifeItemGameBox::STATE_Write(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write(tNetPacket);
	m_user_data.r_seek(0);
	if (m_user_data.B.count > 0)
	{
		tNetPacket.w_u32(m_user_data.B.count);
		while (!m_user_data.r_eof())
			tNetPacket.w_u8(m_user_data.r_u8());
	}
	else
		tNetPacket.w_u32(0);
}

void CSE_ALifeItemGameBox::UPDATE_Read(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read(tNetPacket);
}

void CSE_ALifeItemGameBox::UPDATE_Write(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write(tNetPacket);
}
#pragma region unused now, but working, exec after client script binder net_spawn/net_destroy
/*void CSE_ALifeItemGameBox::add_online(const bool& update_registries)
{
	CSE_ALifeItem::add_online(update_registries);
	Msg("CSE_ALifeItemGameBox::add_online");
}

void CSE_ALifeItemGameBox::add_offline(const xr_vector<ALife::_OBJECT_ID>& saved_children, const bool& update_registries)
{
	CSE_ALifeItem::add_offline(saved_children, update_registries);
	Msg("CSE_ALifeItemGameBox::add_offline");
}
*/
#pragma endregion

void CSE_ALifeItemGameBox::FillProps(LPCSTR pref, PropItemVec& values)
{
	inherited::FillProps(pref, values);
}
#pragma endregion

#pragma region CSE_ALifeItemWeapon
CSE_ALifeItemWeapon::CSE_ALifeItemWeapon	(LPCSTR caSection) : CSE_ALifeItem(caSection)
{
	a_current					= 90;
	a_elapsed					= 0;
	wpn_flags					= 0;
	wpn_state					= 0;
	ammo_type					= 0;
	m_fHitPower					= pSettings->r_float(caSection,"hit_power");
	m_tHitType					= ALife::g_tfString2HitType(pSettings->r_string(caSection,"hit_type"));
	m_caAmmoSections			= pSettings->r_string(caSection,"ammo_class");
	if (pSettings->section_exist(caSection) && pSettings->line_exist(caSection,"visual"))
		set_visual				(pSettings->r_string(caSection,"visual"));

	m_addon_flags.zero			();

	m_scope_status				=	(EWeaponAddonStatus)pSettings->r_s32(s_name,"scope_status");
	m_silencer_status			=	(EWeaponAddonStatus)pSettings->r_s32(s_name,"silencer_status");
	m_grenade_launcher_status	=	(EWeaponAddonStatus)pSettings->r_s32(s_name,"grenade_launcher_status");
	m_ef_main_weapon_type		= READ_IF_EXISTS(pSettings,r_u32,caSection,"ef_main_weapon_type",u32(-1));
	m_ef_weapon_type			= READ_IF_EXISTS(pSettings,r_u32,caSection,"ef_weapon_type",u32(-1));
}

CSE_ALifeItemWeapon::~CSE_ALifeItemWeapon	()
{
}

u32	CSE_ALifeItemWeapon::ef_main_weapon_type() const
{
	VERIFY	(m_ef_main_weapon_type != u32(-1));
	return	(m_ef_main_weapon_type);
}

u32	CSE_ALifeItemWeapon::ef_weapon_type() const
{
	VERIFY	(m_ef_weapon_type != u32(-1));
	return	(m_ef_weapon_type);
}

void CSE_ALifeItemWeapon::UPDATE_Read(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
	tNetPacket.r_float_q8		(m_fCondition,0.0f,1.0f);
	tNetPacket.r_u8				(wpn_flags);
	tNetPacket.r_u16			(a_elapsed);
	tNetPacket.r_u8				(m_addon_flags.flags);
	tNetPacket.r_u8				(ammo_type);
	tNetPacket.r_u8				(wpn_state);
	tNetPacket.r_u8				(m_bZoom);
}

void CSE_ALifeItemWeapon::UPDATE_Write(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);

	tNetPacket.w_float_q8		(m_fCondition,0.0f,1.0f);
	tNetPacket.w_u8				(wpn_flags);
	tNetPacket.w_u16			(a_elapsed);
	tNetPacket.w_u8				(m_addon_flags.get());
	tNetPacket.w_u8				(ammo_type);
	tNetPacket.w_u8				(wpn_state);
	tNetPacket.w_u8				(m_bZoom);
}

void CSE_ALifeItemWeapon::STATE_Read(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket, size);
	tNetPacket.r_u16			(a_current);
	tNetPacket.r_u16			(a_elapsed);
	tNetPacket.r_u8				(wpn_state);
	
	if (m_wVersion > 40)
		tNetPacket.r_u8			(m_addon_flags.flags);

	if (m_wVersion > 46)
		tNetPacket.r_u8			(ammo_type);
}

void CSE_ALifeItemWeapon::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
	tNetPacket.w_u16			(a_current);
	tNetPacket.w_u16			(a_elapsed);
	tNetPacket.w_u8				(wpn_state);
	tNetPacket.w_u8				(m_addon_flags.get());
	tNetPacket.w_u8				(ammo_type);
}

void CSE_ALifeItemWeapon::OnEvent			(NET_Packet	&tNetPacket, u16 type, u32 time, ClientID sender )
{
	inherited::OnEvent			(tNetPacket,type,time,sender);
	switch (type) {
		case GE_WPN_STATE_CHANGE:
			{			
				tNetPacket.r_u8	(wpn_state);			
//				u8 sub_state = 
					tNetPacket.r_u8();		
//				u8 NewAmmoType = 
					tNetPacket.r_u8();
//				u8 AmmoElapsed = 
					tNetPacket.r_u8();	
			}break;
	}
}

u8	 CSE_ALifeItemWeapon::get_slot			()
{
	return						((u8)pSettings->r_u8(s_name,"slot"));
}

u16	 CSE_ALifeItemWeapon::get_ammo_limit	()
{
	return						(u16) pSettings->r_u16(s_name,"ammo_limit");
}

u16	 CSE_ALifeItemWeapon::get_ammo_total	()
{
	return						((u16)a_current);
}

u16	 CSE_ALifeItemWeapon::get_ammo_elapsed	()
{
	return						((u16)a_elapsed);
}

u8	CSE_ALifeItemWeapon::get_addon_state()
{
	return m_addon_flags.get();
}

u16	 CSE_ALifeItemWeapon::get_ammo_magsize	()
{
	if (pSettings->line_exist(s_name,"ammo_mag_size"))
		return					(pSettings->r_u16(s_name,"ammo_mag_size"));
	else
		return					0;
}

BOOL CSE_ALifeItemWeapon::Net_Relevant()
{
	return (wpn_flags==1);
}

void CSE_ALifeItemWeapon::FillProps			(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps			(pref, items);
	PHelper().CreateU8			(items,PrepareKey(pref,*s_name,"Ammo type:"), &ammo_type,0,255,1);
	PHelper().CreateU16			(items,PrepareKey(pref,*s_name,"Ammo: in magazine"),	&a_elapsed,0,30,1);
	

	if (m_scope_status == eAddonAttachable)
		   PHelper().CreateFlag8(items,PrepareKey(pref,*s_name,"Addons\\Scope"), 	&m_addon_flags, eWeaponAddonScope);

	if (m_silencer_status == eAddonAttachable)
		PHelper().CreateFlag8	(items,PrepareKey(pref,*s_name,"Addons\\Silencer"), 	&m_addon_flags, eWeaponAddonSilencer);

	if (m_grenade_launcher_status == eAddonAttachable)
		PHelper().CreateFlag8	(items,PrepareKey(pref,*s_name,"Addons\\Podstvolnik"),&m_addon_flags,eWeaponAddonGrenadeLauncher);
}
#pragma endregion

#pragma region CSE_ALifeItemWeaponShotGun
CSE_ALifeItemWeaponShotGun::CSE_ALifeItemWeaponShotGun	(LPCSTR caSection) : CSE_ALifeItemWeaponMagazined(caSection)
{
	m_AmmoIDs.clear();
}

CSE_ALifeItemWeaponShotGun::~CSE_ALifeItemWeaponShotGun	()
{
}

void CSE_ALifeItemWeaponShotGun::UPDATE_Read		(NET_Packet& P)
{
	inherited::UPDATE_Read(P);
	m_AmmoIDs.clear();
	u8 AmmoCount = P.r_u8();
	for (u8 i=0; i<AmmoCount; i++)
	{
		m_AmmoIDs.push_back(P.r_u8());
	}
}
void CSE_ALifeItemWeaponShotGun::UPDATE_Write	(NET_Packet& P)
{
	inherited::UPDATE_Write(P);
	P.w_u8(u8(m_AmmoIDs.size()));
	for (u32 i=0; i<m_AmmoIDs.size(); i++)
	{
		P.w_u8(u8(m_AmmoIDs[i]));
	}
}
void CSE_ALifeItemWeaponShotGun::STATE_Read		(NET_Packet& P, u16 size)
{
	inherited::STATE_Read(P, size);
	m_AmmoIDs.clear();
	if (base()->m_wVersion > 125)
	{
		u8 AmmoCount = P.r_u8();
		for (u8 i = 0; i < AmmoCount; i++)
		{
			m_AmmoIDs.push_back(P.r_u8());
		}
	}
}

void CSE_ALifeItemWeaponShotGun::STATE_Write		(NET_Packet& P)
{
	/*if (xr_strcmp(base()->name(),"wpn_browningauto5")==0)
	{
		string256 types={0};
		std::for_each(m_AmmoIDs.begin(), m_AmmoIDs.end(), [&](u8 at)
		{
			sprintf_s(types, "%s %d", types, at);
		});
		Msg("STATE_Write [%s] [%d][%s]",base()->name_replace(), m_AmmoIDs.size(),types);
	}*/
	inherited::STATE_Write(P);
	P.w_u8(u8(m_AmmoIDs.size()));
	for (u32 i = 0; i<m_AmmoIDs.size(); i++)
	{
		P.w_u8(u8(m_AmmoIDs[i]));
	}
}

void CSE_ALifeItemWeaponShotGun::FillProps			(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps			(pref, items);
};
#pragma endregion

#pragma region CSE_ALifeItemWeaponMagazined
CSE_ALifeItemWeaponMagazined::CSE_ALifeItemWeaponMagazined	(LPCSTR caSection) : CSE_ALifeItemWeapon(caSection)
{
	m_u8CurFireMode = 0;
	m_fSilencerCondition = 1.0f;
}

CSE_ALifeItemWeaponMagazined::~CSE_ALifeItemWeaponMagazined	()
{
}

void CSE_ALifeItemWeaponMagazined::UPDATE_Read		(NET_Packet& P)
{
	inherited::UPDATE_Read(P);
	m_u8CurFireMode = P.r_u8();
}
void CSE_ALifeItemWeaponMagazined::UPDATE_Write	(NET_Packet& P)
{
	inherited::UPDATE_Write(P);
	P.w_u8(m_u8CurFireMode);	
}

void CSE_ALifeItemWeaponMagazined::STATE_Read		(NET_Packet& P, u16 size)
{
	inherited::STATE_Read(P, size);
	if (base()->m_wVersion>124)
		P.r_float(m_fSilencerCondition);

}
void CSE_ALifeItemWeaponMagazined::STATE_Write		(NET_Packet& P)
{
	inherited::STATE_Write(P);
	P.w_float(m_fSilencerCondition);
}

void CSE_ALifeItemWeaponMagazined::FillProps			(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps			(pref, items);
};
#pragma endregion

#pragma region CSE_ALifeItemWeaponMagazinedWGL
CSE_ALifeItemWeaponMagazinedWGL::CSE_ALifeItemWeaponMagazinedWGL	(LPCSTR caSection) : CSE_ALifeItemWeaponMagazined(caSection)
{
	m_bGrenadeMode = 0;
}

CSE_ALifeItemWeaponMagazinedWGL::~CSE_ALifeItemWeaponMagazinedWGL	()
{
}

void CSE_ALifeItemWeaponMagazinedWGL::UPDATE_Read		(NET_Packet& P)
{
	m_bGrenadeMode = !!P.r_u8();
	inherited::UPDATE_Read(P);

}
void CSE_ALifeItemWeaponMagazinedWGL::UPDATE_Write	(NET_Packet& P)
{
	P.w_u8(m_bGrenadeMode ? 1 : 0);	
	inherited::UPDATE_Write(P);

}
void CSE_ALifeItemWeaponMagazinedWGL::STATE_Read		(NET_Packet& P, u16 size)
{
	inherited::STATE_Read(P, size);
}
void CSE_ALifeItemWeaponMagazinedWGL::STATE_Write		(NET_Packet& P)
{
	inherited::STATE_Write(P);
}

void CSE_ALifeItemWeaponMagazinedWGL::FillProps			(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps			(pref, items);
};
#pragma endregion

#pragma region CSE_ALifeItemAmmo
CSE_ALifeItemAmmo::CSE_ALifeItemAmmo		(LPCSTR caSection) : CSE_ALifeItem(caSection)
{
	a_elapsed					= m_boxSize = (u16)pSettings->r_s32(caSection, "box_size");
	if (pSettings->section_exist(caSection) && pSettings->line_exist(caSection,"visual"))
		set_visual				(pSettings->r_string(caSection,"visual"));
}

CSE_ALifeItemAmmo::~CSE_ALifeItemAmmo		()
{
}

void CSE_ALifeItemAmmo::STATE_Read			(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
	tNetPacket.r_u16			(a_elapsed);
}

void CSE_ALifeItemAmmo::STATE_Write			(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
	tNetPacket.w_u16			(a_elapsed);
}

void CSE_ALifeItemAmmo::UPDATE_Read			(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);

	tNetPacket.r_u16			(a_elapsed);
}

void CSE_ALifeItemAmmo::UPDATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);

	tNetPacket.w_u16			(a_elapsed);
}

void CSE_ALifeItemAmmo::FillProps			(LPCSTR pref, PropItemVec& values) {
	inherited::FillProps			(pref,values);
	PHelper().CreateU16			(values, PrepareKey(pref, *s_name, "Ammo: left"), &a_elapsed, 0, m_boxSize, m_boxSize);
}

bool CSE_ALifeItemAmmo::can_switch_online	() const
{
	return inherited::can_switch_online();
}

bool CSE_ALifeItemAmmo::can_switch_offline	() const
{
	return ( inherited::can_switch_offline() && a_elapsed!=0 );
}
#pragma endregion

#pragma region CSE_ALifeItemDetector
CSE_ALifeItemDetector::CSE_ALifeItemDetector(LPCSTR caSection) : CSE_ALifeItem(caSection)
{
	m_ef_detector_type	= pSettings->r_u32(caSection,"ef_detector_type");
}

CSE_ALifeItemDetector::~CSE_ALifeItemDetector()
{
}

u32	 CSE_ALifeItemDetector::ef_detector_type	() const
{
	return	(m_ef_detector_type);
}

void CSE_ALifeItemDetector::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	if (m_wVersion > 20)
		inherited::STATE_Read	(tNetPacket,size);
}

void CSE_ALifeItemDetector::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeItemDetector::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeItemDetector::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

void CSE_ALifeItemDetector::FillProps		(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps			(pref,items);
}
#pragma endregion

#pragma region CSE_ALifeItemArtefact
CSE_ALifeItemArtefact::CSE_ALifeItemArtefact(LPCSTR caSection) : CSE_ALifeItem(caSection)
{
	m_fAnomalyValue				= 100.f;
}

CSE_ALifeItemArtefact::~CSE_ALifeItemArtefact()
{
}

void CSE_ALifeItemArtefact::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
}

void CSE_ALifeItemArtefact::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeItemArtefact::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeItemArtefact::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

void CSE_ALifeItemArtefact::FillProps		(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps			(pref,items);
	PHelper().CreateFloat			(items, PrepareKey(pref, *s_name, "Anomaly value:"), &m_fAnomalyValue, 0.f, 200.f);
}

BOOL CSE_ALifeItemArtefact::Net_Relevant	()
{
	return							(inherited::Net_Relevant());
}
#pragma endregion

#pragma region CSE_ALifeItemPDA
CSE_ALifeItemPDA::CSE_ALifeItemPDA		(LPCSTR caSection) : CSE_ALifeItem(caSection)
{
	m_original_owner		= 0xffff;
	m_specific_character	= NULL;
	m_info_portion			= NULL;
}


CSE_ALifeItemPDA::~CSE_ALifeItemPDA		()
{
}

void CSE_ALifeItemPDA::STATE_Read(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read(tNetPacket, size);
	if (m_wVersion > 58)
		tNetPacket.r(&m_original_owner, sizeof(m_original_owner));

	if (m_wVersion > 89)

		if ((m_wVersion > 89) && (m_wVersion < 98))
		{
			int tmp, tmp2;
			tNetPacket.r(&tmp, sizeof(int));
			tNetPacket.r(&tmp2, sizeof(int));
			m_info_portion = NULL;
			m_specific_character = NULL;
		}
		else {
			tNetPacket.r_stringZ(m_specific_character);
			tNetPacket.r_stringZ(m_info_portion);

		}
}

void CSE_ALifeItemPDA::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
	tNetPacket.w				(&m_original_owner,sizeof(m_original_owner));
#ifdef XRGAME_EXPORTS
	tNetPacket.w_stringZ		(m_specific_character);
	tNetPacket.w_stringZ		(m_info_portion);
#else
	shared_str		tmp_1	= NULL;
	shared_str						tmp_2	= NULL;

	tNetPacket.w_stringZ		(tmp_1);
	tNetPacket.w_stringZ		(tmp_2);
#endif

}

void CSE_ALifeItemPDA::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeItemPDA::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

void CSE_ALifeItemPDA::FillProps		(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps			(pref,items);
}
#pragma endregion

#pragma region CSE_ALifeItemDocument
CSE_ALifeItemDocument::CSE_ALifeItemDocument(LPCSTR caSection): CSE_ALifeItem(caSection)
{
	m_wDoc					= nullptr;
}

CSE_ALifeItemDocument::~CSE_ALifeItemDocument()
{
}

void CSE_ALifeItemDocument::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);

	if ( m_wVersion < 98  ){
		u16 tmp;
		tNetPacket.r_u16			(tmp);
		m_wDoc = nullptr;
	}else
		tNetPacket.r_stringZ		(m_wDoc);
}

void CSE_ALifeItemDocument::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
	tNetPacket.w_stringZ		(m_wDoc);
}

void CSE_ALifeItemDocument::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeItemDocument::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

void CSE_ALifeItemDocument::FillProps		(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps			(pref,items);
//	PHelper().CreateU16			(items, PrepareKey(pref, *s_name, "Document index :"), &m_wDocIndex, 0, 65535);
	PHelper().CreateRText		(items, PrepareKey(pref, *s_name, "Info portion :"), &m_wDoc);
}
#pragma endregion

#pragma region CSE_ALifeItemGrenade
CSE_ALifeItemGrenade::CSE_ALifeItemGrenade	(LPCSTR caSection): CSE_ALifeItem(caSection)
{
	m_ef_weapon_type	= READ_IF_EXISTS(pSettings,r_u32,caSection,"ef_weapon_type",u32(-1));
}

CSE_ALifeItemGrenade::~CSE_ALifeItemGrenade	()
{
}

u32	CSE_ALifeItemGrenade::ef_weapon_type() const
{
	VERIFY	(m_ef_weapon_type != u32(-1));
	return	(m_ef_weapon_type);
}

void CSE_ALifeItemGrenade::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
}

void CSE_ALifeItemGrenade::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeItemGrenade::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeItemGrenade::UPDATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

void CSE_ALifeItemGrenade::FillProps			(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps			(pref,items);
}
#pragma endregion

#pragma region CSE_ALifeItemExplosive
CSE_ALifeItemExplosive::CSE_ALifeItemExplosive	(LPCSTR caSection): CSE_ALifeItem(caSection)
{
}

CSE_ALifeItemExplosive::~CSE_ALifeItemExplosive	()
{
}

void CSE_ALifeItemExplosive::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
}

void CSE_ALifeItemExplosive::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeItemExplosive::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeItemExplosive::UPDATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

void CSE_ALifeItemExplosive::FillProps			(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps			(pref,items);
}
#pragma endregion

#pragma region CSE_ALifeItemBolt
CSE_ALifeItemBolt::CSE_ALifeItemBolt		(LPCSTR caSection) : CSE_ALifeItem(caSection)
{
	m_flags.set					(flUseSwitches,FALSE);
	m_flags.set					(flSwitchOffline,FALSE);
	m_ef_weapon_type			= READ_IF_EXISTS(pSettings,r_u32,caSection,"ef_weapon_type",u32(-1));
}

CSE_ALifeItemBolt::~CSE_ALifeItemBolt		()
{
}

u32	CSE_ALifeItemBolt::ef_weapon_type() const
{
	VERIFY	(m_ef_weapon_type != u32(-1));
	return	(m_ef_weapon_type);
}

void CSE_ALifeItemBolt::STATE_Write			(NET_Packet &tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeItemBolt::STATE_Read			(NET_Packet &tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket, size);
}

void CSE_ALifeItemBolt::UPDATE_Write		(NET_Packet &tNetPacket)
{
	inherited::UPDATE_Write	(tNetPacket);
};

void CSE_ALifeItemBolt::UPDATE_Read			(NET_Packet &tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
};

bool CSE_ALifeItemBolt::can_save			() const
{
	return						(false);//!attached());
}
bool CSE_ALifeItemBolt::used_ai_locations		() const
{
	return false;
}
void CSE_ALifeItemBolt::FillProps			(LPCSTR pref, PropItemVec& values)
{
	inherited::FillProps			(pref,	 values);
}
#pragma endregion

#pragma region CSE_ALifeItemCustomOutfit
CSE_ALifeItemCustomOutfit::CSE_ALifeItemCustomOutfit	(LPCSTR caSection): CSE_ALifeItem(caSection)
{
	m_ef_equipment_type		= pSettings->r_u32(caSection,"ef_equipment_type");
}

CSE_ALifeItemCustomOutfit::~CSE_ALifeItemCustomOutfit	()
{
}

u32	CSE_ALifeItemCustomOutfit::ef_equipment_type		() const
{
	return			(m_ef_equipment_type);
}

void CSE_ALifeItemCustomOutfit::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
}

void CSE_ALifeItemCustomOutfit::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeItemCustomOutfit::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read			(tNetPacket);
	tNetPacket.r_float_q8			(m_fCondition,0.0f,1.0f);
}

void CSE_ALifeItemCustomOutfit::UPDATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write			(tNetPacket);
	tNetPacket.w_float_q8			(m_fCondition,0.0f,1.0f);
}

void CSE_ALifeItemCustomOutfit::FillProps			(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps			(pref,items);
}

BOOL CSE_ALifeItemCustomOutfit::Net_Relevant		()
{
	return							(true);
}
#pragma endregion


#pragma region CSE_ALifeItemExoOutfit
CSE_ALifeItemExoOutfit::CSE_ALifeItemExoOutfit(LPCSTR caSection) : CSE_ALifeItemCustomOutfit(caSection)
{
	m_fCurrentBatteryCharge = 0;
	m_sCurrentBatterySection = nullptr;
}

CSE_ALifeItemExoOutfit::~CSE_ALifeItemExoOutfit()
{
}


void CSE_ALifeItemExoOutfit::STATE_Read(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read(tNetPacket, size);
	if (base()->m_wVersion>123)
	{
		tNetPacket.r_stringZ(m_sCurrentBatterySection);
		m_fCurrentBatteryCharge = tNetPacket.r_float_q8(0.0f, 1.0f);
	}
}

void CSE_ALifeItemExoOutfit::STATE_Write(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write(tNetPacket);
	tNetPacket.w_stringZ(m_sCurrentBatterySection);
	tNetPacket.w_float_q8(m_fCurrentBatteryCharge, 0.0f, 1.0f);
}

void CSE_ALifeItemExoOutfit::FillProps(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps(pref, items);
}

void CSE_ALifeItemExoOutfit::UPDATE_Read(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read(tNetPacket);
#pragma todo("maybe remove second part from NG")
	if (base()->m_wVersion > 123 || (base()->m_wVersion < 124 && !tNetPacket.r_eof()))
	{
//		tNetPacket.r_stringZ(m_sCurrentBatterySection);
		m_fCurrentBatteryCharge = tNetPacket.r_float_q8(0.0f, 1.0f);
	}
}

void CSE_ALifeItemExoOutfit::UPDATE_Write(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write(tNetPacket);
	//tNetPacket.w_stringZ(m_sCurrentBatterySection.size()>0 ? m_sCurrentBatterySection.c_str() : "");
	tNetPacket.w_float_q8(m_fCurrentBatteryCharge, 0.0f, 1.0f);
}
#pragma endregion