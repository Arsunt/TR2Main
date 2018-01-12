/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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

#ifndef GLOBAL_TYPES_H_INCLUDED
#define GLOBAL_TYPES_H_INCLUDED

/*
 * Inject macro
 */

#pragma pack(push, 1)
typedef struct {
	BYTE opCode;	// must be 0xE9;
	DWORD offset;	// jump offset
} JMP;
#pragma pack(pop)

#define INJECT(from,to) { \
	((JMP*)(from))->opCode = 0xE9; \
	((JMP*)(from))->offset = (DWORD)(to) - ((DWORD)(from) + sizeof(JMP)); \
}

/*
 * Defined values
 */
// General values
#define REQ_LEVEL_VERSION	(45)

// FPS values
#define FRAMES_PER_SECOND	(30)
#define TICKS_PER_FRAME		(2)
#define TICKS_PER_SECOND	(TICKS_PER_FRAME * FRAMES_PER_SECOND)

// Formula values
#define W2V_SHIFT			(14) // World to View shift value
#define W2V_SCALE			(1<<W2V_SHIFT) // World to View scale value
#define PHD_ONE				(0x10000) // unsigned short int equivalent of 1.0
#define PHD_IONE			(PHD_ONE/2) // signed short int equivalent of 1.0
#define PHD_HALF			(0x100) // half size of PHD_ONE

// Geometry values
#define WALL_SHIFT			(10)

// Angle values
#define PHD_360				(PHD_ONE)
#define PHD_180				(PHD_ONE/2)
#define PHD_90				(PHD_ONE/4)
#define PHD_45				(PHD_ONE/8)
#define PHD_DEGREE			(PHD_ONE/360)

// Math macros
#define ABS(a)				(((a)<0)?-(a):(a))
#define SQR(a)				((a)*(a))
#define CLAMPL(a,b)			{if((a)<(b)) (a)=(b);}
#define CLAMPG(a,b)			{if((a)>(b)) (a)=(b);}
#define CLAMP(a,b,c)		{if((a)<(b)) (a)=(b); else if((a)>(c)) (a)=(c);}
#define	TRIGMULT2(a,b)		(((a)*(b))>>W2V_SHIFT)
#define	TRIGMULT3(a,b,c)	(TRIGMULT2((TRIGMULT2(a,b)),c))
#define	VBUF_VISIBLE(a,b,c)	(((a).ys-(b).ys)*((c).xs-(b).xs)>=((c).ys-(b).ys)*((a).xs-(b).xs))

// View distance values
#define VIEW_NEAR			(0x14 * 0x001)
#define VIEW_FAR			(0x14 * 0x400)

// DepthQ fog values
#define DEPTHQ_END			(VIEW_FAR)
#define DEPTHQ_RANGE		(DEPTHQ_END * 2/5)
#define DEPTHQ_START		(DEPTHQ_END - DEPTHQ_RANGE)

// Water effect table parameters
#define WIBBLE_SIZE			(32)
#define MAX_WIBBLE			(2)
#define MAX_SHADE			(0x300)
#define MAX_ROOMLIGHT_UNIT	(0x2000 / (WIBBLE_SIZE/2))

// SW Renderer Detail Settings
#define SW_DETAIL_LOW		(0 * 0x400 * W2V_SCALE)
#define SW_DETAIL_MEDIUM	(3 * 0x400 * W2V_SCALE)
#define SW_DETAIL_HIGH		(6 * 0x400 * W2V_SCALE)

// ClearBuffers flags
#define CLRB_PrimaryBuffer			(0x0001)
#define CLRB_BackBuffer				(0x0002)
#define CLRB_ThirdBuffer			(0x0004)
#define CLRB_ZBuffer				(0x0008)
#define CLRB_RenderBuffer			(0x0010)
#define CLRB_PictureBuffer			(0x0020)
#define CLRB_WindowedPrimaryBuffer	(0x0040)
#define CLRB_Reserved				(0x0080)
#define CLRB_PhdWinSize				(0x0100)

// TextStrInfo flags
#define TIF_Active					(0x0001)
#define TIF_Flash					(0x0002)
#define TIF_RotateH					(0x0004)
#define TIF_RotateV					(0x0008)
#define TIF_CentreH					(0x0010)
#define TIF_CentreV					(0x0020)
#define TIF_Unused					(0x0040)
#define TIF_Right					(0x0080)
#define TIF_Bottom					(0x0100)
#define TIF_Bgnd					(0x0200)
#define TIF_Outline					(0x0400)
#define TIF_Multiline				(0x0800)

// GameFlow flags
#define GFF_DemoVersion				(0x0001)
#define GFF_TitleDisabled			(0x0002)
#define GFF_CheatModeCheckDisabled	(0x0004)
#define GFF_NoInputTimeout			(0x0008)
#define GFF_LoadSaveDisabled		(0x0010)
#define GFF_ScreenSizingDisabled	(0x0020)
#define GFF_LockoutOptionRing		(0x0040)
#define GFF_DozyCheatEnabled		(0x0080)
#define GFF_UseSecurityTag			(0x0100)
#define GFF_Unknown					(0x0200)
#define GFF_SelectAnyLevel			(0x0400)
#define GFF_EnableCheatCode			(0x0800)

// StartInfo flags
#define SIF_Available				(0x0001)
#define SIF_HasPistols				(0x0002)
#define SIF_HasMagnums				(0x0004)
#define SIF_HasUzis					(0x0008)
#define SIF_HasShotgun				(0x0010)
#define SIF_HasM16					(0x0020)
#define SIF_HasGrenade				(0x0040)
#define SIF_HasHarpoon				(0x0080)

