#include "stdafx.h"

#include "WeaponKnife.h"
#include "WeaponHUD.h"
#include "Entity.h"
#include "Actor.h"
#include "level.h"
#include "xr_level_controller.h"
#include "game_cl_base.h"
#include "../skeletonanimated.h"
#include "gamemtllib.h"
#include "level_bullet_manager.h"
#include "ai_sounds.h"
#include "game_cl_single.h"
#include "InventoryOwner.h"
#include "Inventory.h"
#include "game_news.h"
#include "ActorCondition.h"

#define KNIFE_MATERIAL_NAME "objects\\knife"

CWeaponKnife::CWeaponKnife() : CWeapon("KNIFE") 
{
	m_attackStart			= false;
	SetState				( eHidden );
	SetNextState			( eHidden );
	knife_material_idx		= (u16)-1;
	m_fCriticalCondition = 0;
}
CWeaponKnife::~CWeaponKnife()
{
	HUD_SOUND::DestroySound(sndShow);
	HUD_SOUND::DestroySound(sndHide);
	HUD_SOUND::DestroySound(m_sndShot);
}

void CWeaponKnife::StopHUDSounds()
{
	HUD_SOUND::StopSound(sndShow);
	HUD_SOUND::StopSound(sndHide);
	HUD_SOUND::StopSound(m_sndShot);
	inherited::StopHUDSounds();
}


void CWeaponKnife::Load	(LPCSTR section)
{
	// verify class
	inherited::Load		(section);

	fWallmarkSize = pSettings->r_float(section,"wm_size");

	// HUD :: Anims
	R_ASSERT			(m_pHUD);
	animGet				(mhud_idle,		pSettings->r_string(*hud_sect,"anim_idle"));
	animGet				(mhud_hide,		pSettings->r_string(*hud_sect,"anim_hide"));
	animGet				(mhud_show,		pSettings->r_string(*hud_sect,"anim_draw"));
	animGet				(mhud_attack,	pSettings->r_string(*hud_sect,"anim_shoot1_start"));
	animGet				(mhud_attack2,	pSettings->r_string(*hud_sect,"anim_shoot2_start"));
	animGet				(mhud_attack_e,	pSettings->r_string(*hud_sect,"anim_shoot1_end"));
	animGet				(mhud_attack2_e,pSettings->r_string(*hud_sect,"anim_shoot2_end"));

	LPCSTR animName="anim_idle_sprint";
	if(pSettings->line_exist(*hud_sect,animName))
		animGet				(mhud_idle_sprint,pSettings->r_string(*hud_sect, animName),*hud_sect,animName);

	animName="anim_idle_moving";
	if(pSettings->line_exist(*hud_sect,animName))
		animGet				(mhud_idle_moving,pSettings->r_string(*hud_sect, animName),*hud_sect,animName);

	HUD_SOUND::LoadSound(section,"snd_shoot"	, m_sndShot		, ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING));
	HUD_SOUND::LoadSound(section,"snd_draw"		, sndShow		, ESoundTypes(SOUND_TYPE_ITEM_TAKING|SOUND_TYPE_WEAPON),false);
	HUD_SOUND::LoadSound(section,"snd_holster"	, sndHide		, ESoundTypes(SOUND_TYPE_ITEM_HIDING|SOUND_TYPE_WEAPON),false);

	knife_material_idx =  GMLib.GetMaterialIdx(KNIFE_MATERIAL_NAME);
	m_fCriticalCondition=READ_IF_EXISTS(pSettings, r_float, section, "critical_condition", 0.03f);
}

void CWeaponKnife::OnStateSwitch	(u32 S)
{
	inherited::OnStateSwitch(S);
	switch (S)
	{
	case eIdle:
		switch2_Idle	();
		break;
	case eShowing:
		switch2_Showing	();
		break;
	case eHiding:
		switch2_Hiding	();
		break;
	case eHidden:
		switch2_Hidden	();
		break;
	case eFire:
		{
			//-------------------------------------------
			m_eHitType		= m_eHitType_1;
			//fHitPower		= fHitPower_1;
			if (ParentIsActor())
			{
				if (GameID() == GAME_SINGLE)
				{
					fCurrentHit		= fvHitPower_1[g_SingleGameDifficulty];
				}
				else
				{
					fCurrentHit		= fvHitPower_1[egdMaster];
				}
			}
			else
			{
				fCurrentHit		= fvHitPower_1[egdMaster];
			}
			fHitImpulse		= fHitImpulse_1;
			//-------------------------------------------
			switch2_Attacking	(S);
		}break;
	case eFire2:
		{
			//-------------------------------------------
			m_eHitType		= m_eHitType_2;
			//fHitPower		= fHitPower_2;
			if (ParentIsActor())
			{
				if (GameID() == GAME_SINGLE)
				{
					fCurrentHit		= fvHitPower_2[g_SingleGameDifficulty];
				}
				else
				{
					fCurrentHit		= fvHitPower_2[egdMaster];
				}
			}
			else
			{
				fCurrentHit		= fvHitPower_2[egdMaster];
			}
			fHitImpulse		= fHitImpulse_2;
			//-------------------------------------------
			switch2_Attacking	(S);
		}break;
	}
}
	
