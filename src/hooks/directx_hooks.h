#pragma once

#include "common.h"

extern GameWindowInfo game_window;
extern EndSceneFunc original_end_scene_function;
extern ResetFunc original_reset_function;
extern LPVOID original_end_scene_address;
extern LPVOID original_reset_address;

HRESULT WINAPI HookedEndScene ( LPDIRECT3DDEVICE9 device );
HRESULT WINAPI HookedReset ( LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* presentation_parameters );
void InstallWindowProcedureHook ();
