#pragma once
#pragma once
#include <string>
#include "../UIListsManipulations.h"

class CInventory;

#include "UIDialogWnd.h"
#include "UIStatic.h"

#include "UIProgressBar.h"

#include "UIPropertiesBox.h"
#include "UIOutfitSlot.h"

#include "UIOutfitInfo.h"
#include "UIItemInfo.h"
#include "../inventory_space.h"
#include "../WeaponAmmo.h"




class CArtefact;
class CUI3tButton;
class CUIDragDropListEx;
class CUICellItem;

class CUIInventoryWnd: public CUIDialogWnd,public CUIListManipulations
{
private:
	typedef CUIDialogWnd	inherited;
	bool					m_b_need_reinit;
	void PaintItemFromSlot(u32 slotId);
	bool m_bKeyControlPress;
	PIItem m_pLastWorkeditem;
public:
							CUIInventoryWnd				();
	virtual					~CUIInventoryWnd			();

	virtual void			Init						();
	void					re_init						();
	void					re_init2					();
	void					InitInventory				();
	void					InitInventory_delayed		();
	virtual bool			StopAnyMove					()					{return false;}

	virtual void			SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);
	virtual bool			OnMouse						(float x, float y, EUIMessages mouse_action);
	virtual bool			OnKeyboard					(int dik, EUIMessages keyboard_action);


	IC CInventory*			GetInventory				()					{return m_pInv;}

	virtual void			Update						();
	virtual void			Draw						();

	virtual void			Show						();
	virtual void			Hide						();

	void					AddItemToBag				(PIItem pItem);
	enum eInventorySndAction {
		eInvSndOpen = 0,
		eInvSndClose,
		eInvItemToSlot,
		eInvItemToBelt,
		eInvItemToRuck,
		eInvProperties,
		eInvDropItem,
		eInvAttachAddon,
		eInvDetachAddon,
		eInvItemUse,
		eInvSndMax
	};
	void						PlaySnd(eInventorySndAction a);
	ref_sound* GetSound(eInventorySndAction a);
protected:
	ref_sound					sounds					[eInvSndMax];

	CUIStatic					UIBeltSlots;
	CUIStatic					UIBack;
	CUIStatic*					UIRankFrame;
	CUIStatic*					UIRank;

	CUIStatic					UIBagWnd;
	CUIStatic					UIMoneyWnd;
	CUIStatic					UIDescrWnd;
	CUIFrameWindow				UIPersonalWnd;

	CUI3tButton*				UIExitButton;

	CUIStatic					UIStaticBottom;
	CUIStatic					UIStaticTime;
	CUIStatic					UIStaticTimeString;

	CUIStatic					UIStaticPersonal;
		
	CUIDragDropListEx*			m_pUIBagList;
	CUIDragDropListEx*			m_pUIBeltList;
	CUIDragDropListEx*			m_pUIKnifeList;
	CUIDragDropListEx*			m_pUIPistolList;
	CUIDragDropListEx*			m_pUIAutomaticList;
	CUIDragDropListEx*			m_pUIShotgunList;
	CUIDragDropListEx*			m_pUIDetectorArtsList;
	CUIDragDropListEx*			m_pUIDetectorAnomsList;
	CUIDragDropListEx*			m_pUIPNVList;
	CUIDragDropListEx*			m_pUIApparatusList;
	CUIDragDropListEx*			m_pUIBiodevList;
	CUIOutfitDragDropList*		m_pUIOutfitList;


	void						ClearAllLists				();
	void						BindDragDropListEvents		(CUIDragDropListEx* lst);
	
	EListType					GetType						(CUIDragDropListEx* l) const;
	CUIDragDropListEx*			GetSlotList					(u32 slot_idx);

	bool		xr_stdcall		OnItemDrop					(CUICellItem* itm);
	bool		xr_stdcall		OnItemStartDrag				(CUICellItem* itm);
	bool		xr_stdcall		OnItemDbClick				(CUICellItem* itm);
	bool		xr_stdcall		OnItemSelected				(CUICellItem* itm);
	bool		xr_stdcall		OnItemRButtonClick			(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusLost				(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusReceive			(CUICellItem* itm);


	CUIStatic					UIProgressBack;
	CUIStatic					UIProgressBack_rank;

	CUIProgressBar				UIProgressBarHealth;	
	CUIProgressBar				UIProgressBarPsyHealth;
	CUIProgressBar				UIProgressBarRadiation;
	CUIProgressBar				UIProgressBarRank;

	CUIPropertiesBox			UIPropertiesBox;
	
	//���������� � ���������
	CUIOutfitInfo				UIOutfitInfo;
	CUIItemInfo					UIItemInfo;

	CInventory*					m_pInv;

	CUICellItem*				m_pCurrentCellItem;

	void hideInventoryWnd(CInventoryItem* weapon) ;

	bool						DropItem					(PIItem itm, CUIDragDropListEx* lst);
	bool						TryUseItem					(PIItem itm);
	//----------------------	-----------------------------------------------
	void						SendEvent_Item2Slot			(PIItem	pItem);
	void						SendEvent_Item2Belt			(PIItem	pItem);
	void						SendEvent_Item2Ruck			(PIItem	pItem);
	void						SendEvent_Item_Drop			(PIItem	pItem);
	void						SendEvent_Item_Eat			(PIItem	pItem);
	void						SendEvent_ActivateSlot		(PIItem	pItem);

	//---------------------------------------------------------------------

	void						ProcessPropertiesBoxClicked	();
	void						ActivatePropertiesBox		();
	void						AddWeaponAttachInfo(u32 slot,CInventoryItem* addon,std::string titleId,bool& needShow);
	void						AddLoadAmmoWeaponInfo(u32 slot,CWeaponAmmo* ammo,bool& needShow);
	void						DropCurrentItem				(bool b_all);
	void						EatItem						(PIItem itm);
	
	bool						ToSlot						(CUICellItem* itm, bool force_place);
	bool						ToBag						(CUICellItem* itm, bool b_use_cursor_pos);
	bool						ToBelt						(CUICellItem* itm, bool b_use_cursor_pos);

	void						AttachAddon					(PIItem item_to_upgrade);
	void						DetachAddon					(const char* addon_name);
	void						SetCurrentItem				(CUICellItem* itm);
	CUICellItem*				CurrentItem					();
	PIItem						CurrentIItem				();
	void						SetItemSelected				(CUICellItem* itm);

	TIItemContainer				ruck_list;
	u32							m_iCurrentActiveSlot;

};

