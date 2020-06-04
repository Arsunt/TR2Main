/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
 * Original game is written by Core Design Ltd. in 1997.
 * Lara Croft and Tomb Raider are trademarks of Square Enix Ltd.
 *
 * This file is part of TR2Main.
 *
 * TR2Main is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TR2Main is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TR2Main.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "global/precompiled.h"
#include "specific/setupdlg.h"
#include "specific/init_display.h"
#include "specific/init_input.h"
#include "specific/init_sound.h"
#include "specific/registry.h"
#include "specific/setupwnd.h"
#include "specific/utils.h"
#include "specific/winvid.h"
#include "global/resource.h"
#include "global/vars.h"
#include <commctrl.h>

static LPCTSTR String_Software3d = "Software (256 Colors)";
static LPCTSTR String_Hardware3d = "Hardware 3D Acceleration";
static LPCTSTR String_FullScreen = "Full Screen";
static LPCTSTR String_Windowed = "Windowed";
static LPCTSTR String_ZBuffered = "Z Buffered";
static LPCTSTR String_BilinearFiltered = "Bilinear Filtered";
static LPCTSTR String_Dithered = "Dithered";
static LPCTSTR String_TripleBuffered = "Triple Buffered";
static LPCTSTR String_PerspectiveCorrect = "Perspective Correct";
static LPCTSTR String_None = "None";
static LPCTSTR String_NA = "n/a";
static LPCTSTR String_Enabled = "Enabled";
static LPCTSTR String_Disabled = "Disabled";
static LPCTSTR String_Lara = "Lara";
static LPCTSTR String_Camera = "Camera";
static LPCTSTR Strings_Aspect[] = {" (4:3)", " (16:9)", ""};

static LPCTSTR String_HighColor = "High Color";
static LPCTSTR String_TrueColor = "True Color";
static LPCTSTR String_256Color = "256 Color";
static LPCTSTR String_ModeX = "Mode X";
static LPCTSTR String_StandardVga = "Standard VGA";

static LPCTSTR String_VidTestPassed = "PASSED";
static LPCTSTR String_VidTestFailed = "FAILED";
static LPCTSTR String_VidSettingsOK = "Current Settings OK";

bool __cdecl OpenGameRegistryKey(LPCTSTR key) {
	if( key == NULL )
		return OpenRegistryKey(REG_TR2_PATH);

	char buf[256];
	wsprintf(buf, "%s\\%s", REG_TR2_PATH, key);
	return OpenRegistryKey(buf);
}

LONG __cdecl CloseGameRegistryKey() {
  return CloseRegistryKey();
}

bool __cdecl SE_WriteAppSettings(APP_SETTINGS *settings) {
	LPCTSTR guidString;

	if( !OpenGameRegistryKey(REG_SYSTEM_KEY) )
		return false;

	if( settings->PreferredDisplayAdapter != NULL && settings->PreferredDisplayAdapter != PrimaryDisplayAdapter ) {
		guidString = GuidBinaryToString(&settings->PreferredDisplayAdapter->body.adapterGuid);
		SetRegistryStringValue(REG_DISPLAY_ADAPTER, guidString, GUID_STRING_SIZE-1);
	} else {
		DeleteRegistryValue(REG_DISPLAY_ADAPTER);
	}

	if( settings->PreferredSoundAdapter != NULL && settings->PreferredSoundAdapter != PrimarySoundAdapter ) {
		guidString = GuidBinaryToString(&settings->PreferredSoundAdapter->body.adapterGuid);
		SetRegistryStringValue(REG_SOUND_ADAPTER, guidString, GUID_STRING_SIZE-1);
	} else {
		DeleteRegistryValue(REG_SOUND_ADAPTER);
	}

	if( settings->PreferredJoystick != NULL ) {
		guidString = GuidBinaryToString(&settings->PreferredJoystick->body.joystickGuid);
		SetRegistryStringValue(REG_JOYSTICK, guidString, GUID_STRING_SIZE-1);
	} else {
		DeleteRegistryValue(REG_JOYSTICK);
	}

	SetRegistryDwordValue(REG_RENDERER, settings->RenderMode);

	if( settings->PreferredDisplayAdapter != NULL && settings->VideoMode != NULL ) {
		SetRegistryDwordValue(REG_FS_WIDTH,		settings->VideoMode->body.width);
		SetRegistryDwordValue(REG_FS_HEIGHT,	settings->VideoMode->body.height);
		SetRegistryDwordValue(REG_FS_BPP,		settings->VideoMode->body.bpp);
	} else {
		DeleteRegistryValue(REG_FS_WIDTH);
		DeleteRegistryValue(REG_FS_HEIGHT);
		DeleteRegistryValue(REG_FS_BPP);
	}

	SetRegistryDwordValue(REG_WIN_WIDTH,		settings->WindowWidth);
	SetRegistryDwordValue(REG_WIN_HEIGHT,		settings->WindowHeight);
	SetRegistryDwordValue(REG_WIN_ASPECT,		settings->AspectMode);

	SetRegistryDwordValue(REG_ADJUST_MODE,		settings->TexelAdjustMode);
	SetRegistryDwordValue(REG_ADJUST_NEAREST,	settings->NearestAdjustment);
	SetRegistryDwordValue(REG_ADJUST_LINEAR,	settings->LinearAdjustment);

	SetRegistryBoolValue(REG_PERSPECTIVE,		settings->PerspectiveCorrect);
	SetRegistryBoolValue(REG_DITHER,			settings->Dither);
	SetRegistryBoolValue(REG_ZBUFFER,			settings->ZBuffer);
	SetRegistryBoolValue(REG_BILINEAR,			settings->BilinearFiltering);
	SetRegistryBoolValue(REG_TRIPLEBUFFER,		settings->TripleBuffering);
	SetRegistryBoolValue(REG_FULLSCREEN,		settings->FullScreen);
	SetRegistryBoolValue(REG_SOUND_ENABLE,		settings->SoundEnabled);
	SetRegistryBoolValue(REG_LARA_MIC,			settings->LaraMic);
	SetRegistryBoolValue(REG_JOY_ENABLE,		settings->JoystickEnabled);
	SetRegistryBoolValue(REG_16BIT_DISABLE,		settings->Disable16BitTextures);
	SetRegistryBoolValue(REG_SORT_DISABLE,		settings->DontSortPrimitives);
	SetRegistryBoolValue(REG_FMV_DISABLE,		settings->DisableFMV);

	CloseGameRegistryKey();
	return true;
}

