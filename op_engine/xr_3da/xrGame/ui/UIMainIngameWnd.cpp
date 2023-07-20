#include "stdafx.h"
#include <functional> 
#include "UIMainIngameWnd.h"
#include "UIMessagesWindow.h"
#include "../UIZoneMap.h"

#include <dinput.h>
#include "../actor.h"
#include "../HUDManager.h"
#include "../PDA.h"
#include "../WeaponHUD.h"
#include "../character_info.h"
#include "../inventory.h"
#include "../UIGameSP.h"
#include "../weaponmagazined.h"
#include "../missile.h"
#include "../Grenade.h"
#include "../xrServer_objects_ALife.h"
#include "../alife_simulator.h"
#include "../alife_object_registry.h"
#include "../game_cl_base.h"
#include "../level.h"
#include "../seniority_hierarchy_holder.h"

#include "../date_time.h"
#include "../xrServer_Objects_ALife_Monsters.h"
#include "../../LightAnimLibrary.h"

#include "UIInventoryUtilities.h"


#include "UIXmlInit.h"
#include "UIPdaMsgListItem.h"
#include "../alife_registry_wrappers.h"
#include "../actorcondition.h"

#include "../string_table.h"
#include "../clsid_game.h"
#include "UIArtefactPanel.h"
#include "../../CustomHUD.h"
#include <cmath>

#ifdef DEBUG
#	include "../attachable_item.h"
#	include "../../xr_input.h"
#endif

#include "UIScrollView.h"
#include "map_hint.h"
#include "UIColorAnimatorWrapper.h"
#include "../game_news.h"
#include "UICellCustomItems.h"
#include "../../defines.h"
#include <stdlib.h>
#include "../exooutfit.h"


#ifdef DEBUG
#	include "../debug_renderer.h"

void test_draw	();
void test_key	(int dik);
void test_update();
#endif


using namespace InventoryUtilities;

//	hud adjust mode
int			g_bHudAdjustMode			= 0;
float		g_fHudAdjustValue			= 0.0f;

const u32	g_clWhite					= 0xffffffff;

#define		DEFAULT_MAP_SCALE			1.f

#define		C_SIZE						0.025f
#define		NEAR_LIM					0.5f

#define		SHOW_INFO_SPEED				0.5f
#define		HIDE_INFO_SPEED				10.f
#define		C_ON_ENEMY					D3DCOLOR_XRGB(0xff,0,0)
#define		C_DEFAULT					D3DCOLOR_XRGB(0xff,0xff,0xff)

#define				MAININGAME_XML				"maingame.xml"

CUIMainIngameWnd::CUIMainIngameWnd()
{
	m_pActor = nullptr;
	m_pWeapon = nullptr;
	m_pGrenade = nullptr;
	m_pItem = nullptr;
	UIZoneMap = xr_new<CUIZoneMap>();
	m_pPickUpItem = nullptr;
	m_artefactPanel = xr_new<CUIArtefactPanel>();
	m_pMPChatWnd = nullptr;
	m_pMPLogWnd = nullptr;
	m_UIIcons = nullptr;
}

#include "UIProgressShape.h"
extern CUIProgressShape* g_MissileForceShape;

CUIMainIngameWnd::~CUIMainIngameWnd()
{
	DestroyFlashingIcons		();
	xr_delete					(UIZoneMap);
	xr_delete					(m_artefactPanel);
	HUD_SOUND::DestroySound		(m_contactSnd);
	xr_delete					(g_MissileForceShape);
}

