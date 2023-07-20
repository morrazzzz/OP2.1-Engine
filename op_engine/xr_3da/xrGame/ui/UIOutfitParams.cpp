#include "stdafx.h"
#include "UIOutfitParams.h"
#include "UIXmlInit.h"
#include "UIListWnd.h"
#include "UIListItemIconed.h"
#include "UIScrollView.h"
#include "../inventory_item.h"
#include "../CustomOutfit.h"
#include "IconedItemsHelper.h"
#include "../OPFuncs/utils.h"
#include <locale>
#include "../exooutfit.h"

CUIOutfitParams::CUIOutfitParams()
{
	immunes = CreateImmunesStringMap();
	modificators = CreateModificatorsStringMap();
	m_list=nullptr;
	m_bShowModifiers=false;
	inOutfit = nullptr;
}

CUIOutfitParams::~CUIOutfitParams()
{
	ClearAll();
	xr_delete(m_list);
}

#define PARAMS_PATH "outfit_params:immunities_list"

void CUIOutfitParams::InitFromXml(CUIXml& xml_doc) 
{
	string256					_buff;
	m_list=xr_new<CUIListWnd>();
	m_list->SetAutoDelete(false);
	CUIXmlInit::InitListWnd(xml_doc,PARAMS_PATH,0,m_list);
	m_list->EnableScrollBar(false);
	m_list->EnableAlwaysShowScroll(false);
	m_list->SetIgnoreScrolling(true);
	m_bShowModifiers=xml_doc.ReadAttribInt(PARAMS_PATH,0,"show_modifiers",0)==1?true:false;
	strconcat(sizeof(_buff),_buff, PARAMS_PATH, ":icons");
	CUIXmlInit::GetStringTable(xml_doc,_buff,0,m_mIconIDs);
	currentFileNameXml= OPFuncs::getFileNameFromPath(xml_doc.m_xml_file_name).c_str();
}

bool CUIOutfitParams::Check(CInventoryItem* outfitItem) const
{
	return smart_cast<CCustomOutfit*>(outfitItem)!=nullptr;
}

bool CUIOutfitParams::Check(shared_str section) const
{
	xr_vector<xr_string> classes;
	classes.push_back("E_STLK");
	classes.push_back("EQU_SCIE");
	classes.push_back("EQU_STLK");
	classes.push_back("EQU_MLTR");
	classes.push_back("EQU_EXO");
	xr_string className=READ_IF_EXISTS(pSettings,r_string,section,"class","");
	std::transform(className.begin(), className.end(), className.begin(), ::toupper);
	if (!className.empty())
		if (std::find(classes.begin(), classes.end(), className) != classes.end())
			return true;
	return false;
}

void CUIOutfitParams::SetInfo(CCustomOutfit* outfitItem,CUIScrollView *parent) 
{
	if (!outfitItem)
		return;
	inOutfit = outfitItem;
	SetInfo(inOutfit->cNameSect(), parent);
}

void CUIOutfitParams::SetInfo(const shared_str& outfit_section, CUIScrollView* parent)
{
	m_list->RemoveAll();
	
#pragma region update immune lines
	std::for_each(immunes.begin(), immunes.end(), [&](std::pair<ALife::EHitType, shared_str> immunePair)
	{
		createImmuneItem(outfit_section, immunePair, false);
	});
	if (m_lImmuneUnsortedItems.size()>0)
	{
		std::sort(m_lImmuneUnsortedItems.begin(), m_lImmuneUnsortedItems.end(), [](CUIListItem* i1, CUIListItem* i2)
		{
			CUIListItemIconed *iconedItem1 = smart_cast<CUIListItemIconed*>(i1);
			CUIListItemIconed *iconedItem2 = smart_cast<CUIListItemIconed*>(i2);
			if (!iconedItem1 || !iconedItem2)
				return false;
			return		lstrcmpi(iconedItem1->GetFieldText(1), iconedItem2->GetFieldText(1))<0;
		});
		std::for_each(m_lImmuneUnsortedItems.begin(), m_lImmuneUnsortedItems.end(), [&](CUIListItemIconed* item)
		{
			m_list->AddItem<CUIListItemIconed>(item);
		});
	}
#pragma endregion
#pragma region update modifiers lines
	if (m_bShowModifiers)
	{
		CExoOutfit* exo = smart_cast<CExoOutfit*>(inOutfit);
		CUIListItemIconed* chargeItem = findIconedItem(m_lModificatorsUnsortedItems, "exo_charge", exo ? false : true, xmlParams(currentFileNameXml, PARAMS_PATH));
		if (chargeItem)
			setIconedItem(m_mIconIDs, chargeItem, "exo_charge", "ui_st_bci_desc", exo ? exo->m_fCurrentCharge:0, 3, 0, 0);


		float outfitAddWeight = inOutfit ? inOutfit->m_additional_weight*inOutfit->GetCondition() : 0;
		float outfitOriginalAddWeight = pSettings->r_float(outfit_section, "additional_inventory_weight");
		CUIListItemIconed* weightItem = findIconedItem(m_lModificatorsUnsortedItems, "additional_weight", fsimilar(outfitAddWeight, 0.0f) && fsimilar(outfitOriginalAddWeight, 0.0f), xmlParams(currentFileNameXml, PARAMS_PATH));
		if (weightItem)
			setIconedItem(m_mIconIDs, weightItem, "additional_weight", "ui_inv_outfit_additional_inventory_weight", outfitAddWeight, 1, outfitOriginalAddWeight, 1);
		std::for_each(modificators.begin(), modificators.end(), [&](std::pair<int, restoreParam> modifPair)
		{
			createModifItem(outfit_section, modifPair, false);
		});
		if (m_lModificatorsUnsortedItems.size()>0)
		{
			if (m_lImmuneUnsortedItems.size()>0)
				addSeparator(m_list);
			std::sort(m_lModificatorsUnsortedItems.begin(), m_lModificatorsUnsortedItems.end(), [](CUIListItem* i1, CUIListItem* i2)
			{
				CUIListItemIconed *iconedItem1 = smart_cast<CUIListItemIconed*>(i1);
				CUIListItemIconed *iconedItem2 = smart_cast<CUIListItemIconed*>(i2);
				if (!iconedItem1 || !iconedItem2)
					return false;
				return		lstrcmpi(iconedItem1->GetFieldText(1), iconedItem2->GetFieldText(1))<0;
			});
			std::for_each(m_lModificatorsUnsortedItems.begin(), m_lModificatorsUnsortedItems.end(), [&](CUIListItemIconed* item)
			{
				m_list->AddItem<CUIListItemIconed>(item);
			});
		}
	}
#pragma endregion
	m_list->SetHeight(m_list->GetItemsCount()*m_list->GetItemHeight() + 5);
	m_list->GetInternalScrollbar()->SetPageSize(m_list->GetItemsCount());
	m_list->ScrollToBegin();
	parent->AddWindow(m_list, false);
}

