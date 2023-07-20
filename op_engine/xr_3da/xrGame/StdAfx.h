#pragma once

#if _MSC_VER >= 1900
    //! Constructor of abstract class '...' ignores initializer for virtual base class '...'
    //! jarni: Known bug in VS2015 not fixed event in update 3
    #pragma warning(disable:4589)
	//! winsor: from luabind
	//! warning C4913: user defined binary operator ',' exists but no overload could convert all operands, default built-in binary operator ',' used
	#pragma warning(disable:4913)

#endif

#pragma warning(disable:4995)
#include "../stdafx.h"
#pragma warning(default:4995)
#pragma warning( 4 : 4018 )
#pragma warning( 4 : 4244 )
#pragma warning(disable:4505)

#include <queue> //! STD headers belong to stdafx.h

// this include MUST be here, since smart_cast is used >1800 times in the project
#include "smart_cast.h"

#define READ_IF_EXISTS(ltx,method,section,name,default_value)\
	((ltx->line_exist(section,name)) ? (ltx->method(section,name)) : (default_value))


#if XRAY_EXCEPTIONS
IC	xr_string	string2xr_string(LPCSTR s) {return *shared_str(s ? s : "");}
IC	void		throw_and_log(const xr_string &s) {Msg("! %s",s.c_str()); throw *shared_str(s.c_str());}
#	define		THROW(xpr)				if (!(xpr)) {throw_and_log (__FILE__LINE__" Expression \""#xpr"\"");}
#	define		THROW2(xpr,msg0)		if (!(xpr)) {throw *shared_str(xr_string(__FILE__LINE__).append(" \"").append(#xpr).append(string2xr_string(msg0)).c_str());}
#	define		THROW3(xpr,msg0,msg1)	if (!(xpr)) {throw *shared_str(xr_string(__FILE__LINE__).append(" \"").append(#xpr).append(string2xr_string(msg0)).append(", ").append(string2xr_string(msg1)).c_str());}
#else
#	define		THROW					VERIFY
#	define		THROW2					VERIFY2
#	define		THROW3					VERIFY3
#endif

#include "../gamefont.h"
#include "../xr_object.h"
#include "../igame_level.h"

#ifndef DEBUG
#	define MASTER_GOLD
#endif // DEBUG