void CUIMainIngameWnd::Init()
{
	CUIXml						uiXml;
	uiXml.Init(CONFIG_PATH, UI_PATH, MAININGAME_XML);

	CUIXmlInit					xml_init;
	CUIWindow::Init(0, 0, UI_BASE_WIDTH, UI_BASE_HEIGHT);

	Enable(false);

	AttachChild(&UIStaticRadiation);
	xml_init.InitStatic(uiXml, "static_radiation", 0, &UIStaticRadiation);
	UIStaticRadiation.AttachChild(&UIRadiationBar);
	xml_init.InitProgressBar(uiXml, "progress_bar_radiation", 0, &UIRadiationBar);
	AttachChild(&UIStaticRadiationDanger);
	xml_init.InitStatic(uiXml, "static_radiation_danger", 0, &UIStaticRadiationDanger);
	UIStaticRadiationDanger.SetTextST("st_radiation_danger");
	UIStaticRadiationDanger.SetVisible(false);

	//static_hud_time

	AttachChild(&UIStaticHealth);
	xml_init.InitStatic(uiXml, "static_health", 0, &UIStaticHealth);

	AttachChild(&UIStaticArmor);
	xml_init.InitStatic(uiXml, "static_armor", 0, &UIStaticArmor);

	AttachChild(&UIWeaponBack);
	xml_init.InitStatic(uiXml, "static_weapon", 0, &UIWeaponBack);

	UIWeaponBack.AttachChild(&UIWeaponSignAmmo);
	xml_init.InitStatic(uiXml, "static_ammo", 0, &UIWeaponSignAmmo);
	UIWeaponSignAmmo.SetElipsis(CUIStatic::eepEnd, 2);

	UIWeaponBack.AttachChild(&UIWeaponIcon);
	xml_init.InitStatic(uiXml, "static_wpn_icon", 0, &UIWeaponIcon);
	UIWeaponIcon.SetShader(GetEquipmentIconsShader());
	UIWeaponIcon_rect = UIWeaponIcon.GetWndRect();
	//---------------------------------------------------------
	AttachChild(&UIPickUpItemIcon);
	xml_init.InitStatic(uiXml, "pick_up_item", 0, &UIPickUpItemIcon);
	UIPickUpItemIcon.SetShader(GetEquipmentIconsShader());
	UIPickUpItemIcon.ClipperOn();

	m_iPickUpItemIconWidth = UIPickUpItemIcon.GetWidth();
	m_iPickUpItemIconHeight = UIPickUpItemIcon.GetHeight();
	m_iPickUpItemIconX = UIPickUpItemIcon.GetWndRect().left;
	m_iPickUpItemIconY = UIPickUpItemIcon.GetWndRect().top;
	//---------------------------------------------------------


	UIWeaponIcon.Enable(false);

	//AttachChild(&UIStaticHudTime);
	xml_init.InitStatic(uiXml, "static_hud_time", 0, &UIStaticHudTime);
	formatTimeString = uiXml.ReadAttrib("static_hud_time", 0, "format", "");
	UIStaticHudTime.SetText(formatTimeString.c_str());


	//���������� 
	UIZoneMap->Init();
	UIZoneMap->SetScale(DEFAULT_MAP_SCALE);

	if (IsGameTypeSingle())
	{
		xml_init.InitStatic(uiXml, "static_pda_online", 0, &UIPdaOnline);
		UIZoneMap->Background().AttachChild(&UIPdaOnline);
		UIZoneMap->Background().AttachChild(&UIStaticHudTime);

	}


	//������ ��������� ��������
	UIStaticHealth.AttachChild(&UIHealthBar);
	//.	xml_init.InitAutoStaticGroup(uiXml,"static_health", &UIStaticHealth);
	xml_init.InitProgressBar(uiXml, "progress_bar_health", 0, &UIHealthBar);

	//������ ��������� ������
	UIStaticArmor.AttachChild(&UIArmorBar);
	//.	xml_init.InitAutoStaticGroup(uiXml,"static_armor", &UIStaticArmor);
	xml_init.InitProgressBar(uiXml, "progress_bar_armor", 0, &UIArmorBar);



	// ���������, ������� ��������� ��� ��������� ������� �� ������
	AttachChild(&UIStaticQuickHelp);
	xml_init.InitStatic(uiXml, "quick_info", 0, &UIStaticQuickHelp);

	uiXml.SetLocalRoot(uiXml.GetRoot());

	m_UIIcons = xr_new<CUIScrollView>(); m_UIIcons->SetAutoDelete(true);
	xml_init.InitScrollView(uiXml, "icons_scroll_view", 0, m_UIIcons);
	AttachChild(m_UIIcons);

	// ��������� ������ 
	if (IsGameTypeSingle())
	{
		xml_init.InitStatic(uiXml, "starvation_static", 0, &UIStarvationIcon);
		UIStarvationIcon.Show(false);

		xml_init.InitStatic(uiXml, "psy_health_static", 0, &UIPsyHealthIcon);
		UIPsyHealthIcon.Show(false);
	}

	xml_init.InitStatic(uiXml, "weapon_jammed_static", 0, &UIWeaponJammedIcon);
	UIWeaponJammedIcon.Show(false);

	xml_init.InitStatic(uiXml, "radiation_static", 0, &UIRadiaitionIcon);
	UIRadiaitionIcon.Show(false);

	xml_init.InitStatic(uiXml, "wound_static", 0, &UIWoundIcon);
	UIWoundIcon.Show(false);

	xml_init.InitStatic(uiXml, "invincible_static", 0, &UIInvincibleIcon);
	UIInvincibleIcon.Show(false);

	xml_init.InitStatic(uiXml, "weight_current_static", 0, &UIWeightIcon);
	UIWeightIcon.Show(false);

	xml_init.InitStatic(uiXml, "exo_charge_static", 0, &UIExoCharge);
	UIExoCharge.Show(false);

	if (GameID() == GAME_ARTEFACTHUNT) {
		xml_init.InitStatic(uiXml, "artefact_static", 0, &UIArtefactIcon);
		UIArtefactIcon.Show(false);
	}

	shared_str warningStrings[8] =
	{
		"jammed",
		"radiation",
		"wounds",
		"starvation",
		"fatigue",
		"weight",
		"exo_charge",
		"invincible"
	};

	// ��������� ��������� �������� ��� �����������
	EWarningIcons j = ewiWeaponJammed;
	while (j < ewiInvincible)
	{
		// ������ ������ ������� ��� ������� ����������
		shared_str cfgRecord = pSettings->r_string("main_ingame_indicators_thresholds", *warningStrings[static_cast<int>(j) - 1]);
		u32 count = _GetItemCount(*cfgRecord);

		char	singleThreshold[8];
		float	f = 0;
		for (u32 k = 0; k < count; ++k)
		{
			_GetItem(*cfgRecord, k, singleThreshold);
			sscanf(singleThreshold, "%f", &f);

			m_Thresholds[j].push_back(f);
		}

		j = static_cast<EWarningIcons>(j + 1);
	}


	// Flashing icons initialize
	uiXml.SetLocalRoot(uiXml.NavigateToNode("flashing_icons"));
	InitFlashingIcons(&uiXml);

	uiXml.SetLocalRoot(uiXml.GetRoot());

	AttachChild(&UICarPanel);
	xml_init.InitWindow(uiXml, "car_panel", 0, &UICarPanel);

	AttachChild(&UIMotionIcon);
	UIMotionIcon.Init();

	if (IsGameTypeSingle())
	{
		m_artefactPanel->InitFromXML(uiXml, "artefact_panel", 0);
		this->AttachChild(m_artefactPanel);
		if (Actor() && Level().CurrentViewEntity() && Level().CurrentViewEntity() == Actor())
			m_artefactPanel->InitIcons(Actor()->GetArtifactsOnBelt());
	}

	AttachChild(&UIStaticDiskIO);
	UIStaticDiskIO.SetWndRect(1000, 750, 16, 16);
	UIStaticDiskIO.GetUIStaticItem().SetRect(0, 0, 16, 16);
	UIStaticDiskIO.InitTexture("ui\\ui_disk_io");
	UIStaticDiskIO.SetOriginalRect(0, 0, 32, 32);
	UIStaticDiskIO.SetStretchTexture(true);


	HUD_SOUND::LoadSound("maingame_ui", "snd_new_contact", m_contactSnd, SOUND_TYPE_IDLE);
}

void CUIMainIngameWnd::UpdateArtPanelVisible() 
{
	bool visible = !!psHUD_Flags.is(HUD_ARTEFACT_PANEL_VISIBLE);
	m_artefactPanel->SetVisible(visible);
}

float UIStaticDiskIO_start_time = 0.0f;
void CUIMainIngameWnd::Draw()
{
#ifdef DEBUG
	test_draw				();
#endif
	// show IO icon
	bool IOActive	= (FS.dwOpenCounter>0);
	if	(IOActive)	UIStaticDiskIO_start_time = Device.fTimeGlobal;

	if ((UIStaticDiskIO_start_time+1.0f) < Device.fTimeGlobal)	UIStaticDiskIO.Show(false); 
	else {
		u32		alpha			= clampr(iFloor(255.f*(1.f-(Device.fTimeGlobal-UIStaticDiskIO_start_time)/1.f)),0,255);
		UIStaticDiskIO.Show		( true  ); 
		UIStaticDiskIO.SetColor	(color_rgba(255,255,255,alpha));
	}
	FS.dwOpenCounter = 0;

	if(!IsGameTypeSingle())
	{
		float		luminocity = smart_cast<CGameObject*>(Level().CurrentEntity())->ROS()->get_luminocity();
		float		power = log(luminocity > .001f ? luminocity : .001f)*(1.f/*luminocity_factor*/);
		luminocity	= exp(power);

		static float cur_lum = luminocity;
		cur_lum = luminocity*0.01f + cur_lum*0.99f;
		UIMotionIcon.SetLuminosity(static_cast<s16>(iFloor(cur_lum * 100.0f)));
	}
	if(!m_pActor) return;

	UIMotionIcon.SetNoise		(static_cast<s16>(0xffff & iFloor(m_pActor->m_snd_noise * 100.0f)));
	CUIWindow::Draw				();
	UIZoneMap->Render			();			

	RenderQuickInfos			();		

#pragma region some debug output
	if (ShowExoOutfitDebug)
	{
		if (CExoOutfit* exo = smart_cast<CExoOutfit*>(g_actor->GetOutfit()))
		{
			UI()->Font()->pFontDI->SetHeightI(0.03f);
			UI()->Font()->pFontDI->OutSet(400, 10);
			UI()->Font()->pFontDI->SetColor(0xffffffff);
			UI()->Font()->pFontDI->OutNext("SECTION: %s", exo->m_sCurrentBattery.c_str());
			UI()->Font()->pFontDI->OutNext("CHARGE:  %f", exo->m_fCurrentCharge);
		}
	}
	
	/*CHudItem* pHudItem = smart_cast<CHudItem*>(g_actor->inventory().ActiveItem());
	if (pHudItem)
	{
		UI()->Font()->pFontDI->SetHeightI(0.03f);
		UI()->Font()->pFontDI->OutSet(400, 50);
		UI()->Font()->pFontDI->SetColor(0xffffffff);
		UI()->Font()->pFontDI->OutNext("WPN_BOBBING:  %s", pHudItem->GetHUD()->GetHudBobbing() ? "enable" : "disable");
	}*/
		
#pragma endregion
#ifdef DEBUG
	draw_adjust_mode			();
#endif
}


