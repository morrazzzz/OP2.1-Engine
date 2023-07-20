#ifndef DefinesH
#define DefinesH

#ifdef	DEBUG
	ENGINE_API	extern BOOL			bDebug;
#else
	#define bDebug 0
#endif

#define _RELEASE(x)			{ if(x) { (x)->Release();       (x)=NULL; } }
#define _SHOW_REF(msg, x)   { if(x) { x->AddRef(); Log(msg,u32(x->Release()));}}

// textures
ENGINE_API extern	int		psTextureLOD;

// psDeviceFlags
enum {
	rsFullscreen					= (1ul<<0ul),
	rsClearBB						= (1ul<<1ul),
	rsVSync							= (1ul<<2ul),
	rsWireframe						= (1ul<<3ul),
	rsOcclusion						= (1ul<<4ul),
	rsStatistic						= (1ul<<5ul),
	rsDetails						= (1ul<<6ul),
	rsRefresh60hz					= (1ul<<7ul),
	rsConstantFPS					= (1ul<<8ul),
	rsDrawStatic					= (1ul<<9ul),
	rsDrawDynamic					= (1ul<<10ul),
	rsDisableObjectsAsCrows			= (1ul<<11ul),

	rsOcclusionDraw					= (1ul<<12ul),
	rsOcclusionStats				= (1ul<<13ul),

	mtSound							= (1ul<<14ul),
	mtPhysics						= (1ul<<15ul),
	mtNetwork						= (1ul<<16ul),
	mtParticles						= (1ul<<17ul),

	rsCameraPos						= (1ul<<18ul),
	rsR2							= (1ul<<19ul),

	// 20-32 bit - reserved to Editor
	//from editor core - 29 bit last
	rsStatisticFPS = (1ul << 30ul),
};


//. ENGINE_API extern	u32			psCurrentMode		;
ENGINE_API extern	u32			psCurrentVidMode[];
ENGINE_API extern	u32			psCurrentBPP		;
ENGINE_API extern	u32			psCurrentFontProfileIndex;
ENGINE_API extern	u32			psCurrentLanguageIndex;
ENGINE_API extern	Flags32		psDeviceFlags		;


ENGINE_API extern	Flags32		g_uCommonFlags;

#if _MSC_VER < 1900
ENGINE_API 
#endif
enum E_COMMON_FLAGS
{
	flAiUseTorchDynamicLights	= 1<<0,
	enShowObjectHit				= 1<<1,
	mwShowInvulnerableIcon		= 1<<2,
	uiShowConditions			= 1<<3,
	uiShowSelected				= 1<<4,
	uiShowExtDesc				= 1<<5,
	uiShowTradeSB				= 1<<6,
	uiShowFocused				= 1<<7,
	invReloadWeapon				= 1<<8,
	uiAllowOpTradeSB			= 1<<9,
	gpDeferredReload			= 1<<10,
	gpFixedReload				= 1<<11,
	gpStickyScope				= 1<<12,
	gpOpenScope					= 1<<13,
	gpDemandReload				= 1<<14,
	gpShowModificators			= 1<<15
};

ENGINE_API extern	float			lpLoadScreenTextOffsetX;
ENGINE_API extern	float			lpLoadScreenTextOffsetY;
ENGINE_API extern	bool			lpLoadScreenEnableProgressBar;
ENGINE_API extern bool pMMMusic;
ENGINE_API extern bool gPlayLevelAmbientMusic;
ENGINE_API extern bool gPlayLevelAmbientSounds;
ENGINE_API extern bool gInvertTrade;

// game path definition
#define _game_data_				"$game_data$"
#define _game_textures_			"$game_textures$"
#define _game_levels_			"$game_levels$"
#define _game_sounds_			"$game_sounds$"
#define _game_meshes_			"$game_meshes$"
#define _game_shaders_			"$game_shaders$"
#define _game_config_			"$game_config$"
#define _game_huds_				"$game_huds$"

// editor path definition
#define _server_root_		    "$server_root$"
#define _server_data_root_	    "$server_data_root$"
#define _local_root_		    "$local_root$"
#define _import_			    "$import$"
#define _sounds_			    "$sounds$"
#define _textures_			    "$textures$"
#define _objects_			    "$objects$"
#define _maps_				    "$maps$"
#define _temp_				    "$temp$"
#define _omotion_			    "$omotion$"
#define _omotions_			    "$omotions$"
#define _smotion_			    "$smotion$"
#define _detail_objects_	    "$detail_objects$"

#endif
