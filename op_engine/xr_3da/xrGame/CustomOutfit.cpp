#include "stdafx.h"

#include "customoutfit.h"
#include "PhysicsShell.h"
#include "inventory_space.h"
#include "Inventory.h"
#include "Actor.h"
#include "game_cl_base.h"
#include "Level.h"
#include "BoneProtections.h"


CCustomOutfit::CCustomOutfit()
{
	m_slot = OUTFIT_SLOT;

	m_flags.set(FUsingCondition, TRUE);

	m_HitTypeProtection.resize(ALife::eHitTypeMax);
	for(int i=0; i<ALife::eHitTypeMax; i++)
		m_HitTypeProtection[i] = 1.0f;

	m_boneProtection = xr_new<SBoneProtections>();
	m_cNightVisionDevice=nullptr;
}

CCustomOutfit::~CCustomOutfit() 
{
	xr_delete(m_boneProtection);
	if (m_cNightVisionDevice)
		xr_delete(m_cNightVisionDevice);
}

void CCustomOutfit::net_Export(NET_Packet& P)
{
	inherited::net_Export	(P);
	P.w_float_q8			(m_fCondition,0.0f,1.0f);
}

void CCustomOutfit::net_Import(NET_Packet& P)
{
	inherited::net_Import	(P);
	P.r_float_q8			(m_fCondition,0.0f,1.0f);
}

void CCustomOutfit::UpdateCL()
{
	inherited::UpdateCL();
	if (H_Parent() && H_Parent()->ID() == Actor()->ID() && m_cNightVisionDevice)
		m_cNightVisionDevice->UpdateSwitchNightVision();
}

void CCustomOutfit::Load(LPCSTR section) 
{
	inherited::Load(section);

	m_HitTypeProtection[ALife::eHitTypeBurn]		= pSettings->r_float(section,"burn_protection");
	m_HitTypeProtection[ALife::eHitTypeStrike]		= pSettings->r_float(section,"strike_protection");
	m_HitTypeProtection[ALife::eHitTypeShock]		= pSettings->r_float(section,"shock_protection");
	m_HitTypeProtection[ALife::eHitTypeWound]		= pSettings->r_float(section,"wound_protection");
	m_HitTypeProtection[ALife::eHitTypeRadiation]	= pSettings->r_float(section,"radiation_protection");
	m_HitTypeProtection[ALife::eHitTypeTelepatic]	= pSettings->r_float(section,"telepatic_protection");
	m_HitTypeProtection[ALife::eHitTypeChemicalBurn]= pSettings->r_float(section,"chemical_burn_protection");
	m_HitTypeProtection[ALife::eHitTypeExplosion]	= pSettings->r_float(section,"explosion_protection");
	m_HitTypeProtection[ALife::eHitTypeFireWound]	= pSettings->r_float(section,"fire_wound_protection");
	m_HitTypeProtection[ALife::eHitTypePhysicStrike]= READ_IF_EXISTS(pSettings, r_float, section, "physic_strike_protection", 0.0f);
	
	m_fHealthRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float,    section, "health_restore_speed",    0.0f );
	m_fRadiationRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float,    section, "radiation_restore_speed", 0.0f );
	m_fSatietyRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float,    section, "satiety_restore_speed",   0.0f );
	m_fPowerRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float,    section, "power_restore_speed",     0.0f );
	m_fBleedingRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float,    section, "bleeding_restore_speed",  0.0f ); 

	if (pSettings->line_exist(section, "actor_visual"))
		m_ActorVisual = pSettings->r_string(section, "actor_visual");
	else
		m_ActorVisual = nullptr;

	m_ef_equipment_type		= pSettings->r_u32(section,"ef_equipment_type");
	if (pSettings->line_exist(section, "power_loss"))
		m_fPowerLoss = pSettings->r_float(section, "power_loss");
	else
		m_fPowerLoss = 1.0f;	

	m_additional_weight				= pSettings->r_float(section,"additional_inventory_weight");
	m_additional_weight2			= pSettings->r_float(section,"additional_inventory_weight2");

	if (pSettings->line_exist(section, "night_vision_device"))
	{
		LPCSTR deviceSection=pSettings->r_string(section, "night_vision_device");
		if (xr_strlen(deviceSection)>0 && pSettings->section_exist(deviceSection))
		{
			m_cNightVisionDevice = xr_new<CNightVisionDevice>();
			m_cNightVisionDevice->Load(deviceSection);
		}
		else
			Msg("! ERROR invalid night_vision_device [%s] for outfit [%s]",deviceSection,section);
	}

	m_full_icon_name								= pSettings->r_string(section,"full_icon_name");
}

