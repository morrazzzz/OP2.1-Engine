#include "stdafx.h"
#include "UIInventoryWnd.h"
#include "UISleepWnd.h"
#include "../level.h"
#include "../actor.h"
#include "../ActorCondition.h"
#include "../hudmanager.h"
#include "../inventory.h"
#include "UIInventoryUtilities.h"
#include "../WeaponMagazined.h"
#include "UICellItem.h"
#include "UICellItemFactory.h"
#include "UIDragDropListEx.h"
#include "UI3tButton.h"
#include "../grenadelauncher.h"
#include "../CustomOutfit.h"
#include "../silencer.h"
#include "../scope.h"
#include "../script_callback_ex.h"
#include "../grenadelauncher.h"
#include "../game_object_space.h"
#include "../OPFuncs/utils.h"
#include "../../defines.h"
#include "../UIGameSP.h"
#include "../exooutfit.h"

CUICellItem* CUIInventoryWnd::CurrentItem()
{
	return m_pCurrentCellItem;
}

PIItem CUIInventoryWnd::CurrentIItem()
{
	return	m_pCurrentCellItem?static_cast<PIItem>(m_pCurrentCellItem->m_pData) : nullptr;
}


void CUIInventoryWnd::SetItemSelected (CUICellItem* itm)
{
	CUICellItem* curr=CurrentItem();
	if (curr!=nullptr  && curr->GetSelected())
		curr->SetSelected(false);
	if (itm!=nullptr && !itm->GetSelected())	
		itm->SetSelected(true);
}

void CUIInventoryWnd::SetCurrentItem(CUICellItem* itm)
{
	if(m_pCurrentCellItem == itm) return;
	SetItemSelected(itm);
	m_pCurrentCellItem				= itm;
	UIItemInfo.InitItem			(CurrentIItem());
	bool processed=false;
	ClearAllSuitables();

	auto currentIItem=CurrentIItem();
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
			LPCSTR ui_show_prop_box = pSettings->r_string("maingame_ui", "on_cell_after_select");
			luabind::functor<void> functor;
			if (!ai().script_engine().functor(ui_show_prop_box, functor))
			{
				Msg("! ERROR function [%s] not exist for on_cell_after_select callback", ui_show_prop_box);
			}
			else
			{
				try
				{
					functor(this, CurrentItem(), processed);
				}
				catch (...)
				{
					Msg("! ERROR function [%s] cause unknown error.", ui_show_prop_box);
				}
			}
		}
	}
}

void CUIInventoryWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if(pWnd == &UIPropertiesBox &&	msg==PROPERTY_CLICKED)
	{
		ProcessPropertiesBoxClicked	();
	}else 
	if (UIExitButton == pWnd && BUTTON_CLICKED == msg)
	{
		GetHolder()->StartStopMenu			(this,true);
	}

	CUIWindow::SendMessage(pWnd, msg, pData);
}


void CUIInventoryWnd::InitInventory_delayed()
{
	m_b_need_reinit = true;
}

void CUIInventoryWnd::PaintItemFromSlot(u32 slotId)
{
	CUIDragDropListEx* destList = GetListByID(static_cast<IWListTypes>(slotId));
	PIItem _itm = m_pInv->m_slots[slotId].m_pIItem;
	if (_itm && destList)
	{
		CUICellItem* itm = create_cell_item(_itm);
		destList->SetItem(itm);
	}
}

