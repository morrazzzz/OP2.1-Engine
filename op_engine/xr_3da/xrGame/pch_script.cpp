#if _MSC_VER >= 1900
    //! 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed.Specify / EHsc
    //! jarni: Don't want to enable /EHsc, maybe it will break BugTrap or something
    #pragma warning(disable:4577)
    //! non-member operator new or delete functions may not be declared inline
    //! jarni: VS2015 doesn't line inline new, can't do it not inlined because it doesn't see it sometimes.
    #pragma warning(disable:4595)
    //! function assumed not to throw an exception but does
    //! jarni: luabind is full of "throw"s in desctructors
    #pragma warning(disable:4297)
    //! "<hash_map> is deprecated and will be REMOVED. Please use <unordered_map>. You can define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS to acknowledge that you have received this warning."
    //! jarni: This will be removed when unordered_map will be used instead of hash_map
    #ifndef _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
    #define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
    #endif
    //! error C2338: /RTCc rejects conformant code, so it isn't supported by the C++ Standard Library. Either remove this compiler option, or define _ALLOW_RTCc_IN_STL to acknowledge that you have received this warning.
    //! jarni: WTF?
    #if (defined DEBUG) && !(defined _ALLOW_RTCc_IN_STL)
    #define _ALLOW_RTCc_IN_STL
    #endif
#endif

#pragma warning(disable:4503)
#include "pch_script.h"