void CUIMainIngameWnd::SetMPChatLog(CUIWindow* pChat, CUIWindow* pLog){
	m_pMPChatWnd = pChat;
	m_pMPLogWnd  = pLog;
}

float get_current_kx()
{
	float h = float(Device.dwHeight);
	float w = float(Device.dwWidth);

	float res = (h / w) / (UI_BASE_HEIGHT / UI_BASE_WIDTH);
	return res;

}
void CUIMainIngameWnd::SetAmmoIcon (const shared_str& sect_name)
{
	if ( !sect_name.size() )
	{
		UIWeaponIcon.Show			(false);
		return;
	};

	UIWeaponIcon.Show			(true);
	//properties used by inventory menu
	UIIconInfo iconInfo(sect_name);
	Frect rect = iconInfo.getOriginalRect();
	UIWeaponIcon.GetUIStaticItem().SetOriginalRect(rect);
	UIWeaponIcon.SetStretchTexture(true);

	// now perform only width scale for ammo, which (W)size >2
	// all others ammo (1x1, 1x2) will be not scaled (original picture)

#if 0
	if (iconInfo.getWidth() > 1)
	{
		UIWeaponIcon.SetWndRect(UIWeaponIcon_rect);
		UIWeaponIcon.SetStretchTexture(true);
	}
	else
	{
		float iconWidth = INV_GRID_WIDTH;// (iconInfo.getWidth() > 2 ? 1.6f : iconInfo.getWidth())*INV_GRID_WIDTH*0.8f;
		float iconHeight = INV_GRID_HEIGHT;// INV_GRID_HEIGHT*0.8f;//1 cell
		float xr = (UIWeaponIcon_rect.width() - iconWidth) / 2.0f;
		float yr = (UIWeaponIcon_rect.height() - iconHeight) / 2.0f;
		UIWeaponIcon.SetWndPos	(UIWeaponIcon_rect.x1+xr, UIWeaponIcon_rect.y1+yr);
		UIWeaponIcon.SetWidth	(iconWidth);
		UIWeaponIcon.SetHeight	(iconHeight);
		UIWeaponIcon.SetStretchTexture(false);
	}
#endif
};

void CUIMainIngameWnd::Update()
{
#ifdef DEBUG
	test_update();
#endif
	if (m_pMPChatWnd)
		m_pMPChatWnd->Update();
	if (m_pMPLogWnd)
		m_pMPLogWnd->Update();


	m_pActor = smart_cast<CActor*>(Level().CurrentViewEntity());
	if (!m_pActor) 
	{
		m_pItem					= nullptr;
		m_pWeapon				= nullptr;
		m_pGrenade				= nullptr;
		CUIWindow::Update		();
		return;
	}

	if( !(Device.dwFrame%30) && IsGameTypeSingle() )
	{
			string256				text_str;
			CPda* _pda	= m_pActor->GetPDA();
			u32 _cn		= 0;
			if(_pda && 0!= (_cn=_pda->ActiveContactsNum()) )
			{
				sprintf_s(text_str, "%d", _cn);
				UIPdaOnline.SetText(text_str);
			}
			else
			{
				UIPdaOnline.SetText("");
			}
	};

	if( !(Device.dwFrame%5) )
	{

		if(!(Device.dwFrame%30))
		{
			bool b_God = (GodMode()||(!Game().local_player)) ? true : Game().local_player->testFlag(GAME_PLAYER_FLAG_INVINCIBLE);
			if(b_God /*&& g_uCommonFlags.test(E_COMMON_FLAGS::mwShowInvulnerableIcon)*/)
				SetWarningIconColor	(ewiInvincible,0xffffffff);
			else
				SetWarningIconColor	(ewiInvincible,0x00ffffff);
			UpdateArtPanelVisible();
		}
		// ewiArtefact
		if( (GameID() == GAME_ARTEFACTHUNT) && !(Device.dwFrame%30) ){
			bool b_Artefact = (NULL != m_pActor->inventory().ItemFromSlot(ARTEFACT_SLOT));
			if(b_Artefact)
				SetWarningIconColor	(ewiArtefact,0xffffffff);
			else
				SetWarningIconColor	(ewiArtefact,0x00ffffff);
		}

		// Armor indicator stuff
		PIItem	pItem = m_pActor->inventory().ItemFromSlot(OUTFIT_SLOT);
		if (pItem)
		{
			UIArmorBar.Show					(true);
			UIStaticArmor.Show				(true);
			UIArmorBar.SetProgressPos		(pItem->GetCondition()*100);
		}
		else
		{
			UIArmorBar.Show					(false);
			UIStaticArmor.Show				(false);
		}

		UpdateActiveItemInfo();
		EWarningIcons i					= ewiWeaponJammed;
		bool exoPresent = false;
		while (i < ewiInvincible)
		{
			float value = 0;
			switch (i)
			{
				//radiation
			case ewiRadiation:
				value = m_pActor->conditions().GetRadiation();
				break;
			case ewiWound:
				value = m_pActor->conditions().BleedingSpeed();
				break;
			case ewiWeaponJammed:
				if (m_pWeapon)
					value = 1 - m_pWeapon->GetConditionToShow();
				break;
			case ewiStarvation:
				value = 1 - m_pActor->conditions().GetSatiety();
				break;		
			case ewiPsyHealth:
				value = 1 - m_pActor->conditions().GetPsyHealth();
				break;
			case ewiWeight:
				{
					float cur_weight = Actor()->inventory().TotalWeight();
					float max_weight = Actor()->MaxCarryWeight();
					value=cur_weight*100/max_weight;
				}
				break;
			case ewiExoCharge:
				if (CExoOutfit* exo = smart_cast<CExoOutfit*>(m_pActor->GetOutfit()))
				{
					value = 1 - exo->m_fCurrentCharge;
					exoPresent = true;
				}
				else
					exoPresent = false;
				break;
			default:
				R_ASSERT(!"Unknown type of warning icon");
			}

			if (i==ewiExoCharge && !exoPresent && UIExoCharge.IsShown())
			{
				TurnOffWarningIcon(i);
				i = static_cast<EWarningIcons>(i + 1);
				continue;
			}
			xr_vector<float>::reverse_iterator	rit;

			// ������� ��������� �� ������ �����������
			rit  = std::find(m_Thresholds[i].rbegin(), m_Thresholds[i].rend(), value);

			// ���� ��� ���, �� ����� ��������� ������� �������� ()
			if (rit == m_Thresholds[i].rend())
				rit = std::find_if(m_Thresholds[i].rbegin(), m_Thresholds[i].rend(), std::bind2nd(std::less<float>(), value));

			// ����������� � ������������ �������� �������
			float min = m_Thresholds[i].front();
			float max = m_Thresholds[i].back();

			if (rit != m_Thresholds[i].rend())
			{
				float v = *rit;
				u32 color=color_argb(0xFF, clampr<u32>(static_cast<u32>(255 * ((v - min) / (max - min) * 2)), 0, 255), 
					clampr<u32>(static_cast<u32>(255 * (2.0f - (v - min) / (max - min) * 2)), 0, 255),
					0);
				if (i==ewiRadiation)
				{
					if (!m_pActor->inventory().ItemFromSlot(DETECTOR_ANOM_SLOT))
						color=0x00ffffff;
					if (value>=0.7)
						UIStaticRadiationDanger.SetVisible(color!=0x00ffffff);
				}
				SetWarningIconColor(i, color);
			}
			else
			{
				TurnOffWarningIcon(i);
				if (i==ewiRadiation && UIStaticRadiationDanger.GetVisible())
					UIStaticRadiationDanger.SetVisible(false);
			}
			i = static_cast<EWarningIcons>(i + 1);
		}
	}

	float radiation=m_pActor->inventory().ItemFromSlot(DETECTOR_ANOM_SLOT) ? m_pActor->conditions().GetRadiation() : 0;
	UIRadiationBar.SetProgressPos(radiation*100.0f);
	
	u32 year = 0, month = 0, day = 0, hours = 0, mins = 0, secs = 0, milisecs = 0;
	split_time(Level().GetGameTime(), year, month, day, hours, mins, secs, milisecs);
	string64 buffer;
	sprintf_s(buffer,"%02i:%02i",hours,mins);
	UIStaticHudTime.SetText(buffer);

	UIHealthBar.SetProgressPos		(m_pActor->GetfHealth()*100.0f);
	UIMotionIcon.SetPower			(m_pActor->conditions().GetPower()*100.0f);

	UIZoneMap->UpdateRadar			(Device.vCameraPosition);
	float h,p;
	Device.vCameraDirection.getHP	(h,p);
	UIZoneMap->SetHeading			(-h);

	UpdatePickUpItem				();
	CUIWindow::Update				();
}