void CUIInventoryWnd::InitInventory() 
{
	CInventoryOwner *pInvOwner	= smart_cast<CInventoryOwner*>(Level().CurrentEntity());
	if(!pInvOwner)				return;

	m_pInv						= &pInvOwner->inventory();
	SetCurrentItem(nullptr);
	UIPropertiesBox.Hide		();
	ClearAllLists				();
	m_pMouseCapturer			= nullptr;

#pragma region put items to slots
	PaintItemFromSlot(DETECTOR_ARTS_SLOT);
	PaintItemFromSlot(DETECTOR_ANOM_SLOT);
	PaintItemFromSlot(APPARATUS_SLOT);
	PaintItemFromSlot(BIODEV_SLOT);
	PaintItemFromSlot(PNV_SLOT);
	PaintItemFromSlot(KNIFE_SLOT);
	PaintItemFromSlot(PISTOL_SLOT);
	PaintItemFromSlot(RIFLE_SLOT);
	PaintItemFromSlot(SHOTGUN_SLOT);

	PIItem _outfit						= m_pInv->m_slots[OUTFIT_SLOT].m_pIItem;
	CUICellItem* outfit					= _outfit ? create_cell_item(_outfit):NULL;
	m_pUIOutfitList->SetItem			(outfit);
#pragma endregion

#pragma region put items to belt
	TIItemContainer::iterator it, it_e;
	for(it=m_pInv->m_belt.begin(),it_e=m_pInv->m_belt.end(); it!=it_e; ++it) 
	{
			CUICellItem* itm = create_cell_item(*it);
			m_pUIBeltList->SetItem(itm);
	}

#pragma endregion

#pragma region put items to ruck
	ruck_list		= m_pInv->m_ruck;
	std::sort		(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);

	int i=1;
	for (it = ruck_list.begin(), it_e = ruck_list.end(); it != it_e; ++it, ++i)
	{
		CUICellItem* itm = create_cell_item(*it);
		m_pUIBagList->SetItem(itm);
	}
	//fake grenade slot
	PIItem _itm								= m_pInv->m_slots[GRENADE_SLOT].m_pIItem;
	if(_itm)
	{
		CUICellItem* itm				= create_cell_item(_itm);
		m_pUIBagList->SetItem			(itm);
	}
#pragma endregion

	UIOutfitInfo.UpdateImmuneView();
	InventoryUtilities::UpdateWeight(UIBagWnd, true);
	m_b_need_reinit = false;
}  

void CUIInventoryWnd::DropCurrentItem(bool b_all)
{

	CActor *pActor			= smart_cast<CActor*>(Level().CurrentEntity());
	if(!pActor)				return;

	if(!b_all && CurrentIItem() && !CurrentIItem()->IsQuestItem())
	{
		SendEvent_Item_Drop		(CurrentIItem());
		SetCurrentItem			(nullptr);
		InventoryUtilities::UpdateWeight			(UIBagWnd, true);
		return;
	}

	if(b_all && CurrentIItem() && !CurrentIItem()->IsQuestItem())
	{
		u32 cnt = CurrentItem()->ChildsCount();

		for(u32 i=0; i<cnt; ++i){
			CUICellItem*	itm				= CurrentItem()->PopChild();
			PIItem			iitm			= static_cast<PIItem>(itm->m_pData);
			SendEvent_Item_Drop				(iitm);
		}

		SendEvent_Item_Drop					(CurrentIItem());
		SetCurrentItem						(nullptr);
		InventoryUtilities::UpdateWeight	(UIBagWnd, true);
		return;
	}
}

//------------------------------------------

bool CUIInventoryWnd::ToSlot(CUICellItem* itm, bool force_place)
{
	CUIDragDropListEx*	old_owner			= itm->OwnerList();
	PIItem	iitem							= static_cast<PIItem>(itm->m_pData);
	u32 _slot								= iitem->GetSlot();

	if(GetInventory()->CanPutInSlot(iitem)){
		CUIDragDropListEx* new_owner		= GetSlotList(_slot);
		if (!new_owner)
			return false;
		if(_slot==GRENADE_SLOT && !new_owner )return true; //fake, sorry (((

		bool result							= GetInventory()->Slot(iitem,true);
		VERIFY								(result);

		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );

		new_owner->SetItem					(i);
	
		//SendEvent_Item2Slot					(iitem);
		//SendEvent_ActivateSlot				(iitem);
		PlaySnd(eInvItemToSlot);
		m_iCurrentActiveSlot = iitem->GetSlot();
		GetInventory()->SetPrevActiveSlot(iitem->GetSlot());
		UIOutfitInfo.UpdateImmuneView();
		InventoryUtilities::UpdateWeight					(UIBagWnd, true);
		return								true;
	}else
	{ // in case slot is busy
		if(!force_place ||  _slot==NO_ACTIVE_SLOT || GetInventory()->m_slots[_slot].m_bPersistent) return false;

		PIItem	_iitem						= GetInventory()->m_slots[_slot].m_pIItem;
		CUIDragDropListEx* slot_list		= GetSlotList(_slot);
		VERIFY								(slot_list->ItemsCount()==1);

		CUICellItem* slot_cell				= slot_list->GetItemIdx(0);
		VERIFY								(slot_cell && (static_cast<PIItem>(slot_cell->m_pData))==_iitem);

		bool result							= ToBag(slot_cell, false);
		VERIFY								(result);
		UIOutfitInfo.UpdateImmuneView();
		InventoryUtilities::UpdateWeight					(UIBagWnd, true);
		return ToSlot						(itm, false);
	}
}

