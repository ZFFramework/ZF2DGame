/**
 * @file Phyx2D.h
 * @brief 2d physics
 */

#ifndef _ZFI_Phyx2D_h_
#define _ZFI_Phyx2D_h_

#include "ZF2DGameDef.h"
ZF_NAMESPACE_GLOBAL_BEGIN

zfclassFwd P2Body;
zfclassFwd P2Unit;
zfclassFwd P2World;

/** @brief see #P2World */
#define P2FilterMaskAll() ((zfflags)-1)
/** @brief see #P2World */
#define P2FilterCategoryDefault() ((zfflags)0x80000000)
/** @brief see #P2World */
#define P2FilterMaskDefault() P2FilterMaskAll()

/** @brief see #P2World */
ZFEXPORT_VAR_READONLY_DECLARE(ZFLIB_ZF2DGame, zfflags, P2FilterMaskAll)
/** @brief see #P2World */
ZFEXPORT_VAR_READONLY_DECLARE(ZFLIB_ZF2DGame, zfflags, P2FilterCategoryDefault)
/** @brief see #P2World */
ZFEXPORT_VAR_READONLY_DECLARE(ZFLIB_ZF2DGame, zfflags, P2FilterMaskDefault)

// ============================================================
zfclassFwd _ZFP_P2ShapePrivate;
/** @brief see #P2World */
zfabstract ZFLIB_ZF2DGame P2Shape : zfextend ZFStyle {
    ZFOBJECT_DECLARE_ABSTRACT(P2Shape, ZFStyle)

public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2Body *, p2_ownerBody)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2Unit *, p2_ownerUnit)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2World *, p2_ownerWorld)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_shapeRemoveLater)

public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfstring, p2_shapeId)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_density, 1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_density)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_sensor)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_sensorEnable)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_sensorEnable)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_contactEnable)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_contactEnable)

    /** @brief see #P2World, what category this shape is */
    ZFPROPERTY_ASSIGN(zfflags, p2_filterCategory, P2FilterCategoryDefault())
    ZFPROPERTY_ON_UPDATE_DECLARE(zfflags, p2_filterCategory)
    /** @brief see #P2World, what type should be collided with this shape */
    ZFPROPERTY_ASSIGN(zfflags, p2_filterMask, P2FilterMaskDefault())
    ZFPROPERTY_ON_UPDATE_DECLARE(zfflags, p2_filterMask)
    /** @brief see #P2World, always collide (>0) or never collide (<0) within same group */
    ZFPROPERTY_ASSIGN(zfint, p2_filterGroup, 0)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfint, p2_filterGroup)

    /** @brief see #P2World, range in [0, 1] */
    ZFPROPERTY_ASSIGN(zffloat, p2_matFriction, 0.6f)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_matFriction)
    /** @brief see #P2World, range in [0, 1], how much to bounce when collide */
    ZFPROPERTY_ASSIGN(zffloat, p2_matBounce)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_matBounce)
    /** @brief see #P2World, range in [0, 1] */
    ZFPROPERTY_ASSIGN(zffloat, p2_matRotationResist)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_matRotationResist)
    /**
     * @brief see #P2World
     *
     * -  0 : normal plain ground, won't be pushed by friction
     * -  [0, 1] : slightly pushed, e.g wind
     * -  [1, 10] : conveyor belt, from low to high speed
     * -  <0 : reverse
     */
    ZFPROPERTY_ASSIGN(zffloat, p2_matTangentSpeed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_matTangentSpeed)

public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(ZFUIRect, p2_AABB)

protected:
    /** @brief for impl only */
    virtual inline void p2impl_shapeCreate(ZF_IN P2Body *ownerBody) {}
    zfoverride
    virtual void objectOnInit(void);
    zfoverride
    virtual void objectOnDealloc(void);

public:
    _ZFP_P2ShapePrivate *_ZFP_P2Shape_d;
    friend zfclassFwd _ZFP_P2ShapePrivate;
};

/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2ShapeBox : zfextend P2Shape {
    ZFOBJECT_DECLARE(P2ShapeBox, P2Shape)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_position)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_width)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_height)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_rotation)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_radius)