bool CUIMainIngameWnd::OnKeyboardPress(int dik)
{
#if 0//def DEBUG
	test_key(dik);
#endif
	// ��������� ������ adjust hud mode
	bool flag = false;
	if (g_bHudAdjustMode)
	{
		CWeaponHUD *pWpnHud = NULL;
		if (m_pWeapon)
		{
			pWpnHud = m_pWeapon->GetHUD();
//			if (!pWpnHud) return false;
		}
		else
			return false;

		Fvector tmpV;

		if (1 == g_bHudAdjustMode) //zoom offset
		{
			if (!pWpnHud) return false;
			tmpV = pWpnHud->ZoomOffset();

			switch (dik)
			{
				// Rotate +y
			case DIK_K:
				pWpnHud->SetZoomRotateX(pWpnHud->ZoomRotateX() + g_fHudAdjustValue);
				flag = true;
				break;
				// Rotate -y
			case DIK_I:
				pWpnHud->SetZoomRotateX(pWpnHud->ZoomRotateX() - g_fHudAdjustValue);
				flag = true;
				break;
				// Rotate +x
			case DIK_L:
				pWpnHud->SetZoomRotateY(pWpnHud->ZoomRotateY() + g_fHudAdjustValue);
				flag = true;
				break;
				// Rotate -x
			case DIK_J:
				pWpnHud->SetZoomRotateY(pWpnHud->ZoomRotateY() - g_fHudAdjustValue);
				flag = true;
				break;
				// Shift +x
			case DIK_W:
				tmpV.y += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -y
			case DIK_S:
				tmpV.y -= g_fHudAdjustValue;
				flag = true;
				break;
				// Shift +x
			case DIK_D:
				tmpV.x += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -x
			case DIK_A:
				tmpV.x -= g_fHudAdjustValue;
				flag = true;
				break;
				// Shift +z
			case DIK_Q:
				tmpV.z += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -z
			case DIK_E:
				tmpV.z -= g_fHudAdjustValue;
				flag = true;
				break;
				// output coordinate info to the console
			case DIK_P:
				string256 tmpStr;
				sprintf_s(tmpStr, "%s",
					*m_pWeapon->cNameSect());
				Log(tmpStr);

					sprintf_s(tmpStr, "zoom_offset\t\t\t= %f,%f,%f",
						pWpnHud->ZoomOffset().x,
						pWpnHud->ZoomOffset().y,
						pWpnHud->ZoomOffset().z);
				Log(tmpStr);
				sprintf_s(tmpStr, "zoom_rotate_x\t\t= %f",
					pWpnHud->ZoomRotateX());
				Log(tmpStr);
				sprintf_s(tmpStr, "zoom_rotate_y\t\t= %f",
					pWpnHud->ZoomRotateY());
				Log(tmpStr);
				flag = true;
				break;
			}

			if (tmpV.x || tmpV.y || tmpV.z)
				pWpnHud->SetZoomOffset(tmpV);
		}
		else if (2 == g_bHudAdjustMode || 5 == g_bHudAdjustMode) //firePoints
		{
			if(TRUE==m_pWeapon->GetHUDmode())
				tmpV = (2 == g_bHudAdjustMode) ? pWpnHud->FirePoint() : pWpnHud->FirePoint2();
			else
				tmpV = (2 == g_bHudAdjustMode) ? m_pWeapon->vLoadedFirePoint : m_pWeapon->vLoadedFirePoint2;

		
			switch (dik)
			{
				// Shift +x
			case DIK_A:
				tmpV.y += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -x
			case DIK_D:
				tmpV.y -= g_fHudAdjustValue;
				flag = true;
				break;
				// Shift +z
			case DIK_Q:
				tmpV.x += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -z
			case DIK_E:
				tmpV.x -= g_fHudAdjustValue;
				flag = true;
				break;
				// Shift +y
			case DIK_S:
				tmpV.z += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -y
			case DIK_W:
				tmpV.z -= g_fHudAdjustValue;
				flag = true;
				break;
				// output coordinate info to the console
			case DIK_P:
				string256 tmpStr;
				if (m_pWeapon)
				{
					sprintf_s(tmpStr, "%s",
						*m_pWeapon->cNameSect());
					Log(tmpStr);
				}

			if(TRUE==m_pWeapon->GetHUDmode())
				Msg("weapon hud section:");
			else
				Msg("weapon section:");

				sprintf_s(tmpStr, "fire_point\t\t\t= %f,%f,%f",
					tmpV.x,
					tmpV.y,
					tmpV.z);
				Log(tmpStr);
				flag = true;
				break;
			}
#ifdef	DEBUG
			if(TRUE==m_pWeapon->GetHUDmode())
				if (2 == g_bHudAdjustMode) pWpnHud->dbg_SetFirePoint(tmpV);
				else pWpnHud->dbg_SetFirePoint2(tmpV);
			else
			{
				if (2 == g_bHudAdjustMode)  m_pWeapon->vLoadedFirePoint = tmpV;
				else m_pWeapon->vLoadedFirePoint2 = tmpV;
			}
#endif
		}
		else if (4 == g_bHudAdjustMode) //ShellPoint
		{
			if(TRUE==m_pWeapon->GetHUDmode())
				tmpV = pWpnHud->ShellPoint();
			else
				tmpV = m_pWeapon->vLoadedShellPoint;

			switch (dik)
			{
				// Shift +x
			case DIK_A:
				tmpV.y += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -x
			case DIK_D:
				tmpV.y -= g_fHudAdjustValue;
				flag = true;
				break;
				// Shift +z
			case DIK_Q:
				tmpV.x += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -z
			case DIK_E:
				tmpV.x -= g_fHudAdjustValue;
				flag = true;
				break;
				// Shift +y
			case DIK_S:
				tmpV.z += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -y
			case DIK_W:
				tmpV.z -= g_fHudAdjustValue;
				flag = true;
				break;
				// output coordinate info to the console
			case DIK_P:
				string256 tmpStr;
				if (m_pWeapon)
				{
					sprintf_s(tmpStr, "%s",
						*m_pWeapon->cNameSect());
					Log(tmpStr);
				}

			if(TRUE==m_pWeapon->GetHUDmode())
				Msg("weapon hud section:");
			else
				Msg("weapon section:");

				sprintf_s(tmpStr, "shell_point\t\t\t= %f,%f,%f",
					tmpV.x,
					tmpV.y,
					tmpV.z);
				Log(tmpStr);
				flag = true;
				break;
			}
#ifdef DEBUG
			if(TRUE==m_pWeapon->GetHUDmode())
				pWpnHud->dbg_SetShellPoint(tmpV);
			else
				m_pWeapon->vLoadedShellPoint = tmpV;

#endif
		}
		else if (3 == g_bHudAdjustMode) //MissileOffset
		{
			CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());

			R_ASSERT(pActor);

			tmpV = pActor->GetMissileOffset();

			if (!pActor) return false;
			switch (dik)
			{
				// Shift +x
			case DIK_E:
				tmpV.y += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -x
			case DIK_Q:
				tmpV.y -= g_fHudAdjustValue;
				flag = true;
				break;
				// Shift +z
			case DIK_D:
				tmpV.x += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -z
			case DIK_A:
				tmpV.x -= g_fHudAdjustValue;
				flag = true;
				break;
				// Shift +y
			case DIK_W:
				tmpV.z += g_fHudAdjustValue;
				flag = true;
				break;
				// Shift -y
			case DIK_S:
				tmpV.z -= g_fHudAdjustValue;
				flag = true;
				break;
				// output coordinate info to the console
			case DIK_P:
				string256 tmpStr;
				if (m_pWeapon)
				{
					sprintf_s(tmpStr, "%s",
						*m_pWeapon->cNameSect());
					Log(tmpStr);
				}

				sprintf_s(tmpStr, "missile_throw_offset\t\t\t= %f,%f,%f",
					pActor->GetMissileOffset().x,
					pActor->GetMissileOffset().y,
					pActor->GetMissileOffset().z);

				Log(tmpStr);
				flag = true;
				break;
			}

			pActor->SetMissileOffset(tmpV);
		}
		

		if (flag) return true;
	}

