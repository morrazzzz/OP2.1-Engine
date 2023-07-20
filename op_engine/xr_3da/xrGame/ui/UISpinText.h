// file:		UISpinNum.h
// description:	Spin Button with text data (unlike numerical data)
// created:		15.06.2005
// author:		Serge Vynnychenko
//

#pragma once
#include "UICustomSpin.h"



class CUISpinText : public CUICustomSpin
{
public:
	CUISpinText();
	// CUIOptionsItem
	virtual void	SetCurrentValue();
	virtual void	SaveValue();
	virtual bool	IsChanged();	

	// own
	virtual void	OnBtnUpClick();
	virtual void	OnBtnDownClick();

			void	AddItem_(const char* item, int id);
			LPCSTR	GetTokenText();
protected:
	virtual bool	CanPressUp		();
	virtual bool	CanPressDown	();
	virtual void	IncVal			(){};
	virtual void	DecVal			(){};
	virtual void	SetItem();
			struct SInfo{
				shared_str	_orig;
				shared_str	_transl;
				int			_id;
			};
	typedef xr_vector< SInfo >		Items;
	typedef Items::iterator			Items_it;

	Items	m_list;
	int		m_curItem;
};

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
#include <luabind/functor.hpp>
#include <luabind/operator.hpp>
#include "../script_callback_ex.h"
class CUISpinTextCustom:public CUISpinText
{
private:
	int m_backupIndex;
	CScriptCallbackEx<void> m_pButtonsSpinClickCallback;
public:
	CUISpinTextCustom();
	~CUISpinTextCustom();
	void SetCurrentValue() override;
	void SaveValue() override;
	bool IsChanged() override;
	void SetSpinButtonCallback(const luabind::functor<void> &functor);
protected:
	void	SetItem() override;
public:
	int GetIdByIndex(int index);
	LPCSTR GetTextByIndex(int index);
	int GetSelectedId();
	void SetSelectedId(int id);
	LPCSTR GetSelectedText();
	void SetSelectedText(LPCSTR text);
	void AddItem(const char* text, int id);
	void SetFont(CGameFont* pFont) override;
	void OnBtnUpClick() override;
	void OnBtnDownClick() override;
};