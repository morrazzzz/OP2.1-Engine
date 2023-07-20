#include "stdafx.h"
#include "UICellItem.h"
#include "../xr_level_controller.h"
#include "../../xr_input.h"
#include "../HUDManager.h"
#include "../level.h"
#include "../object_broker.h"
#include "UIDragDropListEx.h"
#include "../Weapon.h"
#include "../Artifact.h"
#include "../CustomOutfit.h"
#include "../../defines.h"
#include "../smart_cast.h"
#include "UITradeWnd.h"
#include "../physic_item.h"
#include "../OPFuncs/utils.h"
#include "UICellCustomItems.h"

CUICellItem::CUICellItem()
{
	m_pParentList		= nullptr;
	m_pData				= nullptr;
	m_custom_draw		= nullptr;
	m_b_already_drawn	= false;
	m_moveableToOther=true;
	m_drawn_frame=0;
	SetAccelerator		(0);
	m_b_destroy_childs	= true;
	m_focused=false;
	m_selected=false;
	m_allowedGrouping=true;
	m_suitable=false;
	m_bIgnoreItemPlace = false;
	m_pConditionProgressBar=xr_new<CUIProgressBar>();
	m_pConditionProgressBar->SetAutoDelete(true);
	m_fSectionCondition = -1.0f;
	m_bForceConditionShow=false;
	CUIWindow::AttachChild(m_pConditionProgressBar);
}

CUICellItem::~CUICellItem()
{

	if(m_b_destroy_childs)
		delete_data	(m_childs);

	delete_data		(m_custom_draw);
}


LPCSTR CUICellItem::GetCellSection()
{
	if (m_pData)
	{
		PIItem itm = static_cast<PIItem>(m_pData);
		if (itm)
			return itm->object().cNameSect().c_str();
	}
	if (m_sSection.size() > 0)
		return m_sSection.c_str();
	return nullptr;
}

void CUICellItem::SetCellCondition(float value)
{
	clamp(value,0.0f,1.0f);
	if (m_pData)
	{
		PIItem itm = static_cast<PIItem>(m_pData);
		if (itm)
		{
			float deltaCondition= value - itm->GetCondition();
			itm->ChangeCondition(deltaCondition);
		}
	}
	else if (m_sSection.size() > 0)
		m_fSectionCondition=value;
	if (m_pConditionProgressBar->GetVisible())
		m_pConditionProgressBar->SetProgressPos(value * 100);
}

float CUICellItem::GetAbsCellCondition() //always get 
{
	if (m_pData)
	{
		PIItem itm = static_cast<PIItem>(m_pData);
		if (itm)
			return itm->GetCondition();
	}
	if (m_sSection.size() > 0)
		return m_fSectionCondition;
	return -1.0f;
}

float CUICellItem::GetCellCondition() //only for showing
{
	float result = -1.0f;
	CInventoryItem* itm = (CInventoryItem*)m_pData;
	if (itm && (itm->m_bUIConditionShow || m_bForceConditionShow))
		result = itm->GetCondition();
	else if (m_sSection.size() > 0 && !fsimilar(m_fSectionCondition, -1.0f, EPS))
		result = m_fSectionCondition;
	return result;
}

void CUICellItem::Draw()
{
	m_drawn_frame=Device.dwFrame;
	m_b_already_drawn		= true;
	inherited::Draw();
	if(m_custom_draw) 
		m_custom_draw->OnDraw(this);
};

bool CUICellItem::OnMouse(float x, float y, EUIMessages mouse_action)
{
	if(mouse_action == WINDOW_LBUTTON_DOWN){
		GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_SELECTED, nullptr);
		return false;
	}else
	if(mouse_action == WINDOW_MOUSE_MOVE && pInput->iGetAsyncBtnState(0)){
		GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_DRAG, nullptr);
		return true;
	}else
	if(mouse_action==WINDOW_LBUTTON_DB_CLICK){
		GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_DB_CLICK, nullptr);
		return true;
	}else
	if(mouse_action==WINDOW_RBUTTON_DOWN){
		GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_RBUTTON_CLICK, nullptr);
		return true;
	}
	
	return false;
};