#ifdef DEBUG
		if(CAttachableItem::m_dbgItem){
			static float rot_d = deg2rad(0.5f);
			static float mov_d = 0.01f;
			bool shift = !!pInput->iGetAsyncKeyState(DIK_LSHIFT);
			flag = true;
			switch (dik)
			{
				// Shift +x
			case DIK_A:
				if(shift)	CAttachableItem::rot_dx(rot_d);
				else		CAttachableItem::mov_dx(rot_d);
				break;
				// Shift -x
			case DIK_D:
				if(shift)	CAttachableItem::rot_dx(-rot_d);
				else		CAttachableItem::mov_dx(-rot_d);
				break;
				// Shift +z
			case DIK_Q:
				if(shift)	CAttachableItem::rot_dy(rot_d);
				else		CAttachableItem::mov_dy(rot_d);
				break;
				// Shift -z
			case DIK_E:
				if(shift)	CAttachableItem::rot_dy(-rot_d);
				else		CAttachableItem::mov_dy(-rot_d);
				break;
				// Shift +y
			case DIK_S:
				if(shift)	CAttachableItem::rot_dz(rot_d);
				else		CAttachableItem::mov_dz(rot_d);
				break;
				// Shift -y
			case DIK_W:
				if(shift)	CAttachableItem::rot_dz(-rot_d);
				else		CAttachableItem::mov_dz(-rot_d);
				break;

			case DIK_SUBTRACT:
				if(shift)	rot_d-=deg2rad(0.01f);
				else		mov_d-=0.001f;
				Msg("rotation delta=[%f]; moving delta=[%f]",rot_d,mov_d);
				break;
			case DIK_ADD:
				if(shift)	rot_d+=deg2rad(0.01f);
				else		mov_d+=0.001f;
				Msg("rotation delta=[%f]; moving delta=[%f]",rot_d,mov_d);
				break;

			case DIK_P:
				Msg("LTX section [%s]",*CAttachableItem::m_dbgItem->item().object().cNameSect());
				Msg("attach_angle_offset [%f,%f,%f]",VPUSH(CAttachableItem::get_angle_offset()));
				Msg("attach_position_offset [%f,%f,%f]",VPUSH(CAttachableItem::get_pos_offset()));
				break;
			default:
				flag = false;
				break;
			}		
		if(flag)return true;;
		}
#endif		

	if(Level().IR_GetKeyState(DIK_LSHIFT) || Level().IR_GetKeyState(DIK_RSHIFT))
	{
		switch (dik)
		{
		case DIK_NUMPADMINUS:
			UIZoneMap->ZoomOut();
			return true;
			break;
		case DIK_NUMPADPLUS:
			UIZoneMap->ZoomIn();
			return true;
			break;

		case DIK_NUMPADSTAR:
			ShowExoOutfitDebug = !ShowExoOutfitDebug;
			return true;
		}
	}
	else
	{
		switch(dik)
		{
		case DIK_NUMPADMINUS:
			//.HideAll();
			psHUD_Flags.set(HUD_GAME_INDICATORS_VISIBLE, FALSE);
			HUD().GetUI()->HideGameIndicators();
			if (!psHUD_Flags.is(HUD_MIN_CROSSHAIR))
			{
				HUD().GetUI()->HideCrosshair();	
			}
			

			return true;
			break;
		case DIK_NUMPADPLUS:
			//.ShowAll();
			psHUD_Flags.set(HUD_GAME_INDICATORS_VISIBLE, TRUE);
			HUD().GetUI()->ShowGameIndicators();
			HUD().GetUI()->ShowCrosshair();

			return true;
			break;
		}
	}

	return false;
}