protected:
    zfoverride
    virtual void p2impl_shapeCreate(ZF_IN P2Body *ownerBody);
};
/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2ShapeCircle : zfextend P2Shape {
    ZFOBJECT_DECLARE(P2ShapeCircle, P2Shape)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_position)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_radius)
protected:
    zfoverride
    virtual void p2impl_shapeCreate(ZF_IN P2Body *ownerBody);
};
/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2ShapeCapsule : zfextend P2Shape {
    ZFOBJECT_DECLARE(P2ShapeCapsule, P2Shape)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_position0)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_position1)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_radius)
protected:
    zfoverride
    virtual void p2impl_shapeCreate(ZF_IN P2Body *ownerBody);
};
/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2ShapePolygon : zfextend P2Shape {
    ZFOBJECT_DECLARE(P2ShapePolygon, P2Shape)
public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_4(void, p2_polygon
            , ZFMP_IN(const ZFCoreArray<ZFUIPoint> &, points)
            , ZFMP_IN_OPT(const ZFUIPoint &, position, ZFUIPointZero())
            , ZFMP_IN_OPT(zffloat, rotation, 0)
            , ZFMP_IN_OPT(zffloat, radius, 0)
            )
protected:
    zfoverride
    virtual void p2impl_shapeCreate(ZF_IN P2Body *ownerBody);

public:
    /** @brief use #p2_polygon to update, do not access manually */
    ZFPROPERTY_ASSIGN(ZFCoreArray<ZFUIPoint>, p2_polygon_points)
    /** @brief use #p2_polygon to update, do not access manually */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_polygon_position)
    /** @brief use #p2_polygon to update, do not access manually */
    ZFPROPERTY_ASSIGN(zffloat, p2_polygon_rotation)
    /** @brief use #p2_polygon to update, do not access manually */
    ZFPROPERTY_ASSIGN(zffloat, p2_polygon_radius)
};

// ============================================================
zfclassFwd _ZFP_P2JointPrivate;
/** @brief see #P2World */
zfabstract ZFLIB_ZF2DGame P2Joint : zfextend ZFStyle {
    ZFOBJECT_DECLARE_ABSTRACT(P2Joint, ZFStyle)

public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2Body *, p2_ownerBody0)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2Body *, p2_ownerBody1)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2World *, p2_ownerWorld)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_jointRemoveLater)

public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfstring, p2_jointId)

    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfstring, p2_bodyId0)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfstring, p2_bodyId1)

    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_collideEnable)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_collideEnable)

protected:
    /** @brief init with body id */
    ZFOBJECT_ON_INIT_DECLARE_2(
            ZFMP_IN(const zfstring &, bodyId0)
            , ZFMP_IN(const zfstring &, bodyId1)
            )
protected:
    /** @brief for impl only */
    virtual inline void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {}
    zfoverride
    virtual void objectOnInit(void);
    zfoverride
    virtual void objectOnDealloc(void);

public:
    _ZFP_P2JointPrivate *_ZFP_P2Joint_d;
    friend zfclassFwd _ZFP_P2JointPrivate;
};

/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZF2DGame, zffloat, P2JointSpringHertzSoft)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZF2DGame, zffloat, P2JointSpringDampingSoft)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZF2DGame, zffloat, P2JointSpringHertzNormal)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZF2DGame, zffloat, P2JointSpringDampingNormal)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZF2DGame, zffloat, P2JointSpringHertzHard)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZF2DGame, zffloat, P2JointSpringDampingHard)

/** @brief see #P2World */
zfabstract ZFLIB_ZF2DGame P2JointSpring : zfextend P2Joint {
    ZFOBJECT_DECLARE_ABSTRACT(P2JointSpring, P2Joint)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_anchor0)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_anchor1)

    /** @brief see #P2World, usually [0, 60], how many times to update spring for each second */
    ZFPROPERTY_ASSIGN(zffloat, p2_springHertz, 0)
    /** @brief see #P2World, [0, 1], from soft to hard, how effective the spring is */
    ZFPROPERTY_ASSIGN(zffloat, p2_springDamping, 0)

    /**
     * @brief see #P2World
     *
     * -  for positional motor: `N m/s`
     * -  for rotational motor: `N degree/s`
     */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorSpeed)
    /** @brief see #P2World, usually [50, 1000] */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorForce)
};