int __cdecl SE_ReadAppSettings(APP_SETTINGS *settings) {
	bool rc;
	GUID guid;
	DISPLAY_MODE targetMode;
	DISPLAY_MODE_LIST *modeList;
	DISPLAY_MODE_NODE *mode;

	if( !OpenGameRegistryKey(REG_SYSTEM_KEY) )
		return 0;

	rc = GetRegistryGuidValue(REG_DISPLAY_ADAPTER, &guid, NULL);
	settings->PreferredDisplayAdapter = WinVidGetDisplayAdapter(rc ? &guid : NULL);

	rc = GetRegistryGuidValue(REG_SOUND_ADAPTER, &guid, NULL);
	settings->PreferredSoundAdapter = GetSoundAdapter(rc ? &guid : NULL);

	rc = GetRegistryGuidValue(REG_JOYSTICK, &guid, NULL);
	settings->PreferredJoystick = GetJoystick(rc ? &guid : NULL);

	GetRegistryDwordValue(REG_RENDERER, (DWORD *)&settings->RenderMode, RM_Hardware);
	if( settings->RenderMode < RM_Software || settings->RenderMode > RM_Hardware )
		settings->RenderMode = RM_Hardware;

	GetRegistryDwordValue(REG_FS_WIDTH,		(DWORD *)&targetMode.width,		640);
	GetRegistryDwordValue(REG_FS_HEIGHT,	(DWORD *)&targetMode.height,	480);
	GetRegistryDwordValue(REG_FS_BPP,		(DWORD *)&targetMode.bpp,		16);

	targetMode.vga = VGA_NoVga;
	if( settings->RenderMode == RM_Software )
		targetMode.bpp = 8;

	if( settings->RenderMode == RM_Hardware )
		modeList = &settings->PreferredDisplayAdapter->body.hwDispModeList;
	else
		modeList = &settings->PreferredDisplayAdapter->body.swDispModeList;

	for( mode = modeList->head; mode; mode = mode->next ) {
		if( !CompareVideoModes(&mode->body, &targetMode) )
			break;
	}
	settings->VideoMode = mode ? mode : modeList->tail;

	GetRegistryDwordValue(REG_WIN_WIDTH,	(DWORD *)&settings->WindowWidth,	512);
	GetRegistryDwordValue(REG_WIN_HEIGHT,	(DWORD *)&settings->WindowHeight,	384);
	GetRegistryDwordValue(REG_WIN_ASPECT,	(DWORD *)&settings->AspectMode,		AM_4_3);

	if( settings->AspectMode < AM_4_3 || settings->AspectMode > AM_Any )
		settings->AspectMode = AM_4_3;

	GetRegistryDwordValue(REG_ADJUST_MODE,		(DWORD *)&settings->TexelAdjustMode,	TAM_Always);
	GetRegistryDwordValue(REG_ADJUST_NEAREST,	(DWORD *)&settings->NearestAdjustment,	16);
	GetRegistryDwordValue(REG_ADJUST_LINEAR,	(DWORD *)&settings->LinearAdjustment,	128);

	if( settings->TexelAdjustMode < TAM_Disabled || settings->TexelAdjustMode > TAM_Always )
		settings->TexelAdjustMode = TAM_Always;

	CLAMP(settings->NearestAdjustment, 0, 256);
	CLAMP(settings->LinearAdjustment, 0, 256);

	GetRegistryBoolValue(REG_PERSPECTIVE,	&settings->PerspectiveCorrect,		settings->RenderMode == RM_Hardware);
	GetRegistryBoolValue(REG_DITHER,		&settings->Dither,					false);
	GetRegistryBoolValue(REG_ZBUFFER,		&settings->ZBuffer,					true);
	GetRegistryBoolValue(REG_BILINEAR,		&settings->BilinearFiltering,		true);
	GetRegistryBoolValue(REG_TRIPLEBUFFER,	&settings->TripleBuffering,			false);
	GetRegistryBoolValue(REG_FULLSCREEN,	&settings->FullScreen,				true);
	GetRegistryBoolValue(REG_SOUND_ENABLE,	&settings->SoundEnabled,			true);
	GetRegistryBoolValue(REG_LARA_MIC,		&settings->LaraMic,					false);
	GetRegistryBoolValue(REG_JOY_ENABLE,	&settings->JoystickEnabled,			true);
	GetRegistryBoolValue(REG_16BIT_DISABLE,	&settings->Disable16BitTextures,	false);
	GetRegistryBoolValue(REG_SORT_DISABLE,	&settings->DontSortPrimitives,		false);
	GetRegistryBoolValue(REG_FLIP_BROKEN,	&settings->FlipBroken,				false);
	GetRegistryBoolValue(REG_FMV_DISABLE,	&settings->DisableFMV,				false);

	CloseGameRegistryKey();
	return IsNewRegistryKeyCreated() ? 2 : 1;
}

bool __cdecl SE_GraphicsTestStart() {
	try {
		WinVidStart();
		RenderStart(true);
	} catch(int error) {
		LPCTSTR errorString = DecodeErrorMessage(error);
		// 'Test Result' Static
		SetDlgItemText(GraphicsDialogHandle, ID_GRAPH_STATIC_TESTRESULT, String_VidTestFailed);
		// 'Test Result Details' Static
		SetDlgItemText(GraphicsDialogHandle, ID_GRAPH_STATIC_TESTDETAIL, errorString);
		return false;
	}
	// 'Test Result' Static
	SetDlgItemText(GraphicsDialogHandle, ID_GRAPH_STATIC_TESTRESULT, String_VidTestPassed);
	// 'Test Result Details' Static
	SetDlgItemText(GraphicsDialogHandle, ID_GRAPH_STATIC_TESTDETAIL, String_VidSettingsOK);
	return true;
}

void __cdecl SE_GraphicsTestFinish() {
	RenderFinish(true);
	WinVidFinish();
	WinVidHideGameWindow();
}

int __cdecl SE_GraphicsTestExecute() {
	return 0;
}

int __cdecl SE_GraphicsTest() {
	int result = 0;
	APP_SETTINGS backupAppSettings = SavedAppSettings;

	if( SE_GraphicsTestStart() )
		result = SE_GraphicsTestExecute();

	SE_GraphicsTestFinish();
	SavedAppSettings = backupAppSettings;
	return result;
}

void __cdecl SE_DefaultGraphicsSettings() {
	DISPLAY_ADAPTER_NODE *adapter, *swAdapter, *hwAdapter;
	DISPLAY_MODE_LIST *modeList;
	DISPLAY_MODE_NODE *mode;
	DISPLAY_MODE targetMode;

	hwAdapter = NULL;
	swAdapter = NULL;

	for( adapter = DisplayAdapterList.head; adapter; adapter = adapter->next ) {
		if( adapter->body.lpAdapterGuid == NULL )
			swAdapter = adapter;
		if( adapter->body.hwRenderSupported && (hwAdapter == NULL || adapter != swAdapter) )
		{
			hwAdapter = adapter;
			ChangedAppSettings.PerspectiveCorrect = adapter->body.perspectiveCorrectSupported;
			ChangedAppSettings.Dither = adapter->body.ditherSupported;
			ChangedAppSettings.ZBuffer = adapter->body.zBufferSupported;
			ChangedAppSettings.BilinearFiltering = adapter->body.linearFilterSupported;
		}
	}

	if( hwAdapter != NULL ) {
		ChangedAppSettings.PreferredDisplayAdapter = hwAdapter;
		ChangedAppSettings.RenderMode = RM_Hardware;
	} else {
		ChangedAppSettings.PreferredDisplayAdapter = swAdapter;
		ChangedAppSettings.RenderMode = RM_Software;
		ChangedAppSettings.PerspectiveCorrect = true;
	}

	ChangedAppSettings.FullScreen = true;
	ChangedAppSettings.WindowWidth = 512;
	ChangedAppSettings.WindowHeight = 384;

	if( ChangedAppSettings.PreferredDisplayAdapter == NULL )
		return;

	targetMode.width = 640;
	targetMode.height = 480;
	targetMode.bpp = 16;
	targetMode.vga = VGA_NoVga;

	if( ChangedAppSettings.RenderMode == RM_Hardware )
		modeList = &ChangedAppSettings.PreferredDisplayAdapter->body.hwDispModeList;
	else
		modeList = &ChangedAppSettings.PreferredDisplayAdapter->body.swDispModeList;

	for( mode = modeList->head; mode; mode = mode->next ) {
		if( !CompareVideoModes(&mode->body, &targetMode) )
			break;
	}
	ChangedAppSettings.VideoMode = mode ? mode : modeList->tail;
}