// Input Status Flags
#define IN_FORWARD			(0x00000001)
#define IN_BACK				(0x00000002)
#define IN_LEFT				(0x00000004)
#define IN_RIGHT			(0x00000008)
#define IN_JUMP				(0x00000010)
#define IN_DRAW				(0x00000020)
#define IN_ACTION			(0x00000040)
#define IN_SLOW				(0x00000080)
#define IN_OPTION			(0x00000100)
#define IN_LOOK				(0x00000200)
#define IN_STEPL			(0x00000400)
#define IN_STEPR			(0x00000800)
#define IN_ROLL				(0x00001000)
#define IN_PAUSE			(0x00002000)
#define IN_RESERVED1		(0x00004000)
#define IN_RESERVED2		(0x00008000)
#define IN_RESERVED3		(0x00010000)
#define IN_RESERVED4		(0x00020000)
#define IN_RESERVED5		(0x00040000)
#define IN_FLARE			(0x00080000)
#define IN_SELECT			(0x00100000)
#define IN_DESELECT			(0x00200000)
#define IN_SAVE				(0x00400000)
#define IN_LOAD				(0x00800000)

// Gameflow directions
#define GF_START_GAME		(0x0000)
#define GF_START_SAVEDGAME	(0x0100)
#define GF_START_CINE		(0x0200)
#define GF_START_FMV		(0x0300)
#define GF_START_DEMO		(0x0400)
#define GF_EXIT_TO_TITLE	(0x0500)
#define GF_LEVEL_COMPLETE	(0x0600)
#define GF_EXIT_GAME		(0x0700)
#define GF_EXIT_TO_OPTION	(0x0800)
#define GF_TITLE_DESELECT	(0x0900)
#define GF_ERROR			(-1)

// SFX flags
#define SFX_UNDERWATER		(1)
#define SFX_ALWAYS			(2)

/*
 * Enums
 */