bool CUIInventoryWnd::ToBag(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= static_cast<PIItem>(itm->m_pData);

	if(GetInventory()->CanPutInRuck(iitem))
	{
		CUIDragDropListEx*	old_owner		= itm->OwnerList();
		CUIDragDropListEx*	new_owner		= NULL;
		if(b_use_cursor_pos){
				new_owner					= CUIDragDropListEx::m_drag_item->BackList();
				VERIFY						(new_owner==m_pUIBagList);
		}else
				new_owner					= m_pUIBagList;


		bool result							= GetInventory()->Ruck(iitem);
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		
		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);

		SendEvent_Item2Ruck					(iitem);
		UIOutfitInfo.UpdateImmuneView();
		InventoryUtilities::UpdateWeight					(UIBagWnd, true);
		return true;
	}
	return false;
}

bool CUIInventoryWnd::ToBelt(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= static_cast<PIItem>(itm->m_pData);

	if(GetInventory()->CanPutInBelt(iitem))
	{
		CUIDragDropListEx*	old_owner		= itm->OwnerList();
		CUIDragDropListEx*	new_owner		= nullptr;
		if(b_use_cursor_pos){
				new_owner					= CUIDragDropListEx::m_drag_item->BackList();
				VERIFY						(new_owner==m_pUIBeltList);
		}else
				new_owner					= m_pUIBeltList;

		bool result							= GetInventory()->Belt(iitem);
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		
	//.	UIBeltList.RearrangeItems();
		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);

		SendEvent_Item2Belt					(iitem);
		UIOutfitInfo.UpdateImmuneView();
		InventoryUtilities::UpdateWeight(UIBagWnd, true);
		return								true;
	}
	return									false;
}

void CUIInventoryWnd::AddItemToBag(PIItem pItem)
{
	CUICellItem* itm						= create_cell_item(pItem);
	m_pUIBagList->SetItem					(itm);
}

bool CUIInventoryWnd::OnItemStartDrag(CUICellItem* itm)
{
	return false; //default behaviour
}

bool CUIInventoryWnd::OnItemSelected(CUICellItem* itm)
{
	SetCurrentItem		(itm);
	return				false;
}