bool __cdecl SE_SoundTestStart() {
	BYTE *waveFile;
	WAVEFORMATEX format;
	DWORD dataSize;
	LPVOID data;
	try {
		WinSndStart(SoundDialogHandle);
	} catch(...) {
		return false;
	}
	waveFile = (BYTE *)UT_LoadResource(MAKEINTRESOURCE(IDR_WAVESAMPLE), "WAVE");
	format = *(WAVEFORMATEX *)&waveFile[0x14];
	format.cbSize = 0;
	dataSize = *(DWORD *)&waveFile[0x28];
	data = &waveFile[0x2C];

	return WinSndMakeSample(0, &format, data, dataSize);
}

void __cdecl SE_SoundTestFinish() {
	WinSndFinish();
}

int __cdecl SE_SoundTestExecute() {
	int channel = WinSndPlaySample(0, VOLUME_PCT(90), PHD_ONE, DSBPAN_CENTER, 0);

	if( channel >= 0 )
		while( WinSndIsChannelPlaying(channel) ) /* just wait */;

	return 0;
}

int __cdecl SE_SoundTest() {
	int result = 0;

	APP_SETTINGS backupAppSettings = SavedAppSettings;
	SavedAppSettings = ChangedAppSettings;

	if( SE_SoundTestStart() )
		result = SE_SoundTestExecute();

	SE_SoundTestFinish();
	SavedAppSettings = backupAppSettings;
	return result;
}

int CALLBACK SE_PropSheetCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam) {
	switch( uMsg ) {
		case PSCB_PRECREATE :
			GraphicsDialogHandle = NULL;
			break;

		case PSCB_INITIALIZED :
			SE_PropSheetWndHandle = hwndDlg;
			SE_OldPropSheetWndProc = (WNDPROC)SetWindowLong(hwndDlg, GWL_WNDPROC, (LONG)SE_NewPropSheetWndProc);
			break;
	}
	return 0;
}

LRESULT CALLBACK SE_NewPropSheetWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch( uMsg ) {
		case WM_DESTROY :
			SetWindowLong(hWnd, GWL_WNDPROC, (LONG)SE_OldPropSheetWndProc);
			break;

		case WM_DISPLAYCHANGE :
			if( GraphicsDialogHandle != NULL )
				PostMessage(GraphicsDialogHandle, uMsg, wParam, lParam);
			break;
	}
	return CallWindowProc(SE_OldPropSheetWndProc, hWnd, uMsg, wParam, lParam);
}

bool __cdecl SE_ShowSetupDialog(HWND hParent, bool isDefault) {
	static bool initOnce = false;
	PROPSHEETHEADER sheetHeader;
	PROPSHEETPAGE sheetPages[5];

	ChangedAppSettings = SavedAppSettings;
	if( isDefault ) {
		SE_DefaultGraphicsSettings();
		ChangedAppSettings.SoundEnabled = true;
	}

	memset(sheetPages, 0, sizeof(sheetPages));
	memset(&sheetHeader, 0, sizeof(sheetHeader));
	for( int i=0; i<5; ++i ) {
		sheetPages[i].dwSize = sizeof(PROPSHEETPAGE);
		sheetPages[i].dwFlags = PSP_DEFAULT;
		sheetPages[i].hInstance = GameModule;
	}

	sheetPages[0].pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS); // 'Options' Tab
	sheetPages[0].pfnDlgProc = SE_OptionsDlgProc;
	sheetPages[1].pszTemplate = MAKEINTRESOURCE(IDD_GRAPHICS); // 'Graphics' Tab
	sheetPages[1].pfnDlgProc = SE_GraphicsDlgProc;
	sheetPages[2].pszTemplate = MAKEINTRESOURCE(IDD_SOUND); // 'Sound' Tab
	sheetPages[2].pfnDlgProc = SE_SoundDlgProc;
	sheetPages[3].pszTemplate = MAKEINTRESOURCE(IDD_CONTROLS); // 'Controls' Tab
	sheetPages[3].pfnDlgProc = SE_ControlsDlgProc;
	sheetPages[4].pszTemplate = MAKEINTRESOURCE(IDD_ADVANCED); // 'Advanced' Tab
	sheetPages[4].pfnDlgProc = SE_AdvancedDlgProc;

	sheetHeader.dwSize = sizeof(PROPSHEETHEADER);
	sheetHeader.dwFlags = PSH_USECALLBACK|PSH_NOAPPLYNOW|PSH_PROPSHEETPAGE|PSH_USEHICON;
	sheetHeader.hwndParent = hParent;
	sheetHeader.hInstance = GameModule;
	sheetHeader.hIcon = (HICON)LoadImage(GameModule, MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	sheetHeader.pszCaption = GameDialogName;
	sheetHeader.nPages = 5;
	sheetHeader.nStartPage = 1;
	sheetHeader.ppsp = sheetPages;
	sheetHeader.pfnCallback = SE_PropSheetCallback;

	if( !initOnce ) {
		InitCommonControls();
		SE_RegisterSetupWindowClass();
		initOnce = true;
	}
	IsSetupDialogCentered = false;

	if( PropertySheet(&sheetHeader) != 1 )
		return false;

	SavedAppSettings = ChangedAppSettings;
	return true;
}

INT_PTR CALLBACK SE_GraphicsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static bool isGraphicsTest = false;

	switch( uMsg ) {
		case WM_INITDIALOG :
			GraphicsDialogHandle = hwndDlg;
			isGraphicsTest = false;
			if( !IsSetupDialogCentered ) {
				UT_CenterWindow(GetParent(hwndDlg));
				IsSetupDialogCentered = true;
			}
			SE_GraphicsDlgInit(hwndDlg);
			return 1;

		case WM_DESTROY :
			GraphicsDialogHandle = NULL;
			break;

		case WM_DISPLAYCHANGE :
			if( !isGraphicsTest ) {
				if( SE_DisplayAdapter != NULL )
					WinVidSetDisplayAdapter(&SE_DisplayAdapter->body);
				SE_GraphicsDlgUpdate(hwndDlg);
			}
			break;

		case WM_HSCROLL :
			if( (HWND)lParam == GetDlgItem(hwndDlg, ID_GRAPH_SLIDER_WINDOWED) ) { // 'Windowed Mode' Slider
				SE_GraphicsDlgUpdate(hwndDlg);
			}
			break;

		case WM_COMMAND :
			if( HIWORD(wParam) == 0 ) { // Buttons
				bool isCheck = ( 1 == SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) );
				switch( LOWORD(wParam) ) {
					case ID_GRAPH_BUTTON_SWRENDER : // 'Software 256 Colour Renderer' RadioButton
						ChangedAppSettings.RenderMode = RM_Software;
						SE_GraphicsDlgFullScreenModesUpdate(hwndDlg);
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_HWRENDER : // 'Hardware 3D Acceleration' RadioButton
						ChangedAppSettings.RenderMode = RM_Hardware;
						SE_GraphicsDlgFullScreenModesUpdate(hwndDlg);
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_PERSPECTIVE : // 'Perspective Correct' CheckBox
						ChangedAppSettings.PerspectiveCorrect = isCheck;
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_DITHER : // 'Dither' CheckBox
						ChangedAppSettings.Dither = isCheck;
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_ZBUFFER : // 'Z Buffer' CheckBox
						ChangedAppSettings.ZBuffer = isCheck;
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_BILINEAR : // 'Bilinear Filter' CheckBox
						ChangedAppSettings.BilinearFiltering = isCheck;
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_TRIPLEBUFFER : // 'Triple Buffer' CheckBox
						ChangedAppSettings.TripleBuffering = isCheck;
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_FULLSCREEN : // 'Full Screen' RadioButton
						ChangedAppSettings.FullScreen = true;
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_WINDOWED : // 'Windowed' RadioButton
						ChangedAppSettings.FullScreen = false;
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_TEST : // 'Test' Button
						isGraphicsTest = true;
						if( 0 != SE_GraphicsTest() ) {
							// test fatal error - close Setup Dialog (this never happens though)
							PostMessage(SE_PropSheetWndHandle, PSM_PRESSBUTTON, PSBTN_CANCEL, 0);
						}
						isGraphicsTest = false;
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_ASPECT_4_3 : // '4:3' RadioButton
						ChangedAppSettings.AspectMode = AM_4_3;
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_ASPECT_16_9 : // '16:9' RadioButton
						ChangedAppSettings.AspectMode = AM_16_9;
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_ASPECT_ANY : // 'Any' RadioButton
						ChangedAppSettings.AspectMode = AM_Any;
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_BUTTON_DEFAULT : // 'Use Defaults' Button
						SE_DefaultGraphicsSettings();
						SE_GraphicsDlgInit(hwndDlg);
						break;
					default:
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
				}
			} else if( HIWORD(wParam) == 1 ) { // ComboBoxes
				DWORD selectedIndex = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				DISPLAY_MODE_NODE *selected = (DISPLAY_MODE_NODE *)SendMessage((HWND)lParam, CB_GETITEMDATA, selectedIndex, 0);
				switch( LOWORD(wParam) ) {
					case ID_GRAPH_COMBOBOX_ADAPTER : // 'Graphics Card' ComboBox
						SE_GraphicsAdapterSet(hwndDlg, (DISPLAY_ADAPTER_NODE *)selected);
						SE_GraphicsDlgUpdate(hwndDlg);
						break;
					case ID_GRAPH_COMBOBOX_FULLSCREEN : // 'Full Screen Mode' ComboBox
						ChangedAppSettings.VideoMode = selected;
						SE_FullScreenMode = selected->body;
						break;
				}
			}
			break;
	}
	return 0;
}