typedef enum {
	ID_NONE = -1,
	ID_LARA = 0,
	ID_LARA_PISTOLS,
	ID_LARA_HAIR,
	ID_LARA_SHOTGUN,
	ID_LARA_MAGNUMS,
	ID_LARA_UZIS,
	ID_LARA_M16,
	ID_LARA_GRENADE,
	ID_LARA_HARPOON,
	ID_LARA_FLARE,
	ID_LARA_SKIDOO,
	ID_LARA_BOAT,
	ID_LARA_EXTRA,
	ID_SKIDOO_FAST,
	ID_BOAT,
	ID_DOG,
	ID_CULT1,
	ID_CULT1A,
	ID_CULT1B,
	ID_CULT2,
	ID_CULT3,
	ID_MOUSE,
	ID_DRAGON_FRONT,
	ID_DRAGON_BACK,
	ID_GONDOLA,
	ID_SHARK,
	ID_EEL,
	ID_BIG_EEL,
	ID_BARRACUDA,
	ID_DIVER,
	ID_WORKER1,
	ID_WORKER2,
	ID_WORKER3,
	ID_WORKER4,
	ID_WORKER5,
	ID_JELLY,
	ID_SPIDER,
	ID_BIG_SPIDER,
	ID_CROW,
	ID_TIGER,
	ID_BARTOLI,
	ID_XIAN_LORD,
	ID_CHINESE2,
	ID_WARRIOR,
	ID_CHINESE4,
	ID_YETI,
	ID_GIANT_YETI,
	ID_EAGLE,
	ID_BANDIT1,
	ID_BANDIT2,
	ID_BANDIT2B,
	ID_SKIDOO_ARMED,
	ID_SKIDMAN,
	ID_MONK1,
	ID_MONK2,
	ID_FALLING_BLOCK1,
	ID_FALLING_BLOCK2,
	ID_FALLING_BLOCK3,
	ID_PENDULUM1,
	ID_SPIKES,
	ID_ROLLING_BALL1,
	ID_DARTS,
	ID_DART_EMITTER,
	ID_DRAW_BRIDGE,
	ID_TEETH_TRAP,
	ID_LIFT,
	ID_GENERAL,
	ID_MOVABLE_BLOCK1,
	ID_MOVABLE_BLOCK2,
	ID_MOVABLE_BLOCK3,
	ID_MOVABLE_BLOCK4,
	ID_BIG_BOWL,
	ID_WINDOW1,
	ID_WINDOW2,
	ID_WINDOW3,
	ID_WINDOW4,
	ID_PROPELLER1,
	ID_PROPELLER2,
	ID_HOOK,
	ID_FALLING_CEILING,
	ID_SPINNING_BLADE,
	ID_BLADE,
	ID_KILLER_STATUE,
	ID_ROLLING_BALL2,
	ID_ICICLE,
	ID_SPIKE_WALL,
	ID_SPRING_BOARD,
	ID_CEILING_SPIKES,
	ID_BELL,
	ID_WATER_SPRITE,
	ID_SNOW_SPRITE,
	ID_SKIDOO_LARA,
	ID_SWITCH_TYPE1,
	ID_SWITCH_TYPE2,
	ID_PROPELLER3,
	ID_PROPELLER4,
	ID_PENDULUM2,
	ID_WORKER_RESERVED1,
	ID_WORKER_RESERVED2,
	ID_LARA_SWAP,
	ID_TEXT_BOX,
	ID_ROLLING_BALL3,
	ID_DEATH_SLIDE,
	ID_SWITCH_TYPE3,
	ID_SWITCH_TYPE4,
	ID_SWITCH_TYPE5,
	ID_DOOR_TYPE1,
	ID_DOOR_TYPE2,
	ID_DOOR_TYPE3,
	ID_DOOR_TYPE4,
	ID_DOOR_TYPE5,
	ID_DOOR_TYPE6,
	ID_DOOR_TYPE7,
	ID_DOOR_TYPE8,
	ID_TRAPDOOR_TYPE1,
	ID_TRAPDOOR_TYPE2,
	ID_TRAPDOOR_TYPE3,
	ID_BRIDGE_FLAT,
	ID_BRIDGE_TILT1,
	ID_BRIDGE_TILT2,
	ID_PASSPORT_OPTION,
	ID_COMPASS_OPTION,
	ID_PHOTO_OPTION,
	ID_PLAYER1,
	ID_PLAYER2,
	ID_PLAYER3,
	ID_PLAYER4,
	ID_PLAYER5,
	ID_PLAYER6,
	ID_PLAYER7,
	ID_PLAYER8,
	ID_PLAYER9,
	ID_PLAYER10,
	ID_PASSPORT_CLOSED,
	ID_COMPASS_ITEM,
	ID_PISTOL_ITEM,
	ID_SHOTGUN_ITEM,
	ID_MAGNUM_ITEM,
	ID_UZI_ITEM,
	ID_HARPOON_ITEM,
	ID_M16_ITEM,
	ID_GRENADE_ITEM,
	ID_PISTOL_AMMO_ITEM,
	ID_SHOTGUN_AMMO_ITEM,
	ID_MAGNUM_AMMO_ITEM,
	ID_UZI_AMMO_ITEM,
	ID_HARPOON_AMMO_ITEM,
	ID_M16_AMMO_ITEM,
	ID_GRENADE_AMMO_ITEM,
	ID_SMALL_MEDIPACK_ITEM,
	ID_LARGE_MEDIPACK_ITEM,
	ID_FLARES_ITEM,
	ID_FLARE_ITEM,
	ID_DETAIL_OPTION,
	ID_SOUND_OPTION,
	ID_CONTROL_OPTION,
	ID_GAMMA_OPTION,
	ID_PISTOL_OPTION,
	ID_SHOTGUN_OPTION,
	ID_MAGNUM_OPTION,
	ID_UZI_OPTION,
	ID_HARPOON_OPTION,
	ID_M16_OPTION,
	ID_GRENADE_OPTION,
	ID_PISTOL_AMMO_OPTION,
	ID_SHOTGUN_AMMO_OPTION,
	ID_MAGNUM_AMMO_OPTION,
	ID_UZI_AMMO_OPTION,
	ID_HARPOON_AMMO_OPTION,
	ID_M16_AMMO_OPTION,
	ID_GRENADE_AMMO_OPTION,
	ID_SMALL_MEDIPACK_OPTION,
	ID_LARGE_MEDIPACK_OPTION,
	ID_FLARES_OPTION,
	ID_PUZZLE_ITEM1,
	ID_PUZZLE_ITEM2,
	ID_PUZZLE_ITEM3,
	ID_PUZZLE_ITEM4,
	ID_PUZZLE_OPTION1,
	ID_PUZZLE_OPTION2,
	ID_PUZZLE_OPTION3,
	ID_PUZZLE_OPTION4,
	ID_PUZZLE_HOLE1,
	ID_PUZZLE_HOLE2,
	ID_PUZZLE_HOLE3,
	ID_PUZZLE_HOLE4,
	ID_PUZZLE_DONE1,
	ID_PUZZLE_DONE2,
	ID_PUZZLE_DONE3,
	ID_PUZZLE_DONE4,
	ID_SECRET1,
	ID_SECRET2,
	ID_SECRET3,
	ID_KEY_ITEM1,
	ID_KEY_ITEM2,
	ID_KEY_ITEM3,
	ID_KEY_ITEM4,
	ID_KEY_OPTION1,
	ID_KEY_OPTION2,
	ID_KEY_OPTION3,
	ID_KEY_OPTION4,
	ID_KEY_HOLE1,
	ID_KEY_HOLE2,
	ID_KEY_HOLE3,
	ID_KEY_HOLE4,
	ID_PICKUP_ITEM1,
	ID_PICKUP_ITEM2,
	ID_PICKUP_OPTION1,
	ID_PICKUP_OPTION2,
	ID_SPHERE_OF_DOOM1,
	ID_SPHERE_OF_DOOM2,
	ID_SPHERE_OF_DOOM3,
	ID_ALARM_SOUND,
	ID_BIRD_TWEETER1,
	ID_DINO,
	ID_BIRD_TWEETER2,
	ID_CLOCK_CHIMES,
	ID_DRAGON_BONES1,
	ID_DRAGON_BONES2,
	ID_DRAGON_BONES3,
	ID_HOT_LIQUID,
	ID_EFFECT_RESERVED1,
	ID_MINE,
	ID_INV_BACKGROUND,
	ID_EFFECT_RESERVED2,
	ID_GONG_BONGER,
	ID_DETONATOR1,
	ID_DETONATOR2,
	ID_COPTER,
	ID_EXPLOSION,
	ID_SPLASH,
	ID_BUBBLES,
	ID_BUBBLE_EMITTER,
	ID_BLOOD,
	ID_DART_EFFECT,
	ID_FLARE_FIRE,
	ID_GLOW,
	ID_EFFECT_RESERVED3,
	ID_RICOCHET,
	ID_TWINKLE,
	ID_GUN_FLASH,
	ID_DUST,
	ID_BODY_PART,
	ID_CAMERA_TARGET,
	ID_WATERFALL,
	ID_MISSILE1,
	ID_MISSILE2,
	ID_MISSILE3,
	ID_ROCKET,
	ID_HARPOON_BOLT,
	ID_LAVA,
	ID_LAVA_EMITTER,
	ID_FLAME,
	ID_FLAME_EMITTER,
	ID_SKYBOX,
	ID_ALPHABET,
	ID_DYING_MONK,
	ID_DING_DONG,
	ID_LARA_ALARM,
	ID_MINI_COPTER,
	ID_WINSTON,
	ID_ASSAULT_DIGITS,
	ID_FINAL_LEVEL_COUNTER,
	ID_CUT_SHOTGUN,
	ID_EARTHQUAKE,
	ID_NUMBER_OBJECTS,
} GAME_OBJECT_ID;

