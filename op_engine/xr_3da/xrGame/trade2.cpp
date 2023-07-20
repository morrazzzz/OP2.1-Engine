#include "pch_script.h"
#include "trade.h"
#include "actor.h"
#include "ai/stalker/ai_stalker.h"
#include "ai/trader/ai_trader.h"
#include "artifact.h"
#include "inventory.h"
#include "xrmessages.h"
#include "character_info.h"
#include "relation_registry.h"
#include "level.h"
#include "script_callback_ex.h"
#include "script_game_object.h"
#include "game_object_space.h"
#include "trade_parameters.h"
#include "WeaponAmmo.h"
#include "WeaponMagazinedWGrenade.h"
#include "trade_parameters.h"
#include "exooutfit.h"


bool CTrade::CanTrade()
{
	CEntity *pEntity;

	m_nearest.clear_not_free		();
	Level().ObjectSpace.GetNearest	(m_nearest,pThis.base->Position(),2.f, NULL);
	if (!m_nearest.empty()) 
	{
		for (u32 i=0, n = m_nearest.size(); i<n; ++i) 
		{
			// ����� �� ������ ���������
			pEntity = smart_cast<CEntity *>(m_nearest[i]);
			if (pEntity && !pEntity->g_Alive()) return false;
			if (SetPartner(pEntity)) break;
		}
	} 
	
	if (!pPartner.base) return false;

	// ������ �����
	float dist = pPartner.base->Position().distance_to(pThis.base->Position());
	if (dist < 0.5f || dist > 4.5f)  
	{
		RemovePartner();
		return false;
	}

	// ������ ������� �� ����
	float yaw, pitch;
	float yaw2, pitch2;

	pThis.base->Direction().getHP(yaw,pitch);
	pPartner.base->Direction().getHP(yaw2,pitch2);
	yaw = angle_normalize(yaw);
	yaw2 = angle_normalize(yaw2);

	float Res = rad2deg(_abs(yaw - yaw2) < PI ? _abs(yaw - yaw2) : 
								 PI_MUL_2 - _abs(yaw - yaw2));
	if (Res < 165.f || Res > 195.f) 
	{
		RemovePartner();
		return false;
	}

	return true;
}

void CTrade::TransferItem(CInventoryItem* pItem, bool bBuying)
{
	// ����� ������ �������� ������� �����������
	// ����� ���� �� ������� �������, ��� ������ �� ����
	u32 dwTransferMoney					= GetItemPrice(pItem, bBuying);

	if(bBuying)
	{
		pPartner.inv_owner->on_before_sell	(pItem);
		pThis.inv_owner->on_before_buy		(pItem);
	}else
	{
		pThis.inv_owner->on_before_sell		(pItem);
		pPartner.inv_owner->on_before_buy	(pItem);
	}

	CGameObject* O1			= smart_cast<CGameObject *>(pPartner.inv_owner);
	CGameObject* O2			= smart_cast<CGameObject *>(pThis.inv_owner);
	
	if(!bBuying)
		swap(O1,O2);

	NET_Packet				P;
	O1->u_EventGen			(P,GE_TRADE_SELL,O1->ID());
	P.w_u16					(pItem->object().ID());
	O1->u_EventSend			(P);

	if(bBuying)
		pPartner.inv_owner->set_money( pPartner.inv_owner->get_money() + dwTransferMoney, false );
	else
		pThis.inv_owner->set_money( pThis.inv_owner->get_money() + dwTransferMoney, false );

	// ����� � ��������
	O2->u_EventGen			(P,GE_TRADE_BUY,O2->ID());
	P.w_u16					(pItem->object().ID());
	O2->u_EventSend			(P);

	if(bBuying)
		pThis.inv_owner->set_money( pThis.inv_owner->get_money() - dwTransferMoney, false );
	else
		pPartner.inv_owner->set_money( pPartner.inv_owner->get_money() - dwTransferMoney, false );


	CAI_Trader* pTrader		= NULL;

	if (pThis.type == TT_TRADER && bBuying) 
	{
		CArtefact* pArtefact	= smart_cast<CArtefact*>(pItem);
		if(pArtefact)
		{
			pTrader							= smart_cast<CAI_Trader*>(pThis.base);
			m_bNeedToUpdateArtefactTasks |= pTrader->BuyArtefact		(pArtefact);
		}
	}

	if((pPartner.type==TT_ACTOR) || (pThis.type==TT_ACTOR))
	{
		bool bDir = (pThis.type!=TT_ACTOR) && bBuying;
		Actor()->callback	(GameObject::eTradeSellBuyItem)(pItem->object().lua_game_object(), bDir, dwTransferMoney);
	}
}