void __cdecl SE_GraphicsDlgFullScreenModesUpdate(HWND hwndDlg) {
	HWND comboBox;
	DWORD addedIndex, selectedIndex;
	DISPLAY_MODE_LIST *modeList;
	DISPLAY_MODE_NODE *mode, *selected;
	char stringBuf[256];

	comboBox = GetDlgItem(hwndDlg, ID_GRAPH_COMBOBOX_FULLSCREEN); // 'Full Screen Mode' ComboBox
	SendMessage(comboBox, CB_RESETCONTENT, 0, 0);

	if( ChangedAppSettings.RenderMode == RM_Hardware ) {
		modeList = &ChangedAppSettings.PreferredDisplayAdapter->body.hwDispModeList;
	} else {
		modeList = &ChangedAppSettings.PreferredDisplayAdapter->body.swDispModeList;
		SE_FullScreenMode.bpp = 8;
	}
	SE_FullScreenMode.vga = VGA_NoVga;

	if( modeList->dwCount == 0 || modeList->head == NULL ) {
		EnableWindow(comboBox, FALSE);
		return;
	}

	for( mode = modeList->head; mode; mode = mode->next ) {
		if( !CompareVideoModes(&mode->body, &SE_FullScreenMode) )
			break;
	}
	selected = mode ? mode : modeList->tail;

	selectedIndex = 0;
	for( mode = modeList->head; mode; mode = mode->next ) {
		LPCTSTR lpColorString = "";
		switch( mode->body.vga ) {
			case VGA_NoVga :
				switch( mode->body.bpp ) {
					case 16:
						lpColorString = String_HighColor;
						break;
					case 24:
					case 32:
						lpColorString = String_TrueColor;
						break;
				}
				break;
			case VGA_256Color :
				lpColorString = String_256Color;
				break;
			case VGA_ModeX :
				lpColorString = String_ModeX;
				break;
			case VGA_Standard :
				lpColorString = String_StandardVga;
				break;
		}
		wsprintf(stringBuf, "%dx%d %s", mode->body.width, mode->body.height, lpColorString);
		addedIndex = SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)stringBuf);
		SendMessage(comboBox, CB_SETITEMDATA, addedIndex, (LPARAM)mode);
		if( mode == selected )
			selectedIndex = addedIndex;
	}

	SendMessage(comboBox, CB_SETCURSEL, selectedIndex, 0);
	selected = (DISPLAY_MODE_NODE *)SendMessage(comboBox, CB_GETITEMDATA, selectedIndex, 0);
	ChangedAppSettings.VideoMode = selected;

	if( selected != NULL )
		SE_FullScreenMode = selected->body;

	EnableWindow(comboBox, (modeList->dwCount > 1));
}

void __cdecl SE_GraphicsAdapterSet(HWND hwndDlg, DISPLAY_ADAPTER_NODE *adapter) {
	ChangedAppSettings.PreferredDisplayAdapter = adapter;
	SE_GraphicsDlgFullScreenModesUpdate(hwndDlg);
}