/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2JointDistance : zfextend P2JointSpring {
    ZFOBJECT_DECLARE(P2JointDistance, P2JointSpring)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distance, 1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distance)
    /** @brief see #P2World, >0 to enable limit */
    ZFMETHOD_DECLARE_0(zffloat, p2_distanceCur)
    /** @brief see #P2World, <0 to enable limit */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMin)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMin)
    /** @brief see #P2World, >0 to enable limit */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMax)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMax)

    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor0)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorSpeed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)

protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2JointRevolute : zfextend P2JointSpring {
    ZFOBJECT_DECLARE(P2JointRevolute, P2JointSpring)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularRef)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularRef)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angular)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angular)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zffloat, p2_angularCur)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularLimitMin)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularLimitMin)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularLimitMax)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularLimitMax)

    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor0)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorSpeed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)

protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2JointPrismatic : zfextend P2JointSpring {
    ZFOBJECT_DECLARE(P2JointPrismatic, P2JointSpring)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_axis, ZFUIPointCreate(1, 0))
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_axis)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularRef)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularRef)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distance)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distance)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zffloat, p2_distanceCur)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMin)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMin)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMax)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMax)

    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor0)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorSpeed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)

protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2JointWheel : zfextend P2JointSpring {
    ZFOBJECT_DECLARE(P2JointWheel, P2JointSpring)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_axis)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_axis)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMin)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMin)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMax)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMax)

    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor0)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorSpeed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)

protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZF2DGame, zffloat, P2JointMotorFactorSoft)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZF2DGame, zffloat, P2JointMotorFactorNormal)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZF2DGame, zffloat, P2JointMotorFactorHard)

/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2JointMotor : zfextend P2Joint {
    ZFOBJECT_DECLARE(P2JointMotor, P2Joint)
public:
    /** @brief see #P2World, usually (0, 1), how hard the motor take effective */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorFactor, P2JointMotorFactorNormal())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorFactor)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorForce, 1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorTorque, 1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorTorque)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_distance)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_distance)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(ZFUIPoint, p2_distanceCur)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angular)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angular)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zffloat, p2_angularCur)
protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2JointWeld : zfextend P2Joint {
    ZFOBJECT_DECLARE(P2JointWeld, P2Joint)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_anchor0)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor0)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_anchor1)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor1)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularRef)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularRef)

    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceHertz)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceDamping)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularHertz)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularDamping)
protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2JointFilter : zfextend P2Joint {
    ZFOBJECT_DECLARE(P2JointFilter, P2Joint)
protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

// ============================================================
/** @brief see #P2World */
ZFENUM_BEGIN(ZFLIB_ZF2DGame, P2BodyType)
    ZFENUM_VALUE(Static) /**< @brief see #P2World */
    ZFENUM_VALUE(Kinematic) /**< @brief see #P2World */
    ZFENUM_VALUE(Dynamic) /**< @brief see #P2World */
ZFENUM_SEPARATOR()
    ZFENUM_VALUE_REGISTER(Static)
    ZFENUM_VALUE_REGISTER(Kinematic)
    ZFENUM_VALUE_REGISTER(Dynamic)
ZFENUM_END(ZFLIB_ZF2DGame, P2BodyType)
ZFENUM_REG(ZFLIB_ZF2DGame, P2BodyType)

zfclassFwd _ZFP_P2BodyPrivate;
/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2Body : zfextend ZFStyle {
    ZFOBJECT_DECLARE(P2Body, ZFStyle)

public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2Unit *, p2_ownerUnit)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2World *, p2_ownerWorld)
    /**
     * @brief remove this body after current simulation step,
     *   if this body is the main #P2Unit::p2_body of an unit,
     *   then the entire unit would be removed instead
     *
     * shape/body/unit may be removed during simulation step
     * (during callback of app code, #P2World::p2impl_contactEvent for example),
     * causing #P2SensorEventData or #P2ContactEventData's shape to be null,
     * which is not expected\n
     * to make things easier,
     * you may use this method to remove things (instead of #P2World::p2_unitRemove),
     * so that they would be removed after your logical code
     */
    ZFMETHOD_DECLARE_0(void, p2_bodyRemoveLater)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_unitRemoveLater)

    /** @brief see #P2World */
    ZFPROPERTY_RETAIN_READONLY(ZFArray *, p2_shapeList, zfobj<ZFArray>())
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_shape
            , ZFMP_IN(P2Shape *, shape)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfindex, p2_shapeCount)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(P2Shape *, p2_shapeAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Shape>, p2_shapeRemoveAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Shape>, p2_shapeRemove
            , ZFMP_IN(P2Shape *, shape)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_shapeRemoveAll)

    /** @brief see #P2World, note this method is expensive */
    ZFMETHOD_DECLARE_1(P2Shape *, p2_shapeFind
            , ZFMP_IN(const zfstring &, shapeId)
            )
    /** @brief see #P2World, note this method is expensive */
    ZFMETHOD_DECLARE_1(P2Joint *, p2_jointFind
            , ZFMP_IN(const zfstring &, jointId)
            )

    /**
     * @brief see #P2World
     *
     * get a copy of all joints that referenced this body,
     * including joints in #P2Unit::p2_jointList and #P2World::p2_jointList
     * must not manually modify,
     * valid only after added to world and first time updated
     */
    ZFMETHOD_DECLARE_0(zfautoT<ZFContainer>, p2_refJointList)

