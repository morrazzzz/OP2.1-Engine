// file:		UITextureMaster.h
// description:	holds info about shared textures. able to initialize external controls
//				through IUITextureControl interface
// created:		11.05.2005
// author:		Serge Vynnychenko
// mail:		narrator@gsc-game.kiev.ua
//
// copyright 2005 GSC Game World


#include "StdAfx.h"
#include "UITextureMaster.h"
#include "uiabstract.h"
#include "xrUIXmlParser.h"
#include "../../CMultiHUDs.h"

xr_map<shared_str, TEX_INFO>	CUITextureMaster::m_textures;
#ifdef DEBUG
u32									CUITextureMaster::m_time = 0;
#endif

void CUITextureMaster::WriteLog(){
#ifdef DEBUG
	Msg("UI texture manager work time is %d ms", m_time);
#endif
}

void CUITextureMaster::Dump()
{
	std::for_each(m_textures.begin(), m_textures.end(), [](std::pair<shared_str, TEX_INFO> info)
	{
		Msg("m_texture [%s][%s]",info.first.c_str(),info.second.get_file_name());
	});
}

void CUITextureMaster::ParseShTexInfo(LPCSTR xml_file, bool fix_dublicate, bool show_duplicate) {
	CUIXml xml;
	xml.Init(CONFIG_PATH, UI_PATH, xml_file);
	shared_str file = xml.Read("file_name", 0, "");
	CHUDProfile* profile = multiHUDs->GetCurrentProfile();
	xr_string pfFile;
	if (profile)
	{
		string_path pf;
		strcpy_s(pf, file.c_str());
		strcat_s(pf, ".dds");
		pfFile = profile->GetFileFromProfile(pf);
	}

	int num = xml.GetNodesNum("", 0, "texture");
	for (int i = 0; i < num; i++)
	{
		TEX_INFO info;

		//info.file = file;
		
		info.file = pfFile.size()>0 ? pfFile.c_str() : file;

		info.rect.x1 = xml.ReadAttribFlt("texture", i, "x");
		info.rect.x2 = xml.ReadAttribFlt("texture", i, "width") + info.rect.x1;
		info.rect.y1 = xml.ReadAttribFlt("texture", i, "y");
		info.rect.y2 = xml.ReadAttribFlt("texture", i, "height") + info.rect.y1;
		shared_str id = xml.ReadAttrib("texture", i, "id");

		if (fix_dublicate || show_duplicate)
		{
			xr_map<shared_str, TEX_INFO>::iterator it = m_textures.find(id);
			if (it != m_textures.end())
			{
				if (show_duplicate)
					Msg("~ WARNING find double texture id [%s] loaded file[%s] input file[%s]", id.c_str(), it->second.get_file_name(), xml_file);
				if (fix_dublicate)
					m_textures.erase(it);
			}
		}
		m_textures.insert(mk_pair(id, info));
	}
}

bool CUITextureMaster::IsSh(const char* texture_name){
	return strstr(texture_name,"\\") ? false : true;
}

void CUITextureMaster::InitTexture(const char* texture_name, IUISimpleTextureControl* tc){
#ifdef DEBUG
	CTimer T;
	T.Start();
#endif

	xr_map<shared_str, TEX_INFO>::iterator	it;

	it = m_textures.find(texture_name);

	if (it != m_textures.end())
	{
		tc->CreateShader(*((*it).second.file));
		tc->SetOriginalRectEx((*it).second.rect);
#ifdef DEBUG
		m_time += T.GetElapsed_ms();
#endif
		return;
	}
	tc->CreateShader(texture_name);
#ifdef DEBUG
	m_time += T.GetElapsed_ms();
#endif
}

void CUITextureMaster::InitTexture(const char* texture_name, const char* shader_name, IUISimpleTextureControl* tc){
#ifdef DEBUG
	CTimer T;
	T.Start();
#endif

	xr_map<shared_str, TEX_INFO>::iterator	it;

	/*CHUDProfile profile = multiHUDs->GetCurrentProfile();
	if (profile && xr_strcmp(texture_name,"preview")==0)
	{
		
	}*/
	it = m_textures.find(texture_name);

	if (it != m_textures.end())
	{
		tc->CreateShader(*((*it).second.file), shader_name);
		tc->SetOriginalRectEx((*it).second.rect);
#ifdef DEBUG
		m_time += T.GetElapsed_ms();
#endif
		return;
	}
	tc->CreateShader(texture_name, shader_name);
#ifdef DEBUG
	m_time += T.GetElapsed_ms();
#endif
}

float CUITextureMaster::GetTextureHeight(const char* texture_name){
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);

	if (it != m_textures.end())
		return (*it).second.rect.height();

	//R_ASSERT3(false,"CUITextureMaster::GetTextureHeight Can't find texture", texture_name);
	Msg("! CUITextureMaster::GetTextureHeight Can't find texture [%s]", texture_name);
	return 0;
}

Frect CUITextureMaster::GetTextureRect(const char* texture_name){
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);
	if (it != m_textures.end())
		return (*it).second.rect;

	//R_ASSERT3(false,"CUITextureMaster::GetTextureHeight Can't find texture", texture_name);
	Msg("! CUITextureMaster::GetTextureHeight Can't find texture [%s]", texture_name);
	return Frect();
}

float CUITextureMaster::GetTextureWidth(const char* texture_name){
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);

	if (it != m_textures.end())
		return (*it).second.rect.width();
	//R_ASSERT3(false,"CUITextureMaster::GetTextureHeight Can't find texture", texture_name);
	Msg("! CUITextureMaster::GetTextureWidth Can't find texture [%s]", texture_name);
	return 0;
}

LPCSTR CUITextureMaster::GetTextureFileName(const char* texture_name){
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);

	if (it != m_textures.end())
		return *((*it).second.file);
	//R_ASSERT3(false,"CUITextureMaster::GetTextureFileName Can't find texture", texture_name);
	Msg("! CUITextureMaster::GetTextureFileName Can't find texture [%s]", texture_name);
	return 0;
}

TEX_INFO CUITextureMaster::FindItem(LPCSTR texture_name, LPCSTR def_texture_name)
{
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);

	if (it != m_textures.end())
		return (it->second);
	else{
		R_ASSERT2(m_textures.find(def_texture_name)!=m_textures.end(),texture_name);
		return FindItem	(def_texture_name, nullptr);
	}
}

void CUITextureMaster::GetTextureShader(LPCSTR texture_name, ref_shader& sh){
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);

	//R_ASSERT3(it != m_textures.end(), "can't find texture", texture_name);
	if (it == m_textures.end())
		Msg("! CUITextureMaster::GetTextureShader Can't find texture [%s]", texture_name);

	sh.create("hud\\default", *((*it).second.file));	
}

void CUITextureMaster::CleanPrefixedTextures(LPCSTR prefixFileName)
{
	xr_map<shared_str, TEX_INFO>::iterator	it= m_textures.begin();
	while (it != m_textures.end()) {
		if (strstr((*it).second.get_file_name(), prefixFileName))
		{
			it = m_textures.erase(it);
		}
		else
			++it;
	}
}