typedef enum {
	AS_WALK,
	AS_RUN,
	AS_STOP,
	AS_FORWARDJUMP,
	AS_POSE,
	AS_FASTBACK,
	AS_TURN_R,
	AS_TURN_L,
	AS_DEATH,
	AS_FASTFALL,
	AS_HANG,
	AS_REACH,
	AS_SPLAT,
	AS_TREAD,
	AS_LAND,
	AS_COMPRESS,
	AS_BACK,
	AS_SWIM,
	AS_GLIDE,
	AS_NULL,
	AS_FASTTURN,
	AS_STEPRIGHT,
	AS_STEPLEFT,
	AS_HIT,
	AS_SLIDE,
	AS_BACKJUMP,
	AS_RIGHTJUMP,
	AS_LEFTJUMP,
	AS_UPJUMP,
	AS_FALLBACK,
	AS_HANGLEFT,
	AS_HANGRIGHT,
	AS_SLIDEBACK,
	AS_SURFTREAD,
	AS_SURFSWIM,
	AS_DIVE,
	AS_PUSHBLOCK,
	AS_PULLBLOCK,
	AS_PPREADY,
	AS_PICKUP,
	AS_SWITCHON,
	AS_SWITCHOFF,
	AS_USEKEY,
	AS_USEPUZZLE,
	AS_UWDEATH,
	AS_ROLL,
	AS_SPECIAL,
	AS_SURFBACK,
	AS_SURFLEFT,
	AS_SURFRIGHT,
	AS_USEMIDAS,
	AS_DIEMIDAS,
	AS_SWANDIVE,
	AS_FASTDIVE,
	AS_GYMNAST,
	AS_WATEROUT,
} LARA_ANIM_STATES;

typedef enum {
	VGA_NoVga,
	VGA_256Color,
	VGA_ModeX,
	VGA_Standard,
} VGA_MODE;

typedef enum {
	RM_Unknown,
	RM_Software,
	RM_Hardware,
} RENDER_MODE;

typedef enum {
	AM_4_3,
	AM_16_9,
	AM_Any,
} ASPECT_MODE;

typedef enum {
	TAM_Disabled,
	TAM_BilinearOnly,
	TAM_Always,
} TEX_ADJUST_MODE;

typedef enum {
	INV_GameMode,
	INV_TitleMode,
	INV_KeysMode,
	INV_SaveMode,
	INV_LoadMode,
	INV_DeathMode,
} INVENTORY_MODE;

typedef enum {
	LGS_Armless,
	LGS_HandBusy,
	LGS_Draw,
	LGS_Undraw,
	LGS_Ready,
} LARA_GUN_STATUS;

typedef enum {
	LGT_Unarmed,
	LGT_Pistols,
	LGT_Magnums,
	LGT_Uzis,
	LGT_Shotgun,
	LGT_M16,
	LGT_Grenade,
	LGT_Harpoon,
	LGT_Flare,
} LARA_GUN_TYPE;

typedef enum {
	CAM_Chase,
	CAM_Fixed,
	CAM_Look,
	CAM_Combat,
	CAM_Cinematic,
	CAM_Heavy,
} CAMERA_TYPE;

typedef enum {
	KM_Forward,
	KM_Back,
	KM_Left,
	KM_Right,
	KM_StepLeft,
	KM_StepRight,
	KM_Slow,
	KM_Jump,
	KM_Action,
	KM_WeaponDraw,
	KM_Flare,
	KM_Look,
	KM_Roll,
	KM_Option,
} KEYMAP;

typedef enum {
	ST_AvgZ,
	ST_MaxZ,
	ST_FarZ,
} SORTTYPE;

/*
 * Structs
 */
#pragma pack(push, 1)

typedef struct SortItem_t {
  int _0;
  int _1;
} SORT_ITEM;

typedef struct RGB_t {
	BYTE red;
	BYTE green;
	BYTE blue;
} RGB;

typedef struct DepthQEntry_t {
	BYTE index[256];
} DEPTHQ_ENTRY;

typedef struct GouraudEntry_t {
	BYTE index[32];
} GOURAUD_ENTRY;

typedef struct DisplayMode_t {
	int width;
	int height;
	int bpp;
	VGA_MODE vga;
} DISPLAY_MODE;

typedef struct DisplayModeNode_t {
	struct DisplayModeNode_t *next;
	struct DisplayModeNode_t *previous;
	struct DisplayMode_t body;
} DISPLAY_MODE_NODE;