void CWeaponKnife::KnifeStrike(const Fvector& pos, const Fvector& dir)
{
	CCartridge						cartridge; 
	cartridge.m_buckShot			= 1;				
	cartridge.m_impair				= 1;
	cartridge.m_kDisp				= 1;
	cartridge.m_kHit				= 1;
	cartridge.m_kImpulse			= 1;
	cartridge.m_kPierce				= 1;
	cartridge.m_flags.set(CCartridge::cfTracer, FALSE);
	cartridge.m_flags.set(CCartridge::cfRicochet, FALSE);
	cartridge.m_flags.set(CCartridge::cfExplosive, FALSE);
	cartridge.fWallmarkSize			= fWallmarkSize;
	cartridge.bullet_material_idx	= knife_material_idx;

	while(m_magazine.size() < 2)	
		m_magazine.push_back(cartridge);
	iAmmoElapsed					= m_magazine.size();
	bool SendHit					= SendHitAllowed(H_Parent());

	PlaySound						(m_sndShot,pos);
	//�������� �����, ������������ ������ ��� ����������� � ����� (������� � ������� �� ���������)
	collide::rq_result	l_rq;
	bool result = !!Level().ObjectSpace.RayPick(pos, dir, fireDistance, collide::rqtBoth, l_rq, H_Parent());
	if (result)
	{
		if (GetCondition()<=m_fCriticalCondition)//������ � ��������� ���������� ��
		{
			CInventoryOwner* owner = smart_cast<CInventoryOwner*>(H_Parent());
			if (owner)
			{
				owner->inventory().Ruck(this);
				return;//��� ������� ����
			}
		}
		//��� ����������� �� ���������
		fCurrentHit=fCurrentHit*GetCondition();
		//���� ���� ������� �� ���� ������... 
		ChangeCondition(-GetWeaponDeterioration());
	}
	if (ParentIsActor() && !l_rq.O)
	{
		float power = float(Actor()->conditions().GetPower()*0.02);
		float weight = Weight() / 200;
		Actor()->conditions().ChangePower(-(power+weight));
	}
	//���� �������... 
	Level().BulletManager().AddBullet(	pos, 
										dir, 
										m_fStartBulletSpeed, 
										fCurrentHit, 
										fHitImpulse, 
										H_Parent()->ID(), 
										ID(), 
										m_eHitType, 
										fireDistance, 
										cartridge, 
										SendHit);
}


void CWeaponKnife::OnAnimationEnd(u32 state)
{
	switch (state)
	{
	case eHiding:	SwitchState(eHidden);	break;
	case eFire: 
	case eFire2: 
		{
			if(m_attackStart) 
			{
				m_attackStart = false;
				MotionID mID;
				if (GetState() == eFire)
				{
					mID = random_anim(mhud_attack_e);
				}
				else
				{
					mID = random_anim(mhud_attack2_e);
				}
				m_pHUD->animPlay(mID, TRUE, this, GetState());
				Fvector	p1, d; 
				p1.set(get_LastFP()); 
				d.set(get_LastFD());

				if(H_Parent()) 
					smart_cast<CEntity*>(H_Parent())->g_fireParams(this, p1,d);
				else break;

				KnifeStrike(p1,d);
			} 
			else 
				SwitchState(eIdle);

		}break;
	case eShowing:
	case eIdle:	
		SwitchState(eIdle);		break;	
	}
}

void CWeaponKnife::state_Attacking	(float)
{
}

void CWeaponKnife::switch2_Attacking	(u32 state)
{
	if(m_bPending)	return;
	MotionID mID;
	if (state == eFire)
	{
		mID = random_anim(mhud_attack);
	}
	else //eFire2
	{
		mID = random_anim(mhud_attack2);
	}
	m_pHUD->animPlay(mID,FALSE, this, state);

	m_attackStart	= true;
	m_bPending		= true;
}

void CWeaponKnife::switch2_Idle	()
{
	VERIFY(GetState()==eIdle);
	if (ParentIsActor() && g_actor->get_state_wishful()&mcAnyMove)
	{
		onMovementChanged(mcAnyMove);
	}
	else
	{
		m_pHUD->animPlay(random_anim(mhud_idle), TRUE, this, GetState());
	}
	m_bPending = false;
}

void CWeaponKnife::switch2_Hiding	()
{
	FireEnd					();
	VERIFY(GetState()==eHiding);
	m_pHUD->animPlay		(random_anim(mhud_hide), TRUE, this, GetState());
//	m_bPending				= true;
}

void CWeaponKnife::switch2_Hidden()
{
	PlaySound	(sndHide,get_LastFP());
	signal_HideComplete		();
	m_bPending = false;
}