void CUIMainIngameWnd::RenderQuickInfos()
{
	if (!m_pActor)
		return;

	static CGameObject *pObject			= NULL;
	LPCSTR actor_action					= m_pActor->GetDefaultActionForObject();
	UIStaticQuickHelp.Show				(NULL!=actor_action);

	if(NULL!=actor_action){
		if(_stricmp(actor_action,UIStaticQuickHelp.GetText()))
			UIStaticQuickHelp.SetTextST				(actor_action);
	}

	if (pObject!=m_pActor->ObjectWeLookingAt())
	{
		UIStaticQuickHelp.SetTextST				(actor_action);
		UIStaticQuickHelp.ResetClrAnimation		();
		pObject	= m_pActor->ObjectWeLookingAt	();
	}
}

void CUIMainIngameWnd::ReceiveNews(GAME_NEWS_DATA* news)
{
	VERIFY(news->texture_name.size());

	HUD().GetUI()->m_pMessagesWnd->AddIconedPdaMessage(*(news->texture_name), news->tex_rect, news->SingleLineText(), news->show_time);
}

void CUIMainIngameWnd::SetWarningIconColor(CUIStatic* s, const u32 cl)
{
	int bOn = (cl>>24);
	bool bIsShown = s->IsShown();

	if(bOn)
		s->SetColor	(cl);

	if(bOn&&!bIsShown){
		m_UIIcons->AddWindow	(s, false);
		s->Show					(true);
	}

	if(!bOn&&bIsShown){
		m_UIIcons->RemoveWindow	(s);
		s->Show					(false);
	}
}

void CUIMainIngameWnd::SetWarningIconColor(EWarningIcons icon, const u32 cl)
{
	bool bMagicFlag = true;

	// ������ ���� ��������� ������
	switch(icon)
	{
	case ewiAll:
		bMagicFlag = false;
	case ewiWeaponJammed:
		SetWarningIconColor		(&UIWeaponJammedIcon, cl);
		if (bMagicFlag) break;
	case ewiWound:
		SetWarningIconColor		(&UIWoundIcon, cl);
		if (bMagicFlag) break;
	case ewiStarvation:
		SetWarningIconColor		(&UIStarvationIcon, cl);
		if (bMagicFlag) break;	
	case ewiPsyHealth:
		SetWarningIconColor		(&UIPsyHealthIcon, cl);
		if (bMagicFlag) break;
	case ewiInvincible:
		SetWarningIconColor		(&UIInvincibleIcon, cl);
		if (bMagicFlag) break;
		break;
	case ewiArtefact:
		SetWarningIconColor		(&UIArtefactIcon, cl);
		break;
	case ewiRadiation:
		SetWarningIconColor		(&UIRadiaitionIcon, cl);
		if (bMagicFlag) break;
		break;
	case ewiWeight:
		SetWarningIconColor		(&UIWeightIcon, cl);
		if (bMagicFlag) break;
	case ewiExoCharge:
		SetWarningIconColor(&UIExoCharge, cl);
		if (bMagicFlag) break;
	default:
		R_ASSERT(!"Unknown warning icon type");
	}
}

void CUIMainIngameWnd::TurnOffWarningIcon(EWarningIcons icon)
{
	SetWarningIconColor(icon, 0x00ffffff);
}


void CUIMainIngameWnd::SetFlashIconState_(EFlashingIcons type, bool enable)
{
	// �������� �������� ��������� ������
	FlashingIcons_it icon = m_FlashingIcons.find(type);
	R_ASSERT2(icon != m_FlashingIcons.end(), "Flashing icon with this type not existed");
	icon->second->Show(enable);
}

void CUIMainIngameWnd::InitFlashingIcons(CUIXml* node)
{
	const char * const flashingIconNodeName = "flashing_icon";
	int staticsCount = node->GetNodesNum("", 0, flashingIconNodeName);

	CUIXmlInit xml_init;
	CUIStatic *pIcon = NULL;
	// ����������� �� ���� ����� � �������������� �� ��� �������
	for (int i = 0; i < staticsCount; ++i)
	{
		pIcon = xr_new<CUIStatic>();
		xml_init.InitStatic(*node, flashingIconNodeName, i, pIcon);
		shared_str iconType = node->ReadAttrib(flashingIconNodeName, i, "type", "none");

		// ������ ���������� ������ � �� ���
		EFlashingIcons type = efiPdaTask;

		if		(iconType == "pda")		type = efiPdaTask;
		else if (iconType == "mail")	type = efiMail;
		else	R_ASSERT(!"Unknown type of mainingame flashing icon");

		R_ASSERT2(m_FlashingIcons.find(type) == m_FlashingIcons.end(), "Flashing icon with this type already exists");

		CUIStatic* &val	= m_FlashingIcons[type];
		val			= pIcon;

		AttachChild(pIcon);
		pIcon->Show(false);
	}
}

void CUIMainIngameWnd::DestroyFlashingIcons()
{
	for (FlashingIcons_it it = m_FlashingIcons.begin(); it != m_FlashingIcons.end(); ++it)
	{
		DetachChild(it->second);
		xr_delete(it->second);
	}

	m_FlashingIcons.clear();
}

void CUIMainIngameWnd::UpdateFlashingIcons()
{
	for (FlashingIcons_it it = m_FlashingIcons.begin(); it != m_FlashingIcons.end(); ++it)
	{
		it->second->Update();
	}
}

void CUIMainIngameWnd::AnimateContacts(bool b_snd)
{
	UIPdaOnline.ResetClrAnimation	();

	if(b_snd)
		HUD_SOUND::PlaySound	(m_contactSnd, Fvector().set(0,0,0), nullptr, true );

}


void CUIMainIngameWnd::SetPickUpItem	(CInventoryItem* PickUpItem)
{
if (m_pPickUpItem != PickUpItem)
	{
		m_pPickUpItem = PickUpItem;
		UIPickUpItemIcon.Show(false);
		UIPickUpItemIcon.DetachAll();
	}
}

void CUIMainIngameWnd::SetActiveItemAmmoInfo(xr_string str_name, xr_string icon_sect_name, xr_string str_count)
{
		UIWeaponSignAmmo.Show		(true						);
		UIWeaponBack.SetText		(str_name.c_str			()	);
		UIWeaponSignAmmo.SetText	(str_count.c_str		()	);
		SetAmmoIcon					(icon_sect_name.c_str	()	);
};

typedef CUIWeaponCellItem::eAddonType eAddonType;

CUIStatic* init_addon(CUIWeaponCellItem *cell_item,LPCSTR sect,float scale,float scale_x,eAddonType idx)
{
	CUIStatic *addon = xr_new<CUIStatic>();
	addon->SetAutoDelete(true);

	auto pos   = cell_item->get_addon_offset(idx); 
	pos.x	  *= scale *scale_x; 
	pos.y	  *= scale;
	
	UIIconInfo     params(sect);
	Frect rect = params.getOriginalRect();			
	addon->SetStretchTexture	(true);
	addon->SetShader(GetEquipmentIconsShader());
	addon->SetOriginalRect		(rect);
	addon->SetWndRect			(pos.x, pos.y, rect.width() * scale * scale_x, rect.height() *scale);
	addon->SetColor				(color_rgba(255,255,255,192));

	return addon;
}