void __cdecl SE_GraphicsDlgUpdate(HWND hwndDlg) {
	HWND hItem;
	char stringWindowRes[16];
	bool renderAvailable;
	bool perspectiveCorrectAvailable;
	bool ditherAvailable;
	bool zBufferAvailable;
	bool bilinearFilteringAvailable;
	bool tripleBufferingAvailable;
	bool windowedSizeAvailable;
	bool windowedModeAvailable;
	bool fullScreenModeAvailable;
	bool fullScreenModeListAvailable;
	bool windowedModeListAvailable;
	DWORD fullScreenVideoModesCount;
	DISPLAY_ADAPTER *preferred = &ChangedAppSettings.PreferredDisplayAdapter->body;
	bool isSWSupported = ( preferred->swDispModeList.dwCount > 0 || preferred->swWindowedSupported );
	bool isHWSupported = ( preferred->hwDispModeList.dwCount > 0 || preferred->hwWindowedSupported );

	if( (ChangedAppSettings.RenderMode == RM_Software && !isSWSupported) ||
		(ChangedAppSettings.RenderMode == RM_Hardware && !isHWSupported) )
	{
		ChangedAppSettings.RenderMode = RM_Unknown;
	}

	if( ChangedAppSettings.RenderMode == RM_Unknown ) {
		if( isHWSupported ) {
			ChangedAppSettings.RenderMode = RM_Hardware;
		} else if( isSWSupported ) {
			ChangedAppSettings.RenderMode = RM_Software;
		}
		SE_GraphicsDlgFullScreenModesUpdate(hwndDlg);
	}

	if( ChangedAppSettings.RenderMode == RM_Software ) {
		// Software Renderer
		perspectiveCorrectAvailable = true;
		ditherAvailable = false;
		zBufferAvailable = false;
		bilinearFilteringAvailable = false;
		tripleBufferingAvailable = true;
		windowedModeAvailable = preferred->swWindowedSupported;
		fullScreenModeAvailable = ( preferred->swDispModeList.dwCount > 0 );
		fullScreenVideoModesCount = preferred->swDispModeList.dwCount;
	} else if( ChangedAppSettings.RenderMode == RM_Hardware ) {
		// Hardware Renderer
		perspectiveCorrectAvailable = preferred->perspectiveCorrectSupported;
		ditherAvailable = preferred->ditherSupported;
		zBufferAvailable = preferred->zBufferSupported;
		bilinearFilteringAvailable = preferred->linearFilterSupported;
		tripleBufferingAvailable = true;
		windowedModeAvailable = preferred->hwWindowedSupported;
		fullScreenModeAvailable = ( preferred->hwDispModeList.dwCount > 0 );
		fullScreenVideoModesCount = preferred->hwDispModeList.dwCount;
	} else {
		// Unknown Renderer
		perspectiveCorrectAvailable = false;
		ditherAvailable = false;
		zBufferAvailable = false;
		bilinearFilteringAvailable = false;
		tripleBufferingAvailable = false;
		windowedModeAvailable = false;
		fullScreenModeAvailable = false;
		fullScreenVideoModesCount = 0;
	}

	if( !windowedModeAvailable ) {
		ChangedAppSettings.FullScreen = true;
	} else if( !fullScreenModeAvailable ) {
		ChangedAppSettings.FullScreen = false;
	}

	if( !ChangedAppSettings.FullScreen )
		tripleBufferingAvailable = false;

	if( !perspectiveCorrectAvailable )
		ChangedAppSettings.PerspectiveCorrect = false;
	if( !ditherAvailable )
		ChangedAppSettings.Dither = false;
	if( !zBufferAvailable )
		ChangedAppSettings.ZBuffer = false;
	if( !bilinearFilteringAvailable )
		ChangedAppSettings.BilinearFiltering = false;
	if( !tripleBufferingAvailable )
		ChangedAppSettings.TripleBuffering = false;

	renderAvailable = ( ChangedAppSettings.RenderMode != RM_Unknown );
	windowedSizeAvailable = ( preferred->screenWidth > 0 );
	fullScreenModeListAvailable = ( ChangedAppSettings.FullScreen && renderAvailable );
	windowedModeListAvailable = ( !ChangedAppSettings.FullScreen && windowedSizeAvailable );

	// 'Graphics Output Method' GroupBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_GROUPBOX_RENDER);
	EnableWindow(hItem, renderAvailable);

	// 'Software 256 Colour Renderer' RadioButton
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_SWRENDER);
	EnableWindow(hItem, isSWSupported);
	SendMessage(hItem, BM_SETCHECK, (ChangedAppSettings.RenderMode == RM_Software), 0);

	// 'Hardware 3D Acceleration' RadioButton
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_HWRENDER);
	EnableWindow(hItem, isHWSupported);
	SendMessage(hItem, BM_SETCHECK, (ChangedAppSettings.RenderMode == RM_Hardware), 0);

	// 'Perspective Correct' CheckBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_PERSPECTIVE);
	EnableWindow(hItem, perspectiveCorrectAvailable);
	SendMessage(hItem, BM_SETCHECK, ChangedAppSettings.PerspectiveCorrect, 0);

	// 'Dither' CheckBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_DITHER);
	EnableWindow(hItem, ditherAvailable);
	SendMessage(hItem, BM_SETCHECK, ChangedAppSettings.Dither, 0);

	// 'Triple Buffer' CheckBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_TRIPLEBUFFER);
	EnableWindow(hItem, tripleBufferingAvailable);
	SendMessage(hItem, BM_SETCHECK, ChangedAppSettings.TripleBuffering, 0);

	// 'Z Buffer' CheckBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_ZBUFFER);
	EnableWindow(hItem, zBufferAvailable);
	SendMessage(hItem, BM_SETCHECK, ChangedAppSettings.ZBuffer, 0);

	// 'Bilinear Filter' CheckBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_BILINEAR);
	EnableWindow(hItem, bilinearFilteringAvailable);
	SendMessage(hItem, BM_SETCHECK, ChangedAppSettings.BilinearFiltering, 0);

	// 'Display Type' GroupBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_GROUPBOX_DISPLAY);
	EnableWindow(hItem, (windowedModeAvailable || fullScreenModeAvailable));

	// 'Windowed' RadioButton
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_WINDOWED);
	EnableWindow(hItem, windowedModeAvailable);
	SendMessage(hItem, BM_SETCHECK, (!ChangedAppSettings.FullScreen && windowedModeAvailable), 0);

	// 'Full Screen' RadioButton
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_FULLSCREEN);
	EnableWindow(hItem, fullScreenModeAvailable);
	SendMessage(hItem, BM_SETCHECK, (ChangedAppSettings.FullScreen && fullScreenModeAvailable), 0);

	// 'Full Screen Mode' GroupBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_GROUPBOX_FULLSCREEN);
	EnableWindow(hItem, fullScreenModeListAvailable);

	// 'Full Screen Mode' ComboBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_COMBOBOX_FULLSCREEN);
	EnableWindow(hItem, (fullScreenModeListAvailable && fullScreenVideoModesCount > 1));

	// 'Windowed Mode' GroupBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_GROUPBOX_WINDOWED);
	EnableWindow(hItem, windowedModeListAvailable);

	// 'Small Windowed Mode' Static
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_STATIC_RES_SMALL);
	EnableWindow(hItem, windowedModeListAvailable);

	// 'Large Windowed Mode' Static
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_STATIC_RES_LARGE);
	EnableWindow(hItem, windowedModeListAvailable);

	// 'Windowed Mode Resolution' Static
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_STATIC_RESOLUTION);
	EnableWindow(hItem, windowedModeListAvailable);

	// '4:3 Aspect' RadioButton
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_ASPECT_4_3);
	EnableWindow(hItem, windowedModeListAvailable);
	SendMessage(hItem, BM_SETCHECK, (ChangedAppSettings.AspectMode == AM_4_3), 0);

	// '16:9 Aspect' RadioButton
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_ASPECT_16_9);
	EnableWindow(hItem, windowedModeListAvailable);
	SendMessage(hItem, BM_SETCHECK, (ChangedAppSettings.AspectMode == AM_16_9), 0);

	// 'Any Aspect' RadioButton
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_ASPECT_ANY);
	EnableWindow(hItem, windowedModeListAvailable);
	SendMessage(hItem, BM_SETCHECK, (ChangedAppSettings.AspectMode == AM_Any), 0);

	// 'Windowed Mode' Slider
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_SLIDER_WINDOWED);
	EnableWindow(hItem, windowedModeListAvailable);
	if( windowedSizeAvailable ) {
		int minPosition = 0;
		int maxWidth = preferred->screenWidth;
		int maxPosition = ((maxWidth - 320 ) / 32) & 0xFFFF;

		SendMessage(hItem, TBM_SETRANGE, 1, ((maxPosition<<0x10)|minPosition));
		DWORD sliderPosition = SendMessage(hItem, TBM_GETPOS, 0, 0);
		ChangedAppSettings.WindowWidth = sliderPosition * 32 + 320;

		switch( ChangedAppSettings.AspectMode ) {
			case AM_4_3 :
				ChangedAppSettings.WindowHeight = ChangedAppSettings.WindowWidth*3/4;
				break;
			case AM_16_9 :
				ChangedAppSettings.WindowHeight = ChangedAppSettings.WindowWidth*9/16;
				break;
			case AM_Any :
#ifdef FEATURE_VIDMODESORT
				ChangedAppSettings.WindowHeight = ChangedAppSettings.WindowWidth*3/4;
#else // !FEATURE_VIDMODESORT
				ChangedAppSettings.WindowHeight = ChangedAppSettings.WindowWidth;
#endif // FEATURE_VIDMODESORT
				break;
		}
	}

	// 'Windowed Mode Resolution' Static
	if( windowedSizeAvailable )
		wsprintf(stringWindowRes, "%dx%d", ChangedAppSettings.WindowWidth, ChangedAppSettings.WindowHeight);
	else
		*stringWindowRes = 0;
	SetDlgItemText(hwndDlg, ID_GRAPH_STATIC_RESOLUTION, stringWindowRes);

	// 'Graphics Options' GroupBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_GROUPBOX_OPTIONS);
	EnableWindow(hItem, renderAvailable);

	// 'Test Settings' GroupBox
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_GROUPBOX_TEST);
	EnableWindow(hItem, renderAvailable);

	// 'Test' Button
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_BUTTON_TEST);
	EnableWindow(hItem, renderAvailable);

	// 'Test Result' Static
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_STATIC_TESTRESULT);
	EnableWindow(hItem, renderAvailable);

	// 'Test Details' Static
	hItem = GetDlgItem(hwndDlg, ID_GRAPH_STATIC_TESTDETAIL);
	EnableWindow(hItem, renderAvailable);
}

