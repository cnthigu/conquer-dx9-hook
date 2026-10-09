#pragma once

#include "common.h"

uintptr_t FindMemoryPattern ( uintptr_t start_address, size_t search_length, const std::vector<int>& pattern );
HWND FindGameWindowHandle ();
Direct3DRenderStateBackup SaveDirect3DRenderStates ( LPDIRECT3DDEVICE9 device );
void RestoreDirect3DRenderStates ( LPDIRECT3DDEVICE9 device, const Direct3DRenderStateBackup& backup );
void SetDirect3DRenderStatesFor2D ( LPDIRECT3DDEVICE9 device );
void DrawStringBackgroundRectangle ( LPDIRECT3DDEVICE9 device, int x1, int y1, int x2, int y2, DWORD color );