void CUIMainIngameWnd::UpdatePickUpItem	()
{
	if (!m_pPickUpItem || !Level().CurrentViewEntity() || Level().CurrentViewEntity()->CLS_ID != CLSID_OBJECT_ACTOR) 
	{
		UIPickUpItemIcon.Show(false);
		return;
	};

	if (UIPickUpItemIcon.IsShown() ) return;
	shared_str sect_name	= m_pPickUpItem->object().cNameSect();

	UIIconInfo iconInfo=m_pPickUpItem->GetIconInfo();
	Frect rect=iconInfo.getOriginalRect();

	float scale_x = m_iPickUpItemIconWidth/rect.width();
	float scale_y = m_iPickUpItemIconHeight/rect.height();

	scale_x = (scale_x>1) ? 1.0f : scale_x;
	scale_y = (scale_y>1) ? 1.0f : scale_y;

	float scale = (scale_x<scale_y?scale_x:scale_y);
	UIPickUpItemIcon.GetUIStaticItem().SetShader(GetEquipmentIconsShader());
	UIPickUpItemIcon.GetUIStaticItem().SetOriginalRect(iconInfo.getOriginalRect());
	UIPickUpItemIcon.SetStretchTexture(true);

	// Real Wolf: ���������� ������������. 10.08.2014.
	scale_x = Device.fASPECT  / 0.75f;

	UIPickUpItemIcon.SetWidth(rect.width()*scale* scale_x);
	UIPickUpItemIcon.SetHeight(rect.height()*scale);

	UIPickUpItemIcon.SetWndPos(
		m_iPickUpItemIconX + 
		(m_iPickUpItemIconWidth - UIPickUpItemIcon.GetWidth())/2,
		m_iPickUpItemIconY + 
		(m_iPickUpItemIconHeight - UIPickUpItemIcon.GetHeight())/2);

	UIPickUpItemIcon.SetColor(color_rgba(255,255,255,192));

	if (auto wpn = m_pPickUpItem->cast_weapon())
	{
		auto cell_item = xr_new<CUIWeaponCellItem>(wpn);
		if (wpn->SilencerAttachable() && wpn->IsSilencerAttached() )
		{
			auto sil = init_addon(cell_item, *wpn->GetSilencerName(), scale, scale_x, eAddonType::eSilencer);
			UIPickUpItemIcon.AttachChild(sil);
		}
		if (wpn->ScopeAttachable() && wpn->IsScopeAttached() )
		{
			auto scope = init_addon(cell_item, *wpn->GetScopeName(), scale, scale_x, eAddonType::eScope);
			UIPickUpItemIcon.AttachChild(scope);
		}
		if (wpn->GrenadeLauncherAttachable() && wpn->IsGrenadeLauncherAttached() )
		{
			auto launcher = init_addon(cell_item, *wpn->GetGrenadeLauncherName(), scale, scale_x, eAddonType::eLauncher);
			UIPickUpItemIcon.AttachChild(launcher);
		}
		delete_data(cell_item);
	}

	UIPickUpItemIcon.Show(true);
};

void CUIMainIngameWnd::UpdateActiveItemInfo()
{
	PIItem item		=  m_pActor->inventory().ActiveItem();
	if(item) 
	{
		xr_string					str_name;
		xr_string					icon_sect_name;
		xr_string					str_count;
		m_pActor->inventory().m_bForceRecalcAmmos=true;
		item->GetBriefInfo			(str_name, icon_sect_name, str_count);
		SetActiveItemAmmoInfo		(str_name, icon_sect_name, str_count);

		//-------------------
		m_pWeapon = smart_cast<CWeapon*> (item);		
	}else
	{
		UIWeaponIcon.Show			(false);
		UIWeaponSignAmmo.Show		(false);
		UIWeaponBack.SetText		("");
		m_pWeapon					= nullptr;
	}
}

void CUIMainIngameWnd::OnConnected()
{
	UIZoneMap->SetupCurrentMap		();
}

void CUIMainIngameWnd::reset_ui()
{
	m_pActor						= nullptr;
	m_pWeapon						= nullptr;
	m_pGrenade						= nullptr;
	m_pItem							= nullptr;
	m_pPickUpItem					= nullptr;
	UIMotionIcon.ResetVisibility	();
}

void CUIMainIngameWnd::re_init()
{
	DetachChild(&UIStaticRadiation);
	UIStaticRadiation.DetachChild(&UIRadiationBar);

	CUIXml						uiXml;
	uiXml.Init(CONFIG_PATH, UI_PATH, MAININGAME_XML);
	CUIXmlInit					xml_init;
	CUIWindow::Init				(0,0, UI_BASE_WIDTH, UI_BASE_HEIGHT);

	AttachChild					(&UIStaticRadiation);
	xml_init.InitStatic			(uiXml, "static_radiation", 0, &UIStaticRadiation);
	UIStaticRadiation.AttachChild	(&UIRadiationBar);
	xml_init.InitProgressBar	(uiXml, "progress_bar_radiation", 0, &UIRadiationBar);

	/*UIStaticHealth.DetachChild(&UIHealthBar);
	UIStaticArmor.DetachChild(&UIArmorBar);
	UIWeaponBack.DetachChild(&UIWeaponSignAmmo);
	UIWeaponBack.DetachChild(&UIWeaponIcon);
	
	DetachChild(m_UIIcons);
	DetachChild(&UICarPanel);
	DetachChild(&UIMotionIcon);
	DetachChild(m_artefactPanel);
	DetachChild(&UIStaticDiskIO);

	
	DetachChild(&UIStaticHealth);
	DetachChild(&UIStaticArmor);
	DetachChild(&UIWeaponBack);
	DetachChild(&UIWeaponSignAmmo);
	DetachChild(&UIWeaponIcon);
	DetachChild(&UIPickUpItemIcon);
	DetachChild(&UIPdaOnline);
	if (IsChild(&UIStaticQuickHelp))
		DetachChild(&UIStaticQuickHelp);
	
	xr_delete					(UIZoneMap);
	UIZoneMap					= xr_new<CUIZoneMap>();*/

}

#ifdef DEBUG
/*
#include "d3dx9core.h"
#include "winuser.h"
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"d3d9.lib")
ID3DXFont*     g_pTestFont = NULL;
ID3DXSprite*        g_pTextSprite = NULL;   // Sprite for batching draw text calls
*/

/*
#include "UIGameTutorial.h"
#include "../actor_statistic_mgr.h"
CUIGameTutorial* g_tut = NULL;
*/
//#include "../postprocessanimator.h"
//CPostprocessAnimator* pp = NULL;
//extern void create_force_progress();

//#include "UIVotingCategory.h"

//CUIVotingCategory* v = NULL;
#include "UIFrameWindow.h"
CUIFrameWindow*		pUIFrame = NULL;