bool CUICellItem::OnKeyboard(int dik, EUIMessages keyboard_action)
{
	if (WINDOW_KEY_PRESSED == keyboard_action)
	{
		if (GetAccelerator() == dik)
		{
			GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_DB_CLICK, nullptr);
			return		true;
		}
	}
	return inherited::OnKeyboard(dik, keyboard_action);
}

CUIDragItem* CUICellItem::CreateDragItem()
{
	CUIDragItem* tmp;
	tmp = xr_new<CUIDragItem>(this);
	Frect r;
	GetAbsoluteRect(r);
	tmp->Init(GetShader(),r,GetUIStaticItem().GetOriginalRect());
	return tmp;
}

bool CUICellItem::SetOwnerList(CUIDragDropListEx* p)	
{
	m_pParentList=p;
	if (!m_pParentList/* || !m_pData*/)
		return false;
	float condition = GetCellCondition();
	bool visible = !fsimilar(condition, -1.0f, EPS);
	if (visible && m_pParentList->GetShowConditionBar() && g_uCommonFlags.test(E_COMMON_FLAGS::uiShowConditions))
	{
		int x_size= GetGridSize().x * m_pParentList->CellSize().x;
		int y_size= GetGridSize().y * m_pParentList->CellSize().y;
		float x,y,width,height;
		bool isHorizontal;
		switch (m_pParentList->cacheData.position)
		{
		case left: 
			{
				x=m_pParentList->cacheData.indent;
				y=m_pParentList->cacheData.indent;
				width=m_pParentList->cacheData.fatness;
				height=y_size-m_pParentList->cacheData.indent*2;
				isHorizontal=false;
			}
			break;
		case right: 
			{
				x=x_size-m_pParentList->cacheData.fatness-m_pParentList->cacheData.indent;
				y=m_pParentList->cacheData.indent;
				width=m_pParentList->cacheData.fatness;
				height=y_size-m_pParentList->cacheData.indent*2;
				isHorizontal=false;
			}break;
		case top: 
			{
				x=m_pParentList->cacheData.indent;
				y=m_pParentList->cacheData.indent;
				width=x_size-m_pParentList->cacheData.indent*2;
				height=m_pParentList->cacheData.fatness;
				isHorizontal=true;
			}
			break;
		case bottom: 
			{
				x=m_pParentList->cacheData.indent;
				y=y_size-m_pParentList->cacheData.fatness-m_pParentList->cacheData.indent;
				width=x_size-m_pParentList->cacheData.indent*2;
				height=m_pParentList->cacheData.fatness;
				isHorizontal=true;
			}
			break;
		case left_top: 
			{
				if (x_size>=y_size)
				{
					x=m_pParentList->cacheData.indent;
					y=m_pParentList->cacheData.indent;
					width=x_size-m_pParentList->cacheData.indent*2;
					height=m_pParentList->cacheData.fatness;
					isHorizontal=true;
				}
				else
				{
					x=m_pParentList->cacheData.indent;
					y=m_pParentList->cacheData.indent;
					width=m_pParentList->cacheData.fatness;
					height=y_size-m_pParentList->cacheData.indent*2;
					isHorizontal=false;
				}
			}
			break;
		case right_top:
			{
				if (x_size>=y_size)
				{
					x=m_pParentList->cacheData.indent;
					y=m_pParentList->cacheData.indent;
					width=x_size-m_pParentList->cacheData.indent*2;
					height=m_pParentList->cacheData.fatness;
					isHorizontal=true;
				}
				else
				{
					x=x_size-m_pParentList->cacheData.fatness-m_pParentList->cacheData.indent;
					y=m_pParentList->cacheData.indent;
					width=m_pParentList->cacheData.fatness;
					height=y_size-m_pParentList->cacheData.indent*2;
					isHorizontal=false;
				}
			}
			break;
		case right_bottom: 
			{
				if (x_size>=y_size)
				{
					x=m_pParentList->cacheData.indent;
					y=y_size-m_pParentList->cacheData.fatness-m_pParentList->cacheData.indent;
					width=x_size-m_pParentList->cacheData.indent*2;
					height=m_pParentList->cacheData.fatness;
					isHorizontal=true;
				}
				else
				{
					x=x_size-m_pParentList->cacheData.fatness-m_pParentList->cacheData.indent;
					y=m_pParentList->cacheData.indent;
					width=m_pParentList->cacheData.fatness;
					height=y_size-m_pParentList->cacheData.indent*2;
					isHorizontal=false;
				}
			}
			break;
		case left_bottom: 
		default:
			{
				if (x_size>=y_size)
				{
					x=m_pParentList->cacheData.indent;
					y=y_size-m_pParentList->cacheData.fatness-m_pParentList->cacheData.indent;
					width=x_size-m_pParentList->cacheData.indent*2;
					height=m_pParentList->cacheData.fatness;
					isHorizontal=true;
				}
				else
				{
					x=m_pParentList->cacheData.indent;y=m_pParentList->cacheData.indent;
					width=m_pParentList->cacheData.fatness;
					height=y_size-m_pParentList->cacheData.indent;
					isHorizontal=false;
				}
			}
			break;
		}
		
		if (m_pParentList->cacheData.fixed_cells>0)
		{
			float fixed_x_size= m_pParentList->cacheData.fixed_cells * m_pParentList->CellSize().x;
			float fixed_y_size= m_pParentList->cacheData.fixed_cells * m_pParentList->CellSize().y;
			if (isHorizontal)
				if ((x_size+m_pParentList->cacheData.indent*2)>fixed_x_size)
					width=fixed_x_size-m_pParentList->cacheData.indent*2;
				else
					width=x_size-m_pParentList->cacheData.indent*2;
			else
				if ((y_size+m_pParentList->cacheData.indent*2)>fixed_y_size)
					height=y_size-m_pParentList->cacheData.indent*2;
				else
					height=y_size-m_pParentList->cacheData.indent*2;
		}
		m_pConditionProgressBar->SetWndPos(x,y);
		m_pConditionProgressBar->SetWndRect(x,y,width,height);		
		m_pConditionProgressBar->SetRange(0,100);
		m_pConditionProgressBar->SetOrientation(isHorizontal);


		m_pConditionProgressBar->m_UIProgressItem.InitTexture(m_pParentList->cacheData.texture.c_str());
		m_pConditionProgressBar->m_UIProgressItem.SetStretchTexture(m_pParentList->cacheData.stretchTexture);
		m_pConditionProgressBar->m_UIProgressItem.TextureAvailable(true);
		m_pConditionProgressBar->m_UIProgressItem.TextureOn();
		m_pConditionProgressBar->m_UIProgressItem.SetWidth(width);
		m_pConditionProgressBar->m_UIProgressItem.SetHeight(height); 

		if (m_pParentList->cacheData.textureBack.size()>0)
		{
			m_pConditionProgressBar->m_UIBackgroundItem.InitTexture(m_pParentList->cacheData.textureBack.c_str());
			m_pConditionProgressBar->m_UIBackgroundItem.SetStretchTexture(m_pParentList->cacheData.stretchTextureBack);
			m_pConditionProgressBar->m_UIBackgroundItem.TextureAvailable(true);
			m_pConditionProgressBar->m_UIBackgroundItem.TextureOn();
			m_pConditionProgressBar->m_UIBackgroundItem.SetWidth(width);
			m_pConditionProgressBar->m_UIBackgroundItem.SetHeight(height);
			m_pConditionProgressBar->SetBackgroundPresent(true);
		}
		// m_pConditionProgressBar->m_minColor.set(color_rgba(255,36,0,255));
		// m_pConditionProgressBar->m_maxColor.set(color_rgba(0,255,0,255));
		m_pConditionProgressBar->m_minColor.set(m_pParentList->cacheData.min_color);
		m_pConditionProgressBar->m_maxColor.set(m_pParentList->cacheData.max_color);
		m_pConditionProgressBar->m_bUseColor=true;
		m_pConditionProgressBar->SetProgressPos(condition*100);

	}
	m_pConditionProgressBar->Show(visible);
	return visible;
}

