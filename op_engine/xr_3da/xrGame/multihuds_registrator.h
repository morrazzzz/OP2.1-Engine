#ifndef multihuds_registrator_h
#define multihuds_registrator_h
#pragma once

#include "script_export_space.h"

struct multihuds_registrator {
	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(multihuds_registrator)
#undef script_type_list
#define script_type_list save_type_list(multihuds_registrator)

#endif