void test_update()
{
	if(pUIFrame)
		pUIFrame->Update();
}

void test_key	(int dik)
{

	if(dik==DIK_K)
	{
		if(!pUIFrame)
		{
			CUIXml uiXML;
			uiXML.Init(CONFIG_PATH, UI_PATH, "talk.xml");

			pUIFrame					= xr_new<CUIFrameWindow>();
			CUIXmlInit::InitFrameWindow	(uiXML, "frame_window", 0, pUIFrame);
		}else
			xr_delete(pUIFrame);
	}

/*
	if(dik==DIK_K){
		if(g_pTestFont){
			g_pTestFont->Release();
			g_pTestFont = NULL;
			
			g_pTextSprite->Release();
			return;
		}
	HRESULT hr;
	static int _height	= -12;
	static u32 _width	= 0;
	static u32 _weigth	= FW_BOLD;
	static BOOL _italic = FALSE;

	hr = D3DXCreateFont( HW.pDevice, _height, _width, _weigth, 1, _italic, DEFAULT_CHARSET, 
						 OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
						 "Times New Roman", &g_pTestFont );


	D3DXCreateSprite( HW.pDevice, &g_pTextSprite );

	g_pTestFont->PreloadText("This is a trivial call to ID3DXFont::DrawText", xr_strlen("This is a trivial call to ID3DXFont::DrawText"));

	}
*/
}
/*
D3DCOLOR _clr	= D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f );
LPCSTR _str		= "This is a trivial call to ID3DXFont::DrawText";
int _len		= 43;
*/
void test_draw	()
{
	if(pUIFrame)
		pUIFrame->Draw();
/*
	if(g_pTestFont){

//	g_pTestFont->PreloadText("This is a trivial call to ID3DXFont::DrawText", xr_strlen("This is a trivial call to ID3DXFont::DrawText"));
//	g_pTestFont2->PreloadText("This is a trivial call to ID3DXFont::DrawText", xr_strlen("This is a trivial call to ID3DXFont::DrawText"));

//	IDirect3DTexture9	*T;
//	RECT				R;
//	POINT				P;
//	g_pTestFont2->PreloadGlyphs(0,255);
//	g_pTestFont2->GetGlyphData(50, &T, &R, &P);
//	R_CHK		(D3DXSaveTextureToFile	("x:\\test_font.dds",D3DXIFF_DDS,T,0));

#define DT_TOP                      0x00000000
#define DT_LEFT                     0x00000000
#define DT_CENTER                   0x00000001
#define DT_RIGHT                    0x00000002
#define DT_VCENTER                  0x00000004
#define DT_BOTTOM                   0x00000008
#define DT_WORDBREAK                0x00000010
#define DT_SINGLELINE               0x00000020
#define DT_EXPANDTABS               0x00000040
#define DT_TABSTOP                  0x00000080
#define DT_NOCLIP                   0x00000100
#define DT_EXTERNALLEADING          0x00000200
#define DT_CALCRECT                 0x00000400
#define DT_NOPREFIX                 0x00000800
#define DT_INTERNAL                 0x00001000


		RECT rc;
		g_pTextSprite->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE );

		rc.left   = 50;
		rc.top    = 150;
		rc.right  = 250;
		rc.bottom = 180;

		for(int i=0; i<13; ++i){
			g_pTestFont->DrawText( g_pTextSprite, _str, _len, &rc, DT_SINGLELINE, _clr);
			rc.top			+= 30; rc.bottom		+= 30;
		}

		g_pTextSprite->End();

	}
*/
}

void CUIMainIngameWnd::draw_adjust_mode()
{
	if (g_bHudAdjustMode&&m_pWeapon) //draw firePoint,ShellPoint etc
	{
		CActor* pActor = smart_cast<CActor*>(Level().CurrentEntity());
		if(!pActor)
			return;

		bool bCamFirstEye = !!m_pWeapon->GetHUDmode();
		string32 hud_view="HUD view";
		string32 _3rd_person_view="3-rd person view";
		CGameFont* F		= UI()->Font()->pFontDI;
		F->SetAligment		(CGameFont::alCenter);
//.		F->SetSizeI			(0.02f);
		F->OutSetI			(0.f,-0.8f);
		F->SetColor			(0xffffffff);
		F->OutNext			("Hud_adjust_mode=%d",g_bHudAdjustMode);
		if(g_bHudAdjustMode==1)
			F->OutNext			("adjusting zoom offset");
		else if(g_bHudAdjustMode==2)
			F->OutNext			("adjusting fire point for %s",bCamFirstEye?hud_view:_3rd_person_view);
		else if(g_bHudAdjustMode==3)
			F->OutNext			("adjusting missile offset");
		else if(g_bHudAdjustMode==4)
			F->OutNext			("adjusting shell point for %s",bCamFirstEye?hud_view:_3rd_person_view);
		else if(g_bHudAdjustMode == 5)
			F->OutNext			("adjusting fire point 2 for %s",bCamFirstEye?hud_view:_3rd_person_view);

		if(bCamFirstEye)
		{
			CWeaponHUD *pWpnHud = NULL;
			pWpnHud = m_pWeapon->GetHUD();

			Fvector FP,SP,FP2;

			CKinematics* V			= smart_cast<CKinematics*>(pWpnHud->Visual());
			VERIFY					(V);
			V->CalculateBones		();

			// fire point&direction
			Fmatrix& fire_mat		= V->LL_GetTransform(u16(pWpnHud->FireBone()));
			Fmatrix& parent			= pWpnHud->Transform	();

			const Fvector& fp		= pWpnHud->FirePoint();
			const Fvector& fp2		= pWpnHud->FirePoint2();
			const Fvector& sp		= pWpnHud->ShellPoint();

			fire_mat.transform_tiny	(FP,fp);
			parent.transform_tiny	(FP);

			fire_mat.transform_tiny	(FP2,fp2);
			parent.transform_tiny	(FP2);

			fire_mat.transform_tiny	(SP,sp);
			parent.transform_tiny	(SP);


			RCache.dbg_DrawAABB(FP,0.01f,0.01f,0.01f,D3DCOLOR_XRGB(255,0,0));
			RCache.dbg_DrawAABB(FP2,0.02f,0.02f,0.02f,D3DCOLOR_XRGB(0,0,255));
			RCache.dbg_DrawAABB(SP,0.01f,0.01f,0.01f,D3DCOLOR_XRGB(0,255,0));
		
		}else{
			Fvector FP = m_pWeapon->get_CurrentFirePoint();
			Fvector FP2 = m_pWeapon->get_CurrentFirePoint2();
			Fvector SP = m_pWeapon->get_LastSP();
			RCache.dbg_DrawAABB(FP,0.01f,0.01f,0.01f,D3DCOLOR_XRGB(255,0,0));
			RCache.dbg_DrawAABB(FP2,0.02f,0.02f,0.02f,D3DCOLOR_XRGB(0,0,255));
			RCache.dbg_DrawAABB(SP,0.02f,0.02f,0.02f,D3DCOLOR_XRGB(0,255,0));
		}
	}
}
#endif