bool CUICellItem::EqualTo(CUICellItem* itm)
{
	return (m_grid_size.x==itm->GetGridSize().x) && (m_grid_size.y==itm->GetGridSize().y);
}

u32 CUICellItem::ChildsCount() const
{
	return m_childs.size();
}

void CUICellItem::PushChild(CUICellItem* c)
{
	R_ASSERT(c->ChildsCount()==0);
	VERIFY				(this!=c);
	m_childs.push_back	(c);
	UpdateItemText		();
}

CUICellItem* CUICellItem::PopChild()
{
	CUICellItem* itm	= m_childs.back();
	m_childs.pop_back	();
	std::swap			(itm->m_pData, m_pData);
	UpdateItemText		();
	R_ASSERT			(itm->ChildsCount()==0);
	itm->SetOwnerList	(nullptr);
	return				itm;
}

bool CUICellItem::HasChild(CUICellItem* item)
{
	return (m_childs.end() != std::find(m_childs.begin(), m_childs.end(), item) );
}

void CUICellItem::UpdateItemText()
{
	string32			str;
	if(ChildsCount())
		sprintf_s				(str,"x%d",ChildsCount()+1);
	else
		sprintf_s				(str,"");

	SetText				(str);
}

void CUICellItem::SetCustomDraw			(ICustomDrawCell* c){
	if (m_custom_draw)
		xr_delete(m_custom_draw);
	m_custom_draw = c;
}