CInventory& CTrade::GetTradeInv(SInventoryOwner owner) const
{
	R_ASSERT(TT_NONE != owner.type);

	//return ((TT_TRADER == owner.type) ? (*owner.inv_owner->m_trade_storage) : (owner.inv_owner->inventory()));
	return owner.inv_owner->inventory();
}

CTrade*	CTrade::GetPartnerTrade() const
{
	return pPartner.inv_owner->GetTrade();
}
CInventory*	CTrade::GetPartnerInventory() const
{
	return &GetTradeInv(pPartner);
}

CInventoryOwner* CTrade::GetPartner() const
{
	return pPartner.inv_owner;
}

float GetAmmoCostInWeapon(shared_str ammoSection)
{
	auto boxCost=pSettings->r_float(ammoSection.c_str(),"cost");
	auto boxSize=pSettings->r_float(ammoSection.c_str(),"box_size");
	return boxCost/boxSize;
}

float calcTradeFactor(const CTradeFactors* tradeFactors,float relationFactor)
{
	if (!tradeFactors)
		return 0;
	float					action_factor;
	if (tradeFactors->friend_factor() <= tradeFactors->enemy_factor())
		action_factor		= 
			tradeFactors->friend_factor() +
			(
				tradeFactors->enemy_factor() -
				tradeFactors->friend_factor()
			)*
			(1.f - relationFactor);
	else
		action_factor		= 
			tradeFactors->enemy_factor() +
			(
				tradeFactors->friend_factor() -
				tradeFactors->enemy_factor()
			)*
			relationFactor;

	clamp					(
		action_factor,
		_min(tradeFactors->enemy_factor(),tradeFactors->friend_factor()),
		_max(tradeFactors->enemy_factor(),tradeFactors->friend_factor())
	);
	return action_factor;
}

const CTradeFactors* CTrade::returnTradeFactors(bool buying,shared_str itemSection) const
{
	const CTradeFactors *p_trade_factors;
	if (buying)
	{
		bool enabled=pThis.inv_owner->trade_parameters().enabled(CTradeParameters::action_buy(nullptr),itemSection);
		if (!enabled) 
			return nullptr;
		p_trade_factors		= &pThis.inv_owner->trade_parameters().factors(CTradeParameters::action_buy(nullptr),itemSection);
	}else
	{
		if( ! pThis.inv_owner->trade_parameters().enabled(CTradeParameters::action_sell(nullptr),itemSection) ) return nullptr;
		p_trade_factors		= &pThis.inv_owner->trade_parameters().factors(CTradeParameters::action_sell(nullptr),itemSection);
	}
	return p_trade_factors;
}