typedef struct DisplayModeList_t {
	struct DisplayModeNode_t *head;
	struct DisplayModeNode_t *tail;
	DWORD dwCount;
} DISPLAY_MODE_LIST;

typedef struct StringFlagged_t {
	LPTSTR lpString;
	bool isPresented;
} STRING_FLAGGED;

typedef struct DisplayAdapter_t {
	LPGUID lpAdapterGuid;
	GUID adapterGuid;
	STRING_FLAGGED driverDescription;
	STRING_FLAGGED driverName;
	DDCAPS driverCaps;
	DDCAPS helCaps;
	GUID deviceGuid;
	D3DDEVICEDESC D3DHWDeviceDesc;
	DISPLAY_MODE_LIST hwDispModeList;
	DISPLAY_MODE_LIST swDispModeList;
	DISPLAY_MODE vgaMode1;
	DISPLAY_MODE vgaMode2;
	DWORD screenWidth;
	bool hwRenderSupported;
	bool swWindowedSupported;
	bool hwWindowedSupported;
	bool isVgaMode1Presented;
	bool isVgaMode2Presented;
	bool perspectiveCorrectSupported;
	bool ditherSupported;
	bool zBufferSupported;
	bool linearFilterSupported;
	bool shadeRestricted;
} DISPLAY_ADAPTER;

typedef struct DisplayAdapterNode_t {
	struct DisplayAdapterNode_t *next;
	struct DisplayAdapterNode_t *previous;
	struct DisplayAdapter_t body;
} DISPLAY_ADAPTER_NODE;

typedef struct DisplayAdapterList_t {
	struct DisplayAdapterNode_t *head;
	struct DisplayAdapterNode_t *tail;
	DWORD dwCount;
} DISPLAY_ADAPTER_LIST;

typedef struct SoundAdapter_t {
	GUID *lpAdapterGuid;
	GUID adapterGuid;
	STRING_FLAGGED description;
	STRING_FLAGGED module;
} SOUND_ADAPTER;

typedef struct SoundAdapterNode_t {
	struct SoundAdapterNode_t *next;
	struct SoundAdapterNode_t *previous;
	SOUND_ADAPTER body;
} SOUND_ADAPTER_NODE;

typedef struct SoundAdapterList_t {
	struct SoundAdapterNode_t *head;
	struct SoundAdapterNode_t *tail;
	DWORD dwCount;
} SOUND_ADAPTER_LIST;

typedef struct Joystick_t {
	GUID *lpJoystickGuid;
	GUID joystickGuid;
	STRING_FLAGGED productName;
	STRING_FLAGGED instanceName;
} JOYSTICK;

typedef struct JoystickNode_t {
	struct JoystickNode_t *next;
	struct JoystickNode_t *previous;
	JOYSTICK body;
} JOYSTICK_NODE;

typedef struct JoystickList_t {
	struct JoystickNode_t *head;
	struct JoystickNode_t *tail;
	DWORD dwCount;
} JOYSTICK_LIST;

typedef struct AppSettings_t {
	DISPLAY_ADAPTER_NODE *PreferredDisplayAdapter;
	SOUND_ADAPTER_NODE *PreferredSoundAdapter;
	JOYSTICK_NODE *PreferredJoystick;
	DISPLAY_MODE_NODE *VideoMode;
	RENDER_MODE RenderMode;
	int WindowWidth;
	int WindowHeight;
	ASPECT_MODE AspectMode;
	bool PerspectiveCorrect;
	bool Dither;
	bool ZBuffer;
	bool BilinearFiltering;
	bool TripleBuffering;
	bool FullScreen;
	bool SoundEnabled;
	bool LaraMic;
	bool JoystickEnabled;
	bool Disable16BitTextures;
	bool DontSortPrimitives;
	bool FlipBroken;
	bool DisableFMV;
	TEX_ADJUST_MODE TexelAdjustMode;
	int NearestAdjustment;
	int LinearAdjustment;
} APP_SETTINGS;

struct TEXPAGE_DESC {
	LPDIRECTDRAWSURFACE3 sysMemSurface;
	LPDIRECTDRAWSURFACE3 vidMemSurface;
	LPDIRECTDRAWPALETTE palette;
	LPDIRECT3DTEXTURE2 texture3d;
	D3DTEXTUREHANDLE texHandle;
	int width;
	int height;
	int status;
};

typedef struct PhdUV_t {
	UINT16 u;
	UINT16 v;
} PHD_UV;

typedef struct PhdTexture_t {
	UINT16 drawtype;
	UINT16 tpage;
	PHD_UV uv[4];
} PHD_TEXTURE;

typedef struct InvColours_t {
	UINT16 black;
	UINT16 gray;
	UINT16 white;
	UINT16 red;
	UINT16 orange;
	UINT16 yellow;
	UINT16 reserved1;
	UINT16 reserved2;
	UINT16 reserved3;
	UINT16 reserved4;
	UINT16 reserved5;
	UINT16 reserved6;
	UINT16 darkGreen;
	UINT16 green;
	UINT16 cyan;
	UINT16 blue;
	UINT16 magenta;
} INV_COLOURS;

typedef struct ColorBitMasks_t {
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwRGBAlphaBitMask;
	DWORD dwRBitDepth;
	DWORD dwGBitDepth;
	DWORD dwBBitDepth;
	DWORD dwRGBAlphaBitDepth;
	DWORD dwRBitOffset;
	DWORD dwGBitOffset;
	DWORD dwBBitOffset;
	DWORD dwRGBAlphaBitOffset;
} COLOR_BIT_MASKS;

