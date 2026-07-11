/**
 * @file Phyx2DUIExt.h
 * @brief 2d physics as #ZFUIView
 */

#ifndef _ZFI_Phyx2DUIExt_h_
#define _ZFI_Phyx2DUIExt_h_

#include "Phyx2D.h"
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
 * just like building #ZFUIView view tree
 */
zfclass P2WorldView : zfextend ZFUIView {
    ZFOBJECT_DECLARE(P2WorldView, ZFUIView)

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
    virtual void layoutOnLayout(ZF_IN const ZFUIRect &bounds);
private:
    _ZFP_P2WorldViewPrivate *d;
};

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_Phyx2DUIExt_h_

