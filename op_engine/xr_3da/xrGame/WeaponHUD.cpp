// WeaponHUD.cpp:	HUD ��� ������ � ������ ���������, �������
//					����� ������� � ����� ���������, ����� ������������
//					��� ������������� �������� � ����� �� 3-�� ����
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "WeaponHUD.h"
#include "Weapon.h"
#include "../Motion.h"
#include "../skeletonanimated.h"
#include "level.h"
#include "MathUtils.h"
weapon_hud_container* g_pWeaponHUDContainer=0;

BOOL weapon_hud_value::load(const shared_str& section, CHudItem* owner)
{	
	// Geometry and transform
	Fvector						pos,ypr;
	pos							= pSettings->r_fvector3(section,"position");
	ypr							= pSettings->r_fvector3(section,"orientation");
	ypr.mul						(PI/180.f);

	m_offset.setHPB				(ypr.x,ypr.y,ypr.z);
	m_offset.translate_over		(pos);

	// Visual
	LPCSTR visual_name			= pSettings->r_string(section, "visual");
	m_animations				= smart_cast<CKinematicsAnimated*>(::Render->model_Create(visual_name));

	// fire bone	
	if(smart_cast<CWeapon*>(owner)){
		LPCSTR fire_bone		= pSettings->r_string					(section,"fire_bone");
		if (!fire_bone || xr_strlen(fire_bone)==0)
		{
			Msg("! ERROR invalid fire_bone value for [%s]", section.c_str());
			FATAL("ENGINE CRASH: See details in log");
		}
		m_fire_bone				= m_animations->LL_BoneID	(fire_bone);
		if (m_fire_bone==BI_NONE)
		{
			Msg("! ERROR fire_bone [%s] for [%s] does not have animations in visual [%s]", fire_bone,section, visual_name);
			FATAL("ENGINE CRASH: See details in log");
		}
		if (m_fire_bone>=m_animations->LL_BoneCount())	
			Debug.fatal	(DEBUG_INFO,"There is no '%s' bone for weapon '%s'.",fire_bone, *section);
		m_fp_offset				= pSettings->r_fvector3					(section,"fire_point");
		if(pSettings->line_exist(section,"fire_point2")) 
			m_fp2_offset		= pSettings->r_fvector3					(section,"fire_point2");
		else 
			m_fp2_offset		= m_fp_offset;
		if(pSettings->line_exist(owner->object().cNameSect(), "shell_particles")) 
			m_sp_offset			= pSettings->r_fvector3	(section,"shell_point");
		else 
			m_sp_offset.set		(0,0,0);
	}else{
		m_fire_bone				= -1;
		m_fp_offset.set			(0,0,0);
		m_fp2_offset.set		(0,0,0);
		m_sp_offset.set			(0,0,0);
	}
	return TRUE;
}

weapon_hud_value::~weapon_hud_value()
{
	::Render->model_Delete		((IRender_Visual*&)m_animations);
}

u32 shared_weapon_hud::motion_length(MotionID M)
{
	CKinematicsAnimated	*skeleton_animated = p_->m_animations;
	VERIFY				(skeleton_animated);
	CMotionDef			*motion_def = skeleton_animated->LL_GetMotionDef(M);
	VERIFY				(motion_def);

	if (motion_def->flags & esmStopAtEnd) {
		CMotion*			motion		= skeleton_animated->LL_GetRootMotion(M);
		return				iFloor(0.5f + 1000.f*motion->GetLength()/ motion_def->Dequantize(motion_def->speed));
	}
	return				0;
}

MotionID shared_weapon_hud::motion_id(LPCSTR name)
{
	return p_->m_animations->ID_Cycle_Safe(name);
}

CWeaponHUD::CWeaponHUD			(CHudItem* pHudItem)
{
	m_bVisible					= false;
	m_pParentWeapon				= pHudItem;
	m_bHidden					= true;
	m_bStopAtEndAnimIsRunning	= false;
	m_pCallbackItem				= nullptr;
	m_Transform.identity		();
	m_pBobbing = xr_new<CWeaponBobbing>();
	m_bEnableBobbing = false;
}

