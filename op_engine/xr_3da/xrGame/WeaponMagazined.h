#ifndef __XR_WEAPON_MAG_H__
#define __XR_WEAPON_MAG_H__
#pragma once

#include "weapon.h"
#include "hudsound.h"
#include "ai_sounds.h"

class ENGINE_API CMotionDef;

//������ ������� ��������� �������������
//����������� ��������, ������, ���� ��������� �������
#define WEAPON_ININITE_QUEUE -1
//#define SHOW_ANIM_WEAPON_PLAYS

class CWeaponMagazined: public CWeapon
{
private:
	typedef CWeapon inherited;

protected:
	// Media :: sounds
	HUD_SOUND		sndShow;
	HUD_SOUND		sndHide;
	HUD_SOUND		sndShot;
	HUD_SOUND		sndEmptyClick;
	HUD_SOUND		sndReload;
	HUD_SOUND		sndChangeFireMode;

	HUD_SOUND		sndZoomIn;
	HUD_SOUND		sndZoomOut;

	HUD_SOUND		sndZoomInc;
	HUD_SOUND		sndZoomDec;

	//���� �������� ��������
	HUD_SOUND*		m_pSndShotCurrent;
	HUD_SOUND		sndPreviewAmmo;
	virtual void	StopHUDSounds		();

	//�������������� ���������� � ���������
	LPCSTR			m_sSilencerFlameParticles;
	LPCSTR			m_sSilencerSmokeParticles;
	HUD_SOUND		sndSilencerShot;

	ESoundTypes		m_eSoundShow;
	ESoundTypes		m_eSoundHide;
	ESoundTypes		m_eSoundShot;
	ESoundTypes		m_eSoundEmptyClick;
	ESoundTypes		m_eSoundChangeFireMode;
	ESoundTypes		m_eSoundPreviewAmmo;
	ESoundTypes		m_eSoundReload;
	ESoundTypes		m_eSoundZoomIn;
	ESoundTypes		m_eSoundZoomOut;
	ESoundTypes		m_eSoundZoomInc;
	ESoundTypes		m_eSoundZoomDec;
	struct SWMmotions{
		MotionSVec		mhud_idle;
		MotionSVec		mhud_idle_aim;
		MotionSVec		mhud_reload;	//
		MotionSVec		mhud_hide;		//
		MotionSVec		mhud_show;		//
		MotionSVec		mhud_shots;		//
		MotionSVec		mhud_idle_sprint;
		MotionSVec		anim_idle_moving;
	};
	SWMmotions			mhud;	
	
	// General
	//���� ������� ��������� UpdateSounds
	u32				dwUpdateSounds_Frame;
	bool			m_bRequredDemandCheck;
	ref_sound* m_pSoundAddonProc;
	bool m_bEmptyScopeTexture;
protected:
	bool ZoomInc	() override;
	bool ZoomDec	() override;


	virtual void	OnMagazineEmpty	();

	virtual void	switch2_Idle	();
	virtual void	switch2_Fire	();
	virtual void	switch2_Fire2	(){}
	virtual void	switch2_Empty	();
	virtual void	switch2_Reload	();
	virtual void	switch2_Hiding	();
	virtual void	switch2_Hidden	();
	virtual void	switch2_Showing	();
	
	virtual void	OnShot			();	
	virtual void	OnEmptyClick	();
	virtual void	OnAnimationEnd	(u32 state);
	virtual void	OnStateSwitch	(u32 S);

	virtual void	UpdateSounds	();

	bool			TryReload		();
	
protected:
	virtual void	ReloadMagazine	();
			void	ApplySilencerKoeffs	();

	virtual void	state_Fire		(float dt);
	virtual void	state_MagEmpty	(float dt);
	virtual void	state_Misfire	(float dt);
	void			FireWeaponEvent	(GameObject::ECallbackType actorCallbackType,GameObject::ECallbackType npcCallbackType);
	void			LoadFireModes	(LPCSTR section);
public:
					CWeaponMagazined	(LPCSTR name="AK74",ESoundTypes eSoundType=SOUND_TYPE_WEAPON_SUBMACHINEGUN);
	virtual			~CWeaponMagazined	();

	virtual void	Load			(LPCSTR section);
	
	
	bool UseScopeTexture() override;

	virtual CWeaponMagazined*cast_weapon_magazined	()		 {return this;}

	virtual void	SetDefaults		();
	virtual void	FireStart		();
	virtual void	FireEnd			();
	virtual void	Reload			();
	

	virtual	void	UpdateCL		();
	virtual void	net_Destroy		();
	virtual BOOL			net_Spawn			(CSE_Abstract* DC);
	virtual void			net_Export			(NET_Packet& P);
	virtual void			net_Import			(NET_Packet& P);

	virtual void	OnH_A_Chield		();

	bool AttachScopeSection(const char* item_section_name,bool singleAttach=true);
	