public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfstring, p2_bodyId)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfstring, p2_bodyId)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(P2BodyType, p2_type)
    ZFPROPERTY_ON_UPDATE_DECLARE(P2BodyType, p2_type)

    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_enable)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_enable)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_sleepEnable)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_sleepEnable)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_bullet)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_bullet)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_rotationFixed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_rotationFixed)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_gravityScale)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_gravityScale)

    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_position)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_position)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_positionVelocity)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_positionVelocity)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_positionDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_positionDamping)

    /** @brief see #P2World, range in [0, 360] */
    ZFPROPERTY_ASSIGN(zffloat, p2_rotation)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_rotation)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_rotationVelocity)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_rotationVelocity)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_rotationDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_rotationDamping)

public:
    /** @brief see #P2World, note: calculate each time called */
    ZFMETHOD_DECLARE_0(ZFUIRect, p2_AABB)
    /** @brief get all child shape's bounding size, calculate and cached automatically */
    ZFMETHOD_DECLARE_0(ZFUISize, p2_bodySize)

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zffloat, p2_mass)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(ZFUIPoint, p2_centerOfMass)
    /**
     * @brief see #P2World
     *
     * value to descript how the body is easy to rotate:
     * -  0 : can not rotate on force
     * -  (0, 10] : smaller value means more easier to rotate
     */
    ZFMETHOD_DECLARE_0(zffloat, p2_rotationInertia)

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(ZFUIPoint, p2_positionCur)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(ZFUIPoint, p2_positionVelocityCur)

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zffloat, p2_rotationCur)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zffloat, p2_rotationVelocityCur)