typedef struct TextureFormat_t {
	DDPIXELFORMAT pixelFmt;
	COLOR_BIT_MASKS colorBitMasks;
	DWORD bpp;
} TEXTURE_FORMAT;

typedef struct TextStrInfo_t {
	DWORD flags;
	UINT16 textFlags;
	UINT16 bgndFlags;
	UINT16 outFlags;
	__int16 xPos;
	__int16 yPos;
	__int16 zPos;
	__int16 letterSpacing;
	__int16 wordSpacing;
	__int16 flashRate;
	__int16 flashCount;
	__int16 bgndColor;
	LPVOID bgndGour;
	__int16 outlColour;
	LPVOID outlGour;
	__int16 bgndSizeX;
	__int16 bgndSizeY;
	__int16 bgndOffX;
	__int16 bgndOffY;
	__int16 bgndOffZ;
	int scaleH;
	int scaleW;
	char *pString;
} TEXT_STR_INFO;

typedef struct StringFixed50_t {
	char str[50];
} STRING_FIXED50;

typedef struct StringFixed64_t {
	char str[64];
} STRING_FIXED64;

typedef struct PcxHeader_t {
	BYTE manufacturer;
	BYTE version;
	BYTE rle;
	BYTE bpp;
	UINT16 xMin;
	UINT16 yMin;
	UINT16 xMax;
	UINT16 yMax;
	UINT16 h_dpi;
	UINT16 v_dpi;
	RGB headerPalette[16];
	BYTE reserved;
	BYTE planes;
	UINT16 bytesPerLine;
	UINT16 palInterpret;
	UINT16 h_res;
	UINT16 v_res;
	BYTE reservedData[54];
} PCX_HEADER;

typedef struct TgaHeader_t {
	UINT8 idLength;
	UINT8 colorMapType;
	UINT8 dataTypeCode;
	UINT16 colorMapOrigin;
	UINT16 colorMapLength;
	UINT8 colorMapDepth;
	UINT16 xOrigin;
	UINT16 yOrigin;
	UINT16 width;
	UINT16 height;
	UINT8 bpp;
	UINT8 imageDescriptor;
} TGA_HEADER;

typedef struct BitmapResource_t {
	LPBITMAPINFO bmpInfo;
	void *bmpData;
	HPALETTE hPalette;
	DWORD flags;
} BITMAP_RESOURCE;

typedef struct RoomLightTable_t {
	int table[32];
} ROOM_LIGHT_TABLE;

typedef struct WavePcmHeader_t {
	DWORD dwRiffChunkID;
	DWORD dwRiffChunkSize;
	DWORD dwFormat;
	DWORD dwFmtSubchunkID;
	DWORD dwFmtSubchunkSize;
	UINT16 wFormatTag;
	__int16 nChannels;
	int nSamplesPerSec;
	int nAvgBytesPerSec;
	__int16 nBlockAlign;
	UINT16 wBitsPerSample;
	DWORD dwDataSubchunkID;
	DWORD dwDataSubchunkSize;
} WAVEPCM_HEADER;

typedef struct SampleInfo_t {
	__int16 sfxID;
	__int16 volume;
	__int16 randomness;
	UINT16 flags;
} SAMPLE_INFO;

typedef struct SfxInfo_t {
	int volume;
	int pan;
	int sampleIdx;
	int freqFactor;
} SFX_INFO;

typedef struct GameFlow_t {
	int firstOption;
	int titleReplace;
	int onDeath_DemoMode;
	int onDeath_InGame;
	int noInput_Time;
	int onDemo_Interrupt;
	int onDemo_End;
	UINT16 reserved1[18];
	UINT16 num_Levels;
	UINT16 num_Pictures;
	UINT16 num_Titles;
	UINT16 num_Fmvs;
	UINT16 num_Cutscenes;
	UINT16 num_Demos;
	UINT16 titleTrack;
	__int16 singleLevel;
	UINT16 reserved2[16];
	UINT16 flags;
	UINT16 reserved3[3];
	BYTE cypherCode;
	BYTE language;
	BYTE secretTrack;
	BYTE levelCompleteTrack;
	UINT16 reserved4[2];
} GAME_FLOW;

typedef struct RequestInfo_t {
	UINT16 reqFlags;
	UINT16 itemsCount;
	UINT16 selected;
	UINT16 visibleCount;
	UINT16 lineOffset;
	UINT16 lineOldOffset;
	UINT16 pixWidth;
	UINT16 lineHeight;
	__int16 xPos;
	__int16 yPos;
	__int16 zPos;
	UINT16 itemStringLen;
	char *lpItemStrings1;
	char *lpItemStrings2;
	DWORD *lpItemFlags1;
	DWORD *lpItemFlags2;
	DWORD headingFlags1;
	DWORD headingFlags2;
	DWORD backgroundFlags;
	DWORD moreupFlags;
	DWORD moredownFlags;
	DWORD itemFlags1[24];
	DWORD itemFlags2[24];
	TEXT_STR_INFO *headingText1;
	TEXT_STR_INFO *headingText2;
	TEXT_STR_INFO *backgroundText;
	TEXT_STR_INFO *moreupText;
	TEXT_STR_INFO *moredownText;
	TEXT_STR_INFO *itemTexts1[24];
	TEXT_STR_INFO *itemTexts2[24];
	char headingString1[32];
	char headingString2[32];
	DWORD renderWidth;
	DWORD renderHeight;

} REQUEST_INFO;

