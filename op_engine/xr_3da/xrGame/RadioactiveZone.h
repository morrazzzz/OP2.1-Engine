#pragma once
#include "customzone.h"
#include "script_export_space.h"

class CRadioactiveZone : public CCustomZone
{
private:
	typedef	CCustomZone	inherited;
public:
	CRadioactiveZone(void);
	virtual ~CRadioactiveZone(void);

	virtual void Load			(LPCSTR section);
	virtual void Affect			(SZoneObjectInfo* O);
	virtual bool EnableEffector	()	{return true;}

	virtual		void	feel_touch_new					(CObject* O	);
	virtual		void	UpdateWorkload					(u32	dt	);				// related to fast-mode optimizations
	virtual		BOOL	feel_touch_contact				(CObject* O	);

protected:
	virtual bool BlowoutState	();
public:
	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CRadioactiveZone)
#undef script_type_list
#define script_type_list save_type_list(CRadioactiveZone)
