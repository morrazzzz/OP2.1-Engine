#include "../pch_script.h"
#include "UICarBodyWnd.h"
#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "../HUDManager.h"
#include "../level.h"
#include "../xr_level_controller.h"
#include "UICharacterInfo.h"
#include "UIDragDropListEx.h"
#include "UIFrameWindow.h"
#include "UIItemInfo.h"
#include "UIPropertiesBox.h"
#include "../ai/monsters/BaseMonster/base_monster.h"
#include "../inventory.h"
#include "UIInventoryUtilities.h"
#include "UICellItem.h"
#include "UICellItemFactory.h"
#include "../WeaponMagazined.h"
#include "../Actor.h"
#include "../eatable_item.h"
#include "../alife_registry_wrappers.h"
#include "UI3tButton.h"
#include "UIListBoxItem.h"
#include "../InventoryBox.h"
#include "../game_object_space.h"
#include "../inventory_space.h"
#include "../script_callback_ex.h"
#include "../script_game_object.h"
#include "../GameObject.h"
#include "../BottleItem.h"
#include "UITradeWnd.h"
#include "../OPFuncs/utils.h"
#include "../string_table.h"
#include "../gbox.h"
#include "../exooutfit.h"

#define				CAR_BODY_XML		"carbody_new.xml"
#define				CARBODY_ITEM_XML	"carbody_item.xml"

void move_item (u16 from_id, u16 to_id, u16 what_id);

CUICarBodyWnd::CUICarBodyWnd()
{
	m_pInventoryBox = nullptr;
	m_pCurrentCellItem = nullptr;
	CUICarBodyWnd::Init();
	CUICarBodyWnd::Hide();
	m_b_need_update = false;
}

CUICarBodyWnd::~CUICarBodyWnd()
{
	m_pCurrentCellItem = nullptr;
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[](CUIDragDropListEx* list){list->ClearAll(true);});
}

void CUICarBodyWnd::re_init()
{
	m_pCurrentCellItem = nullptr;
	sourceDragDropLists.clear();
	m_pUIOurIcon->DetachAll();
	m_pUIOthersIcon->DetachAll();
	m_pUIOthersBagWnd->DetachAll();
	m_pUIOurBagWnd->DetachAll();
	m_pUIDescWnd->DetachAll();
	m_pUIPropertiesBox->RemoveAll();
	DetachAll();
	Init();
}

