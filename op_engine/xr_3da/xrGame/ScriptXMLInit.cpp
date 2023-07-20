#include "pch_script.h"
#include "ScriptXmlInit.h"
#include "ui/UIXmlInit.h"
#include "ui/UITextureMaster.h"
#include "ui/UICheckButton.h" //#include "ui\UI3tButton.h"
#include "ui/UISpinNum.h"
#include "ui/UISpinText.h"
#include "ui/UIComboBox.h"
#include "ui/UIListWnd.h"
#include "ui/UITabControl.h"
#include "ui/UIFrameWindow.h"
#include "ui/UILabel.h"
#include "ui/ServerList.h"
#include "ui/UIMapList.h"
#include "ui/UIKeyBinding.h"
#include "ui/UIEditBox.h"
#include "ui/UIAnimatedStatic.h"
#include "ui/UITrackBar.h"
#include "ui/UIListItemIconed.h"
#include "ui/UIEditBoxEx.h"
#include "ui/UIMapInfo.h"
#include "ui/UIMMShniaga.h"
#include "ui/UIScrollView.h"
#include "ui/UIProgressBar.h"
#include "ui/UIDragDropListEx.h"
#include "ai_space.h"
#include "script_engine.h"

using namespace luabind;

void _attach_child(CUIWindow* _child, CUIWindow* _parent)
{
	if(!_parent)	return;
	CUIScrollView* _parent_scroll = smart_cast<CUIScrollView*>(_parent);
	if(_parent_scroll)
		_parent_scroll->AddWindow	(_child, true);
	else
		_parent->AttachChild		(_child);
}
CScriptXmlInit::CScriptXmlInit(){

}

CScriptXmlInit::CScriptXmlInit(const CScriptXmlInit&){
	// do nothing
}

CScriptXmlInit& CScriptXmlInit::operator =(const CScriptXmlInit& other){
	// do nothing
	return (*this);
}

void CScriptXmlInit::ParseFile(LPCSTR xml_file){
	m_xml.Init(CONFIG_PATH, UI_PATH, xml_file);
}

void CScriptXmlInit::InitWindow(LPCSTR path, int index, CUIWindow* pWnd){
	CUIXmlInit::InitWindow(m_xml, path, index, pWnd);
}

CUIDragDropListEx* CScriptXmlInit::InitDragDropListEx(LPCSTR path, CUIWindow* parent)
{
	CUIDragDropListEx* pList= xr_new<CUIDragDropListEx>();
	pList->m_bEnableDragDrop = false;
	CUIXmlInit::InitDragDropListEx(m_xml, path, 0, pList);
	pList->SetAutoDelete(true);
	_attach_child(pList, parent);
	return pList;
}

