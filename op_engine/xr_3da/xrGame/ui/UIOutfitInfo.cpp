#include "StdAfx.h"
#include "UIOutfitInfo.h"
#include "UIStatic.h"
#include "UIScrollView.h"
#include "../actor.h"
#include "../CustomOutfit.h"
#include "../string_table.h"
#include "UIListItem.h"
#include "UIListWnd.h"
#include "UIListItemIconed.h"
#include "UIFrameLineWnd.h"
#include "UIXmlInit.h"
#include "../ActorCondition.h"
#include "../InventoryOwner.h"
#include "../entity_alive.h"
#include "../inventory.h"
#include "../Artifact.h"
#include "IconedItemsHelper.h"
#include "../../defines.h"

#define PARAMS_PATH "outfit_info:immunities_list"
#define FILE_PATH "inventory_new.xml"
static xmlParams s_xmlParams(FILE_PATH,PARAMS_PATH);

CUIOutfitInfo::CUIOutfitInfo(): m_outfit(nullptr), m_list(nullptr), m_bShowModifiers(false)
{
	immunes = CreateImmunesStringMap();
	modificators = CreateModificatorsStringMap();
}

CUIOutfitInfo::~CUIOutfitInfo() {}

void CUIOutfitInfo::InitFromXml(CUIXml& xml_doc)
{
	LPCSTR _base				= "outfit_info";
	string256					_buff;
	CUIXmlInit::InitWindow		(xml_doc, _base, 0, this);

	m_list=xr_new<CUIListWnd>();
	m_list->SetAutoDelete(true);
	
	CUIXmlInit::InitListWnd(xml_doc,PARAMS_PATH,0,m_list);
	m_list->SetMessageTarget(this);
	m_list->EnableScrollBar(true);
	AttachChild(m_list);
	m_bShowModifiers=xml_doc.ReadAttribInt(PARAMS_PATH,0,"show_modifiers",0)==1?true:false;
	strconcat(sizeof(_buff),_buff, PARAMS_PATH, ":icons");
	CUIXmlInit::GetStringTable(xml_doc,_buff,0,iconIDs);
}

void CUIOutfitInfo::ClearAll()
{
	ClearItems(m_lImmuneUnsortedItems);
	ClearItems(m_lModificatorsUnsortedItems);
}

void CUIOutfitInfo::ClearItems(std::vector<CUIListItemIconed*> &baseList)
{
	while(!baseList.empty())
	{
		auto item=baseList.front();
		baseList.erase(std::remove(baseList.begin(),baseList.end(),item),baseList.end());
		item->DetachAll();
		xr_delete(item);
	}
}

void CUIOutfitInfo::createImmuneItem(CCustomOutfit* outfit,std::pair<ALife::EHitType,shared_str> immunePair, bool force_add)
{
	float _val_outfit			= outfit ? outfit->GetDefHitTypeProtection(ALife::EHitType(immunePair.first)) : 1.0f;
	_val_outfit			= 1.0f - _val_outfit;
	float _val_af				= Actor()->HitArtefactsOnBelt(1.0f,ALife::EHitType(immunePair.first));
	_val_af				= 1.0f - _val_af;
	bool emptyParam=fsimilar(_val_outfit, 0.0f) && fsimilar(_val_af, 0.0f) && !force_add;
	LPCSTR hitName= ALife::g_cafHitType2String(immunePair.first);
	CUIListItemIconed* item= findIconedItem(m_lImmuneUnsortedItems,hitName,emptyParam,s_xmlParams);
	if (!item)
		return;
	setIconedItem(iconIDs,item,hitName,immunePair.second,_val_outfit,0,_val_af,0);
}