void CUICarBodyWnd::Init()
{
	CUIXml						uiXml;
	uiXml.Init					(CONFIG_PATH, UI_PATH, CAR_BODY_XML);
	
	CUIXmlInit					xml_init;

	xml_init.InitWindow			(uiXml, "main", 0, this);

	m_pUIStaticTop				= xr_new<CUIStatic>(); m_pUIStaticTop->SetAutoDelete(true);
	AttachChild					(m_pUIStaticTop);
	xml_init.InitStatic			(uiXml, "top_background", 0, m_pUIStaticTop);


	m_pUIStaticBottom			= xr_new<CUIStatic>(); m_pUIStaticBottom->SetAutoDelete(true);
	AttachChild					(m_pUIStaticBottom);
	xml_init.InitStatic			(uiXml, "bottom_background", 0, m_pUIStaticBottom);

	m_pUIOurIcon				= xr_new<CUIStatic>(); m_pUIOurIcon->SetAutoDelete(true);
	AttachChild					(m_pUIOurIcon);
	xml_init.InitStatic			(uiXml, "static_icon", 0, m_pUIOurIcon);

	m_pUIOthersIcon				= xr_new<CUIStatic>(); m_pUIOthersIcon->SetAutoDelete(true);
	AttachChild					(m_pUIOthersIcon);
	xml_init.InitStatic			(uiXml, "static_icon", 1, m_pUIOthersIcon);


	m_pUICharacterInfoLeft		= xr_new<CUICharacterInfo>(); m_pUICharacterInfoLeft->SetAutoDelete(true);
	m_pUIOurIcon->AttachChild	(m_pUICharacterInfoLeft);
	m_pUICharacterInfoLeft->Init(0,0, m_pUIOurIcon->GetWidth(), m_pUIOurIcon->GetHeight(), "trade_character.xml");


	m_pUICharacterInfoRight			= xr_new<CUICharacterInfo>(); m_pUICharacterInfoRight->SetAutoDelete(true);
	m_pUIOthersIcon->AttachChild	(m_pUICharacterInfoRight);
	m_pUICharacterInfoRight->Init	(0,0, m_pUIOthersIcon->GetWidth(), m_pUIOthersIcon->GetHeight(), "trade_character.xml");

	m_pUIOurBagWnd					= xr_new<CUIStatic>(); m_pUIOurBagWnd->SetAutoDelete(true);
	AttachChild						(m_pUIOurBagWnd);
	xml_init.InitStatic				(uiXml, "our_bag_static", 0, m_pUIOurBagWnd);


	m_pUIOthersBagWnd				= xr_new<CUIStatic>(); m_pUIOthersBagWnd->SetAutoDelete(true);
	AttachChild						(m_pUIOthersBagWnd);
	xml_init.InitStatic				(uiXml, "others_bag_static", 0, m_pUIOthersBagWnd);

	m_pUIOurBagList					= xr_new<CUIDragDropListEx>(); 
	m_pUIOurBagList->SetAutoDelete(true);
	m_pUIOurBagWnd->AttachChild		(m_pUIOurBagList);	
	xml_init.InitDragDropListEx		(uiXml, "dragdrop_list_our", 0, m_pUIOurBagList);
	m_pUIOurBagList->SetUIListId(IWListTypes::ltCarbodyOurBag);
	sourceDragDropLists.push_back(m_pUIOurBagList);


	m_pUIOthersBagList				= xr_new<CUIDragDropListEx>(); 
	m_pUIOthersBagList->SetAutoDelete(true);
	m_pUIOthersBagWnd->AttachChild	(m_pUIOthersBagList);	
	xml_init.InitDragDropListEx		(uiXml, "dragdrop_list_other", 0, m_pUIOthersBagList);
	m_pUIOthersBagList->SetUIListId(IWListTypes::ltCarbodyOtherBag);
	sourceDragDropLists.push_back(m_pUIOthersBagList);

	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[this](CUIDragDropListEx* list){BindDragDropListEvents(list);});

	//���������� � ��������
	m_pUIDescWnd					= xr_new<CUIFrameWindow>(); m_pUIDescWnd->SetAutoDelete(true);
	AttachChild						(m_pUIDescWnd);
	xml_init.InitFrameWindow		(uiXml, "frame_window", 0, m_pUIDescWnd);

	m_pUIStaticDesc					= xr_new<CUIStatic>(); m_pUIStaticDesc->SetAutoDelete(true);
	m_pUIDescWnd->AttachChild		(m_pUIStaticDesc);
	xml_init.InitStatic				(uiXml, "descr_static", 0, m_pUIStaticDesc);

	m_pUIItemInfo					= xr_new<CUIItemInfo>(); m_pUIItemInfo->SetAutoDelete(true);
	m_pUIDescWnd->AttachChild		(m_pUIItemInfo);
	m_pUIItemInfo->Init				(0,0, m_pUIDescWnd->GetWidth(), m_pUIDescWnd->GetHeight(), CARBODY_ITEM_XML);


	xml_init.InitAutoStatic			(uiXml, "auto_static", this);

	m_pUIPropertiesBox				= xr_new<CUIPropertiesBox>(); m_pUIPropertiesBox->SetAutoDelete(true);
	AttachChild						(m_pUIPropertiesBox);
	m_pUIPropertiesBox->Init		(0,0,300,300);
	m_pUIPropertiesBox->Hide		();

	SetCurrentItem					(nullptr);
	m_pUIStaticDesc->SetText		(nullptr);

	m_pUITakeAll					= xr_new<CUI3tButton>(); m_pUITakeAll->SetAutoDelete(true);
	AttachChild						(m_pUITakeAll);
	xml_init.Init3tButton				(uiXml, "take_all_btn", 0, m_pUITakeAll);

	uiXml.SetLocalRoot					(uiXml.NavigateToNode		("action_sounds",0));
	if (LPCSTR data=uiXml.Read("snd_open",		0,	nullptr))
		::Sound->create						(sounds[eInvSndOpen],data,st_Effect,sg_SourceType);
	if (LPCSTR data=uiXml.Read("snd_close",		0,	nullptr))
		::Sound->create						(sounds[eInvSndClose],data,st_Effect,sg_SourceType);
	if (LPCSTR data=uiXml.Read("snd_properties",		0,	nullptr))
		::Sound->create						(sounds[eInvProperties],data,st_Effect,sg_SourceType);
	if (LPCSTR data=uiXml.Read("snd_detach_addon",		0,	nullptr))
		::Sound->create						(sounds[eInvDetachAddon],data,st_Effect,sg_SourceType);
	if (LPCSTR data=uiXml.Read("snd_item_use",		0,	nullptr))
		::Sound->create						(sounds[eInvItemUse],data,st_Effect,sg_SourceType);
	if (LPCSTR data=uiXml.Read("snd_item_move",		0,	nullptr))
		::Sound->create						(sounds[eInvItemMove],data,st_Effect,sg_SourceType);
}