CUIFrameWindow*	CScriptXmlInit::InitFrame(LPCSTR path, CUIWindow* parent){
	CUIFrameWindow* pWnd = xr_new<CUIFrameWindow>();
	CUIXmlInit::InitFrameWindow(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}


CUIFrameLineWnd* CScriptXmlInit::InitFrameLine(LPCSTR path, CUIWindow* parent){
	CUIFrameLineWnd* pWnd = xr_new<CUIFrameLineWnd>();
	CUIXmlInit::InitFrameLine(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUILabel* CScriptXmlInit::InitLabel(LPCSTR path, CUIWindow* parent){
	CUILabel* pWnd = xr_new<CUILabel>();
	CUIXmlInit::InitLabel(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}


CUIEditBox* CScriptXmlInit::InitEditBox(LPCSTR path, CUIWindow* parent){
	CUIEditBox* pWnd = xr_new<CUIEditBox>();
	CUIXmlInit::InitEditBox(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUIEditBoxEx* CScriptXmlInit::InitEditBoxEx(LPCSTR path, CUIWindow* parent) {
	CUIEditBoxEx* pWnd = xr_new<CUIEditBoxEx>();
	CUIXmlInit::InitEditBoxEx(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
	//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUIStatic* CScriptXmlInit::InitStatic(LPCSTR path, CUIWindow* parent){
	CUIStatic* pWnd = xr_new<CUIStatic>();
	CUIXmlInit::InitStatic(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

void CScriptXmlInit::InitAutoStaticGroup(LPCSTR path, CUIWindow* pWnd)
{
	CUIXmlInit::InitAutoStaticGroup(m_xml, path, 0, pWnd);
}

CUIStatic* CScriptXmlInit::InitAnimStatic(LPCSTR path, CUIWindow* parent){
	CUIAnimatedStatic* pWnd = xr_new<CUIAnimatedStatic>();
	CUIXmlInit::InitAnimatedStatic(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUIScrollView* CScriptXmlInit::InitScrollView(LPCSTR path, CUIWindow* parent){
	CUIScrollView* pWnd = xr_new<CUIScrollView>();
	CUIXmlInit::InitScrollView(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}



CUICheckButton* CScriptXmlInit::InitCheck(LPCSTR path, CUIWindow* parent){
	CUICheckButton* pWnd = xr_new<CUICheckButton>();
	CUIXmlInit::InitCheck(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUISpinNum* CScriptXmlInit::InitSpinNum(LPCSTR path, CUIWindow* parent){
	CUISpinNum* pWnd = xr_new<CUISpinNum>();
	CUIXmlInit::InitSpin(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUISpinFlt* CScriptXmlInit::InitSpinFlt(LPCSTR path, CUIWindow* parent){
	CUISpinFlt* pWnd = xr_new<CUISpinFlt>();
	CUIXmlInit::InitSpin(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUISpinText* CScriptXmlInit::InitSpinText(LPCSTR path, CUIWindow* parent){
	CUISpinText* pWnd = xr_new<CUISpinText>();
	CUIXmlInit::InitSpin(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUISpinTextCustom* CScriptXmlInit::InitSpinTextCustom(LPCSTR path, CUIWindow* parent)
{
	CUISpinTextCustom* pWnd = xr_new<CUISpinTextCustom>();
	CUIXmlInit::InitSpinCustom(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUIComboBox* CScriptXmlInit::InitComboBox(LPCSTR path, CUIWindow* parent){
	CUIComboBox* pWnd = xr_new<CUIComboBox>();
	CUIXmlInit::InitComboBox(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUIComboBoxCustom* CScriptXmlInit::InitComboBoxCustom(LPCSTR path, CUIWindow* parent){
	CUIComboBoxCustom* pWnd = xr_new<CUIComboBoxCustom>();
	CUIXmlInit::InitComboBoxCustom(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUIButton* CScriptXmlInit::InitButton(LPCSTR path, CUIWindow* parent){
	CUIButton* pWnd = xr_new<CUIButton>();
	CUIXmlInit::InitButton(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}


CUI3tButton* CScriptXmlInit::Init3tButton(LPCSTR path, CUIWindow* parent){
	CUI3tButton* pWnd = xr_new<CUI3tButton>();
	CUIXmlInit::Init3tButton(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;	
}


CUIListWnd* CScriptXmlInit::InitList(LPCSTR path, CUIWindow* parent){
	CUIListWnd* pWnd = xr_new<CUIListWnd>();
	CUIXmlInit::InitListWnd(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}


CUITabControl* CScriptXmlInit::InitTab(LPCSTR path, CUIWindow* parent){
	CUITabControl* pWnd = xr_new<CUITabControl>();
	CUIXmlInit::InitTabControl(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;	
}

void CScriptXmlInit::ParseShTexInfo(LPCSTR xml_file){
	CUITextureMaster::ParseShTexInfo(xml_file);
}

CServerList* CScriptXmlInit::InitServerList(LPCSTR path, CUIWindow* parent){
	CServerList* pWnd = xr_new<CServerList>();
	pWnd->InitFromXml(m_xml, path);	
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;	
}

CUIMapList* CScriptXmlInit::InitMapList(LPCSTR path, CUIWindow* parent){
	CUIMapList* pWnd = xr_new<CUIMapList>();
	pWnd->InitFromXml(m_xml, path);	
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;	
}

CUIMMShniaga* CScriptXmlInit::InitMMShniaga(LPCSTR path, CUIWindow* parent){
	CUIMMShniaga* pWnd = xr_new<CUIMMShniaga>();
	pWnd->Init(m_xml, path);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;
}

CUIMapInfo* CScriptXmlInit::InitMapInfo(LPCSTR path, CUIWindow* parent){
	CUIMapInfo* pWnd = xr_new<CUIMapInfo>();
	CUIXmlInit::InitWindow(m_xml,path,0,pWnd);
	pWnd->SetAutoDelete(true);
	_attach_child(pWnd, parent);
//.	if(parent) parent->AttachChild(pWnd);
	return pWnd;	
}

CUIWindow* CScriptXmlInit::InitKeyBinding(LPCSTR path, CUIWindow* parent){
	CUIKeyBinding* pWnd				= xr_new<CUIKeyBinding>();
	pWnd->InitFromXml				(m_xml, path);	
	pWnd->SetAutoDelete				(true);
	_attach_child					(pWnd, parent);
	return							pWnd;
}

CUITrackBar* CScriptXmlInit::InitTrackBar(LPCSTR path, CUIWindow* parent){
	CUITrackBar* pWnd				= xr_new<CUITrackBar>();
	CUIXmlInit::InitTrackBar		(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete				(true);
	_attach_child					(pWnd, parent);
	return							pWnd;	
}

CUITrackBarCustom* CScriptXmlInit::InitTrackBarCustom(LPCSTR path, CUIWindow* parent)
{
	CUITrackBarCustom* pWnd				= xr_new<CUITrackBarCustom>();
	CUIXmlInit::InitTrackBarCustom		(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete				(true);
	_attach_child					(pWnd, parent);
	return							pWnd;
}

CUIProgressBar* CScriptXmlInit::InitProgressBar(LPCSTR path, CUIWindow* parent)
{
	CUIProgressBar* pWnd			= xr_new<CUIProgressBar>();
	CUIXmlInit::InitProgressBar		(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete				(true);
	_attach_child					(pWnd, parent);
	return							pWnd;	
}

CUIListItemIconed*	CScriptXmlInit::InitIconedColumns(LPCSTR path, CUIWindow* parent)
{
	CUIListItemIconed * pWnd=xr_new<CUIListItemIconed>();
	CUIXmlInit::InitIconedColumns(m_xml, path, 0, pWnd);
	pWnd->SetAutoDelete				(true);
	_attach_child					(pWnd, parent);
	return							pWnd;	
}

luabind::object CScriptXmlInit::TryReadTable(LPCSTR tablePath) 
{
	luabind::object resultTable=luabind::newtable(ai().script_engine().lua());
	XML_NODE* nodes		= m_xml.NavigateToNode(tablePath,0);

	if (nodes)
	{
		for (XML_NODE* node=nodes->FirstChild(); node; node=node->NextSibling())
		{
			if (node)
			{	
				LPCSTR id=node->Value();
				LPCSTR value=nullptr;
				XML_NODE *data=node->FirstChild();
				if (data)
				{
					TiXmlText *text			= data->ToText();
					if (text)				
						value=text->Value();
					resultTable[id]=value;
					//iconIDs.insert(mk_pair(id,value));

				}
			}
		}
	}
	return resultTable;
}
#pragma optimize("s",on)
void CScriptXmlInit::script_register(lua_State *L) {
	module(L)
		[
			class_<CScriptXmlInit>("CScriptXmlInit")
			.def(constructor<>())
		.def("TryReadTable", &CScriptXmlInit::TryReadTable)
		.def("InitIconedColumns", &CScriptXmlInit::InitIconedColumns)
		.def("ParseFile", &CScriptXmlInit::ParseFile)
		.def("ParseShTexInfo", &CScriptXmlInit::ParseShTexInfo)
		.def("InitWindow", &CScriptXmlInit::InitWindow)
		.def("InitFrame", &CScriptXmlInit::InitFrame)
		.def("InitFrameLine", &CScriptXmlInit::InitFrameLine)
		.def("InitLabel", &CScriptXmlInit::InitLabel)
		.def("InitEditBox", &CScriptXmlInit::InitEditBox)
		.def("InitEditBoxEx", &CScriptXmlInit::InitEditBoxEx)
		.def("InitStatic", &CScriptXmlInit::InitStatic)
		.def("InitAnimStatic", &CScriptXmlInit::InitAnimStatic)
		.def("InitCheck", &CScriptXmlInit::InitCheck)
		.def("InitSpinNum", &CScriptXmlInit::InitSpinNum)
		.def("InitSpinFlt", &CScriptXmlInit::InitSpinFlt)
		.def("InitSpinText", &CScriptXmlInit::InitSpinText)
		.def("InitSpinTextCustom", &CScriptXmlInit::InitSpinTextCustom)
		.def("InitComboBox", &CScriptXmlInit::InitComboBox)
		.def("InitComboBoxCustom", &CScriptXmlInit::InitComboBoxCustom)
		.def("InitButton", &CScriptXmlInit::InitButton)
		.def("Init3tButton", &CScriptXmlInit::Init3tButton)
		.def("InitList", &CScriptXmlInit::InitList)
		.def("InitTab", &CScriptXmlInit::InitTab)
		.def("InitServerList", &CScriptXmlInit::InitServerList)
		.def("InitMapList", &CScriptXmlInit::InitMapList)
		.def("InitMapInfo", &CScriptXmlInit::InitMapInfo)
		.def("InitTrackBar", &CScriptXmlInit::InitTrackBar)
		.def("InitTrackBarCustom", &CScriptXmlInit::InitTrackBarCustom)
		.def("InitKeyBinding", &CScriptXmlInit::InitKeyBinding)
		.def("InitMMShniaga", &CScriptXmlInit::InitMMShniaga)
		.def("InitScrollView", &CScriptXmlInit::InitScrollView)
		.def("InitAutoStaticGroup", &CScriptXmlInit::InitAutoStaticGroup)
		.def("InitProgressBar", &CScriptXmlInit::InitProgressBar)
		.def("InitDragDropListEx", &CScriptXmlInit::InitDragDropListEx)
		];
}