	virtual bool	Attach(PIItem pIItem, bool b_send_event);
	virtual bool	Detach(const char* item_section_name, bool b_spawn_item, float item_condition = 1.0f);
	virtual bool	CanAttach(PIItem pIItem);
	virtual bool	CanDetach(const char* item_section_name);
	bool			CanLoadAmmo			(CWeaponAmmo *pAmmo,bool checkFullMagazine=false) override;
	void			LoadAmmo			(CWeaponAmmo *pAmmo) override;

	virtual void	InitAddons();

	virtual bool	Action			(s32 cmd, u32 flags);
	virtual void	onMovementChanged	(ACTOR_DEFS::EMoveCommand cmd);
	bool			IsAmmoAvailable	();
	virtual void	UnloadMagazine	(bool spawn_ammo = true);

	void	GetBriefInfo(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count) override;

	u32 getCurrentAmmoType();
	void			PlayEmptySnd()	{PlaySound	(sndEmptyClick,get_LastFP());};
	void			PlayHideSnd()	{PlaySound	(sndHide,get_LastFP());};
	//////////////////////////////////////////////
	// ��� �������� ��������� ��� ����������
	//////////////////////////////////////////////
public:
	virtual bool	SwitchMode				();
	virtual bool	SingleShotMode			()			{return 1 == m_iQueueSize;}
	virtual void	SetQueueSize			(int size);
	IC		int		GetQueueSize			() const	{return m_iQueueSize;};
	virtual bool	StopedAfterQueueFired	()			{return m_bStopedAfterQueueFired; }
	virtual void	StopedAfterQueueFired	(bool value){m_bStopedAfterQueueFired = value; }
	
	bool m_bforceReloadAfterIdle;
	bool GetForceReloadFlag() const {return m_bforceReloadAfterIdle;}
	void SetForceReloadFlag(bool flag) {m_bforceReloadAfterIdle=flag;}
protected:
	//������������ ������ �������, ������� ����� ����������
	int				m_iQueueSize;
	//���������� ������� ����������� ��������
	int				m_iShotNum;
	//  [7/20/2005]
	//����� ������ �������, ��� ����������� ��������, ���������� ������ (������� ��-�� �������)
	int				m_iShootEffectorStart;
	Fvector			m_vStartPos, m_vStartDir;
	//  [7/20/2005]
	//���� ����, ��� �� ������������ ����� ���� ��� ����������
	//����� ������� ��������, ������� ���� ������ � m_iQueueSize
	bool			m_bStopedAfterQueueFired;
	//���� ����, ��� ���� �� ���� ������� �� ������ �������
	//(���� ���� ����� ������ ������ �� ����� � ��������� FireEnd)
	bool			m_bFireSingleShot;
	//������ ��������
	bool			m_bHasDifferentFireModes;
	xr_vector<int>	m_aFireModes;
	int				m_iCurFireMode;
	string16		m_sCurFireMode;
	int				m_iPrefferedFireMode;

	//���������� ��������� �������������
	//������ ������ ����� ��������
	bool m_bLockType;

	float m_fSilencerCondition;
	float m_fSilencerConditionDecreasePerShot;
	float m_fSilencerConditionCriticalValue;
	shared_str m_sSilncerCriticalMessage;
	bool m_bSilencerBreak;
	//////////////////////////////////////////////
	// ����� �����������
	//////////////////////////////////////////////
public:
	float GetSilencerCondition() const { return m_fSilencerCondition; }
	void SetSilencerCondition(float condition)
	{
		clamp(condition, 0.0f, 1.0f);
		m_fSilencerCondition= condition;
	}
	virtual void	OnZoomIn			();
	virtual void	OnZoomOut			();
	virtual	void	OnNextFireMode		();
	virtual	void	OnPrevFireMode		();
	virtual bool	HasFireModes		() { return m_bHasDifferentFireModes; };
	virtual	int		GetCurrentFireMode	() { return m_aFireModes[m_iCurFireMode]; };	
	virtual LPCSTR	GetCurrentFireModeStr	() {return m_sCurFireMode;};

	virtual void	save				(NET_Packet &output_packet);
	virtual void	load				(IReader &input_packet);

protected:
	virtual bool	AllowFireWhileWorking() {return false;}

	//����������� ������� ��� ������������ �������� HUD

	virtual void	StartIdleAnim		();
	virtual	int		ShotsFired			() { return m_iShotNum; }
	virtual float	GetWeaponDeterioration	();
public:
	virtual void	PlayAnimShow();
	virtual void	PlayAnimHide();
	virtual void	PlayAnimReload();
	virtual void	PlayAnimIdle();
	virtual void	PlayAnimShoot();
	virtual void	PlayReloadSound		();
	virtual bool			TryPlayAnimIdle	();
	
private:
	void PlayPreviewAmmoSound() override;
};

bool is_fake_scope(LPCSTR section);

#endif //__XR_WEAPON_MAG_H__