void CUICarBodyWnd::InitCarBody(CInventoryOwner* pOur, CInventoryBox* pInvBox)
{
	m_pOurObject									= pOur;
	m_pOthersObject									= nullptr;
	m_pInventoryBox									= pInvBox;
	m_pInventoryBox->m_in_use						= true;

	u16 our_id										= smart_cast<CGameObject*>(m_pOurObject)->ID();
	m_pUICharacterInfoLeft->InitCharacter			(our_id);
	m_pUIOthersIcon->Show							(false);
	m_pUICharacterInfoRight->ClearInfo				();
	m_pUIPropertiesBox->Hide						();
	EnableAll										();
	UpdateLists										();

}

void CUICarBodyWnd::InitCarBody(CInventoryOwner* pOur, CInventoryOwner* pOthers)
{

	m_pOurObject									= pOur;
	m_pOthersObject									= pOthers;
	m_pInventoryBox									= nullptr;
	
	u16 our_id										= smart_cast<CGameObject*>(m_pOurObject)->ID();
	u16 other_id									= smart_cast<CGameObject*>(m_pOthersObject)->ID();

	m_pUICharacterInfoLeft->InitCharacter			(our_id);
	m_pUIOthersIcon->Show							(true);
	
	CBaseMonster *monster = nullptr;
	if(m_pOthersObject) {
		monster										= smart_cast<CBaseMonster *>(m_pOthersObject);
		if (monster || m_pOthersObject->use_simplified_visual() ) 
		{
			m_pUICharacterInfoRight->ClearInfo		();
			if(monster)
			{
				m_pUICharacterInfoRight->m_bShowRelationData=false;
				shared_str monster_tex_name = pSettings->r_string(monster->cNameSect(),"icon");
				m_pUICharacterInfoRight->UIIcon().InitTexture(monster_tex_name.c_str());
				m_pUICharacterInfoRight->UIIcon().SetStretchTexture(true);
				shared_str shortName=monster->GetShortName();
				shared_str fullName=monster->cNameSect();
				shared_str translatedName;
				if (shortName.size()>0)
					translatedName=CStringTable().translate(shortName);
				else if (fullName.size()>0)
					translatedName=CStringTable().translate(fullName);
				if (translatedName.size()>0)
				{
					m_pUICharacterInfoRight->UIName().SetText(translatedName.c_str());
					m_pUICharacterInfoRight->UIName().Show(true);
				}
			}
		}else 
		{
			m_pUICharacterInfoRight->m_bShowRelationData=true;
			m_pUICharacterInfoRight->InitCharacter	(other_id);
		}
	}

	m_pUIPropertiesBox->Hide						();
	EnableAll										();
	UpdateLists										();

	if(!monster){
		CInfoPortionWrapper	*known_info_registry	= xr_new<CInfoPortionWrapper>();
		known_info_registry->registry().init		(other_id);
		CKnownInfoContainer& known_info				= known_info_registry->registry().objects();

		auto it = known_info.begin();
		for(int i=0;it!=known_info.end();++it,++i){
			NET_Packet		P;
			CGameObject::u_EventGen		(P,GE_INFO_TRANSFER, our_id);
			P.w_u16						(0);//not used
			P.w_stringZ					((*it).second.info_id.c_str());			//���������
			P.w_u8						(1);						//���������� ���������
			CGameObject::u_EventSend	(P);
		}
		known_info.clear	();
		xr_delete			(known_info_registry);
	}
}  

void CUICarBodyWnd::UpdateLists_delayed()
{
		m_b_need_update = true;
}

#include "UIInventoryUtilities.h"

void CUICarBodyWnd::Hide()
{
	PlaySnd								(eInvSndClose);
	InventoryUtilities::SendInfoToActor			("ui_car_body_hide");
	SetCurrentItem(nullptr);
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[](CUIDragDropListEx* list)
	{
		list->GetParent()->SetCapture(nullptr, false);
		list->ClearAll(true);
	});
	if (CUIDragDropListEx::m_drag_item)
		delete_data(CUIDragDropListEx::m_drag_item);

	inherited::Hide								();
	m_pUIPropertiesBox->Hide();
	m_pUIPropertiesBox->RemoveAll();
	if(m_pInventoryBox)
		m_pInventoryBox->m_in_use				= false;
}


void CUICarBodyWnd::AddSingleItemToList(CUICellItem* itm,CUIDragDropListEx* to)
{
		CInventoryItem* iitem=static_cast<CInventoryItem*>(itm->m_pData);
		if (g_uCommonFlags.is(E_COMMON_FLAGS::uiShowTradeSB) && OPFuncs::IsUsedInInventory(m_pOurObject,iitem))
		{
			itm->SetColor				(color_rgba(255,100,100,255));
			itm->SetMoveableToOther(!!g_uCommonFlags.is(E_COMMON_FLAGS::uiAllowOpTradeSB));
			itm->SetAllowedGrouping(false);
			to->SetItem(itm);
		}
		else
		{
			itm->SetAllowedGrouping(true);
			to->SetItem(itm);
		}
}