void __cdecl SE_GraphicsDlgInit(HWND hwndDlg) {
	HWND comboBox, slider;
	DWORD addedIndex, selectedIndex;
	DISPLAY_ADAPTER_NODE *adapter, *selected;

	comboBox = GetDlgItem(hwndDlg, ID_GRAPH_COMBOBOX_ADAPTER); // 'Graphics Card' ComboBox
	SendMessage(comboBox, CB_RESETCONTENT, 0, 0);

	selectedIndex = 0;
	SE_DisplayAdapter = NULL;
	for( adapter = DisplayAdapterList.head; adapter; adapter = adapter->next ) {
		addedIndex = SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)adapter->body.driverDescription.lpString);
		SendMessage(comboBox, CB_SETITEMDATA, addedIndex, (LPARAM)adapter);
		if( adapter == ChangedAppSettings.PreferredDisplayAdapter )
			selectedIndex = addedIndex;
		if( adapter->body.lpAdapterGuid == NULL )
			SE_DisplayAdapter = adapter;
		WinVidSetDisplayAdapter(&adapter->body);
	}

	SendMessage(comboBox, CB_SETCURSEL, selectedIndex, 0);
	EnableWindow(comboBox, (DisplayAdapterList.dwCount > 1)); // NOTE: original was be >0. But there is no reason to select 1/1 adapter
	if( DisplayAdapterList.dwCount > 0 ) {
		selected = (DISPLAY_ADAPTER_NODE *)SendMessage(comboBox, CB_GETITEMDATA, selectedIndex, 0);
		if( ChangedAppSettings.VideoMode != NULL ) {
			SE_FullScreenMode = ChangedAppSettings.VideoMode->body;
			SE_GraphicsAdapterSet(hwndDlg, selected);
		} else {
			SE_FullScreenMode.width = 640;
			SE_FullScreenMode.height = 480;
			SE_FullScreenMode.bpp = 0;
			SE_FullScreenMode.vga = VGA_NoVga;
			SE_GraphicsAdapterSet(hwndDlg, selected);
		}
	}

	slider = GetDlgItem(hwndDlg, ID_GRAPH_SLIDER_WINDOWED); // 'Windowed Mode' Slider
	SendMessage(slider, TBM_SETPOS, TRUE, (ChangedAppSettings.WindowWidth - 320) / 32);

	SE_GraphicsDlgUpdate(hwndDlg);
}

INT_PTR CALLBACK SE_SoundDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HWND hImage = NULL;

	switch( uMsg ) {
		case WM_INITDIALOG :
			SoundDialogHandle = hwndDlg;
			hImage = GetDlgItem(hwndDlg, ID_SOUND_WINDOW_IMAGE);
			SE_SoundDlgInit(hwndDlg);
			return 1;

		case WM_COMMAND :
			if( HIWORD(wParam) == 0 ) { // Buttons
				switch( LOWORD(wParam) ) {
					case ID_SOUND_BUTTON_SFX_ENABLE : // 'Enable Sound' CheckBox
						ChangedAppSettings.SoundEnabled = (1 == SendMessage((HWND)lParam, BM_GETCHECK, 0, 0));
						break;
					case ID_SOUND_BUTTON_MIC_CAMERA : // 'Camera Microphone Position' RadioButton
						ChangedAppSettings.LaraMic = false;
						break;
					case ID_SOUND_BUTTON_MIC_LARA : // 'Lara Microphone Position' RadioButton
						ChangedAppSettings.LaraMic = true;
						break;
					case ID_SOUND_BUTTON_TEST : // 'Test' Button
						if( 0 != SE_SoundTest() ) {
							// test fatal error - close Setup Dialog (this never happens though)
							PostMessage(SE_PropSheetWndHandle, PSM_PRESSBUTTON, PSBTN_CANCEL, 0);
						}
						SendMessage((HWND)lParam, BM_SETCHECK, 0, 0);
						break;
				}
			} else if( HIWORD(wParam) == 1 ) { // ComboBoxes
				if( LOWORD(wParam) == ID_SOUND_COMBOBOX_ADAPTER ) { // 'Sound Card' ComboBox
					DWORD index = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
					SOUND_ADAPTER_NODE *adapter = (SOUND_ADAPTER_NODE *)SendMessage((HWND)lParam, CB_GETITEMDATA, index, 0);
					SE_SoundAdapterSet(hwndDlg, adapter);
					SE_SoundDlgUpdate(hwndDlg);
				}
			}
			break;
	}
	SE_PassMessageToImage(hImage, uMsg, wParam);
	return 0;
}

void __cdecl SE_SoundAdapterSet(HWND hwndDlg, SOUND_ADAPTER_NODE *adapter) {
	ChangedAppSettings.PreferredSoundAdapter = adapter;
}

void __cdecl SE_SoundDlgUpdate(HWND hwndDlg) {
	HWND hItem;
	bool isAvailable = ( SoundAdapterList.dwCount > 0 );

	// 'Microphone Position' GroupBox
	hItem = GetDlgItem(hwndDlg, ID_SOUND_GROUPBOX_MIC);
	EnableWindow(hItem, isAvailable);

	// 'Sound Output' GroupBox
	hItem = (HWND)GetDlgItem(hwndDlg, ID_SOUND_GROUPBOX_OUTPUT);
	EnableWindow(hItem, isAvailable);

	// 'Enable Sound' CheckBox
	hItem = (HWND)GetDlgItem(hwndDlg, ID_SOUND_BUTTON_SFX_ENABLE);
	EnableWindow(hItem, isAvailable);
	SendMessage(hItem, BM_SETCHECK, (isAvailable && ChangedAppSettings.SoundEnabled), 0);

	// 'Camera' RadioButton
	hItem = (HWND)GetDlgItem(hwndDlg, ID_SOUND_BUTTON_MIC_CAMERA);
	EnableWindow(hItem, isAvailable);
	SendMessage(hItem, BM_SETCHECK, !ChangedAppSettings.LaraMic, 0);

	// 'Lara' RadioButton
	hItem = (HWND)GetDlgItem(hwndDlg, ID_SOUND_BUTTON_MIC_LARA);
	EnableWindow(hItem, isAvailable);
	SendMessage(hItem, BM_SETCHECK, ChangedAppSettings.LaraMic, 0);

	// 'Test' Button
	hItem = (HWND)GetDlgItem(hwndDlg, ID_SOUND_BUTTON_TEST);
	EnableWindow(hItem, isAvailable);
}

