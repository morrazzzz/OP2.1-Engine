#pragma once

namespace GameObject {
	enum ECallbackType {
		eTradeStart = u32(0),
		eTradeStop,
		eTradeSellBuyItem,
		eTradePerformTradeOperation,

		eZoneEnter,
		eZoneExit,
		eExitLevelBorder,
		eEnterLevelBorder,
		eDeath,

		ePatrolPathInPoint,

		eInventoryPda,
		eInventoryInfo,
		eArticleInfo,
		eTaskStateChange,
		eMapLocationAdded,

		eUseObject,

		eHit,

		eSound,

		eActionTypeMovement,
		eActionTypeWatch,
		eActionTypeAnimation,
		eActionTypeSound,
		eActionTypeParticle,
		eActionTypeObject,

		eActorSleep,

		eHelicopterOnPoint,
		eHelicopterOnHit,

		eOnItemTake,
		eOnItemDrop,

		eScriptAnimation,
		
		eTraderGlobalAnimationRequest,
		eTraderHeadAnimationRequest,
		eTraderSoundEnd,

		eInvBoxItemTake,

		eOnActorWeaponFire,

		eOnNPCWeaponFire,

		eOnObjectHit,

		eOnUIShowPropBox,

		eOnReserved1,
		eOnReserved2,

		OnInventoryItemPlaceChange,
		OnKeyboardPress,
		OnKeyboardRelease,
		OnPrepareItemInfo,
		OnDisableInfoPortion,
		OnDragDropListCellFocusReceive,
		OnDragDropListCellFocusLost,
		OnDragDropListCellSelected,
		OnDragDropListDBLClickCell,
		eDummy = u32(-1),
	};
};

enum EBoneCallbackType{
	bctDummy			= u32(0),	// 0 - required!!!
	bctPhysics,
	bctCustom,
	bctForceU32			= u32(-1),
};