void CUICarBodyWnd::UpdateLists()
{
	TIItemContainer								ruck_list;
	m_pUIOurBagList->ClearAll					(true);
	int i_pos = m_pUIOthersBagList->ScrollPos();
	m_pUIOthersBagList->ClearAll				(true);

	ruck_list.clear								();
	bool useAdds=!!g_uCommonFlags.test(E_COMMON_FLAGS::uiShowTradeSB);
	m_pOurObject->inventory().AddAvailableItems	(ruck_list, true,useAdds,useAdds);
	std::sort									(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);

	//��� ������
	TIItemContainer::iterator it;
	for(it =  ruck_list.begin(); ruck_list.end() != it; ++it) 
	{
			CUICellItem* itm = create_cell_item(*it);
			AddSingleItemToList(itm, m_pUIOurBagList);
	}


	ruck_list.clear									();
	if(m_pOthersObject)
		m_pOthersObject->inventory().AddAvailableItems	(ruck_list, false); //����� �����
	else
		m_pInventoryBox->AddAvailableItems			(ruck_list); //����� �������

	std::sort										(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);

	//����� ������
	for(it =  ruck_list.begin(); ruck_list.end() != it; ++it) 
	{
		CUICellItem* itm							= create_cell_item(*it);
		m_pUIOthersBagList->SetItem					(itm);
	}


	m_pUIOthersBagList->SetScrollPos(i_pos);

	InventoryUtilities::UpdateWeight				(*m_pUIOurBagWnd);
	m_b_need_update									= false;
}

void CUICarBodyWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if (BUTTON_CLICKED == msg && m_pUITakeAll == pWnd)
	{
		TakeAll();
	}
	else if (pWnd == m_pUIPropertiesBox &&	msg == PROPERTY_CLICKED)
	{
		if (m_pUIPropertiesBox->GetClickedItem())
		{
			u32 itemTag = m_pUIPropertiesBox->GetClickedItem()->GetTAG();
			switch (itemTag)
			{
			case INVENTORY_DISCHARGE_EXO:
				if (CExoOutfit* exo = (CExoOutfit*)m_pUIPropertiesBox->GetClickedItem()->GetData())
					exo->RemoveFromBatterySlot(true);
				break;
			case INVENTORY_EAT_ACTION:	//������ ������
				EatItem();
				break;
			case INVENTORY_UNLOAD_MAGAZINE:
			{
#pragma todo("more correctness call throug GetData()... maybe rework all CurrentItem() calls?")
				CUICellItem* cellItem = static_cast<CUICellItem*>(m_pUIPropertiesBox->GetClickedItem()->GetData()); 
				if (!cellItem)
					break;
				CWeapon* weapon = static_cast<CWeapon*>(cellItem->m_pData);
				if (!weapon)
					break;
				CWeaponMagazined* wg = smart_cast<CWeaponMagazined*>(weapon);
				if (!wg)
					break;
				wg->PlayEmptySnd();
				OPFuncs::UnloadWeapon(wg);
				for (size_t i = 0; i < cellItem->ChildsCount(); ++i)
				{
					CUICellItem * child_itm = cellItem->Child(i);
					OPFuncs::UnloadWeapon(smart_cast<CWeaponMagazined*>(static_cast<CWeapon*>(child_itm->m_pData)));
				}
				SetCurrentItem(nullptr);
			}break;
			case INVENTORY_CB_MOVE_ALL:
			case INVENTORY_CB_MOVE_SINGLE:
			{
				CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());
				if (!pActor && !CurrentItem())
					break;
				CUIDragDropListEx* owner_list = CurrentItem()->OwnerList();
				if (!owner_list)
					break;
				u16 from_id;
				u16 to_id;
				u16 bag_id = (m_pInventoryBox) ? m_pInventoryBox->ID() : smart_cast<CGameObject*>(m_pOthersObject)->ID();
				u16 currItem_id = CurrentIItem()->object().ID();
				u16 actor_id = Actor()->ID();
				from_id = owner_list == m_pUIOthersBagList ? bag_id : actor_id;
				to_id = owner_list == m_pUIOthersBagList ? actor_id : bag_id;
				CUICellItem* currItem = CurrentItem();
				PlaySnd(eInvItemMove);
				float to_max_weight = m_pOthersObject ? m_pOthersObject->inventory().GetMaxWeight() : 10000;
				float to_current_weight= m_pOthersObject ? to_id== Actor()->ID() ? 0 : m_pOthersObject->inventory().CalcTotalWeight() : 0;
				if (itemTag == INVENTORY_CB_MOVE_ALL)
				{
					for (unsigned int i = 0; i < currItem->ChildsCount(); ++i)
					{
						PIItem			iitm = static_cast<PIItem>(currItem->Child(i)->m_pData);
						if (to_current_weight + iitm->Weight() < to_max_weight)
						{
							move_item(from_id, to_id, iitm->object().ID());
							to_current_weight += iitm->Weight();
						}
						else
						{
							Msg("# Object[%d] - maximum internal capacity reached", to_id);
							break;
						}
					}
				}
				if (to_current_weight + CurrentIItem()->Weight() < to_max_weight)
					move_item(from_id, to_id, currItem_id);
				else
					Msg("# Object[%d] - maximum internal capacity reached", to_id);
			}
			break;
			case INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON:
				DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetGrenadeLauncherName());
				break;
			case INVENTORY_DETACH_SCOPE_ADDON:
				DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetScopeName());
				break;
			case INVENTORY_DETACH_SILENCER_ADDON:
				DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetSilencerName());
				break;
			}
			UpdateLists_delayed();
		}
	}

	inherited::SendMessage(pWnd, msg, pData);
}

