/**
 * @file TileView.h
 * @brief tile view util
 */

#ifndef _ZFI_TileView_h_
#define _ZFI_TileView_h_

#include "ZF2DGameDef.h"
ZF_NAMESPACE_GLOBAL_BEGIN

/**
 * @brief tile view
 */
zfclass ZFLIB_ZF2DGame TileView : zfextend ZFUIDrawableView {
    ZFOBJECT_DECLARE_WITH_CUSTOM_CTOR(TileView, ZFUIDrawableView)

public:
    /** @brief the tile image */
    ZFPROPERTY_RETAIN(zfanyT<ZFUIImage>, tile)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfanyT<ZFUIImage>, tile)

    /** @brief tile scale, 0 means scale to fill the view, <0 means scale and keep aspect ratio */
    ZFPROPERTY_ASSIGN(ZFUISize, tileScale, ZFUISizeCreate(1))
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUISize, tileScale)

    /** @brief extra space between each tile */
    ZFPROPERTY_ASSIGN(ZFUISize, tileSpace)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUISize, tileSpace)

    /** @brief extra scale for tileOffset */
    ZFPROPERTY_ASSIGN(ZFUISize, tileOffsetScale, ZFUISizeCreate(1))
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUISize, tileOffsetScale)

    /** @brief tile offset */
    ZFMETHOD_DECLARE_0(const ZFUIPoint &, tileOffset)
    /** @brief tile offset */
    ZFMETHOD_DECLARE_1(void, tileOffset
            , ZFMP_IN(const ZFUIPoint &, v)
            )
    /** @brief util to update #tileOffset by step diff */
    ZFMETHOD_DECLARE_2(void, tileOffsetStep
            , ZFMP_IN(zffloat, x)
            , ZFMP_IN(zffloat, y)
            )

public:
    zfoverride
    virtual void onDraw(void);

private:
    ZFUIPoint _tileOffset;
protected:
    /** @cond ZFPrivateDoc */
    TileView(void) : _tileOffset(ZFUIPointZero()) {}
    /** @endcond */
};

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_TileView_h_