bool CUIInventoryWnd::OnItemDrop(CUICellItem* itm)
{
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= CUIDragDropListEx::m_drag_item->BackList();
	if (!old_owner || !new_owner)
		return false;
	CUICellItem* focusedCellItem=new_owner->GetCellContainer()->GetFocusedCellItem();
	CInventoryItem *draggedItem=static_cast<CInventoryItem *>(itm->m_pData);
	CInventoryItem *focusedItem=focusedCellItem!=nullptr ? static_cast<CInventoryItem *>(focusedCellItem->m_pData) : nullptr;

	EListType t_new		= GetType(new_owner);
	EListType t_old		= GetType(old_owner);
	bool processed=false;
	if (t_new == iwBag && t_old == iwBag)
	{
		//����� ��� ������� ������� �� ������
		CWeapon*			weapon				= smart_cast<CWeapon*>			(focusedItem);
		if (weapon)
		{
			CScope*				pScope				= smart_cast<CScope*>			(draggedItem);
			CSilencer*			pSilencer			= smart_cast<CSilencer*>		(draggedItem);
			CGrenadeLauncher*	pGrenadeLauncher	= smart_cast<CGrenadeLauncher*>	(draggedItem);
#pragma region disabled due i do not wont move active item to slot
			//CWeaponAmmo*		pAmmo				= smart_cast<CWeaponAmmo*>		(draggedItem);
#pragma endregion
			m_pUIBagList->m_i_scroll_pos=m_pUIBagList->ScrollPos();
			if ((pScope && weapon->CanAttach(pScope))|| (pSilencer && weapon->CanAttach(pSilencer))|| (pGrenadeLauncher && weapon->CanAttach(pGrenadeLauncher)))
			{
				SetCurrentItem(itm);
				AttachAddon(weapon);
				processed=true;
			} 
#pragma region disabled due i do not wont move active item to slot
			/*else if (pAmmo!=nullptr && weapon->CanLoadAmmo(pAmmo,true)) 
			{
				weapon->LoadAmmo(pAmmo);
				processed=true;
			}*/
#pragma endregion
		}
		CExoOutfit* exo = smart_cast<CExoOutfit*>(focusedItem);
		if (exo && exo->isSuitableBattery(draggedItem->object().cNameSect()))
		{
			exo->PutToBatterySlot(draggedItem);
			processed = true;
		}
		
		if (pSettings->line_exist("maingame_ui", "on_cell_item_drop"))
		{
			CGameObject* GO = smart_cast<CGameObject*>(CurrentIItem());
			if (GO)
			{
				LPCSTR on_cell_item_drop = pSettings->r_string("maingame_ui", "on_cell_item_drop");
				luabind::functor<void> functor;
				if (!ai().script_engine().functor(on_cell_item_drop, functor))
				{
					Msg("! ERROR function [%s] not exist for on_cell_item_drop callback", on_cell_item_drop);
				}
				else
				{
					try
					{
						functor(this, old_owner, new_owner, itm, focusedCellItem, processed);
					}
					catch (...)
					{
						Msg("! ERROR function [%s] cause unknown error.", on_cell_item_drop);
					}
				}
			}
		}

		return true;
	}
	else if (t_new == t_old)
		return true;

	switch(t_new){
		case iwSlot:{
			CUIDragDropListEx *slotForitem=GetSlotList(CurrentIItem()->GetSlot());
			if(slotForitem==new_owner)
					processed=ToSlot	(itm, true);
			else
			{
				if (new_owner == m_pUIOutfitList)
				{
					CExoOutfit* exo = nullptr;
					if (focusedItem)
						exo = smart_cast<CExoOutfit*>(focusedItem);
					else
					{
						CUICellItem* cell = m_pUIOutfitList->GetItemIdx(0);
						if (cell && cell->m_pData)
						{
							PIItem _iitem = static_cast<PIItem>(cell->m_pData);
							exo = smart_cast<CExoOutfit*>(_iitem);
						}
					}
					if (exo && exo->isSuitableBattery(draggedItem->object().cNameSect()))
						exo->PutToBatterySlot(draggedItem);
				}
			}
#pragma region disabled due i do not wont move active item to slot
			/*else
			{
				CWeapon*			weapon				= smart_cast<CWeapon*>			(focusedItem); 
				if (weapon)
				{
					CWeaponAmmo*		pAmmo				= smart_cast<CWeaponAmmo*>		(draggedItem);
					if (pAmmo!=nullptr && weapon->CanLoadAmmo(pAmmo,true))
					{
						weapon->LoadAmmo(pAmmo);
						processed=true;
					}
				}
			}*/
#pragma endregion
		}break;
		case iwBag:{
			processed=ToBag	(itm, true);
		}break;
		case iwBelt:{
			processed=ToBelt	(itm, true);
		}break;
	default: break;
	};

	DropItem				(CurrentIItem(), new_owner);

	if (pSettings->line_exist("maingame_ui", "on_cell_item_drop"))
	{
		CGameObject* GO = smart_cast<CGameObject*>(CurrentIItem());
		if (GO)
		{
			LPCSTR on_cell_item_drop = pSettings->r_string("maingame_ui", "on_cell_item_drop");
			luabind::functor<void> functor;
			if (!ai().script_engine().functor(on_cell_item_drop, functor))
			{
				Msg("! ERROR function [%s] not exist for on_cell_item_drop callback", on_cell_item_drop);
			}
			else
			{
				try
				{
					functor(this, old_owner, new_owner, itm, focusedCellItem, processed);
				}
				catch (...)
				{
					Msg("! ERROR function [%s] cause unknown error.", on_cell_item_drop);
				}
			}
		}
	}
	return true;
}
void CUIInventoryWnd::hideInventoryWnd(CInventoryItem* weapon) 
{
	SetCurrentItem(nullptr);
	CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
	pGameSP->InventoryMenu->GetHolder()->StartStopMenu(pGameSP->InventoryMenu,true);
	m_pInv->m_bForceRecalcAmmos=true;
	if (m_pInv->ActiveItem()!=weapon)
		m_pInv->ProcessSlotAction(true,weapon->GetSlot());
}
bool CUIInventoryWnd::OnItemDbClick(CUICellItem* itm)
{
	CInventoryItem *invItem=static_cast<PIItem>(itm->m_pData);
	if (!invItem)
		return false;
	if(TryUseItem(invItem))
		return true;

	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	EListType t_old						= GetType(old_owner);
	switch(t_old){
		case iwSlot:{
			ToBag	(itm, false);
		}break;
		case iwBag:{
			if(!ToSlot(itm, false))
			{
				if( !ToBelt(itm, false) )
					ToSlot	(itm, true);
			}
			else
				break;

			if (CExoOutfit* exo = smart_cast<CExoOutfit*>(g_actor->GetOutfit()))
				if (exo->isSuitableBattery(itm->GetCellSection()))
				{
					exo->PutToBatterySlot(CurrentIItem());
					break;
				}


			CInventoryItem *pistol=m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
			CInventoryItem *rifle=m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			CInventoryItem *shotgun=m_pInv->m_slots[SHOTGUN_SLOT].m_pIItem;
#pragma region disabled due to hard learning async state machine and playng animation
			/*CWeaponAmmo* ammo=smart_cast<CWeaponAmmo*>(invItem);
			if (ammo)
			{				
				if (pistol != nullptr && pistol->CanLoadAmmo(ammo,true))
				{
					CWeaponMagazined* weapon=smart_cast<CWeaponMagazined*>(pistol);
					hideInventoryWnd(weapon);
					weapon->LoadAmmo(ammo);
					break;
				}
				if (shotgun != nullptr && shotgun->CanLoadAmmo(ammo,true))
				{
					CWeaponMagazined* weapon=smart_cast<CWeaponMagazined*>(shotgun);
					hideInventoryWnd(weapon);
					weapon->LoadAmmo(ammo);
					break;
				}				
				if (rifle != nullptr && rifle->CanLoadAmmo(ammo,true))
				{
					CWeaponMagazined* weapon=smart_cast<CWeaponMagazined*>(rifle);
					hideInventoryWnd(weapon);
					weapon->LoadAmmo(ammo);
					break;
				}
			}*/
#pragma endregion
			CScope*				pScope				= smart_cast<CScope*>			(invItem);
			CSilencer*			pSilencer			= smart_cast<CSilencer*>		(invItem);
			CGrenadeLauncher*	pGrenadeLauncher	= smart_cast<CGrenadeLauncher*> (invItem);
			if (pScope || pSilencer || pGrenadeLauncher)
			{
				pistol=smart_cast<CWeaponMagazined*>(pistol);
				rifle=smart_cast<CWeaponMagazined*>(rifle);
				shotgun=smart_cast<CWeaponMagazined*>(shotgun);
				if (pistol!=nullptr && (pistol->CanAttach(pScope) || pistol->CanAttach(pSilencer) || pistol->CanAttach(pGrenadeLauncher)))
				{
					SetCurrentItem(itm);
					AttachAddon(pistol);
					break;
				}
				if (shotgun!=nullptr && (shotgun->CanAttach(pScope) || shotgun->CanAttach(pSilencer) || shotgun->CanAttach(pGrenadeLauncher)))
				{
					SetCurrentItem(itm);
					AttachAddon(shotgun);
					break;
				}
				if (rifle!=nullptr && (rifle->CanAttach(pScope) || rifle->CanAttach(pSilencer) || rifle->CanAttach(pGrenadeLauncher)))
				{
					SetCurrentItem(itm);
					AttachAddon(rifle);
					break;
				}
			}
		}break;
		case iwBelt:{
			ToBag	(itm, false);
		}break;
		default: break;
	};

	return true;
}