CWeaponHUD::~CWeaponHUD()
{
	xr_delete(m_pBobbing);
}

void CWeaponHUD::SetHudBobbong(bool value)
{
	m_bEnableBobbing = value;
//	Msg("set bobbing to %s for %s",value ?"true":"false", m_pParentWeapon->object().cNameSect().c_str());
}

bool CWeaponHUD::GetHudBobbing()
{
	return m_bEnableBobbing;
}

void CWeaponHUD::Load(LPCSTR section)
{
	m_shared_data.create		(section,m_pParentWeapon);
}

void  CWeaponHUD::Init()
{
	m_bStopAtEndAnimIsRunning	= false;
	m_pCallbackItem				= nullptr;
}


void  CWeaponHUD::net_DestroyHud()
{
	m_bStopAtEndAnimIsRunning	= false;
	m_pCallbackItem				= nullptr;
	Visible						(false);
}

void CWeaponHUD::UpdatePosition(const Fmatrix& trans)
{
	Fmatrix xform = trans;
	if (m_bEnableBobbing)
		m_pBobbing->Update(xform);
	m_Transform.mul(xform, m_shared_data.get_value()->m_offset);
	VERIFY						(!fis_zero(DET(m_Transform)));
}

MotionID CWeaponHUD::animGet(LPCSTR name)
{
	return m_shared_data.motion_id	(name);
}

void CWeaponHUD::animDisplay(MotionID M, BOOL bMixIn)
{
	if(m_bVisible){
		CKinematicsAnimated* PKinematicsAnimated		= smart_cast<CKinematicsAnimated*>(Visual());
		VERIFY											(PKinematicsAnimated);
		PKinematicsAnimated->PlayCycle					(M,bMixIn);
		PKinematicsAnimated->CalculateBones_Invalidate	();
	}
}
void CWeaponHUD::animPlay			(MotionID M,	BOOL bMixIn, CHudItem* W, u32 state)
{
//.	if(m_bStopAtEndAnimIsRunning)	
//.		StopCurrentAnim				();


	m_startedAnimState				= state;
	Show							();
	animDisplay						(M, bMixIn);
	u32 anim_time					= m_shared_data.motion_length(M);
	if (anim_time>0){
		m_bStopAtEndAnimIsRunning	= true;
		m_pCallbackItem				= W;
		m_dwAnimEndTime				= Device.dwTimeGlobal + anim_time;
	}else{
		m_pCallbackItem				= NULL;
	}
}

void CWeaponHUD::Update				()
{
	if(m_bStopAtEndAnimIsRunning && Device.dwTimeGlobal > m_dwAnimEndTime)
		StopCurrentAnim				();
	if(m_bVisible)
		smart_cast<CKinematicsAnimated*>(Visual())->UpdateTracks		();
}

void CWeaponHUD::StopCurrentAnim()
{
	m_dwAnimEndTime						= 0;
	m_bStopAtEndAnimIsRunning			= false;
	if(m_pCallbackItem)
		m_pCallbackItem->OnAnimationEnd	(m_startedAnimState);
}

void CWeaponHUD::StopCurrentAnimWithoutCallback		()
{
	m_dwAnimEndTime = 0;
	m_bStopAtEndAnimIsRunning = false;

	m_pCallbackItem = NULL;
}

void CWeaponHUD::CreateSharedContainer	()
{
	VERIFY(0==g_pWeaponHUDContainer);
	g_pWeaponHUDContainer	= xr_new<weapon_hud_container>();
}
void CWeaponHUD::DestroySharedContainer	()
{
	xr_delete				(g_pWeaponHUDContainer);
}
void CWeaponHUD::CleanSharedContainer	()
{
	VERIFY(g_pWeaponHUDContainer);
	// true - do not leave items in cache
	g_pWeaponHUDContainer->clean(true);
}

MotionID random_anim(MotionSVec& v)
{
	return v[Random.randI(v.size())];
}