typedef struct InventoryItem_t {
	char *lpString;
	__int16 objectID;
	__int16 framesTotal;
	__int16 currentFrame;
	__int16 goalFrame;
	__int16 openFrame;
	__int16 animDirection;
	__int16 animSpeed;
	__int16 animCount;
	__int16 xRotPtSel;
	__int16 xRotPt;
	__int16 xRotSel;
	__int16 xRot;
	__int16 yRotSel;
	__int16 yRot;
	__int16 zRot;
	int yTransSel;
	int yTrans;
	int zTransSel;
	int zTrans;
	int meshesSel;
	int meshesDrawn;
	__int16 invPos;
	void *sprites;
	DWORD reserved1;
	DWORD reserved2;
	DWORD reserved3;
	DWORD reserved4;
} INVENTORY_ITEM;

typedef struct StatisticsInfo_t {
	DWORD timer;
	DWORD shots;
	DWORD hits;
	DWORD distance;
	UINT16 kills;
	BYTE secrets;
	BYTE mediPacks;
} STATISTICS_INFO;

typedef struct StartInfo_t {
	UINT16 pistolAmmo;
	UINT16 magnumAmmo;
	UINT16 uziAmmo;
	UINT16 shotgunAmmo;
	UINT16 m16Ammo;
	UINT16 grenadeAmmo;
	UINT16 harpoonAmmo;
	BYTE smallMedipacks;
	BYTE largeMedipacks;
	BYTE unused;
	BYTE flares;
	BYTE gunStatus;
	BYTE gunType;
	UINT16 flags;
	UINT16 reserved;
	STATISTICS_INFO statistics;
} START_INFO;

typedef struct SaveGame_t {
	START_INFO start[24];
	STATISTICS_INFO statistics;
	__int16 currentLevel;
	bool bonusFlag;
	BYTE numPickup[2];
	BYTE numPuzzle[4];
	BYTE numKey[4];
	UINT16 reserved;
	BYTE buffer[6272];
} SAVEGAME_INFO;

typedef struct VertexPos_t {
	__int16 x;
	__int16 y;
	__int16 z;
} VERTEX_POS;

typedef struct PhdVector_t {
	int x;
	int y;
	int z;
} PHD_VECTOR;

typedef struct GameVector_t {
	int x;
	int y;
	int z;
	__int16 roomNumber;
	__int16 boxNumber;
} GAME_VECTOR;

typedef struct ObjectVector_t {
	int x;
	int y;
	int z;
	UINT16 data;
	UINT16 flags;
} OBJECT_VECTOR;

typedef struct VectorAngles_t {
	__int16 yaw;
	__int16 pitch;
} VECTOR_ANGLES;

typedef struct Phd3dPos_t {
	int x;
	int y;
	int z;
	__int16 rotX;
	__int16 rotY;
	__int16 rotZ;
} PHD_3DPOS;

typedef struct CameraInfo_t {
	GAME_VECTOR pos;
	GAME_VECTOR target;
	CAMERA_TYPE type;
	int shift;
	DWORD flags;
	int fixedCamera;
	int numberFrames;
	int bounce;
	int underwater;
	int targetDistance;
	int targetSquare;
	__int16 targetAngle;
	__int16 actualAngle;
	__int16 targetElevation;
	__int16 box;
	__int16 number;
	__int16 last;
	__int16 timer;
	__int16 speed;
	LPVOID item;
	LPVOID last_item;
	OBJECT_VECTOR *fixed;
} CAMERA_INFO;

typedef struct ItemInfo_t {
	int floor;
	DWORD touchBits;
	DWORD meshBits;
	__int16 objectID;
	__int16 currentAnimState;
	__int16 goalAnimState;
	__int16 requiredAnimState;
	__int16 animNumber;
	__int16 frameNumber;
	__int16 roomNumber;
	__int16 nextItem;
	__int16 nextActive;
	__int16 speed;
	__int16 fallSpeed;
	__int16 hitPoints;
	__int16 boxNumber;
	__int16 timer;
	UINT16 flags;
	__int16 shade1;
	__int16 shade2;
	__int16 reserved;
	LPVOID data;
	PHD_3DPOS pos;
	UINT16 bitFields;
} ITEM_INFO;

typedef struct CollSide_t {
	int floor;
	int ceiling;
	int type;
} COLL_SIDE;

typedef struct CollInfo_t {
	COLL_SIDE sideMid;
	COLL_SIDE sideFront;
	COLL_SIDE sideLeft;
	COLL_SIDE sideRight;
	int radius;
	int badPos;
	int badNeg;
	int badCeiling;
	PHD_VECTOR shift;
	PHD_VECTOR old;
	__int16 facing;
	__int16 quadrant;
	__int16 collType;
	__int16 *trigger;
	char xTilt;
	char zTilt;
	char hitByBaddie;
	char hitStatic;
	UINT16 flags;
} COLL_INFO;

typedef struct ObjectInfo_t {
	__int16 nMeshes;
	__int16 meshIndex;
	int boneIndex;
	__int16 *frameBase;
	void (__cdecl *initialise)(__int16 itemNumber);
	void (__cdecl *control)(__int16 itemNumber);
	void (__cdecl *floor)(ITEM_INFO *item, int x, int y, int z, __int16 *height);
	void (__cdecl *ceiling)(ITEM_INFO *item, int x, int y, int z, __int16 *height);
	void (__cdecl *drawRoutine)(ITEM_INFO *item);
	void (__cdecl *collision)(__int16 itemNum, ITEM_INFO *laraItem, COLL_INFO *coll);
	__int16 animIndex;
	__int16 hitPoints;
	__int16 pivotLength;
	__int16 radius;
	__int16 shadowSize;
	UINT16 flags;
} OBJECT_INFO;

