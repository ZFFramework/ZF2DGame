/**
 * @file ShakeEffect.h
 * @brief effect animation util
 */

#ifndef _ZFI_ShakeEffect_h_
#define _ZFI_ShakeEffect_h_

#include "ZF2DGameDef.h"
ZF_NAMESPACE_GLOBAL_BEGIN

/**
 * @brief effect animation util
 */
zfclass ZFLIB_ZF2DGame ShakeEffect : zfextend ZFAniForTimer {
    ZFOBJECT_DECLARE(ShakeEffect, ZFAniForTimer)

public:
    /**
     * @brief shake direction, -1 for random, [0, 360) to use specified direction
     */
    ZFPROPERTY_ASSIGN(zffloat, direction, -1)
    /**
     * @brief shake offset
     */
    ZFPROPERTY_ASSIGN(zffloat, offset, 4)
    /**
     * @brief whether reduce #offset during shaking
     */
    ZFPROPERTY_ASSIGN(zfbool, damping, zftrue)

protected:
    zfoverride
    virtual void aniTimerOnUpdate(ZF_IN zffloat progress);
    zfoverride
    virtual void aniOnStop(ZF_IN ZFResultType resultType);
private:
    zfbool _positive;
};

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_ShakeEffect_h_