void CCustomOutfit::Hit(SHit *pHDS)
{
	float power=pHDS->power;
	if (power <= 0) return;
	ALife::EHitType hit_type = pHDS->hit_type;
	power*=m_HitTypeK[hit_type];
	ChangeCondition(-power);
}

float CCustomOutfit::GetDefHitTypeProtection(ALife::EHitType hit_type)
{
	float result=1.0f - m_HitTypeProtection[hit_type]*GetCondition();
	return result > 0.001f ? result : 0.001f;
}

float CCustomOutfit::GetDefHitTypeProtectionOriginal(ALife::EHitType hit_type)
{
	float result=1.0f - m_HitTypeProtection[hit_type];
	return result > 0.001f ? result : 0.001f;
}

float CCustomOutfit::GetHitTypeProtection(SHit *pHDS)
{
	float fBase = m_HitTypeProtection[pHDS->hit_type]*GetCondition();
	float bone = m_boneProtection->getBoneProtection(pHDS->boneID);
	float result=1.0f - fBase*bone;
	return result > 0 ? result : 0;
}

float	CCustomOutfit::HitThruArmour(SHit *pHDS)
{
	float hit_power = pHDS->power;
	float BoneArmour = m_boneProtection->getBoneArmour(pHDS->boneID)*GetCondition()*(1-pHDS->ap);	
	float NewHitPower = hit_power - BoneArmour;
	if (NewHitPower < hit_power*m_boneProtection->m_fHitFrac) return hit_power*m_boneProtection->m_fHitFrac;
	return NewHitPower > 0 ? NewHitPower :0;
};

BOOL	CCustomOutfit::BonePassBullet					(int boneID)
{
	return m_boneProtection->getBonePassBullet(s16(boneID));
};

#include "torch.h"
void	CCustomOutfit::OnMoveToSlot		()
{
	if (m_pCurrentInventory)
	{
		CActor* pActor = smart_cast<CActor*> (m_pCurrentInventory->GetOwner());
		if (pActor)
		{
			if (m_ActorVisual.size())
			{
				shared_str NewVisual = NULL;
				char* TeamSection = Game().getTeamSection(pActor->g_Team());
				if (TeamSection)
				{
					if (pSettings->line_exist(TeamSection, *cNameSect()))
					{
						NewVisual = pSettings->r_string(TeamSection, *cNameSect());
						string256 SkinName;
						strcpy_s(SkinName, pSettings->r_string("mp_skins_path", "skin_path"));
						strcat_s(SkinName, *NewVisual);
						strcat_s(SkinName, ".ogf");
						NewVisual._set(SkinName);
					}
				}
				
				if (!NewVisual.size())
					NewVisual = m_ActorVisual;

				pActor->ChangeVisual(NewVisual);
			}
			if(pSettings->line_exist(cNameSect(),"bones_koeff_protection")){
				m_boneProtection->reload( pSettings->r_string(cNameSect(),"bones_koeff_protection"), smart_cast<CKinematics*>(pActor->Visual()) );

			};
		}
	}
};

void	CCustomOutfit::OnMoveToRuck		()
{
	if (m_pCurrentInventory)
	{
		CActor* pActor = smart_cast<CActor*> (m_pCurrentInventory->GetOwner());
		if (pActor )
		{
			if (m_cNightVisionDevice)
				m_cNightVisionDevice->SwitchNightVision(false);
			CInventoryItem* currentOutfitItem=pActor->GetCurrentOutfit();
			if (currentOutfitItem)
				return;
			if (m_ActorVisual.size())
			{
				shared_str DefVisual = pActor->GetDefaultVisualOutfit();
				if (DefVisual.size())
					pActor->ChangeVisual(DefVisual);
			}
		}
	}
};

u32	CCustomOutfit::ef_equipment_type	() const
{
	return		(m_ef_equipment_type);
}

float CCustomOutfit::GetPowerLoss() 
{
	if (m_fPowerLoss<1 && GetCondition() <= 0)
	{
		return 1.0f;			
	};
	return m_fPowerLoss;
};