public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfbool, sleeping)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, sleeping
            , ZFMP_IN(zfbool, v)
            )

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_3(void, p2_moveTo
            , ZFMP_IN(zftimet, duration)
            , ZFMP_IN(const ZFUIPoint &, position)
            , ZFMP_IN_OPT(zffloat, rotation, -1)
            )

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_3(void, p2_applyPositionForceInWorld
            , ZFMP_IN(const ZFUIPoint &, force)
            , ZFMP_IN(const ZFUIPoint &, worldPosition)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_2(void, p2_applyPositionForce
            , ZFMP_IN(const ZFUIPoint &, force)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_2(void, p2_applyRotationForce
            , ZFMP_IN(zffloat, force)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_3(void, p2_applyPositionImpulseInWorld
            , ZFMP_IN(const ZFUIPoint &, impulse)
            , ZFMP_IN(const ZFUIPoint &, worldPosition)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_2(void, p2_applyPositionImpulse
            , ZFMP_IN(const ZFUIPoint &, impulse)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_2(void, p2_applyRotationImpulse
            , ZFMP_IN(zffloat, impulse)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )

protected:
    zfoverride
    virtual void objectOnInit(void);
    zfoverride
    virtual void objectOnDealloc(void);

public:
    _ZFP_P2BodyPrivate *_ZFP_P2Body_d;
};

// ============================================================
zfclassFwd _ZFP_P2UnitPrivate;
/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2Unit : zfextend ZFStyle {
    ZFOBJECT_DECLARE(P2Unit, ZFStyle)

public:
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when unit visibility changed by #P2World::p2_UIVisibleArea,
     * current visibility can be checked by #p2_unitVisible
     */
    ZFEVENT(P2UnitOnVisibilityUpdate)

    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this unit got #P2SensorEvent::p2_sensorEnterList,
     * and this unit is the sensor\n
     * param0 is the visitor #P2Unit
     */
    ZFEVENT(P2UnitOnSensorEnter)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this unit got #P2SensorEvent::p2_sensorExitList,
     * and this unit is the sensor\n
     * param0 is the visitor #P2Unit
     */
    ZFEVENT(P2UnitOnSensorExit)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this unit got #P2SensorEvent::p2_sensorEnterList,
     * and this unit is the visitor\n
     * param0 is the sensor #P2Unit
     */
    ZFEVENT(P2UnitOnVisitorEnter)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this unit got #P2SensorEvent::p2_sensorExitList,
     * and this unit is the visitor\n
     * param0 is the sensor #P2Unit
     */
    ZFEVENT(P2UnitOnVisitorExit)

    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this unit got #P2ContactEvent::p2_contactEnterList\n
     * param0 is the other #P2Unit
     */
    ZFEVENT(P2UnitOnContactEnter)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this unit got #P2ContactEvent::p2_contactExitList\n
     * param0 is the other #P2Unit
     */
    ZFEVENT(P2UnitOnContactExit)

public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2World *, p2_ownerWorld)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_unitRemoveLater)

public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfstring, p2_unitId)

    /** @brief see #P2World */
    ZFPROPERTY_RETAIN(P2Body *, p2_body)
    ZFPROPERTY_ON_ATTACH_DECLARE(P2Body *, p2_body)
    ZFPROPERTY_ON_DETACH_DECLARE(P2Body *, p2_body)
    /** @brief see #P2World, must not manually modify */
    ZFPROPERTY_RETAIN_READONLY(ZFArray *, p2_partList, zfobj<ZFArray>())
    /** @brief see #P2World, must not manually modify */
    ZFPROPERTY_RETAIN_READONLY(ZFArray *, p2_jointList, zfobj<ZFArray>())

public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_part
            , ZFMP_IN(P2Body *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfindex, p2_partCount)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(P2Body *, p2_partAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Body>, p2_partRemoveAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Body>, p2_partRemove
            , ZFMP_IN(P2Body *, part)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_partRemoveAll)

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_joint
            , ZFMP_IN(P2Joint *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfindex, p2_jointCount)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(P2Joint *, p2_jointAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Joint>, p2_jointRemoveAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Joint>, p2_jointRemove
            , ZFMP_IN(P2Joint *, joint)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_jointRemoveAll)

public:
    /** @brief see #P2World, note this method is expensive */
    ZFMETHOD_DECLARE_1(P2Body *, p2_bodyFind
            , ZFMP_IN(const zfstring &, bodyId)
            )
    /** @brief see #P2World, note this method is expensive */
    ZFMETHOD_DECLARE_1(P2Shape *, p2_shapeFind
            , ZFMP_IN(const zfstring &, shapeId)
            )
    /** @brief see #P2World, note this method is expensive */
    ZFMETHOD_DECLARE_1(P2Joint *, p2_jointFind
            , ZFMP_IN(const zfstring &, jointId)
            )

public:
    /**
     * @brief see #P2World
     *
     * get a copy of all joints in owner world that referenced this unit's body or parts,
     * not including the #p2_jointList inside this unit,
     * must not manually modify,
     * valid only after added to world and first time updated
     */
    ZFMETHOD_DECLARE_0(zfautoT<ZFContainer>, p2_refJointList)

    /** @brief see #P2World::p2_UIVisibleArea */
    ZFMETHOD_DECLARE_0(zfbool, p2_unitVisible)

protected:
    zfoverride
    virtual void objectOnInit(void);
    zfoverride
    virtual void objectOnDealloc(void);
    zfoverride
    virtual void observerOnAdd(ZF_IN zfidentity eventId);
    zfoverride
    virtual void observerOnRemove(ZF_IN zfidentity eventId);

public:
    _ZFP_P2UnitPrivate *_ZFP_P2Unit_d;
};

// ============================================================
/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2UnitVisibilityEvent : zfextend ZFObject {
    ZFOBJECT_DECLARE(P2UnitVisibilityEvent, ZFObject)
public:
    /** @brief see #P2World */
    ZFCoreArray<P2Unit *> p2_unitEnterList;
    /** @brief see #P2World */
    ZFCoreArray<P2Unit *> p2_unitExitList;
};

/** @brief see #P2World */
zfclassPOD ZFLIB_ZF2DGame P2BodyMoveEventData {
public:
    /** @brief see #P2World */
    P2Body *p2_body;
    /** @brief see #P2World */
    ZFUIPoint p2_position;
    /** @brief see #P2World */
    zffloat p2_rotation;
};
ZFTYPEID_ACCESS_ONLY_DECLARE(ZFLIB_ZF2DGame, P2BodyMoveEventData, P2BodyMoveEventData)
ZFTYPEID_ACCESS_ONLY_REG(ZFLIB_ZF2DGame, P2BodyMoveEventData, P2BodyMoveEventData)
ZFOUTPUT_TYPE_DECLARE(ZFLIB_ZF2DGame, P2BodyMoveEventData)
ZFCORE_POD_DECLARE(P2BodyMoveEventData)

/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2BodyMoveEvent : zfextend ZFObject {
    ZFOBJECT_DECLARE(P2BodyMoveEvent, ZFObject)
public:
    /** @brief see #P2World */
    ZFCoreArray<P2BodyMoveEventData> p2_moveEventList;
};

/** @brief see #P2World */
zfclassPOD ZFLIB_ZF2DGame P2SensorEventData {
public:
    /** @brief may be null, see #P2Body::p2_bodyRemoveLater */
    P2Shape *p2_shape0;
    /** @brief may be null, see #P2Body::p2_bodyRemoveLater */
    P2Shape *p2_shape1;
};
ZFTYPEID_ACCESS_ONLY_DECLARE(ZFLIB_ZF2DGame, P2SensorEventData, P2SensorEventData)
ZFTYPEID_ACCESS_ONLY_REG(ZFLIB_ZF2DGame, P2SensorEventData, P2SensorEventData)
ZFOUTPUT_TYPE_DECLARE(ZFLIB_ZF2DGame, P2SensorEventData)
ZFCORE_POD_DECLARE(P2SensorEventData)

/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2SensorEvent : zfextend ZFObject {
    ZFOBJECT_DECLARE(P2SensorEvent, ZFObject)
public:
    /** @brief see #P2World */
    ZFCoreArray<P2SensorEventData> p2_sensorEnterList;
    /** @brief see #P2World, #P2Body::p2_bodyRemoveLater */
    ZFCoreArray<P2SensorEventData> p2_sensorExitList;
};

/** @brief see #P2World */
zfclassPOD ZFLIB_ZF2DGame P2ContactEventData {
public:
    /** @brief may be null, see #P2Body::p2_bodyRemoveLater */
    P2Shape *p2_shape0;
    /** @brief may be null, see #P2Body::p2_bodyRemoveLater */
    P2Shape *p2_shape1;
};
ZFTYPEID_ACCESS_ONLY_DECLARE(ZFLIB_ZF2DGame, P2ContactEventData, P2ContactEventData)
ZFTYPEID_ACCESS_ONLY_REG(ZFLIB_ZF2DGame, P2ContactEventData, P2ContactEventData)
ZFOUTPUT_TYPE_DECLARE(ZFLIB_ZF2DGame, P2ContactEventData)
ZFCORE_POD_DECLARE(P2ContactEventData)

/** @brief see #P2World */
zfclass ZFLIB_ZF2DGame P2ContactEvent : zfextend ZFObject {
    ZFOBJECT_DECLARE(P2ContactEvent, ZFObject)
public:
    /** @brief see #P2World */
    ZFCoreArray<P2ContactEventData> p2_contactEnterList;
    /** @brief see #P2World, #P2Body::p2_bodyRemoveLater */
    ZFCoreArray<P2ContactEventData> p2_contactExitList;
};

// ============================================================
/** @brief see #P2World */
zfclassNotPOD P2WorldImpl {
public:
    virtual ~P2WorldImpl(void) {}
public:
    /** @brief called when body added to world, before #bodyMoveEvent */
    virtual void bodyAdd(ZF_IN P2World *world, ZF_IN P2Body *body) zfpurevirtual;
    /** @brief called when body removed from world */
    virtual void bodyRemove(ZF_IN P2World *world, ZF_IN P2Body *body) zfpurevirtual;
    /** @brief called to update body position */
    virtual void bodyMoveEvent(ZF_IN P2World *world, ZF_IN P2BodyMoveEvent *event) zfpurevirtual;
    /**
     * @brief called when #P2World::p2_UIOffset or #P2World::p2_UIScale changed,
     *   called only once after current simulation step
     */
    virtual void UIUpdate(ZF_IN P2World *world) zfpurevirtual;
};

zfclassFwd _ZFP_P2WorldPrivate;
/**
 * @brief base physics world
 *
 * note: this module is more or less a packaging of box2d,
 * you should be familiar with box2d before using this module\n
 *
 * main classes:
 * -  #P2World : manage the physics world
 * -  #P2Unit : a group of body, to simulate player or enemy or other unit,
 *   it must have exact one main body, and optionally one or more body parts,
 *   and usually all parts should be connected by joints,
 *   explicitly or implicitly to the main body
 * -  #P2Body : base physics body, one body usually should have one or more shapes
 * -  #P2Shape : base shape
 * -  #P2Joint : joint to connect two bodies, it can:
 *   belong to #P2World (to control two logical unit in world),
 *   or belong to #P2Unit (to describe connection within one logical unit)
 *
 * you can use #P2World in two different way:
 * -  create a #P2World, then implement #P2World::p2impl,
 *   to implement render and body management
 * -  use #P2WorldView,
 *   and use any #ZFUIView type to act as #P2Body
 *   (#ZFUIView already marked as `ZFCLASS_EXTEND(ZFUIView, P2Body)`),
 *   and the #P2WorldView would help you to manage your views as #P2Body
 */
zfclass ZFLIB_ZF2DGame P2World : zfextend ZFStyle {
    ZFOBJECT_DECLARE(P2World, ZFStyle)

public:
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called before each simulation step
     */
    ZFEVENT(P2StepPrev)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called before each simulation step
     */
    ZFEVENT(P2StepPost)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child unit's visibility changed during each simulation step,
     * param0 is #P2UnitVisibilityEvent
     */
    ZFEVENT(P2UnitVisibilityEvent)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape fired sensor event during each simulation step,
     * param0 is #P2SensorEvent
     */
    ZFEVENT(P2SensorEvent)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape fired contact event during each simulation step,
     * param0 is #P2ContactEvent
     */
    ZFEVENT(P2ContactEvent)

public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_gravity, ZFUIPointCreate(0, -10))
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_gravity)