void CUIOutfitParams::ClearAll()
{
	ClearItems(m_lImmuneUnsortedItems);
	ClearItems(m_lModificatorsUnsortedItems);
}

void CUIOutfitParams::ClearItems(std::vector<CUIListItemIconed*>& baseList) const
{
	while (!baseList.empty())
	{
		auto item = baseList.front();
		baseList.erase(std::remove(baseList.begin(), baseList.end(), item), baseList.end());
		item->DetachAll();
		xr_delete(item);
	}
}

void CUIOutfitParams::createImmuneItem(shared_str outfit_section, std::pair<ALife::EHitType, shared_str> immunePair, bool force_add)
{
	float curr_val_outfit = inOutfit ? inOutfit->GetDefHitTypeProtection(ALife::EHitType(immunePair.first)) : 1.0f;
	curr_val_outfit = 1.0f - curr_val_outfit;
	LPCSTR hitName = ALife::g_cafHitType2String(immunePair.first);
	string64 buf;
	sprintf_s(buf, "%s_protection", hitName);
	float original_val_outfit = pSettings->r_float(outfit_section, buf);

	bool emptyParam = fsimilar(curr_val_outfit, 0.0f) && !force_add;
	CUIListItemIconed* item = findIconedItem(m_lImmuneUnsortedItems, hitName, emptyParam, xmlParams(currentFileNameXml, PARAMS_PATH));
	if (!item)
		return;
	setIconedItem(m_mIconIDs, item, hitName, immunePair.second, curr_val_outfit, 0, original_val_outfit, 0);
}

void CUIOutfitParams::createModifItem(shared_str outfit_section, std::pair<int, restoreParam> modifPair, bool force_add)
{
	float modifValue = READ_IF_EXISTS(pSettings, r_float, outfit_section, modifPair.second.paramName.c_str(), modifPair.first == POWER_LOSS_ID ? 1.0f : 0.0f);
	float outfitValue=0;
	float outfitValue1=0;
	switch (modifPair.first)
	{
		case BLEEDING_RESTORE_ID:
			{
				outfitValue1 = modifValue *1000*-1;
				outfitValue=outfitValue1*(inOutfit? inOutfit->GetCondition(): 1.0f);
			}
			break;
		case SATIETY_RESTORE_ID:
			{
				outfitValue1 = modifValue *1000;
				outfitValue=outfitValue1*(inOutfit ? inOutfit->GetCondition() : 1.0f);
			}
			break;
		case RADIATION_RESTORE_ID:
			{
				outfitValue1 = modifValue *1000;
				outfitValue=outfitValue1*(inOutfit ? inOutfit->GetCondition() : 1.0f);
			}
			break;
		case HEALTH_RESTORE_ID:
			{
				outfitValue1 = modifValue *1000;
				outfitValue=outfitValue1*(inOutfit ? inOutfit->GetCondition() : 1.0f);
			}
			break;
		case POWER_RESTORE_ID:
			{
				outfitValue1 = modifValue*1000;
				outfitValue=outfitValue1*(inOutfit ? inOutfit->GetCondition() : 1.0f);

			}
			break;
		case POWER_LOSS_ID:
			{
				outfitValue1 = modifValue*100;
				outfitValue=outfitValue1/(inOutfit ? inOutfit->GetCondition() : 1.0f);
			}
			break;
		default:
			return;
	}
	bool emptyParam=fsimilar(outfitValue, 0.0f) && fsimilar(outfitValue1, 0.0f) && !force_add;
	CUIListItemIconed* item= findIconedItem(m_lModificatorsUnsortedItems,modifPair.second.paramName.c_str(),emptyParam,xmlParams(currentFileNameXml,PARAMS_PATH));
	if (!item)
		return;
	setIconedItem(m_mIconIDs,item,modifPair.second.paramName.c_str(),modifPair.second.paramDesc,outfitValue,2,outfitValue1,2,modifPair.first);
}
