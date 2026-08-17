/**
 * @file Phyx2DUIExt.h
 * @brief 2d physics as #ZFUIView
 */

#ifndef _ZFI_Phyx2DUIExt_h_
#define _ZFI_Phyx2DUIExt_h_

#include "Phyx2D.h"
#include "TileView.h"
ZF_NAMESPACE_GLOBAL_BEGIN

zfclassFwd _ZFP_P2WorldViewPrivate;
/**
 * @brief 2d physics as #ZFUIView
 *
 * this is a helper class to make physics more easier to use,
 * typical usage in lua:
 * @code
 *   local world = P2WorldView()
 *   world
 *      :p2_unit(SomeView()
 *          :p2_bodyId('body id 0')
 *          :p2_shape(SomeShape())
 *      )
 *      :p2_unit(P2Unit()
 *          :p2_body(SomeView()
 *              :p2_bodyId('body id 1')
 *          )
 *          :p2_part(SomeView()
 *              :p2_bodyId('body id 2')
 *          )
 *          :p2_joint(SomeJoint('body id 1', 'body id 2'))
 *      )
 *      :p2_unit(zfres('some_unit.xml'))
 *      :p2_joint(SomeJoint('body id 0', 'body id 1'))
 * @endcode
 *
 * you can build 2d physics world quickly,
 * just like building #ZFUIView view tree\n
 * \n
 * NOTE:
 * -  for #P2World, origin point is at left down zero point,
 *   and rotation is counter clockwise
 * -  for #ZFUIView, origin point is at left top zero point,
 *   and rotation is clockwise
 * -  when using #P2WorldView,
 *   you should always use methods declared in #P2World (all method name starts with `p2_`)
 *   to manage children/body/shapes
 */
zfclass ZFLIB_ZF2DGame P2WorldView : zfextend ZFUIView {
    ZFOBJECT_DECLARE(P2WorldView, ZFUIView)

public:
    /** @brief draw debug colors to indicates all body/shape/joint */
    ZFPROPERTY_ASSIGN(zfbool, debugDraw)
    ZFPROPERTY_ON_ATTACH_DECLARE(zfbool, debugDraw)
    ZFPROPERTY_ON_DETACH_DECLARE(zfbool, debugDraw)

    /**
     * @brief tiled bg automatically updated with #P2World::p2_UIOffset
     *
     * by default, tiles would be updated by #P2World::p2_UIOffset,
     * the update mode can be changed by:
     * #tileUpdateByUI / #tileSpeed / #tileOffset,
     * and can be checked by:
     * #tileIsUpdateByUI / #tileIsUpdateBySpeed / #tileIsUpdateByOffset
     */
    ZFPROPERTY_ASSIGN(ZFCoreArray<zfautoT<TileView> >, tileBg)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFCoreArray<zfautoT<TileView> >, tileBg)
    /** @brief see #tileBg */
    ZFPROPERTY_ASSIGN(ZFCoreArray<zfautoT<TileView> >, tileFg)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFCoreArray<zfautoT<TileView> >, tileFg)

    /** @brief see #tileBg, make #tileBg and #tileFg auto update by #P2World::p2_UIOffset */
    ZFMETHOD_DECLARE_0(void, tileUpdateByUI)
    /** @brief see #tileBg, whether #tileBg and #tileFg update by #tileUpdateByUI */
    ZFMETHOD_DECLARE_0(zfbool, tileIsUpdateByUI)

    /** @brief see #tileBg, make #tileBg and #tileFg auto update by specified speed */
    ZFMETHOD_DECLARE_1(void, tileSpeed
            , ZFMP_IN(const ZFUIPoint &, v)
            )
    /** @brief #ZFUIPointZero if not #tileIsUpdateBySpeed */
    ZFMETHOD_DECLARE_0(ZFUIPoint, tileSpeed)
    /** @brief see #tileBg, whether #tileBg and #tileFg update by #tileSpeed */
    ZFMETHOD_DECLARE_0(zfbool, tileIsUpdateBySpeed)

    /** @brief see #tileBg, make #tileBg and #tileFg manually update by specified offset */
    ZFMETHOD_DECLARE_1(void, tileOffset
            , ZFMP_IN(const ZFUIPoint &, v)
            )
    /** @brief #ZFUIPointZero if not #tileIsUpdateByOffset */
    ZFMETHOD_DECLARE_0(ZFUIPoint, tileOffset)
    /** @brief see #tileBg, whether #tileBg and #tileFg update by #tileOffset */
    ZFMETHOD_DECLARE_0(zfbool, tileIsUpdateByOffset)

protected:
    zfoverride
    virtual void objectOnInit(void);
    zfoverride
    virtual void objectOnDealloc(void);
    zfoverride
    virtual void objectOnInitFinish(void);
    zfoverride
    virtual void objectOnDeallocPrepare(void);
    zfoverride
    virtual void observerOnAdd(ZF_IN zfidentity eventId) {
        zfsuper::observerOnAdd(eventId);
        ZFObserver::forwardOnAdd(zfcast(P2World *, this), eventId);
    }
    zfoverride
    virtual void observerOnRemove(ZF_IN zfidentity eventId) {
        ZFObserver::forwardOnRemove(zfcast(P2World *, this), eventId);
        zfsuper::observerOnRemove(eventId);
    }
    zfoverride
    virtual void layoutOnLayout(ZF_IN const ZFUIRect &bounds);
private:
    _ZFP_P2WorldViewPrivate *d;
};

// ============================================================
/** @brief transform coordinate */
ZFMETHOD_FUNC_DECLARE_6(ZFLIB_ZF2DGame, void, P2CoordinateToRectT
        , ZFMP_OUT(ZFUIRect &, rect)
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIPoint &, position)
        , ZFMP_IN(const ZFUIRect &, aabbLocal)
        , ZFMP_IN_OPT(zffloat, rotation, 0)
        , ZFMP_IN_OPT(const ZFUIPoint &, centerOfMass, ZFUIPointZero())
        )
/** @brief transform coordinate */
ZFMETHOD_FUNC_DECLARE_5(ZFLIB_ZF2DGame, ZFUIRect, P2CoordinateToRect
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIPoint &, position)
        , ZFMP_IN(const ZFUIRect &, aabbLocal)
        , ZFMP_IN_OPT(zffloat, rotation, 0)
        , ZFMP_IN_OPT(const ZFUIPoint &, centerOfMass, ZFUIPointZero())
        )

/** @brief transform coordinate */
ZFMETHOD_FUNC_DECLARE_3(ZFLIB_ZF2DGame, void, P2AABBToRectT
        , ZFMP_OUT(ZFUIRect &, rect)
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, aabb)
        )
/** @brief transform coordinate */
ZFMETHOD_FUNC_DECLARE_2(ZFLIB_ZF2DGame, ZFUIRect, P2AABBToRect
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, aabb)
        )

/** @brief transform coordinate */
ZFMETHOD_FUNC_DECLARE_3(ZFLIB_ZF2DGame, void, P2AABBFromRectT
        , ZFMP_OUT(ZFUIRect &, aabb)
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, rect)
        )
/** @brief transform coordinate */
ZFMETHOD_FUNC_DECLARE_2(ZFLIB_ZF2DGame, ZFUIRect, P2AABBFromRect
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, rect)
        )

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_Phyx2DUIExt_h_