public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_UIOffset)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_UIOffset)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_UIScale, 50)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_UIScale)
    /** @brief see #P2World, extra margin to calculate proper #p2_UIVisibleArea */
    ZFPROPERTY_ASSIGN(ZFUIMargin, p2_UIVisibleAreaMargin, ZFUIMarginCreate(-1))
    /** @brief see #P2World, visible area according to #p2_UIOffset */
    ZFMETHOD_DECLARE_0(const ZFUIRect &, p2_UIVisibleArea)
    /** @brief see #P2World, visible area according to #p2_UIOffset, should be updated by #p2impl */
    ZFMETHOD_DECLARE_1(void, p2_UIVisibleArea
            , ZFMP_IN(const ZFUIRect &, v)
            )

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_start)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_stop)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfbool, p2_started)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_manualStep)

public:
    /** @brief see #P2World, must not manually modify */
    ZFPROPERTY_RETAIN_READONLY(ZFArray *, p2_unitList, zfobj<ZFArray>())
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_unit
            , ZFMP_IN(P2Unit *, unit)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_unit
            , ZFMP_IN(P2Body *, body)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfindex, p2_unitCount)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(P2Unit *, p2_unitAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Unit>, p2_unitRemoveAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Unit>, p2_unitRemove
            , ZFMP_IN(P2Unit *, unit)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_unitRemoveAll)

    /** @brief see #P2World, must not manually modify */
    ZFPROPERTY_RETAIN_READONLY(ZFArray *, p2_jointList, zfobj<ZFArray>())
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_joint
            , ZFMP_IN(P2Joint *, joint)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfindex, p2_jointCount)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(P2Joint *, p2_jointAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Joint>, p2_jointRemoveAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Joint>, p2_jointRemove
            , ZFMP_IN(P2Joint *, joint)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_jointRemoveAll)

