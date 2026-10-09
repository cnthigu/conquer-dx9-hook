#pragma once

#include "common.h"

extern ShowStringExFunc original_show_string_ex_function;
extern std::vector<StringConfiguration> item_string_configurations;
extern std::vector<CapturedStringData> captured_strings;
extern bool server_name_use_rainbow;

void UpdateRainbowColors ();
const StringConfiguration* FindItemStringConfiguration ( const std::string& matched_string );
void InstallShowStringExHook ();