void CUICellItem::SaveColors()
{
	m_preAnimTexColor.set(GetTextureColor());
	m_preAnimTextColor.set(GetTextColor());
}

void CUICellItem::RestoreColors()
{
	SetTextureColor(m_preAnimTexColor.get());
	SetTextColor(m_preAnimTextColor.get());
}

void CUICellItem::SetSelected(bool value)
{
	m_selected = value;
	if (m_selected)
	{
		if (m_pParentList && m_pParentList->GetSelectedCell() != this)
			m_pParentList->SetSelectedCell(this);
	}
	else
		if (m_pParentList && m_pParentList->GetSelectedCell() == this)
			m_pParentList->SetSelectedCell(nullptr);
}

CUIDragItem::CUIDragItem(CUICellItem* parent)
{
	m_back_list						= nullptr;
	m_pParent						= parent;
	CUIWindow::AttachChild			(&m_static);
	Device.seqRender.Add			(this, REG_PRIORITY_LOW-5000);
	Device.seqFrame.Add				(this, REG_PRIORITY_LOW-5000);
	VERIFY							(m_pParent->GetMessageTarget());
}

CUIDragItem::~CUIDragItem()
{
	Device.seqRender.Remove			(this);
	Device.seqFrame.Remove			(this);
}

void CUIDragItem::Init(const ref_shader& sh, const Frect& rect, const Frect& text_rect)
{
	SetWndRect						(rect);
	m_static.SetShader				(sh);
	m_static.SetOriginalRect		(text_rect);
	m_static.SetWndPos				(0.0f,0.0f);
	m_static.SetWndSize				(GetWndSize());
	m_static.TextureAvailable		(true);
	m_static.TextureOn				();
	m_static.SetColor				(color_rgba(255,255,255,170));
	m_static.SetStretchTexture		(true);
	m_pos_offset.sub				(rect.lt, GetUICursor()->GetCursorPosition());
}

bool CUIDragItem::OnMouse(float x, float y, EUIMessages mouse_action)
{
	if(mouse_action == WINDOW_LBUTTON_UP)
	{
		m_pParent->GetMessageTarget()->SendMessage(m_pParent,DRAG_DROP_ITEM_DROP,nullptr);
		return true;
	}
	return false;
}

void CUIDragItem::OnRender()
{
	Draw			();
}

void CUIDragItem::OnFrame()
{
	Update			();
}

void CUIDragItem::Draw()
{
	Fvector2 tmp;
	tmp.sub					(GetWndPos(), GetUICursor()->GetCursorPosition());
	tmp.sub					(m_pos_offset);
	tmp.mul					(-1.0f);
	MoveWndDelta			(tmp);
	UI()->PushScissor		(UI()->ScreenRect(),true);

	inherited::Draw();

	UI()->PopScissor();
}

void CUIDragItem::SetBackList(CUIDragDropListEx*l)
{
	if(m_back_list!=l){
		m_back_list=l;
	}
}

Fvector2 CUIDragItem::GetPosition()
{
	return Fvector2().add(m_pos_offset, GetUICursor()->GetCursorPosition());
}