void CUIOutfitInfo::createModifItem(CCustomOutfit* outfit,std::pair<int, restoreParam> modifPair, bool force_add)
{
	float outfitValue=0;
	float artsValue=artefactRestores[modifPair.first];
	switch (modifPair.first)
	{
		case JUMP_SPEED_DELTA_ID:
			break;
		case BLEEDING_RESTORE_ID:
			{
				float actorVal= pSettings->r_float	("actor_condition", "wound_incarnation_v");
				artsValue=(artsValue/actorVal)*100*-1;
				outfitValue = outfit ? outfit->m_fBleedingRestoreSpeed*outfit->GetCondition():0;
				outfitValue=outfitValue*1000*-1;
			}
			break;
		case SATIETY_RESTORE_ID:
			{
				float actorVal= pSettings->r_float	("actor_condition", "satiety_v");
				artsValue=(artsValue/actorVal)*100;
				outfitValue = outfit ? outfit->m_fSatietyRestoreSpeed*outfit->GetCondition():0;
				outfitValue=outfitValue*1000;
			}
			break;
		case RADIATION_RESTORE_ID:
			{
				float actorVal= pSettings->r_float	("actor_condition", "radiation_v");
				artsValue=(artsValue/actorVal);
				outfitValue = outfit ? outfit->m_fRadiationRestoreSpeed*outfit->GetCondition():0;
				outfitValue=outfitValue*1000;
			}
			break;
		case HEALTH_RESTORE_ID:
			{
				float actorVal= pSettings->r_float	("actor_condition", "satiety_health_v");
				artsValue=(artsValue/actorVal)*100;
				outfitValue = outfit ? outfit->m_fHealthRestoreSpeed*outfit->GetCondition():0;
				outfitValue=outfitValue*1000;
			}
			break;
		case POWER_RESTORE_ID:
			{
				float actorVal= pSettings->r_float	("actor_condition", "satiety_power_v");
				artsValue=(artsValue/actorVal);
				outfitValue = outfit ? outfit->m_fPowerRestoreSpeed*outfit->GetCondition():0;
				outfitValue=outfitValue*1000;
			}
			break;
		case POWER_LOSS_ID:
			{
				outfitValue = outfit ? outfit->GetPowerLoss()/outfit->GetCondition():0;
				outfitValue=outfitValue*100;
			}
			break;
		default:
			NODEFAULT;
	}
	bool emptyParam=fsimilar(outfitValue, 0.0f) && fsimilar(artsValue, 0.0f) && !force_add;
	CUIListItemIconed* item= findIconedItem(m_lModificatorsUnsortedItems,modifPair.second.paramName.c_str(),emptyParam,s_xmlParams);
	if (!item)
		return;
	setIconedItem(iconIDs,item,modifPair.second.paramName.c_str(),modifPair.second.paramDesc,outfitValue,2,artsValue,2,modifPair.first);
}