public:
    /** @brief see #P2World, note this method is expensive */
    ZFMETHOD_DECLARE_1(P2Unit *, p2_unitFind
            , ZFMP_IN(const zfstring &, unitId)
            )
    /** @brief see #P2World, note this method is expensive */
    ZFMETHOD_DECLARE_1(P2Body *, p2_bodyFind
            , ZFMP_IN(const zfstring &, bodyId)
            )
    /** @brief see #P2World, note this method is expensive */
    ZFMETHOD_DECLARE_1(P2Shape *, p2_shapeFind
            , ZFMP_IN(const zfstring &, shapeId)
            )
    /** @brief see #P2World, note this method is expensive */
    ZFMETHOD_DECLARE_1(P2Joint *, p2_jointFind
            , ZFMP_IN(const zfstring &, jointId)
            )

public:
    /**
     * @brief see #P2World
     *
     * callback's param0 would be #P2Shape that was overlapped\n
     * you may:
     * -  set #ZFArgs::eventFiltered to true to stop test
     * -  set #ZFArgs::result, the last result would be returned as this method's result
     */
    ZFMETHOD_DECLARE_4(zfauto, p2_overlapTest
            , ZFMP_IN(const ZFUIRect &, rect)
            , ZFMP_IN(const ZFListener &, callback)
            , ZFMP_IN_OPT(zfflags, filterMask, P2FilterMaskAll())
            , ZFMP_IN_OPT(zfflags, filterCategory, P2FilterMaskAll())
            )
    /**
     * @brief see #P2World
     *
     * callback's param0 would be #P2Shape that was hit by ray test\n
     * you may:
     * -  set #ZFArgs::eventFiltered to true to stop test,
     *   and it's initialized as true,
     *   so that it's stopped for first hit
     * -  set #ZFArgs::result, the last result would be returned as this method's result
     */
    ZFMETHOD_DECLARE_5(zfauto, p2_rayTest
            , ZFMP_IN(const ZFUIPoint &, src)
            , ZFMP_IN(const ZFUIPoint &, direction)
            , ZFMP_IN(const ZFListener &, callback)
            , ZFMP_IN_OPT(zfflags, filterMask, P2FilterMaskAll())
            , ZFMP_IN_OPT(zfflags, filterCategory, P2FilterMaskAll())
            )