void __cdecl SE_SoundDlgInit(HWND hwndDlg) {
	HWND comboBox;
	DWORD addedIndex, selectedIndex;
	SOUND_ADAPTER_NODE *adapter, *selected;

	// 'Sound Adapter' ComboBox
	comboBox = GetDlgItem(hwndDlg, ID_SOUND_COMBOBOX_ADAPTER);
	SendMessage(comboBox, CB_RESETCONTENT, 0, 0);

	selectedIndex = 0;
	for( adapter = SoundAdapterList.head; adapter; adapter = adapter->next ) {
		addedIndex = SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)adapter->body.description.lpString);
		SendMessage(comboBox, CB_SETITEMDATA, addedIndex, (LPARAM)adapter);
		if( adapter == ChangedAppSettings.PreferredSoundAdapter )
			selectedIndex = addedIndex;
	}

	SendMessage(comboBox, CB_SETCURSEL, selectedIndex, 0);
	EnableWindow(comboBox, (SoundAdapterList.dwCount > 1));
	if( SoundAdapterList.dwCount > 0 ) {
		selected = (SOUND_ADAPTER_NODE *)SendMessage(comboBox, CB_GETITEMDATA, selectedIndex, 0);
		SE_SoundAdapterSet(hwndDlg, selected);
	}
	SE_SoundDlgUpdate(hwndDlg);
}

INT_PTR CALLBACK SE_ControlsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HWND hImage = NULL;

	switch( uMsg ) {
		case WM_INITDIALOG :
			hImage = GetDlgItem(hwndDlg, ID_CTRLS_WINDOW_IMAGE);
			SE_ControlsDlgInit(hwndDlg);
			return 1;

		case WM_COMMAND :
			if( HIWORD(wParam) == 0 ) { // Buttons
				switch( LOWORD(wParam) ) {
					case ID_CTRLS_BUTTON_JOYSTICK_ENABLE : // 'Enable Joystick' checkbox
						ChangedAppSettings.JoystickEnabled = (1 == SendMessage((HWND)lParam, BM_GETCHECK, 0, 0));
						break;
					case ID_CTRLS_BUTTON_CTL_PANEL : // 'Run Control Panel' Button
						DInputCreate();
						WinInRunControlPanel(hwndDlg);
						DInputRelease();
						break;
				}
			} else if( HIWORD(wParam) == 1 ) { // ComboBoxes
				if( LOWORD(wParam) == ID_CTRLS_COMBOBOX_JOYSTICK ) { // 'Joystick' ComboBox
					DWORD index = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
					JOYSTICK_NODE *joystick = (JOYSTICK_NODE *)SendMessage((HWND)lParam, CB_GETITEMDATA, index, 0);
					SE_ControlsJoystickSet(hwndDlg, joystick);
					SE_ControlsDlgUpdate(hwndDlg);
				}
			}
	}
	SE_PassMessageToImage(hImage, uMsg, wParam);
	return 0;
}

void __cdecl SE_ControlsJoystickSet(HWND hwndDlg, JOYSTICK_NODE *joystick) {
	ChangedAppSettings.PreferredJoystick = joystick;
}

void __cdecl SE_ControlsDlgUpdate(HWND hwndDlg) {
	bool isAvailable = ( JoystickList.dwCount > 0 );
	bool isCheck = ( isAvailable && ChangedAppSettings.JoystickEnabled );
	// 'Enable Joystick' CheckBox
	HWND checkBox = GetDlgItem(hwndDlg, ID_CTRLS_BUTTON_JOYSTICK_ENABLE);

	EnableWindow(checkBox, isAvailable);
	SendMessage(checkBox, BM_SETCHECK, isCheck, 0);
}

void __cdecl SE_ControlsDlgInit(HWND hwndDlg) {
	HWND comboBox;
	DWORD addedIndex, selectedIndex;
	JOYSTICK_NODE *joystick, *selected;

	comboBox = GetDlgItem(hwndDlg, ID_CTRLS_COMBOBOX_JOYSTICK); // 'Joystick' ComboBox
	SendMessage(comboBox, CB_RESETCONTENT, 0, 0);

	selectedIndex = 0;
	for( joystick = JoystickList.head; joystick; joystick = joystick->next ) {
		addedIndex = SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)joystick->body.productName.lpString);
		SendMessage(comboBox, CB_SETITEMDATA, addedIndex, (LPARAM)joystick);
		if( joystick == ChangedAppSettings.PreferredJoystick )
			selectedIndex = addedIndex;
	}

	SendMessage(comboBox, CB_SETCURSEL, selectedIndex, 0);
	EnableWindow(comboBox, (JoystickList.dwCount > 1)); // NOTE: original was >0. But there is no reason to select 1/1 joystick
	if( JoystickList.dwCount > 0 ) {
		selected = (JOYSTICK_NODE *)SendMessage(comboBox, CB_GETITEMDATA, selectedIndex, 0);
		SE_ControlsJoystickSet(hwndDlg, selected);
	}
	SE_ControlsDlgUpdate(hwndDlg);
}

INT_PTR CALLBACK SE_OptionsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HWND hImage = NULL;
	switch( uMsg ) {
		case WM_INITDIALOG :
			hImage = GetDlgItem(hwndDlg, ID_OPTNS_WINDOW_IMAGE);
			if( !IsSetupDialogCentered ) {
				UT_CenterWindow(GetParent(hwndDlg));
				IsSetupDialogCentered = true;
			}
			return 1;

		case WM_NOTIFY :
			if( ((LPNMHDR)lParam)->code == ((UINT)PSN_SETACTIVE) )
				SE_OptionsDlgUpdate(hwndDlg);
			break;
	}
	SE_PassMessageToImage(hImage, uMsg, wParam);
	return 0;
}

void __cdecl SE_OptionsDlgUpdate(HWND hwndDlg) {
	LPCTSTR lpString;
	char resultString[256];

	if( ChangedAppSettings.PreferredDisplayAdapter != NULL ) {
		// 'Graphics' Static
		SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_GRAPHICS, ChangedAppSettings.PreferredDisplayAdapter->body.driverDescription.lpString);

		// 'Using:' Static
		if( ChangedAppSettings.RenderMode == RM_Hardware )
			lpString = String_Hardware3d;
		else
			lpString = String_Software3d;
		SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_USING, lpString);

		// 'Display:' Static
		if( ChangedAppSettings.FullScreen && ChangedAppSettings.VideoMode )
			wsprintf(resultString, "%s %dx%dx%d", String_FullScreen, ChangedAppSettings.VideoMode->body.width, ChangedAppSettings.VideoMode->body.height, ChangedAppSettings.VideoMode->body.bpp);
		else
			wsprintf(resultString, "%s %dx%d%s", String_Windowed, ChangedAppSettings.WindowWidth, ChangedAppSettings.WindowHeight, Strings_Aspect[ChangedAppSettings.AspectMode]);
		SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_DISPLAY, resultString);

		// 'Options:' Static
		bool isNext = false;
		LPTSTR pResultString = resultString;
		SE_OptionsStrCat(&pResultString, ChangedAppSettings.ZBuffer, &isNext, String_ZBuffered);
		SE_OptionsStrCat(&pResultString, ChangedAppSettings.BilinearFiltering, &isNext, String_BilinearFiltered);
		SE_OptionsStrCat(&pResultString, ChangedAppSettings.Dither, &isNext, String_Dithered);
		SE_OptionsStrCat(&pResultString, ChangedAppSettings.TripleBuffering, &isNext, String_TripleBuffered);
		SE_OptionsStrCat(&pResultString, ChangedAppSettings.PerspectiveCorrect, &isNext, String_PerspectiveCorrect);
		*pResultString = 0;
		SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_OPTIONS, resultString);
	} else {
		// 'Graphics' Static
		SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_GRAPHICS, String_None);
		// 'Using:' Static
		SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_USING, String_NA);
		// 'Display:' Static
		SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_DISPLAY, String_NA);
		// 'Options:' Static
		SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_OPTIONS, String_NA);
	}

	// 'Sound:' Static
	if( ChangedAppSettings.PreferredSoundAdapter != NULL )
		lpString = ChangedAppSettings.PreferredSoundAdapter->body.description.lpString;
	else
		lpString = String_None;
	SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_SOUND, lpString);

	// 'Sound Effects:' Static
	if( ChangedAppSettings.SoundEnabled )
		lpString = String_Enabled;
	else
		lpString = String_Disabled;
	SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_SFX, lpString);

	// 'Microphone Position' Static
	if( ChangedAppSettings.LaraMic )
		lpString = String_Lara;
	else
		lpString = String_Camera;
	SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_MIC, lpString);

	// 'Joystick' Static
	if( ChangedAppSettings.PreferredJoystick != NULL )
		lpString = ChangedAppSettings.PreferredJoystick->body.productName.lpString;
	else
		lpString = String_None;
	SetDlgItemText(hwndDlg, ID_OPTNS_STATIC_JOYSTICK, lpString);
}