void CUIOutfitInfo::Update(CCustomOutfit* outfitP)
{
	m_outfit				= outfitP;
	m_list->RemoveAll();
#pragma region update immune lines
	std::for_each(immunes.begin(),immunes.end(),[&](std::pair<ALife::EHitType,shared_str> immunePair)
	{
		createImmuneItem(m_outfit,immunePair,false);
	});
	if (m_lImmuneUnsortedItems.size()>0)
	{
		//addSeparator(m_list,"ui_st_params");
		std::sort(m_lImmuneUnsortedItems.begin(),m_lImmuneUnsortedItems.end(),[](CUIListItem* i1, CUIListItem* i2)
		{
			CUIListItemIconed *iconedItem1=smart_cast<CUIListItemIconed*>(i1);
			CUIListItemIconed *iconedItem2=smart_cast<CUIListItemIconed*>(i2);
			if (!iconedItem1 || !iconedItem2)
				return false;
			return		lstrcmpi(iconedItem1->GetFieldText(1),iconedItem2->GetFieldText(1))<0;
		});
		std::for_each(m_lImmuneUnsortedItems.begin(),m_lImmuneUnsortedItems.end(),[&](CUIListItemIconed* item)
		{
			m_list->AddItem<CUIListItemIconed>(item);
		});
	}
#pragma endregion
	if (m_bShowModifiers && !!g_uCommonFlags.test(gpShowModificators))
	{
#pragma region update modifier lines
		#pragma region update weight modificator output disabled at the request of the mod developers community
		/*float outfitAddWeight = m_outfit ? m_outfit->m_additional_weight * m_outfit->GetCondition() : 0;
		float artefactsWeight = g_actor ? Actor()->GetArtefactAdditionalWeight() : 0;
		CUIListItemIconed* weightItem = findIconedItem(m_lModificatorsUnsortedItems, "additional_weight", fsimilar(outfitAddWeight, 0.0f) && fsimilar(artefactsWeight, 0.0f), s_xmlParams);
		if (weightItem)
			setIconedItem(iconIDs, weightItem, "additional_weight", "ui_inv_outfit_additional_inventory_weight", outfitAddWeight, 1, artefactsWeight, 1);*/
		#pragma endregion
		std::for_each(modificators.begin(),modificators.end(),[&](std::pair<int, restoreParam> modifPair)
		{
			#pragma region some modificators output disabled at the request of the mod developers community
			bool showSingleModif = true;
			switch (modifPair.first)
			{
				case POWER_RESTORE_ID:
				case SATIETY_RESTORE_ID:
				case POWER_LOSS_ID:
					showSingleModif = false;
					break;
				case HEALTH_RESTORE_ID:
				case RADIATION_RESTORE_ID:
				case BLEEDING_RESTORE_ID:
				case JUMP_SPEED_DELTA_ID:
				default: break;
			}
			if (showSingleModif)
			#pragma endregion
				createModifItem(m_outfit,modifPair,false);
		});
		if (m_lModificatorsUnsortedItems.size()>0)
		{
			//addSeparator(m_list);
			std::sort(m_lModificatorsUnsortedItems.begin(),m_lModificatorsUnsortedItems.end(),[](CUIListItem* i1, CUIListItem* i2)
			{
				CUIListItemIconed *iconedItem1=smart_cast<CUIListItemIconed*>(i1);
				CUIListItemIconed *iconedItem2=smart_cast<CUIListItemIconed*>(i2);
				if (!iconedItem1 || !iconedItem2)
					return false;
				return		lstrcmpi(iconedItem1->GetFieldText(1),iconedItem2->GetFieldText(1))<0;
			});
			std::for_each(m_lModificatorsUnsortedItems.begin(),m_lModificatorsUnsortedItems.end(),[&](CUIListItemIconed* item)
			{
				m_list->AddItem<CUIListItemIconed>(item);
			});
		}
#pragma endregion
	}
}

void CUIOutfitInfo::UpdateImmuneView()
{
	CEntityAlive *pEntityAlive = smart_cast<CEntityAlive*>(Level().CurrentEntity());
	CInventoryOwner* pOurInvOwner = smart_cast<CInventoryOwner*>(pEntityAlive);
	CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(pOurInvOwner->inventory().m_slots[OUTFIT_SLOT].m_pIItem);
	if (g_actor)
	{
		artefactRestores.clear();//�������� ���������� � ������������� � ����������
		for (u32 i = 0; i < modificators.size(); i++)
			artefactRestores.push_back(0);
		std::for_each(Actor()->inventory().m_belt.begin(), Actor()->inventory().m_belt.end(), [&](CInventoryItem* item)
		{
			CArtefact*	artefact = smart_cast<CArtefact*>(item);
			if (artefact)
			{
				artefactRestores[BLEEDING_RESTORE_ID] += artefact->m_fBleedingRestoreSpeed;
				artefactRestores[RADIATION_RESTORE_ID] += artefact->m_fRadiationRestoreSpeed;
				artefactRestores[HEALTH_RESTORE_ID] += artefact->m_fHealthRestoreSpeed;
				artefactRestores[SATIETY_RESTORE_ID] += artefact->m_fSatietyRestoreSpeed;
				artefactRestores[POWER_RESTORE_ID] += artefact->m_fPowerRestoreSpeed;
				artefactRestores[JUMP_SPEED_DELTA_ID] += artefact->m_fArtJumpSpeedDelta;
			}
		});
	}
	Update(pOutfit);
}