u32	CTrade::GetItemPrice(PIItem pItem, bool b_buying)
{
	CArtefact				*pArtefact = smart_cast<CArtefact*>(pItem);

	#pragma region computing relation factor
	float					relation_factor;
	CHARACTER_GOODWILL		attitude;
	if (gInvertTrade)
		attitude = RELATION_REGISTRY().GetAttitude(pThis.inv_owner, pPartner.inv_owner);
	else
		attitude = RELATION_REGISTRY().GetAttitude(pPartner.inv_owner, pThis.inv_owner);
	if (NO_GOODWILL == attitude)
		relation_factor		= 0.f;
	else
		relation_factor		= float(attitude + 1000.f)/2000.f;

	clamp					(relation_factor,0.f,1.f);

#pragma endregion

	// computing base_cost
	float base_cost;
	float glCost=0;
	float slCost=0;
	float scCost=0;
	float amiwCost=0;
	float griwCost=0;
	float exoBatteryCost = 0;
	if (pArtefact && (pThis.type == TT_ACTOR) && (pPartner.type == TT_TRADER)) {
		CAI_Trader			*pTrader = smart_cast<CAI_Trader*>(pPartner.inv_owner);
		VERIFY				(pTrader);
		base_cost			= static_cast<float>(pTrader->ArtefactPrice(pArtefact));
	}
	else
	{
		float itemCost=static_cast<float>(pItem->Cost());
		CWeaponAmmo *ammo= smart_cast<CWeaponAmmo*>(pItem);
		CWeaponMagazined* weapon=smart_cast<CWeaponMagazined*>(pItem);
		CExoOutfit* exo = smart_cast<CExoOutfit*>(pItem);
		base_cost=-1;
		if (ammo)
		{
			base_cost=itemCost/ammo->m_boxSize*ammo->m_boxCurr;
		}
		else if (weapon)
		{
			if (weapon->IsGrenadeLauncherAttached() && weapon->GrenadeLauncherAttachable())
			{
				shared_str glName=weapon->GetGrenadeLauncherName();
				glCost=pSettings->r_float(glName,"cost");
				glCost*=calcTradeFactor(returnTradeFactors(b_buying,glName),relation_factor);
			}
			if (weapon->IsScopeAttached() && weapon->ScopeAttachable())
			{
				shared_str scName=weapon->GetScopeName();
				scCost=pSettings->r_float(scName,"cost");
				scCost*=calcTradeFactor(returnTradeFactors(b_buying,scName),relation_factor);
			}
			if (weapon->IsSilencerAttached() && weapon->SilencerAttachable())
			{
				shared_str slName=weapon->GetSilencerName();
				slCost=pSettings->r_float(slName,"cost");
				slCost*= calcTradeFactor(returnTradeFactors(b_buying, slName), relation_factor);
			}
			int ammoInWeapon=weapon->GetAmmoElapsed();
			if (ammoInWeapon>0)
			{
				shared_str amiwName=weapon->m_magazine.back().m_ammoSect;
				amiwCost=GetAmmoCostInWeapon(amiwName)*ammoInWeapon;
				amiwCost*=calcTradeFactor(returnTradeFactors(b_buying,amiwName),relation_factor);
			}
			CWeaponMagazinedWGrenade* weaponG=smart_cast<CWeaponMagazinedWGrenade*>(pItem);
			if (weaponG)
				if (weaponG->m_magazine2.size()>0)
				{
					shared_str griwName=weaponG->m_magazine2.back().m_ammoSect;
					griwCost=GetAmmoCostInWeapon(griwName)*weaponG->m_magazine2.size();
					griwCost*=calcTradeFactor(returnTradeFactors(b_buying,griwName),relation_factor);
				}
		}
		else if (exo)
		{
			if (exo->isBatteryPresent())
			{
				shared_str batteryName= exo->m_sCurrentBattery;
				exoBatteryCost = pSettings->r_float(batteryName, "cost");
				float charge_factor=powf(exo->m_fCurrentCharge*0.9f + .1f, 0.75f);
				exoBatteryCost*= calcTradeFactor(returnTradeFactors(b_buying, batteryName), relation_factor)*charge_factor;;
			}
		}
		if (base_cost==-1)
			base_cost			= itemCost;
	}
	
#pragma region computing condition factor
	// for "dead" weapon we use 10% from base cost, for "good" weapon we use full base cost
	float					condition_factor = powf(pItem->GetCondition()*0.9f + .1f, 0.75f); 
#pragma endregion 
	

	
#pragma region computing action factor
	float action_factor = calcTradeFactor(returnTradeFactors(b_buying,pItem->object().cNameSect()),relation_factor);
#pragma endregion 

#pragma region computing deficit_factor
#if 0
	float					deficit_factor = partner.inv_owner->deficit_factor(pItem->object().cNameSect());
#else
	float					deficit_factor = 1.f;
#endif
#pragma endregion

#pragma region total price calculation
	base_cost *= condition_factor*action_factor;
	float totalCost = base_cost + glCost + slCost + scCost + amiwCost + griwCost + exoBatteryCost;
	totalCost *= deficit_factor;
#pragma endregion
	return					(iFloor(totalCost));
}