void __cdecl SE_OptionsStrCat(LPTSTR *dstString, bool isEnabled, bool *isNext, LPCTSTR srcString) {
	if( !isEnabled )
		return;

	if( *isNext ) {
		*(*dstString)++ = ',';
		*(*dstString)++ = ' ';
	}

	while( *srcString )
		*(*dstString)++ = *srcString++;

	*isNext = true;
}

INT_PTR CALLBACK SE_AdvancedDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HWND hImage = NULL;

	switch( uMsg ) {
		case WM_INITDIALOG :
			hImage = GetDlgItem(hwndDlg, ID_ADVNC_WINDOW_IMAGE);
			SE_AdvancedDlgInit(hwndDlg);
			return 1;

		case WM_COMMAND :
			if( HIWORD(wParam) == 0 ) { // Buttons
				bool isCheck = ( 1 == SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) );
				switch( LOWORD(wParam) ) {
					case ID_ADVNC_BUTTON_16BIT_DISABLE : // 'Disable 16 bit textures' CheckBox
						ChangedAppSettings.Disable16BitTextures = isCheck;
						break;
					case ID_ADVNC_BUTTON_SORT_DISABLE : // 'Don't sort transparent polys' CheckBox
						ChangedAppSettings.DontSortPrimitives = isCheck;
						break;
					case ID_ADVNC_BUTTON_FMV_DISABLE : // 'Disable FMV' CheckBox
						ChangedAppSettings.DisableFMV = isCheck;
						break;
					case ID_ADVNC_BUTTON_ADJUST_DISABLE : // 'Don't adjust' RadioButton
						ChangedAppSettings.TexelAdjustMode = TAM_Disabled;
						break;
					case ID_ADVNC_BUTTON_ADJUST_BILINEAR : // 'Adjust when bilinear filtering' RadioButton
						ChangedAppSettings.TexelAdjustMode = TAM_BilinearOnly;
						break;
					case ID_ADVNC_BUTTON_ADJUST_ALWAYS : // 'Always adjust' RadioButton
						ChangedAppSettings.TexelAdjustMode = TAM_Always;
						break;
				}
				SE_AdvancedDlgUpdate(hwndDlg);
			}
			break;
	}
	SE_PassMessageToImage(hImage, uMsg, wParam);
	return 0;
}

void __cdecl SE_AdvancedDlgUpdate(HWND hwndDlg) {
	// 'Disable 16 bit textures' CheckBox
	CheckDlgButton(hwndDlg, ID_ADVNC_BUTTON_16BIT_DISABLE,		ChangedAppSettings.Disable16BitTextures);
	// 'Don't sort transparent polys' CheckBox
	CheckDlgButton(hwndDlg, ID_ADVNC_BUTTON_SORT_DISABLE,		ChangedAppSettings.DontSortPrimitives);
	// 'Disable FMV' CheckBox
	CheckDlgButton(hwndDlg, ID_ADVNC_BUTTON_FMV_DISABLE,		ChangedAppSettings.DisableFMV);
	// 'Don't adjust' RadioButton
	CheckDlgButton(hwndDlg, ID_ADVNC_BUTTON_ADJUST_DISABLE,		ChangedAppSettings.TexelAdjustMode == TAM_Disabled);
	// 'Adjust when bilinear filtering' RadioButton
	CheckDlgButton(hwndDlg, ID_ADVNC_BUTTON_ADJUST_BILINEAR,	ChangedAppSettings.TexelAdjustMode == TAM_BilinearOnly);
 	// 'Always adjust' RadioButton
 	CheckDlgButton(hwndDlg, ID_ADVNC_BUTTON_ADJUST_ALWAYS,		ChangedAppSettings.TexelAdjustMode == TAM_Always);
}

void __cdecl SE_AdvancedDlgInit(HWND hwndDlg) {
	SE_AdvancedDlgUpdate(hwndDlg);
}

HWND __cdecl SE_FindSetupDialog() {
	return FindWindow(WC_DIALOG, GameDialogName);
}

/*
 * Inject function
 */
void Inject_SetupDlg() {
	INJECT(0x00452480, OpenGameRegistryKey);
	INJECT(0x004524D0, CloseGameRegistryKey);
	INJECT(0x004524E0, SE_WriteAppSettings);
	INJECT(0x00452760, SE_ReadAppSettings);
	INJECT(0x00452AC0, SE_GraphicsTestStart);
	INJECT(0x00452B90, SE_GraphicsTestFinish);
	INJECT(0x00452BB0, SE_GraphicsTestExecute);
	INJECT(0x00452BC0, SE_GraphicsTest);
	INJECT(0x00452C20, SE_DefaultGraphicsSettings);
	INJECT(0x00452D70, SE_SoundTestStart);
	INJECT(0x00452E30, SE_SoundTestFinish);
	INJECT(0x00452E40, SE_SoundTestExecute);
	INJECT(0x00452E80, SE_SoundTest);
	INJECT(0x00452EE0, SE_PropSheetCallback);
	INJECT(0x00452F20, SE_NewPropSheetWndProc);
	INJECT(0x00452F80, SE_ShowSetupDialog);
	INJECT(0x00453150, SE_GraphicsDlgProc);
	INJECT(0x00453560, SE_GraphicsDlgFullScreenModesUpdate);
	INJECT(0x00453750, SE_GraphicsAdapterSet);
	INJECT(0x00453770, SE_GraphicsDlgUpdate);
	INJECT(0x00453EB0, SE_GraphicsDlgInit);
	INJECT(0x00454030, SE_SoundDlgProc);
	INJECT(0x004541C0, SE_SoundAdapterSet);
	INJECT(0x004541D0, SE_SoundDlgUpdate);
	INJECT(0x004542F0, SE_SoundDlgInit);
	INJECT(0x004543B0, SE_ControlsDlgProc);
	INJECT(0x004544D0, SE_ControlsJoystickSet);
	INJECT(0x004544D0, SE_ControlsDlgUpdate);
	INJECT(0x00454540, SE_ControlsDlgInit);
	INJECT(0x00454690, SE_OptionsDlgUpdate);
	INJECT(0x004548D0, SE_OptionsStrCat);
	INJECT(0x00454920, SE_AdvancedDlgProc);
	INJECT(0x00454A30, SE_AdvancedDlgUpdate);
	INJECT(0x00454AE0, SE_AdvancedDlgInit);
	INJECT(0x00454AF0, SE_FindSetupDialog);
}