void CWeaponKnife::switch2_Showing	()
{
	PlaySound	(sndShow,get_LastFP());
	VERIFY(GetState()==eShowing);
	m_pHUD->animPlay		(random_anim(mhud_show), FALSE, this, GetState());

//	m_bPending				= true;
}


void CWeaponKnife::FireStart()
{	
	inherited::FireStart();
	SwitchState			(eFire);
}


void CWeaponKnife::Fire2Start () 
{
	inherited::Fire2Start();
	SwitchState(eFire2);
	/*if (ParentIsActor())
		g_actor->set_state_wishful(g_actor->get_state_wishful() & (~mcSprint) );*/
}


bool CWeaponKnife::Action(s32 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;
	switch(cmd) 
	{
		case kWPN_ZOOM : 
			if(flags&CMD_START) Fire2Start();
			else Fire2End();
			return true;
	}
	return false;
}

void CWeaponKnife::LoadFireParams(LPCSTR section, LPCSTR prefix)
{
	inherited::LoadFireParams(section, prefix);

	string256			full_name;
	string32			buffer;
	shared_str			s_sHitPower_2;
	//fHitPower_1		= fHitPower;
	fvHitPower_1		= fvHitPower;
	fHitImpulse_1		= fHitImpulse;
	m_eHitType_1		= ALife::g_tfString2HitType(pSettings->r_string(section, "hit_type"));

	//fHitPower_2			= pSettings->r_float	(section,strconcat(full_name, prefix, "hit_power_2"));
	s_sHitPower_2		= pSettings->r_string_wb	(section,strconcat(sizeof(full_name),full_name, prefix, "hit_power_2"));
	fvHitPower_2[egdMaster]	= (float)atof(_GetItem(*s_sHitPower_2,0,buffer));//������ �������� - ��� ��� ��� ������ ���� ������

	fvHitPower_2[egdVeteran]	= fvHitPower_2[egdMaster];//���������� ��������� ��� ������ �������
	fvHitPower_2[egdStalker]	= fvHitPower_2[egdMaster];//���������
	fvHitPower_2[egdNovice]		= fvHitPower_2[egdMaster];//����� ��

	int num_game_diff_param=_GetItemCount(*s_sHitPower_2);//����� ����������� ���������� ��� �����
	if (num_game_diff_param>1)//���� ����� ������ �������� ����
	{
		fvHitPower_2[egdVeteran]	= (float)atof(_GetItem(*s_sHitPower_2,1,buffer));//�� ���������� ��� ��� ������ ��������
	}
	if (num_game_diff_param>2)//���� ����� ������ �������� ����
	{
		fvHitPower_2[egdStalker]	= (float)atof(_GetItem(*s_sHitPower_2,2,buffer));//�� ���������� ��� ��� ������ ��������
	}
	if (num_game_diff_param>3)//���� ����� �������� �������� ����
	{
		fvHitPower_2[egdNovice]	= (float)atof(_GetItem(*s_sHitPower_2,3,buffer));//�� ���������� ��� ��� ������ �������
	}

	fHitImpulse_2		= pSettings->r_float	(section,strconcat(sizeof(full_name),full_name, prefix, "hit_impulse_2"));
	m_eHitType_2		= ALife::g_tfString2HitType(pSettings->r_string(section, "hit_type_2"));
}

void CWeaponKnife::StartIdleAnim()
{
	m_pHUD->animDisplay(mhud_idle[Random.randI(mhud_idle.size())], TRUE);
}
void CWeaponKnife::GetBriefInfo(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count)
{
	str_name = "";//NameShort();
	str_count		= NameShort();
	icon_sect_name = "";// *cNameSect();
}

void CWeaponKnife::net_Export(NET_Packet& P)
{
	iAmmoElapsed=0;
	inherited::net_Export(P);
}

void CWeaponKnife::save(NET_Packet& output_packet)
{
	iAmmoElapsed=0;
	inherited::save	(output_packet);
}

#include "OPFuncs/utils.h"
void CWeaponKnife::onMovementChanged	(ACTOR_DEFS::EMoveCommand cmd)
{
	if (GetState()!=eIdle)
		return;
	if (g_actor->is_sprint()) 
	{
		MotionID mID;
		if (mhud_idle_sprint.size()>0)
		{
			mID=random_anim(mhud_idle_sprint);
		}
		else
		{
			mID=random_anim(mhud_idle);
		}
		m_pHUD->animPlay(mID, TRUE, this, eIdle);
	}
	else if (g_actor->AnyMove())
	{
		MotionID mID;
		if (mhud_idle_moving.size()>0)
		{
			mID=random_anim(mhud_idle_moving);
		}
		else
		{
			mID=random_anim(mhud_idle);
		}
		m_pHUD->animPlay(mID, TRUE, this,  eIdle);
	}
	else
	{
		SwitchState(GetState() );
	}
}