bool CUIInventoryWnd::OnItemFocusReceive(CUICellItem* itm)
{
	if (itm)
		itm->SetFocused(true);
	return						false;
}

bool CUIInventoryWnd::OnItemFocusLost(CUICellItem* itm)
{
	if (itm)
		itm->SetFocused(false);
	return						false;
}


bool CUIInventoryWnd::OnItemRButtonClick(CUICellItem* itm)
{
	SetCurrentItem				(itm);
	ActivatePropertiesBox		();
	return						false;
}

CUIDragDropListEx* CUIInventoryWnd::GetSlotList(u32 slot_idx)
{
	if(slot_idx == NO_ACTIVE_SLOT || GetInventory()->m_slots[slot_idx].m_bPersistent)	return nullptr;
	switch (slot_idx)
	{
		case PNV_SLOT:
			return m_pUIPNVList;
			break;
		case DETECTOR_ANOM_SLOT:
			return m_pUIDetectorAnomsList;
			break;
		case DETECTOR_ARTS_SLOT:
			return m_pUIDetectorArtsList;
			break;
		case APPARATUS_SLOT:
			return m_pUIApparatusList;
			break;
		case BIODEV_SLOT:
			return m_pUIBiodevList;
			break;
		case KNIFE_SLOT:
			return m_pUIKnifeList;
			break;

		case PISTOL_SLOT:
			return m_pUIPistolList;
			break;
		case SHOTGUN_SLOT:
			return m_pUIShotgunList;
			break;
		case RIFLE_SLOT:
			return m_pUIAutomaticList;
			break;

		case OUTFIT_SLOT:
			return m_pUIOutfitList;
			break;

	};
	return nullptr;
}



void CUIInventoryWnd::ClearAllLists()
{
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[](CUIDragDropListEx* list)
	{
		list->GetParent()->SetCapture(nullptr, false);
		list->ClearAll(true);
	});
}