void CUICarBodyWnd::Draw()
{
	inherited::Draw	();
}


void CUICarBodyWnd::Update()
{
	if(	m_b_need_update||
		m_pOurObject->inventory().ModifyFrame()==Device.dwFrame || 
		(m_pOthersObject&&m_pOthersObject->inventory().ModifyFrame()==Device.dwFrame))

		UpdateLists		();

	
	if(m_pOthersObject && (smart_cast<CGameObject*>(m_pOurObject))->Position().distance_to((smart_cast<CGameObject*>(m_pOthersObject))->Position()) > 3.0f)
	{
		GetHolder()->StartStopMenu(this,true);
	}
	inherited::Update();
}


void CUICarBodyWnd::Show() 
{ 
	PlaySnd								(eInvSndOpen);
	InventoryUtilities::SendInfoToActor		("ui_car_body");
	inherited::Show							();
	SetCurrentItem							(nullptr);
	InventoryUtilities::UpdateWeight		(*m_pUIOurBagWnd);
}

void CUICarBodyWnd::DisableAll()
{
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[](CUIDragDropListEx* list){list->Enable(false);});
}

void CUICarBodyWnd::EnableAll()
{
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[](CUIDragDropListEx* list){list->Enable(true);});

}

CUICellItem* CUICarBodyWnd::CurrentItem()
{
	return m_pCurrentCellItem;
}

PIItem CUICarBodyWnd::CurrentIItem()
{
	return	(m_pCurrentCellItem)?static_cast<PIItem>(m_pCurrentCellItem->m_pData) : nullptr;
}

void CUICarBodyWnd::SetItemSelected(CUICellItem* itm)
{
	CUICellItem* curr=CurrentItem();
	if (curr  && curr->GetSelected())
		curr->SetSelected(false);
	if (itm!=nullptr && !itm->GetSelected())	
		itm->SetSelected(true);
}

void CUICarBodyWnd::SetCurrentItem(CUICellItem* itm)
{
	if(m_pCurrentCellItem == itm) return;
	SetItemSelected(itm);
	m_pCurrentCellItem		= itm;
	m_pUIItemInfo->InitItem(CurrentIItem());
	ClearAllSuitables();
	if (!m_pCurrentCellItem)
		return;
	
	bool processed=false;
	CInventoryItem* currentIItem=CurrentIItem();
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[&processed,currentIItem](CUIDragDropListEx* list)
	{
		bool ls=list->select_suitables_by_item(currentIItem);
		processed=processed || ls;
	});

	if (pSettings->line_exist("maingame_ui", "on_cell_after_select"))
	{
		CGameObject* GO = smart_cast<CGameObject*>(CurrentIItem());
		if (GO)
		{
			LPCSTR on_cell_after_select = pSettings->r_string("maingame_ui", "on_cell_after_select");
			luabind::functor<void> functor;
			if (!ai().script_engine().functor(on_cell_after_select, functor))
			{
				Msg("! ERROR function [%s] not exist for on_cell_after_select callback", on_cell_after_select);
			}
			else
			{
				try
				{
					functor(this, CurrentItem(), processed);
				}
				catch (...)
				{
					Msg("! ERROR function [%s] cause unknown error.", on_cell_after_select);
				}
			}
		}
	}
}

