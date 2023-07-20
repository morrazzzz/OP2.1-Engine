#pragma once
#include "UIWindow.h"
#include "../inventory_space.h"
#include "UIStatic.h"
#include "UIMultiTextStatic.h"
#include "UIDragDropListEx.h"
#include "uicharacterinfo.h"
#include "UI3tButton.h"
#include "UIItemInfo.h"
#include "../uigamecustom.h"
#include "UIPropertiesBox.h"
#include "../UIListsManipulations.h"

class CInventoryOwner;
class CEatableItem;
class CTrade;

class CUIDragDropListEx;
class CUICellItem;

class CUITradeWnd: public CUIWindow,public CUIListManipulations
{
private:
	typedef CUIWindow inherited;

public:
						CUITradeWnd					();
	virtual				~CUITradeWnd				();

	virtual void		Init						();

	virtual void		SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);

	void				InitTrade					(CInventoryOwner* pOur, CInventoryOwner* pOthers);
	
	virtual void 		Draw						();
	virtual void 		Update						();
	virtual void 		Show						();
	virtual void 		Hide						();

	void 				DisableAll					();
	void 				EnableAll					();

	void 				SwitchToTalk				();
	void 				StartTrade					();
	void 				StopTrade					();
protected:
	CUIStatic			UIStaticTop;
	CUIStatic			UIStaticBottom;

	CUIStatic			UIOurBagWnd;
	CUIStatic			UIOurMoneyStatic;
	CUIStatic			UIOthersBagWnd;
	CUIStatic			UIOtherMoneyStatic;
	CUIDragDropListEx	UIOurBagList;
	CUIDragDropListEx	UIOthersBagList;

	CUIStatic			UIOurTradeWnd;
	CUIStatic			UIOthersTradeWnd;
	CUIMultiTextStatic	UIOurPriceCaption;
	CUIMultiTextStatic	UIOthersPriceCaption;
	CUIDragDropListEx	UIOurTradeList;
	CUIDragDropListEx	UIOthersTradeList;

	//������
	CUI3tButton			UIPerformTradeButton;
	CUI3tButton			UIToTalkButton;

	//���������� � ���������� 
	CUIStatic			UIOurIcon;
	CUIStatic			UIOthersIcon;
	CUICharacterInfo	UICharacterInfoLeft;
	CUICharacterInfo	UICharacterInfoRight;

	//���������� � ��������������� ��������
	CUIStatic			UIDescWnd;
	CUIItemInfo			UIItemInfo;

	CUIPropertiesBox			UIPropertiesBox;
	void						ProcessPropertiesBoxClicked	();
	void						ActivatePropertiesBox		();
	void						DetachAddon(const char* addon_name);

	SDrawStaticStruct*	UIDealMsg;

	bool				bStarted;
	bool 				ToOurTrade					();
	bool 				ToOthersTrade				();
	bool 				ToOurBag					();
	bool 				ToOthersBag					();
	//void 				SendEvent_ItemDrop			(PIItem pItem);
	
	u32					CalcItemsPrice				(CUIDragDropListEx* pList, CTrade* pTrade, bool bBuying);
	float				CalcItemsWeight				(CUIDragDropListEx* pList);

	void				TransferItems				(CUIDragDropListEx* pSellList, CUIDragDropListEx* pBuyList, CTrade* pTrade, bool bBuying);

	void				PerformTrade				();
	void				UpdatePrices				();
	void				ColorizeItem				(CUICellItem* itm, bool b);

	enum EListType{eNone,e1st,e2nd,eBoth};
	void				UpdateLists					(EListType);

	void				FillList					(TIItemContainer& cont, CUIDragDropListEx& list, bool do_colorize);

	bool				m_bDealControlsVisible;
	void				move_item(CUICellItem* itm, CUIDragDropListEx* from, CUIDragDropListEx* to) ;
	bool				CanMoveToOther				(PIItem pItem);
	void				AddSingleItemToList(CUICellItem* itm,CUIDragDropListEx* to);
	//��������� ������ � ���� � ��� �������
	CInventory*			m_pInv;
	CInventory*			m_pOthersInv;
	CInventoryOwner*	m_pInvOwner;
	CInventoryOwner*	m_pOthersInvOwner;
	CTrade*				m_pTrade;
	CTrade*				m_pOthersTrade;

	u32					m_iOurTradePrice;
	u32					m_iOthersTradePrice;


	CUICellItem*		m_pCurrentCellItem;
	TIItemContainer		ruck_list;


	void				SetCurrentItem				(CUICellItem* itm);
	void				SetItemSelected				(CUICellItem* itm);
	CUICellItem*		CurrentItem					();
	PIItem				CurrentIItem				();

	bool		xr_stdcall		OnItemDrop			(CUICellItem* itm);
	bool		xr_stdcall		OnItemStartDrag		(CUICellItem* itm);
	bool		xr_stdcall		OnItemDbClick		(CUICellItem* itm);
	bool		xr_stdcall		OnItemSelected		(CUICellItem* itm);
	bool		xr_stdcall		OnItemRButtonClick	(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusLost		(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusReceive	(CUICellItem* itm);

	bool			OnMouse						(float x, float y, EUIMessages mouse_action) override;

	void						SendEvent_Item_Drop			(PIItem	pItem);

	void						BindDragDropListEvents		(CUIDragDropListEx* lst);
	void						UpdateItemUICost(CUICellItem* cellItem);

	enum eTradeSoundActions{	eInvSndOpen	=0,
								eInvSndClose,
								eInvProperties,
								eInvDropItem,
								//eInvAttachAddon,
								eInvDetachAddon,
								eInvItemUse,
								eInvTradeDone,
								eInvSndMax};
	ref_sound					sounds					[eInvSndMax];

public:
	void PlaySnd(eTradeSoundActions a);
	void						re_init();
};