public:
    /** @brief the impl */
    P2WorldImpl *p2impl;

protected:
    zfoverride
    virtual void objectOnInit(void);
    zfoverride
    virtual void objectOnDealloc(void);
    zfoverride
    virtual void objectOnDeallocPrepare(void);
    zfoverride
    virtual void observerOnAdd(ZF_IN zfidentity eventId);
    zfoverride
    virtual void observerOnRemove(ZF_IN zfidentity eventId);

public:
    _ZFP_P2WorldPrivate *_ZFP_P2World_d;
};

/** @brief see #P2World */
ZFMETHOD_FUNC_DECLARE_3(ZFLIB_ZF2DGame, ZFUIPoint, P2World_toLocalPosition
        , ZFMP_IN(const ZFUIPoint &, relPosition)
        , ZFMP_IN(zffloat, relRotation)
        , ZFMP_IN(ZFUIPoint, worldPosition)
        )
/** @brief see #P2World */
ZFMETHOD_FUNC_DECLARE_3(ZFLIB_ZF2DGame, ZFUIPoint, P2World_toWorldPosition
        , ZFMP_IN(const ZFUIPoint &, relPosition)
        , ZFMP_IN(zffloat, relRotation)
        , ZFMP_IN(ZFUIPoint, localPosition)
        )

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_Phyx2D_h_