void CUICarBodyWnd::TakeAll()
{
	u32 cnt				= m_pUIOthersBagList->ItemsCount();
	u16 tmp_id = 0;
	if(m_pInventoryBox){
		tmp_id	= (smart_cast<CGameObject*>(m_pOurObject))->ID();
	}

	for(u32 i=0; i<cnt; ++i)
	{
		CUICellItem*	ci = m_pUIOthersBagList->GetItemIdx(i);
		for(u32 j=0; j<ci->ChildsCount(); ++j)
		{
			PIItem _itm		= (PIItem)(ci->Child(j)->m_pData);
			if(m_pOthersObject)
				TransferItem	(_itm, m_pOthersObject, m_pOurObject, false);
			else{
				move_item		(m_pInventoryBox->ID(), tmp_id, _itm->object().ID());
//.				Actor()->callback(GameObject::eInvBoxItemTake)( m_pInventoryBox->lua_game_object(), _itm->object().lua_game_object() );
			}
		
		}
		PIItem itm		= (PIItem)(ci->m_pData);
		if(m_pOthersObject)
			TransferItem	(itm, m_pOthersObject, m_pOurObject, false);
		else{
			move_item		(m_pInventoryBox->ID(), tmp_id, itm->object().ID());
//.			Actor()->callback(GameObject::eInvBoxItemTake)(m_pInventoryBox->lua_game_object(), itm->object().lua_game_object() );
		}

	}
}


bool CUICarBodyWnd::OnKeyboard(int dik, EUIMessages keyboard_action)
{
	if( inherited::OnKeyboard(dik,keyboard_action) )return true;
	if (keyboard_action == WINDOW_KEY_PRESSED)
	{
		EGameActions action = get_binded_action(dik);
		bool processed = false;
		switch (action)
		{
			case kUSE:
				GetHolder()->StartStopMenu(this, true);
				processed = true;
				break;
			case kCARBODY_TAKE_ALL:
				TakeAll();
				processed = true;
				break;
			default:
				break;
		}
		return processed;
		
	}
	return false;
}

#include "../Medkit.h"
#include "../Antirad.h"

void CUICarBodyWnd::ActivatePropertiesBox()
{
	//if(m_pInventoryBox)	return;
	
	m_pUIPropertiesBox->RemoveAll();
	
	CWeaponMagazined*		pWeapon			= smart_cast<CWeaponMagazined*>(CurrentIItem());
	CEatableItem*			pEatableItem	= smart_cast<CEatableItem*>(CurrentIItem());
	CMedkit*				pMedkit			= smart_cast<CMedkit*>			(CurrentIItem());
	CAntirad*				pAntirad		= smart_cast<CAntirad*>			(CurrentIItem());
	CBottleItem*			pBottleItem		= smart_cast<CBottleItem*>		(CurrentIItem());
	CExoOutfit*			pExo = smart_cast<CExoOutfit*>		(CurrentIItem());
	CGBox*			pBox = smart_cast<CGBox*>			(CurrentIItem());
	bool					b_show			= false;
	
	if (pWeapon)
	{
		if(pWeapon->GrenadeLauncherAttachable() && pWeapon->IsGrenadeLauncherAttached())
		{
			m_pUIPropertiesBox->AddItem(OPFuncs::getComplexString("st_detach_gl",nullptr, OPFuncs::getAddonInvName(pWeapon->GetGrenadeLauncherName().c_str()),"st_detach_gl_full").c_str(),  nullptr, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON);
			b_show			= true;
		}
		if(pWeapon->ScopeAttachable() && pWeapon->IsScopeAttached())
		{
			m_pUIPropertiesBox->AddItem(OPFuncs::getComplexString("st_detach_scope",nullptr, OPFuncs::getAddonInvName(pWeapon->GetScopeName().c_str()),"st_detach_scope_full").c_str(),  nullptr, INVENTORY_DETACH_SCOPE_ADDON);
			b_show			= true;
		}	
		if(pWeapon->SilencerAttachable() && pWeapon->IsSilencerAttached())
		{
			m_pUIPropertiesBox->AddItem(OPFuncs::getComplexString("st_detach_silencer",nullptr, OPFuncs::getAddonInvName(pWeapon->GetSilencerName().c_str()),"st_detach_silencer_full").c_str(),  nullptr, INVENTORY_DETACH_SILENCER_ADDON);
			b_show			= true;
		}
		if(IsGameTypeSingle())
		{
			bool b = (0!=pWeapon->GetAmmoElapsed());
			if(!b)
			{
				CUICellItem * itm = CurrentItem();
				for(size_t i=0; i<itm->ChildsCount(); ++i)
				{
					pWeapon		= smart_cast<CWeaponMagazined*>(static_cast<CWeapon*>(itm->Child(i)->m_pData));
					if(pWeapon->GetAmmoElapsed())
					{
						b = true;
						break;
					}
				}
			}
			if(b){
				m_pUIPropertiesBox->AddItem(OPFuncs::getComplexString("st_unload_magazine",pWeapon).c_str(),  CurrentItem(), INVENTORY_UNLOAD_MAGAZINE);
				b_show			= true;
			}
		}
	}
	else if (pExo)
	{
		if (pExo->isBatteryPresent())
		{
			m_pUIPropertiesBox->AddItem("st_discharge_exo", pExo, INVENTORY_DISCHARGE_EXO);
			b_show = true;
		}
	}

	LPCSTR _action				= nullptr;
	
	if(pMedkit || pAntirad)
	{
		_action						= "st_use";
		b_show						= true;
	}
	else if(pEatableItem)
	{
		if(pBottleItem)
			_action					= "st_drink";
		else
			_action					= "st_eat";
		b_show						= true;
	}
	if(_action)
		m_pUIPropertiesBox->AddItem(_action, nullptr, INVENTORY_EAT_ACTION);

	if (!CurrentIItem()->IsQuestItem())
		if (CurrentItem()->GetMoveableToOther())
		{
			m_pUIPropertiesBox->AddItem("ui_carbody_move_single", nullptr, INVENTORY_CB_MOVE_SINGLE);
			if (CurrentItem()->ChildsCount())
				m_pUIPropertiesBox->AddItem("ui_carbody_move_all", nullptr, INVENTORY_CB_MOVE_ALL);
			b_show						= true;
		}
			

	if(b_show){
		PlaySnd							(eInvProperties);
		m_pUIPropertiesBox->AutoUpdateSize	();
		m_pUIPropertiesBox->BringAllToTop	();
		Fvector2						cursor_pos2;
		Frect							vis_rect;
		GetAbsoluteRect					(vis_rect);
		cursor_pos2						= GetUICursor()->GetCursorPosition();
		cursor_pos2.sub					(vis_rect.lt);
		m_pUIPropertiesBox->Show		(vis_rect, cursor_pos2);
	}
}