typedef struct PhdMatrix_t {
	int _00, _01, _02, _03;
	int _10, _11, _12, _13;
	int _20, _21, _22, _23;
} PHD_MATRIX;

typedef struct DoorInfo_t {
	__int16 room;
	__int16 x;
	__int16 y;
	__int16 z;
	VERTEX_POS vertex[4];
} DOOR_INFO;

typedef struct DoorInfos_t {
	__int16 wCount;
	DOOR_INFO door[];
} DOOR_INFOS;

typedef struct FloorInfo_t {
	__int16 index;
	__int16 box;
	char pitRoom;
	char floor;
	char skyRoom;
	char ceiling;
} FLOOR_INFO;

typedef struct LightInfo_t {
	int x;
	int y;
	int z;
	__int16 intensity1;
	__int16 intensity2;
	int fallOff1;
	int fallOff2;
} LIGHT_INFO;

typedef struct MeshInfo_t {
	int x;
	int y;
	int z;
	__int16 yRot;
	__int16 shade1;
	__int16 shade2;
	__int16 staticNumber;
} MESH_INFO;

typedef struct RoomInfo_t {
	__int16 *data;
	DOOR_INFOS *doors;
	FLOOR_INFO *floor;
	LIGHT_INFO *light;
	MESH_INFO *mesh;
	int x;
	int y;
	int z;
	int minFloor;
	int maxCeiling;
	__int16 xSize;
	__int16 ySize;
	__int16 ambient1;
	__int16 ambient2;
	__int16 lightMode;
	__int16 numLights;
	__int16 numMeshes;
	__int16 boundLeft;
	__int16 boundRight;
	__int16 boundTop;
	__int16 boundBotom;
	__int16 boundActive;
	__int16 left;
	__int16 right;
	__int16 top;
	__int16 bottom;
	__int16 itemNumber;
	__int16 fxNumber;
	__int16 flippedRoom;
	UINT16 flags;
} ROOM_INFO;

typedef struct PhdVBuf_t {
	float xv;
	float yv;
	float zv;
	float rhw;
	float xs;
	float ys;
	__int16 clip;
	__int16 g;
	__int16 u;
	__int16 v;
} PHD_VBUF;

typedef struct PointInfo_t {
	float xv;
	float yv;
	float zv;
	float rhw;
	float xs;
	float ys;
	float u;
	float v;
	float g;
} POINT_INFO;

typedef struct ShadowInfo_t {
	__int16 x;
	__int16 y;
	__int16 z;
	__int16 radius;
	__int16 polyCount;
	__int16 vertexCount;
	VERTEX_POS vertex[8];
} SHADOW_INFO;

typedef struct VertexInfo_t {
	float x;
	float y;
	float rhw;
	float u;
	float v;
	float g;
} VERTEX_INFO;

typedef struct RoomVertexInfo_t {
	__int16 x;
	__int16 y;
	__int16 z;
	__int16 lightBase;
	BYTE lightTableValue;
	BYTE flags;
	__int16 lightAdder;
} ROOM_VERTEX_INFO;

typedef struct AnimStruct_t {
	__int16 *framePtr;
	__int16 interpolation;
	__int16 currentAnimState;
	int velocity;
	int acceleration;
	__int16 frameBase;
	__int16 frameEnd;
	__int16 jumpAnimNum;
	__int16 jumpFrameNum;
	__int16 numberChanges;
	__int16 changeIndex;
	__int16 numberCommands;
	__int16 commandIndex;
} ANIM_STRUCT;

typedef struct ChangeStruct_t {
	__int16 goalAnimState;
	__int16 numberRanges;
	__int16 rangeIndex;
} CHANGE_STRUCT;

typedef struct RangeStruct_t {
	__int16 startFrame;
	__int16 endFrame;
	__int16 linkAnimNum;
	__int16 linkFrameNum;
} RANGE_STRUCT;

typedef struct StaticBounds_t {
	__int16 xMin;
	__int16 xMax;
	__int16 yMin;
	__int16 yMax;
	__int16 zMin;
	__int16 zMax;
} STATIC_BOUNDS;

typedef struct StaticInfo_t {
	__int16 meshIndex;
	UINT16 flags;
	STATIC_BOUNDS drawBounds;
	STATIC_BOUNDS collisionBounds;
} STATIC_INFO;

typedef struct PhdSprite_t {
	UINT16 texPage;
	UINT16 offset;
	UINT16 width;
	UINT16 height;
	__int16 x1;
	__int16 y1;
	__int16 x2;
	__int16 y2;
} PHD_SPRITE;

typedef struct BoxInfo_t {
	BYTE left;
	BYTE right;
	BYTE top;
	BYTE bottom;
	UINT16 height;
	UINT16 overlapIndex;
} BOX_INFO;

typedef struct CineFrameInfo_t {
	__int16 xTarget;
	__int16 yTarget;
	__int16 zTarget;
	__int16 zPos;
	__int16 yPos;
	__int16 xPos;
	__int16 fov;
	__int16 roll;
} CINE_FRAME_INFO;

#pragma pack(pop)

#endif // GLOBAL_TYPES_H_INCLUDED
