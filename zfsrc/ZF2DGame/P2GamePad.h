/**
 * @file P2GamePad.h
 * @brief game pad util
 */

#ifndef _ZFI_P2GamePad_h_
#define _ZFI_P2GamePad_h_

#include "ZF2DGameDef.h"
ZF_NAMESPACE_GLOBAL_BEGIN

/**
 * @brief game pad button define
 */
ZFENUM_BEGIN(ZFLIB_ZF2DGame, P2GamePadButton)
    ZFENUM_VALUE(DPadL = 1 << 0) /**< @brief dpad left */
    ZFENUM_VALUE(DPadT = 1 << 1) /**< @brief dpad top */
    ZFENUM_VALUE(DPadR = 1 << 2) /**< @brief dpad right */
    ZFENUM_VALUE(DPadB = 1 << 3) /**< @brief dpad bottom */
    ZFENUM_VALUE(BtnL = 1 << 4) /**< @brief button left */
    ZFENUM_VALUE(BtnT = 1 << 5) /**< @brief button top */
    ZFENUM_VALUE(BtnR = 1 << 6) /**< @brief button right */
    ZFENUM_VALUE(BtnB = 1 << 7) /**< @brief button bottom */
    ZFENUM_VALUE(Select = 1 << 8) /**< @brief select button */
    ZFENUM_VALUE(Start = 1 << 9) /**< @brief start button */
    ZFENUM_VALUE(L1 = 1 << 10) /**< @brief L1 */
    ZFENUM_VALUE(R1 = 1 << 11) /**< @brief R1 */
    ZFENUM_VALUE(L2 = 1 << 12) /**< @brief L2 */
    ZFENUM_VALUE(R2 = 1 << 13) /**< @brief R2 */
    ZFENUM_VALUE(L3 = 1 << 14) /**< @brief L3 */
    ZFENUM_VALUE(R3 = 1 << 15) /**< @brief R3 */
    ZFENUM_VALUE(LStickX = 1 << 16) /**< @brief left stick's X axis, right is positive */
    ZFENUM_VALUE(LStickY = 1 << 17) /**< @brief left stick's Y axis, top is positive */
    ZFENUM_VALUE(RStickX = 1 << 18) /**< @brief left stick's X axis, right is positive */
    ZFENUM_VALUE(RStickY = 1 << 19) /**< @brief left stick's Y axis, top is positive */
    ZFENUM_VALUE(Custom0 = 1 << 20) /**< @brief custom button */
    ZFENUM_VALUE(Custom1 = 1 << 21) /**< @brief custom button */
    ZFENUM_VALUE(Custom2 = 1 << 22) /**< @brief custom button */
    ZFENUM_VALUE(Custom3 = 1 << 23) /**< @brief custom button */
    ZFENUM_VALUE(Custom4 = 1 << 24) /**< @brief custom button */
    ZFENUM_VALUE(Custom5 = 1 << 25) /**< @brief custom button */
    ZFENUM_VALUE(Custom6 = 1 << 26) /**< @brief custom button */
    ZFENUM_VALUE(Custom7 = 1 << 27) /**< @brief custom button */
ZFENUM_SEPARATOR()
    ZFENUM_VALUE_REGISTER(DPadL)
    ZFENUM_VALUE_REGISTER(DPadT)
    ZFENUM_VALUE_REGISTER(DPadR)
    ZFENUM_VALUE_REGISTER(DPadB)
    ZFENUM_VALUE_REGISTER(BtnL)
    ZFENUM_VALUE_REGISTER(BtnT)
    ZFENUM_VALUE_REGISTER(BtnR)
    ZFENUM_VALUE_REGISTER(BtnB)
    ZFENUM_VALUE_REGISTER(Select)
    ZFENUM_VALUE_REGISTER(Start)
    ZFENUM_VALUE_REGISTER(L1)
    ZFENUM_VALUE_REGISTER(R1)
    ZFENUM_VALUE_REGISTER(L2)
    ZFENUM_VALUE_REGISTER(R2)
    ZFENUM_VALUE_REGISTER(L3)
    ZFENUM_VALUE_REGISTER(R3)
    ZFENUM_VALUE_REGISTER(LStickX)
    ZFENUM_VALUE_REGISTER(LStickY)
    ZFENUM_VALUE_REGISTER(RStickX)
    ZFENUM_VALUE_REGISTER(RStickY)
    ZFENUM_VALUE_REGISTER(Custom0)
    ZFENUM_VALUE_REGISTER(Custom1)
    ZFENUM_VALUE_REGISTER(Custom2)
    ZFENUM_VALUE_REGISTER(Custom3)
    ZFENUM_VALUE_REGISTER(Custom4)
    ZFENUM_VALUE_REGISTER(Custom5)
    ZFENUM_VALUE_REGISTER(Custom6)
    ZFENUM_VALUE_REGISTER(Custom7)
ZFENUM_END(ZFLIB_ZF2DGame, P2GamePadButton)
ZFENUM_REG(ZFLIB_ZF2DGame, P2GamePadButton)

// ============================================================
ZF_NAMESPACE_BEGIN(ZFGlobalEvent)
/**
 * @brief called when #P2GamePadIndexList changed
 */
ZFEVENT_GLOBAL(ZFLIB_ZF2DGame, P2GamePadChange)
ZF_NAMESPACE_END(ZFGlobalEvent)

/**
 * @brief max possible pads
 */
ZFMETHOD_FUNC_DECLARE_0(ZFLIB_ZF2DGame, zfindex, P2GamePadMax)

/**
 * @brief return an observer for specified padIndex
 *
 * callback's param0 would be a #v_P2GamePadButton holds button
 * whose #P2GamePadPressed or #P2GamePadValue changed,
 * param1 would be a #v_zfbool holds #P2GamePadPressed
 *
 * it's ensured safe to attach or detach observers
 * even if game pad not connected,
 * but the returned observer object should not be stored for future use\n
 * note the observers previously attached won't be cleared
 * even if game pad disconnected (until #ZFFrameworkCleanup),
 * you must manage them manually
 */
ZFMETHOD_FUNC_DECLARE_1(ZFLIB_ZF2DGame, ZFObserver, P2GamePadObserver
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )
/**
 * @brief check whether specified button pressed
 */
ZFMETHOD_FUNC_DECLARE_2(ZFLIB_ZF2DGame, zfbool, P2GamePadPressed
        , ZFMP_IN(P2GamePadButton, btn)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )
/**
 * @brief check specified button's value
 *
 * -  for normal button, possible values are 0 and 1
 * -  for joy sticks, ranged in [-1, 1],
 *   values inside dead zone already normalized to 0
 * -  for L2/R2, may be only 0 or 1, may be range in [0, 1], depends on game pad
 */
ZFMETHOD_FUNC_DECLARE_2(ZFLIB_ZF2DGame, zffloat, P2GamePadValue
        , ZFMP_IN(P2GamePadButton, btn)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )

/**
 * @brief a list of currently connected game pad's index
 *
 * note:
 * -  the index may or may not be continuous
 * -  the index ranged in [0, #P2GamePadMax)
 */
ZFMETHOD_FUNC_DECLARE_0(ZFLIB_ZF2DGame, ZFCoreArray<zfindex>, P2GamePadIndexList)

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_P2GamePad_h_