void CUICarBodyWnd::EatItem()
{
	PlaySnd									(eInvItemUse);
	CActor *pActor				= smart_cast<CActor*>(Level().CurrentEntity());
	if(!pActor)					return;

	CUIDragDropListEx* owner_list		= CurrentItem()->OwnerList();
	if(owner_list==m_pUIOthersBagList)
	{
		u16 owner_id				= (m_pInventoryBox)?m_pInventoryBox->ID():smart_cast<CGameObject*>(m_pOthersObject)->ID();

		move_item(	owner_id, //from
					Actor()->ID(), //to
					CurrentIItem()->object().ID());
	}

	CGBox*	pBox = smart_cast<CGBox*>(CurrentIItem());
	NET_Packet					P;
	if (pBox)
		CGameObject::u_EventGen(P, GEG_PLAYER_ITEM_USE, Actor()->ID());
	else
		CGameObject::u_EventGen(P, GEG_PLAYER_ITEM_EAT, Actor()->ID());
	P.w_u16						(CurrentIItem()->object().ID());
	CGameObject::u_EventSend	(P);

}


bool CUICarBodyWnd::OnItemDrop(CUICellItem* itm)
{
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= CUIDragDropListEx::m_drag_item->BackList();
	
	if (old_owner == new_owner || !old_owner || !new_owner)
	{
		return true;
	}

	if(m_pOthersObject)
	{
		if( TransferItem		(	CurrentIItem(),
								(old_owner==m_pUIOthersBagList)?m_pOthersObject:m_pOurObject, 
								(old_owner==m_pUIOurBagList)?m_pOthersObject:m_pOurObject, 
								(old_owner==m_pUIOurBagList)
							)
			)
		{
			CUICellItem* ci					= old_owner->RemoveItem(CurrentItem(), false);
			new_owner->SetItem				(ci);
		}
	}else
	{
		u16 tmp_id	= (smart_cast<CGameObject*>(m_pOurObject))->ID();

		bool bMoveDirection		= (old_owner==m_pUIOthersBagList);

		move_item				(
								bMoveDirection?m_pInventoryBox->ID():tmp_id,
								bMoveDirection?tmp_id:m_pInventoryBox->ID(),
								CurrentIItem()->object().ID());


//		Actor()->callback		(GameObject::eInvBoxItemTake)(m_pInventoryBox->lua_game_object(), CurrentIItem()->object().lua_game_object() );

		CUICellItem* ci			= old_owner->RemoveItem(CurrentItem(), false);
		new_owner->SetItem		(ci);
	}

	SetCurrentItem					(nullptr);
	return				true;
}

bool CUICarBodyWnd::OnItemStartDrag(CUICellItem* itm)
{
	if (!CurrentItem()->GetMoveableToOther()) 
		return true;
	return				false; //default behaviour
}

