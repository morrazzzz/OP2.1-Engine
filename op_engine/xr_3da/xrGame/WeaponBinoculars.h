#pragma once

#include "WeaponCustomPistol.h"
#include "script_export_space.h"

class CUIFrameWindow;
class CUIStatic;
class CBinocularsVision;

class CWeaponBinoculars: public CWeaponCustomPistol
{
private:
	typedef CWeaponCustomPistol inherited;
protected:
	bool			m_bVision;
public:
					CWeaponBinoculars	(); 
	virtual			~CWeaponBinoculars	();

	void			Load				(LPCSTR section);

	virtual void	OnZoomIn			();
	virtual void	OnZoomOut			();
	virtual void	net_Destroy			();
	virtual BOOL	net_Spawn			(CSE_Abstract* DC);

	virtual void	save				(NET_Packet &output_packet);
	virtual void	load				(IReader &input_packet);

	bool	Action				(s32 cmd, u32 flags) override;
	virtual void	UpdateCL			();
	virtual void	OnDrawUI			();
	void	GetBriefInfo		(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count) override;
	virtual void	net_Relcase			(CObject *object);
	int				GetCurrentFireMode	() override;	

protected:
	CBinocularsVision*					m_binoc_vision;
	void			FireEnd				() override;
	void			switch2_Fire		() override;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CWeaponBinoculars)
#undef script_type_list
#define script_type_list save_type_list(CWeaponBinoculars)