bool CUICarBodyWnd::OnItemDbClick(CUICellItem* itm)
{
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner = (old_owner==m_pUIOthersBagList)?m_pUIOurBagList:m_pUIOthersBagList;
	if (!CurrentItem()->GetMoveableToOther())
	{
		return true;
	}

	if(m_pOthersObject)
	{
		if( TransferItem		(CurrentIItem(),
								(old_owner==m_pUIOthersBagList)?m_pOthersObject:m_pOurObject, 
								(old_owner==m_pUIOurBagList)?m_pOthersObject:m_pOurObject, 
								(old_owner==m_pUIOurBagList)
								)
			)
		{
			CUICellItem* ci			= old_owner->RemoveItem(CurrentItem(), false);
			new_owner->SetItem		(ci);
		}
	}else
	{
		//if(false && old_owner==m_pUIOurBagList) return true;
		bool bMoveDirection		= (old_owner==m_pUIOthersBagList);

		u16 tmp_id				= (smart_cast<CGameObject*>(m_pOurObject))->ID();
		move_item				(
								bMoveDirection?m_pInventoryBox->ID():tmp_id,
								bMoveDirection?tmp_id:m_pInventoryBox->ID(),
								CurrentIItem()->object().ID());
//.		Actor()->callback		(GameObject::eInvBoxItemTake)(m_pInventoryBox->lua_game_object(), CurrentIItem()->object().lua_game_object() );

	}
	SetCurrentItem				(nullptr);
	return						true;
}

bool CUICarBodyWnd::OnItemSelected(CUICellItem* itm)
{
	SetCurrentItem		(itm);
	return				false;
}

bool CUICarBodyWnd::OnItemRButtonClick(CUICellItem* itm)
{
	SetCurrentItem				(itm);
	ActivatePropertiesBox		();
	return						false;
}

bool CUICarBodyWnd::OnItemFocusLost(CUICellItem* itm) const
{
	if (itm)
		itm->SetFocused(false);
	return						false;
}

bool CUICarBodyWnd::OnItemFocusReceive(CUICellItem* itm) const
{
	if (itm)
		itm->SetFocused(true);
	return						false;
}

void move_item (u16 from_id, u16 to_id, u16 what_id)
{
	NET_Packet P;
	CGameObject::u_EventGen					(P,	GE_OWNERSHIP_REJECT,from_id);
	P.w_u16									(what_id);
	CGameObject::u_EventSend				(P);

	//������� ��������� - ����� ���� 
	CGameObject::u_EventGen					(P,GE_OWNERSHIP_TAKE,to_id);
	P.w_u16									(what_id);
	CGameObject::u_EventSend				(P);

}

bool CUICarBodyWnd::TransferItem(PIItem itm, CInventoryOwner* owner_from, CInventoryOwner* owner_to, bool b_check)
{
	VERIFY									(NULL==m_pInventoryBox);
	CGameObject* go_from					= smart_cast<CGameObject*>(owner_from);
	CGameObject* go_to						= smart_cast<CGameObject*>(owner_to);

	//if(smart_cast<CBaseMonster*>(go_to))	return false;
	if(b_check)
	{
		float invWeight						= owner_to->inventory().CalcTotalWeight();
		float maxWeight						= owner_to->inventory().GetMaxWeight();
		float itmWeight						= itm->Weight();
		if(invWeight+itmWeight >=maxWeight)	return false;
	}

	move_item(go_from->ID(), go_to->ID(), itm->object().ID());

	return true;
}

bool CUICarBodyWnd::OnMouse(float x, float y, EUIMessages mouse_action)
{
	switch (mouse_action)
	{
		case WINDOW_CBUTTON_DOWN:
			if (is_binded(kCARBODY_TAKE_ALL, MOUSE_3))
				TakeAll();
			break;
		case WINDOW_RBUTTON_DOWN:
			if (m_pUIPropertiesBox->IsShown())
				m_pUIPropertiesBox->Hide();
			break;
		default:break;
	}
	CUIWindow::OnMouse					(x, y, mouse_action);
	return true; 
}

void CUICarBodyWnd::DetachAddon(const char* addon_name)
{
	PlaySnd										(eInvDetachAddon);
	OPFuncs::DetachAddon(CurrentIItem(),addon_name);
	SetCurrentItem(nullptr);
}

void CUICarBodyWnd::PlaySnd(eTradeSoundActions a)
{
	if (sounds[a]._handle())
		sounds[a].play(nullptr, sm_2D);
}

void CUICarBodyWnd::BindDragDropListEvents(CUIDragDropListEx* lst)
{
	lst->m_bEnableDragDrop = false;
	lst->m_f_item_drop				= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUICarBodyWnd::OnItemDrop);
	lst->m_f_item_start_drag		= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUICarBodyWnd::OnItemStartDrag);
	lst->m_f_item_db_click			= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUICarBodyWnd::OnItemDbClick);
	lst->m_f_item_selected			= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUICarBodyWnd::OnItemSelected);
	lst->m_f_item_rbutton_click		= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUICarBodyWnd::OnItemRButtonClick);
	lst->m_f_item_focus_lost		= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUICarBodyWnd::OnItemFocusLost);
	lst->m_f_item_focus_received	= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUICarBodyWnd::OnItemFocusReceive);
}
