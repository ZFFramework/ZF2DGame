#include "Phyx2D.h"
#include "../../zfsrc_ext/ZFImpl/_repo/box2d/box2d/box2d.h"
#include "ZFCore/ZFSTLWrapper/zfstlhashmap.h"
#include <cmath> // for fmodf

ZF_NAMESPACE_GLOBAL_BEGIN

ZF_STATIC_REGISTER_INIT(P2EnvSetup) {
    b2SetAssertFcn(b2fn_assert);
}
private:
    static int b2fn_assert(const char *condition, const char *fileName, int lineNumber) {
        ZFCoreCriticalMessageTrim(
                "[%s (%s)] box2d assert fail: %s"
                , fileName
                , lineNumber
                , condition
                );
        return 0;
    }
ZF_STATIC_REGISTER_END(P2EnvSetup)

// ============================================================
static zffloat b2DistanceLarge(void) {
    return 1000000 * b2GetLengthUnitsPerMeter();
}

static ZFUIPoint b2Vec2ToZF(ZF_IN const b2Vec2 &v) {
    return ZFUIPointCreate(v.x, v.y);
}
static b2Vec2 b2Vec2FromZF(ZF_IN const ZFUIPoint &v) {
    b2Vec2 ret;
    ret.x = v.x;
    ret.y = v.y;
    return ret;
}
static zffloat b2RotToZF(ZF_IN const b2Rot &v) {
    return atan2(v.s, v.c) * 180 / B2_PI;
}
static b2Rot b2RotFromZF(ZF_IN zffloat v) {
    return b2MakeRot(v * B2_PI / 180);
}
static zffloat b2RadToZF(ZF_IN float v) {
    return v * 180 / B2_PI;
}
static float b2RadFromZF(ZF_IN zffloat v) {
    return v * B2_PI / 180;
}
static b2Transform b2TransformFromZF(ZF_IN const ZFUIPoint &position, ZF_IN zffloat rotation) {
    b2Transform ret;
    ret.p = b2Vec2FromZF(position);
    ret.q = b2RotFromZF(rotation);
    return ret;
}
static ZFUIRect b2AABBToZF(ZF_IN const b2AABB &v) {
    return ZFUIRectCreate(
            v.lowerBound.x
            , v.lowerBound.y
            , v.upperBound.x - v.lowerBound.x
            , v.upperBound.y - v.lowerBound.y
            );
}
static b2AABB b2AABBFromZF(ZF_IN const ZFUIRect &v) {
    b2AABB ret;
    ret.lowerBound.x = v.x;
    ret.lowerBound.y = v.y;
    ret.upperBound.x = v.x + v.width;
    ret.upperBound.y = v.y + v.height;
    return ret;
}

// ============================================================
ZFEXPORT_VAR_READONLY_DEFINE(zfflags, P2FilterMaskAll, P2FilterMaskAll())
ZFEXPORT_VAR_READONLY_DEFINE(zfflags, P2FilterCategoryDefault, P2FilterCategoryDefault())
ZFEXPORT_VAR_READONLY_DEFINE(zfflags, P2FilterMaskDefault, P2FilterMaskDefault())

// ============================================================
static void _ZFP_P2JointAttach(ZF_IN ZFObject *jointOwner, ZF_IN P2Joint *joint);
static void _ZFP_P2JointDetach(ZF_IN P2Joint *joint);
static void _ZFP_P2BodyAttach(ZF_IN P2Unit *ownerUnit, ZF_IN P2Body *body);
static void _ZFP_P2BodyDetach(ZF_IN P2Body *body);
static void _ZFP_P2UnitAttach(ZF_IN P2World *ownerWorld, ZF_IN P2Unit *unit);
static void _ZFP_P2UnitDetach(ZF_IN P2Unit *unit);

static void _ZFP_P2BodyImplMassUpdateRequest(ZF_IN P2Body *body);
static void _ZFP_P2WorldImplStep(ZF_IN P2World *world);

zfclassNotPOD _ZFP_P2ShapePrivate {
public:
    b2ShapeId implShapeId;
public:
    _ZFP_P2ShapePrivate(void)
    : implShapeId(b2_nullShapeId)
    {
    }
public:
    static void shapeCreate(ZF_IN P2Body *ownerBody, ZF_IN P2Shape *shape) {
        shape->p2impl_shapeCreate(ownerBody);
    }
};

zfclassNotPOD _ZFP_P2JointPrivate {
public:
    b2JointId implJointId;
    ZFObject *jointOwner; // P2Unit or P2World, according to who added this joint
public:
    _ZFP_P2JointPrivate(void)
    : implJointId(b2_nullJointId)
    , jointOwner(zfnull)
    {
    }
public:
    static void jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1, ZF_IN P2Joint *joint) {
        joint->p2impl_jointCreate(ownerBody0, ownerBody1);
    }
};

zfclassNotPOD _ZFP_P2BodyPrivate {
public:
    b2BodyId implBodyId;
    P2Unit *ownerUnit;
    zfimplhashmap<P2Joint *, zfbool> bodyRefJointList;
    zfbool massNeedUpdate;
public:
    _ZFP_P2BodyPrivate(void)
    : implBodyId(b2_nullBodyId)
    , ownerUnit(zfnull)
    , bodyRefJointList()
    , massNeedUpdate(zftrue)
    {
    }
public:
    static const char *implName(ZF_IN const zfstring &name) {
        if(name) {
            if(name.length() <= 31) {
                return name.cString();
            }
            else {
                return name.cString() + (name.length() - 31);
            }
        }
        else {
            return zfnull;
        }
    }
};
zfclassNotPOD _ZFP_P2UnitPrivate {
public:
    P2World *ownerWorld;
    zfimplhashmap<P2Joint *, zfbool> unitRefJointList;
public:
    _ZFP_P2UnitPrivate(void)
    : ownerWorld(zfnull)
    , unitRefJointList()
    {
    }
};
zfclassNotPOD _ZFP_P2WorldPrivate {
public:
    b2WorldId implWorldId;
    ZFListener implTimer;
    zfimplhashmap<zfstring, P2Body *> bodyIdMap; // cache for finding
    zfimplhashmap<P2Body *, zfbool> pendingBody; // bodies needs to create or update mass
    zfimplhashmap<P2Joint *, zfbool> pendingJoint; // joints needs to create or update body
    P2BodyMoveEvent *bodyMoveEvent;
    P2ContactEvent *contactEvent;
    P2SensorEvent *sensorEvent;
    zfimplhashmap<P2Unit *, zfbool> visibleUnits;
    ZFUIRect visibleArea;
public:
    _ZFP_P2WorldPrivate(void)
    : implWorldId(b2_nullWorldId)
    , implTimer()
    , pendingBody()
    , pendingJoint()
    , bodyMoveEvent(zfobjAlloc(P2BodyMoveEvent))
    , contactEvent(zfobjAlloc(P2ContactEvent))
    , sensorEvent(zfobjAlloc(P2SensorEvent))
    , visibleUnits()
    , visibleArea(ZFUIRectZero())
    {
    }
    ~_ZFP_P2WorldPrivate(void) {
        zfobjRelease(sensorEvent);
        zfobjRelease(contactEvent);
        zfobjRelease(bodyMoveEvent);
    }
};

// ============================================================
ZFOBJECT_REGISTER(P2Shape)

ZFMETHOD_DEFINE_0(P2Shape, P2Body *, p2_ownerBody) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2BodyId implBodyId = b2Shape_GetBody(_ZFP_P2Shape_d->implShapeId);
        ZFCoreAssert(B2_IS_NON_NULL(implBodyId));
        return (P2Body *)b2Body_GetUserData(implBodyId);
    }
    else {
        return zfnull;
    }
}
ZFMETHOD_DEFINE_0(P2Shape, P2Unit *, p2_ownerUnit) {
    P2Body *ownerBody = this->p2_ownerBody();
    if(ownerBody) {
        return ownerBody->p2_ownerUnit();
    }
    else {
        return zfnull;
    }
}
ZFMETHOD_DEFINE_0(P2Shape, P2World *, p2_ownerWorld) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2WorldId implWorldId = b2Shape_GetWorld(_ZFP_P2Shape_d->implShapeId);
        ZFCoreAssert(B2_IS_NON_NULL(implWorldId));
        return (P2World *)b2World_GetUserData(implWorldId);
    }
    else {
        return zfnull;
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zffloat, p2_density) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Shape_SetDensity(_ZFP_P2Shape_d->implShapeId, propertyValue, zffalse);
        _ZFP_P2BodyImplMassUpdateRequest(this->p2_ownerBody());
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zfbool, p2_contactEnable) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Shape_EnableContactEvents(_ZFP_P2Shape_d->implShapeId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zfbool, p2_sensorEnable) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Shape_EnableSensorEvents(_ZFP_P2Shape_d->implShapeId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zfflags, p2_filterCategory) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Filter implFilter = b2Shape_GetFilter(_ZFP_P2Shape_d->implShapeId);
        implFilter.categoryBits = propertyValue;
        b2Shape_SetFilter(_ZFP_P2Shape_d->implShapeId, implFilter);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zfflags, p2_filterMask) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Filter implFilter = b2Shape_GetFilter(_ZFP_P2Shape_d->implShapeId);
        implFilter.maskBits = propertyValue;
        b2Shape_SetFilter(_ZFP_P2Shape_d->implShapeId, implFilter);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zfint, p2_filterGroup) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Filter implFilter = b2Shape_GetFilter(_ZFP_P2Shape_d->implShapeId);
        implFilter.groupIndex = propertyValue;
        b2Shape_SetFilter(_ZFP_P2Shape_d->implShapeId, implFilter);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zffloat, p2_matFriction) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2SurfaceMaterial implMat = b2Shape_GetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId);
        implMat.friction = propertyValue;
        b2Shape_SetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId, implMat);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zffloat, p2_matBounce) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2SurfaceMaterial implMat = b2Shape_GetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId);
        implMat.restitution = propertyValue;
        b2Shape_SetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId, implMat);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zffloat, p2_matRotationResist) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2SurfaceMaterial implMat = b2Shape_GetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId);
        implMat.rollingResistance = propertyValue;
        b2Shape_SetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId, implMat);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zffloat, p2_matTangentSpeed) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2SurfaceMaterial implMat = b2Shape_GetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId);
        implMat.tangentSpeed = propertyValue;
        b2Shape_SetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId, implMat);
    }
}

ZFMETHOD_DEFINE_0(P2Shape, ZFUIRect, p2_AABB) {
    if(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)) {
        return ZFUIRectZero();
    }
    return b2AABBToZF(b2Shape_GetAABB(_ZFP_P2Shape_d->implShapeId));
}

void P2Shape::objectOnInit(void) {
    zfsuper::objectOnInit();
    _ZFP_P2Shape_d = zfpoolNew(_ZFP_P2ShapePrivate);
}
void P2Shape::objectOnDealloc(void) {
    ZFCoreAssert(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId));
    zfpoolDelete(_ZFP_P2Shape_d);
    zfsuper::objectOnDealloc();
}

static void _ZFP_P2Shape_implShapeDef(ZF_IN_OUT b2ShapeDef &cfg, ZF_IN P2Shape *owner, ZF_IN P2Body *ownerBody) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(owner->_ZFP_P2Shape_d->implShapeId)
            , "shape already attached to body, shape: %s, body: %s, world: %s, adding to body: %d"
            , owner
            , owner->p2_ownerBody()
            , owner->p2_ownerWorld()
            , ownerBody
            );
    ZFCoreAssertWithMessageTrim(B2_IS_NON_NULL(ownerBody->_ZFP_P2Body_d->implBodyId)
            , "shape owner body not attached to world, shape: %s, adding to body: %s"
            , owner
            , ownerBody
            );

    cfg.userData = owner;
    cfg.material.friction = owner->p2_matFriction();
    cfg.material.restitution = owner->p2_matBounce();
    cfg.material.rollingResistance = owner->p2_matRotationResist();
    cfg.material.tangentSpeed = owner->p2_matTangentSpeed();
    cfg.density = owner->p2_density();
    cfg.filter.categoryBits = (uint64_t)owner->p2_filterCategory();
    cfg.filter.maskBits = (uint64_t)owner->p2_filterMask();
    cfg.filter.groupIndex = (int)owner->p2_filterGroup();
    cfg.isSensor = owner->p2_sensor();
    cfg.enableSensorEvents = owner->p2_sensorEnable();
    cfg.enableContactEvents = owner->p2_contactEnable();
    cfg.invokeContactCreation = zffalse;
    cfg.updateBodyMass = zffalse;
}

// ============================================================
ZFOBJECT_REGISTER(P2ShapeBox)
void P2ShapeBox::p2impl_shapeCreate(ZF_IN P2Body *ownerBody) {
    zfsuper::p2impl_shapeCreate(ownerBody);
    ZFCoreAssert(this->p2_width() > 0 && this->p2_height() > 0);

    b2ShapeDef implShapeDef = b2DefaultShapeDef();
    _ZFP_P2Shape_implShapeDef(implShapeDef, this, ownerBody);
    b2Polygon implPolygon = this->p2_position() == ZFUIPointZero() && this->p2_rotation() == 0
        ? this->p2_radius() == 0
            ? b2MakeBox(
                    this->p2_width() / 2
                    , this->p2_height() / 2
                    )
            : b2MakeRoundedBox(
                    this->p2_width() / 2
                    , this->p2_height() / 2
                    , this->p2_radius()
                    )
        : this->p2_radius() == 0
            ? b2MakeOffsetBox(
                    this->p2_width() / 2
                    , this->p2_height() / 2
                    , b2Vec2FromZF(this->p2_position())
                    , b2RotFromZF(this->p2_rotation())
                    )
            : b2MakeOffsetRoundedBox(
                    this->p2_width() / 2
                    , this->p2_height() / 2
                    , b2Vec2FromZF(this->p2_position())
                    , b2RotFromZF(this->p2_rotation())
                    , this->p2_radius()
                    )
        ;
    _ZFP_P2Shape_d->implShapeId = b2CreatePolygonShape(
            ownerBody->_ZFP_P2Body_d->implBodyId
            , &implShapeDef
            , &implPolygon
            );
}

ZFOBJECT_REGISTER(P2ShapeCircle)
void P2ShapeCircle::p2impl_shapeCreate(ZF_IN P2Body *ownerBody) {
    zfsuper::p2impl_shapeCreate(ownerBody);
    ZFCoreAssert(this->p2_radius() > 0);

    b2ShapeDef implShapeDef = b2DefaultShapeDef();
    _ZFP_P2Shape_implShapeDef(implShapeDef, this, ownerBody);
    b2Circle implCircle;
    implCircle.center = b2Vec2FromZF(this->p2_position());
    implCircle.radius = this->p2_radius();
    _ZFP_P2Shape_d->implShapeId = b2CreateCircleShape(
            ownerBody->_ZFP_P2Body_d->implBodyId
            , &implShapeDef
            , &implCircle
            );
}

ZFOBJECT_REGISTER(P2ShapeCapsule)
void P2ShapeCapsule::p2impl_shapeCreate(ZF_IN P2Body *ownerBody) {
    zfsuper::p2impl_shapeCreate(ownerBody);
    ZFCoreAssert(this->p2_radius() > 0);

    b2ShapeDef implShapeDef = b2DefaultShapeDef();
    _ZFP_P2Shape_implShapeDef(implShapeDef, this, ownerBody);
    if(this->p2_position0() == this->p2_position1()) {
        b2Circle implCircle;
        implCircle.center = b2Vec2FromZF(this->p2_position0());
        implCircle.radius = this->p2_radius();
        _ZFP_P2Shape_d->implShapeId = b2CreateCircleShape(
                ownerBody->_ZFP_P2Body_d->implBodyId
                , &implShapeDef
                , &implCircle
                );
    }
    else {
        b2Capsule implCapsule;
        implCapsule.center1 = b2Vec2FromZF(this->p2_position0());
        implCapsule.center2 = b2Vec2FromZF(this->p2_position1());
        implCapsule.radius = this->p2_radius();
        _ZFP_P2Shape_d->implShapeId = b2CreateCapsuleShape(
                ownerBody->_ZFP_P2Body_d->implBodyId
                , &implShapeDef
                , &implCapsule
                );
    }
}

ZFOBJECT_REGISTER(P2ShapePolygon)
ZFMETHOD_DEFINE_4(P2ShapePolygon, void, p2_polygon
        , ZFMP_IN(const ZFCoreArray<ZFUIPoint> &, points)
        , ZFMP_IN_OPT(const ZFUIPoint &, position, ZFUIPointZero())
        , ZFMP_IN_OPT(zffloat, rotation, 0)
        , ZFMP_IN_OPT(zffloat, radius, 0)
        ) {
    ZFCoreAssert(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId));
    ZFCoreAssert(!points.isEmpty());
    this->p2_polygon_points(points);
    this->p2_polygon_position(position);
    this->p2_polygon_rotation(rotation);
    this->p2_polygon_radius(radius);
}
void P2ShapePolygon::p2impl_shapeCreate(ZF_IN P2Body *ownerBody) {
    zfsuper::p2impl_shapeCreate(ownerBody);
    ZFCoreAssert(!this->p2_polygon_points().isEmpty());

    b2ShapeDef implShapeDef = b2DefaultShapeDef();
    _ZFP_P2Shape_implShapeDef(implShapeDef, this, ownerBody);
    b2Hull implHull = b2ComputeHull((const b2Vec2 *)(this->p2_polygon_points().arrayBuf()), (int)this->p2_polygon_points().count());
    ZFCoreAssertWithMessageTrim(implHull.count > 0
            , "invalid polygon points, shape: %s"
            , this->p2_polygon_points().objectInfoOfContent()
            , this
            );
    b2Polygon implPolygon = this->p2_polygon_position() == ZFUIPointZero() && this->p2_polygon_rotation() == 0
        ? b2MakePolygon(
                &implHull
                , this->p2_polygon_radius()
                )
        : b2MakeOffsetRoundedPolygon(
                &implHull
                , b2Vec2FromZF(this->p2_polygon_position())
                , b2RotFromZF(this->p2_polygon_rotation())
                , this->p2_polygon_radius()
                )
        ;
    _ZFP_P2Shape_d->implShapeId = b2CreatePolygonShape(
            ownerBody->_ZFP_P2Body_d->implBodyId
            , &implShapeDef
            , &implPolygon
            );
}

// ============================================================
ZFOBJECT_REGISTER(P2Joint)

ZFMETHOD_DEFINE_0(P2Joint, P2Body *, p2_ownerBody0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2BodyId implBodyId = b2Joint_GetBodyA(_ZFP_P2Joint_d->implJointId);
        ZFCoreAssert(B2_IS_NON_NULL(implBodyId));
        return (P2Body *)b2Body_GetUserData(implBodyId);
    }
    else {
        return zfnull;
    }
}
ZFMETHOD_DEFINE_0(P2Joint, P2Body *, p2_ownerBody1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2BodyId implBodyId = b2Joint_GetBodyB(_ZFP_P2Joint_d->implJointId);
        ZFCoreAssert(B2_IS_NON_NULL(implBodyId));
        return (P2Body *)b2Body_GetUserData(implBodyId);
    }
    else {
        return zfnull;
    }
}
ZFMETHOD_DEFINE_0(P2Joint, P2World *, p2_ownerWorld) {
    if(_ZFP_P2Joint_d->jointOwner) {
        if(_ZFP_P2Joint_d->jointOwner->classData()->classIsTypeOf(P2Unit::ClassData())) {
            return zfcast(P2Unit *, _ZFP_P2Joint_d->jointOwner)->p2_ownerWorld();
        }
        else {
            return zfcast(P2World *, _ZFP_P2Joint_d->jointOwner);
        }
    }
    else {
        return zfnull;
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2Joint, zfbool, p2_collideEnable) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetCollideConnected(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}

void P2Joint::objectOnInit(void) {
    zfsuper::objectOnInit();
    _ZFP_P2Joint_d = zfpoolNew(_ZFP_P2JointPrivate);
}
void P2Joint::objectOnDealloc(void) {
    ZFCoreAssert(B2_IS_NULL(_ZFP_P2Joint_d->implJointId));
    zfpoolDelete(_ZFP_P2Joint_d);
    zfsuper::objectOnDealloc();
}

static void _ZFP_P2Joint_implJointDef(ZF_IN P2Joint *owner, ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(owner->_ZFP_P2Joint_d->implJointId)
            , "joint already attached to body, joint: %s, body0: %s, body1: %s, world: %s, adding to body0: %d, body1: %s"
            , owner
            , owner->p2_ownerBody0()
            , owner->p2_ownerBody1()
            , owner->p2_ownerWorld()
            , ownerBody0
            , ownerBody1
            );
    ZFCoreAssertWithMessageTrim(B2_IS_NON_NULL(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , "joint ownerBody0 not attached to world, joint: %s, body: %s"
            , owner
            , ownerBody0
            );
    ZFCoreAssertWithMessageTrim(B2_IS_NON_NULL(ownerBody1->_ZFP_P2Body_d->implBodyId)
            , "joint ownerBody1 not attached to world, joint: %s, body: %s"
            , owner
            , ownerBody1
            );
}

ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringHertzSoft, 4)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringDampingSoft, 0.5f)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringHertzNormal, 8)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringDampingNormal, 0.7f)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringHertzHard, 12)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringDampingHard, 0.9f)

ZFOBJECT_REGISTER(P2JointSpring)

// ============================================================
ZFOBJECT_REGISTER(P2JointDistance)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, ZFUIPoint, p2_anchor0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, ZFUIPoint, p2_anchor1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_springHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2DistanceJoint_SetSpringHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2DistanceJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_springDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2DistanceJoint_SetSpringDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2DistanceJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_motorSpeed) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2DistanceJoint_SetMotorSpeed(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2DistanceJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_motorForce) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2DistanceJoint_SetMaxMotorForce(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2DistanceJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_distance) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2DistanceJoint_SetLength(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFMETHOD_DEFINE_0(P2JointDistance, zffloat, p2_distanceCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        return b2DistanceJoint_GetCurrentLength(_ZFP_P2Joint_d->implJointId);
    }
    else {
        return this->p2_distance();
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_distanceLimitMin) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() < 0 || this->p2_distanceLimitMax() > 0) {
            b2DistanceJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            b2DistanceJoint_SetLengthRange(_ZFP_P2Joint_d->implJointId
                    , this->p2_distanceLimitMin() < 0 ? (float)(this->p2_distance() + this->p2_distanceLimitMin()) : (float)0
                    , this->p2_distanceLimitMax() > 0 ? (float)(this->p2_distance() + this->p2_distanceLimitMax()) : (float)b2DistanceLarge()
                    );
        }
        else {
            b2DistanceJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_distanceLimitMax) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() < 0 || this->p2_distanceLimitMax() > 0) {
            b2DistanceJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            b2DistanceJoint_SetLengthRange(_ZFP_P2Joint_d->implJointId
                    , this->p2_distanceLimitMin() < 0 ? (float)(this->p2_distance() + this->p2_distanceLimitMin()) : (float)0
                    , this->p2_distanceLimitMax() > 0 ? (float)(this->p2_distance() + this->p2_distanceLimitMax()) : (float)b2DistanceLarge()
                    );
        }
        else {
            b2DistanceJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}

void P2JointDistance::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);

    b2DistanceJointDef implJointDef = b2DefaultDistanceJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_collideEnable();
    implJointDef.localAnchorA = b2Vec2FromZF(this->p2_anchor0());
    implJointDef.localAnchorB = b2Vec2FromZF(this->p2_anchor1());
    if(this->p2_springHertz() > 0 && this->p2_springDamping() > 0) {
        implJointDef.enableSpring = zftrue;
        implJointDef.hertz = this->p2_springHertz();
        implJointDef.dampingRatio = this->p2_springDamping();
    }
    else {
        implJointDef.enableSpring = zffalse;
    }
    if(this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0) {
        implJointDef.enableMotor = zftrue;
        implJointDef.motorSpeed = this->p2_motorSpeed();
        implJointDef.maxMotorForce = this->p2_motorForce();
    }
    else {
        implJointDef.enableMotor = zffalse;
    }

    implJointDef.length = this->p2_distance();
    if(this->p2_distanceLimitMin() < 0 || this->p2_distanceLimitMax() > 0) {
        implJointDef.enableLimit = zftrue;
        if(this->p2_distanceLimitMin() < 0) {
            implJointDef.minLength = this->p2_distance() + this->p2_distanceLimitMin();
            if(implJointDef.minLength < 0) {
                implJointDef.minLength = 0;
            }
        }
        else {
            implJointDef.minLength = 0;
        }
        if(this->p2_distanceLimitMax() > 0) {
            implJointDef.maxLength = this->p2_distance() + this->p2_distanceLimitMax();
        }
        else {
            implJointDef.maxLength = b2DistanceLarge();
        }
    }
    else {
        implJointDef.enableLimit = zffalse;
    }
    _ZFP_P2Joint_d->implJointId = b2CreateDistanceJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFOBJECT_REGISTER(P2JointRevolute)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, ZFUIPoint, p2_anchor0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, ZFUIPoint, p2_anchor1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_springHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2RevoluteJoint_SetSpringHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2RevoluteJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_springDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2RevoluteJoint_SetSpringDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2RevoluteJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_motorSpeed) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2RevoluteJoint_SetMotorSpeed(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2RevoluteJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_motorForce) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2RevoluteJoint_SetMaxMotorTorque(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2RevoluteJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_angularRef) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetReferenceAngle(_ZFP_P2Joint_d->implJointId, b2RadFromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_angular) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2RevoluteJoint_SetTargetAngle(_ZFP_P2Joint_d->implJointId, b2RadFromZF(propertyValue));
    }
}
ZFMETHOD_DEFINE_0(P2JointRevolute, zffloat, p2_angularCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        return b2RadToZF(b2RevoluteJoint_GetAngle(_ZFP_P2Joint_d->implJointId));
    }
    else {
        return this->p2_angular();
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_angularLimitMin) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_angularLimitMin() < this->p2_angularLimitMax()) {
            b2RevoluteJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            b2RevoluteJoint_SetLimits(_ZFP_P2Joint_d->implJointId, b2RadFromZF(this->p2_angularLimitMin()), b2RadFromZF(this->p2_angularLimitMax()));
        }
        else {
            b2RevoluteJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_angularLimitMax) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_angularLimitMin() < this->p2_angularLimitMax()) {
            b2RevoluteJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            b2RevoluteJoint_SetLimits(_ZFP_P2Joint_d->implJointId, b2RadFromZF(this->p2_angularLimitMin()), b2RadFromZF(this->p2_angularLimitMax()));
        }
        else {
            b2RevoluteJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}

void P2JointRevolute::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);

    b2RevoluteJointDef implJointDef = b2DefaultRevoluteJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_collideEnable();
    implJointDef.localAnchorA = b2Vec2FromZF(this->p2_anchor0());
    implJointDef.localAnchorB = b2Vec2FromZF(this->p2_anchor1());
    if(this->p2_springHertz() > 0 && this->p2_springDamping() > 0) {
        implJointDef.enableSpring = zftrue;
        implJointDef.hertz = this->p2_springHertz();
        implJointDef.dampingRatio = this->p2_springDamping();
    }
    else {
        implJointDef.enableSpring = zffalse;
    }
    if(this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0) {
        implJointDef.enableMotor = zftrue;
        implJointDef.motorSpeed = this->p2_motorSpeed();
        implJointDef.maxMotorTorque = this->p2_motorForce();
    }
    else {
        implJointDef.enableMotor = zffalse;
    }

    implJointDef.referenceAngle = b2RadFromZF(this->p2_angularRef());
    implJointDef.targetAngle = b2RadFromZF(this->p2_angular());
    if(this->p2_angularLimitMin() < this->p2_angularLimitMax()) {
        implJointDef.enableLimit = zftrue;
        implJointDef.lowerAngle = b2RadFromZF(this->p2_angularLimitMin());
        implJointDef.upperAngle = b2RadFromZF(this->p2_angularLimitMax());
    }
    else {
        implJointDef.enableLimit = zffalse;
    }
    _ZFP_P2Joint_d->implJointId = b2CreateRevoluteJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFOBJECT_REGISTER(P2JointPrismatic)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, ZFUIPoint, p2_anchor0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, ZFUIPoint, p2_anchor1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_springHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2PrismaticJoint_SetSpringHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2PrismaticJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_springDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2PrismaticJoint_SetSpringDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2PrismaticJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_motorSpeed) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2PrismaticJoint_SetMotorSpeed(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2PrismaticJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_motorForce) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2PrismaticJoint_SetMaxMotorForce(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2PrismaticJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, ZFUIPoint, p2_axis) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAxisA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_angularRef) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetReferenceAngle(_ZFP_P2Joint_d->implJointId, b2RadFromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_distance) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2PrismaticJoint_SetTargetTranslation(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFMETHOD_DEFINE_0(P2JointPrismatic, zffloat, p2_distanceCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        return b2PrismaticJoint_GetTranslation(_ZFP_P2Joint_d->implJointId);
    }
    else {
        return this->p2_distance();
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_distanceLimitMin) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() < this->p2_distanceLimitMax()) {
            b2PrismaticJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            b2PrismaticJoint_SetLimits(_ZFP_P2Joint_d->implJointId, this->p2_distanceLimitMin(), this->p2_distanceLimitMax());
        }
        else {
            b2PrismaticJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_distanceLimitMax) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() < this->p2_distanceLimitMax()) {
            b2PrismaticJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            b2PrismaticJoint_SetLimits(_ZFP_P2Joint_d->implJointId, this->p2_distanceLimitMin(), this->p2_distanceLimitMax());
        }
        else {
            b2PrismaticJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}

void P2JointPrismatic::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);

    b2PrismaticJointDef implJointDef = b2DefaultPrismaticJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_collideEnable();
    implJointDef.localAnchorA = b2Vec2FromZF(this->p2_anchor0());
    implJointDef.localAnchorB = b2Vec2FromZF(this->p2_anchor1());
    if(this->p2_springHertz() > 0 && this->p2_springDamping() > 0) {
        implJointDef.enableSpring = zftrue;
        implJointDef.hertz = this->p2_springHertz();
        implJointDef.dampingRatio = this->p2_springDamping();
    }
    else {
        implJointDef.enableSpring = zffalse;
    }
    if(this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0) {
        implJointDef.enableMotor = zftrue;
        implJointDef.motorSpeed = this->p2_motorSpeed();
        implJointDef.maxMotorForce = this->p2_motorForce();
    }
    else {
        implJointDef.enableMotor = zffalse;
    }

    implJointDef.localAxisA = b2Vec2FromZF(this->p2_axis());
    implJointDef.referenceAngle = b2RadFromZF(this->p2_angularRef());
    implJointDef.targetTranslation = this->p2_distance();
    if(this->p2_distanceLimitMin() < this->p2_distanceLimitMax()) {
        implJointDef.enableLimit = zftrue;
        implJointDef.lowerTranslation = this->p2_distanceLimitMin();
        implJointDef.upperTranslation = this->p2_distanceLimitMax();
    }
    else {
        implJointDef.enableLimit = zffalse;
    }
    _ZFP_P2Joint_d->implJointId = b2CreatePrismaticJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFOBJECT_REGISTER(P2JointWheel)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, ZFUIPoint, p2_anchor0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, ZFUIPoint, p2_anchor1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_springHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WheelJoint_SetSpringHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2WheelJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_springDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WheelJoint_SetSpringDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2WheelJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_motorSpeed) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WheelJoint_SetMotorSpeed(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2WheelJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_motorForce) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WheelJoint_SetMaxMotorTorque(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2WheelJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, ZFUIPoint, p2_axis) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAxisA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_distanceLimitMin) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() < this->p2_distanceLimitMax()) {
            b2WheelJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            b2WheelJoint_SetLimits(_ZFP_P2Joint_d->implJointId, this->p2_distanceLimitMin(), this->p2_distanceLimitMax());
        }
        else {
            b2WheelJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_distanceLimitMax) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() < this->p2_distanceLimitMax()) {
            b2WheelJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            b2WheelJoint_SetLimits(_ZFP_P2Joint_d->implJointId, this->p2_distanceLimitMin(), this->p2_distanceLimitMax());
        }
        else {
            b2WheelJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
        }
    }
}

void P2JointWheel::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);

    b2WheelJointDef implJointDef = b2DefaultWheelJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_collideEnable();
    implJointDef.localAnchorA = b2Vec2FromZF(this->p2_anchor0());
    implJointDef.localAnchorB = b2Vec2FromZF(this->p2_anchor1());
    if(this->p2_springHertz() > 0 && this->p2_springDamping() > 0) {
        implJointDef.enableSpring = zftrue;
        implJointDef.hertz = this->p2_springHertz();
        implJointDef.dampingRatio = this->p2_springDamping();
    }
    else {
        implJointDef.enableSpring = zffalse;
    }
    if(this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0) {
        implJointDef.enableMotor = zftrue;
        implJointDef.motorSpeed = this->p2_motorSpeed();
        implJointDef.maxMotorTorque = this->p2_motorForce();
    }
    else {
        implJointDef.enableMotor = zffalse;
    }

    implJointDef.localAxisA = b2Vec2FromZF(this->p2_axis());
    if(this->p2_distanceLimitMin() < this->p2_distanceLimitMax()) {
        implJointDef.enableLimit = zftrue;
        implJointDef.lowerTranslation = this->p2_distanceLimitMin();
        implJointDef.upperTranslation = this->p2_distanceLimitMax();
    }
    else {
        implJointDef.enableLimit = zffalse;
    }
    _ZFP_P2Joint_d->implJointId = b2CreateWheelJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFEXPORT_VAR_DEFINE(zffloat, P2JointMotorFactorSoft, 0.2f)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointMotorFactorNormal, 0.3f)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointMotorFactorHard, 0.5f)

ZFOBJECT_REGISTER(P2JointMotor)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMotor, zffloat, p2_motorFactor) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2MotorJoint_SetCorrectionFactor(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMotor, zffloat, p2_motorForce) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2MotorJoint_SetMaxForce(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMotor, zffloat, p2_motorTorque) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2MotorJoint_SetMaxTorque(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMotor, ZFUIPoint, p2_distance) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2MotorJoint_SetLinearOffset(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFMETHOD_DEFINE_0(P2JointMotor, ZFUIPoint, p2_distanceCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        return b2Vec2ToZF(b2MotorJoint_GetLinearOffset(_ZFP_P2Joint_d->implJointId));
    }
    else {
        return this->p2_distance();
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMotor, zffloat, p2_angular) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2MotorJoint_SetAngularOffset(_ZFP_P2Joint_d->implJointId, b2RadFromZF(propertyValue));
    }
}
ZFMETHOD_DEFINE_0(P2JointMotor, zffloat, p2_angularCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        return b2RadToZF(b2MotorJoint_GetAngularOffset(_ZFP_P2Joint_d->implJointId));
    }
    else {
        return this->p2_angular();
    }
}

void P2JointMotor::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);

    b2MotorJointDef implJointDef = b2DefaultMotorJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_collideEnable();

    implJointDef.correctionFactor = this->p2_motorFactor();
    implJointDef.maxForce = this->p2_motorForce();
    implJointDef.maxTorque = this->p2_motorTorque();
    implJointDef.linearOffset = b2Vec2FromZF(this->p2_distance());
    implJointDef.angularOffset = b2RadFromZF(this->p2_angular());
    _ZFP_P2Joint_d->implJointId = b2CreateMotorJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFOBJECT_REGISTER(P2JointWeld)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, ZFUIPoint, p2_anchor0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, ZFUIPoint, p2_anchor1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, zffloat, p2_angularRef) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetReferenceAngle(_ZFP_P2Joint_d->implJointId, b2RadFromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, zffloat, p2_distanceHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WeldJoint_SetLinearHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, zffloat, p2_distanceDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WeldJoint_SetLinearDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, zffloat, p2_angularHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WeldJoint_SetAngularHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, zffloat, p2_angularDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WeldJoint_SetAngularDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}

void P2JointWeld::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);

    b2WeldJointDef implJointDef = b2DefaultWeldJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_collideEnable();

    implJointDef.localAnchorA = b2Vec2FromZF(this->p2_anchor0());
    implJointDef.localAnchorB = b2Vec2FromZF(this->p2_anchor1());
    implJointDef.referenceAngle = b2RadFromZF(this->p2_angularRef());
    implJointDef.linearHertz = this->p2_distanceHertz();
    implJointDef.linearDampingRatio = this->p2_distanceDamping();
    implJointDef.angularHertz = this->p2_angularHertz();
    implJointDef.angularDampingRatio = this->p2_angularDamping();
    _ZFP_P2Joint_d->implJointId = b2CreateWeldJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFOBJECT_REGISTER(P2JointFilter)
void P2JointFilter::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);

    b2FilterJointDef implJointDef = b2DefaultFilterJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    _ZFP_P2Joint_d->implJointId = b2CreateFilterJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

// ============================================================
ZFOBJECT_REGISTER(P2Body)

ZFMETHOD_DEFINE_0(P2Body, P2Unit *, p2_ownerUnit) {
    return _ZFP_P2Body_d->ownerUnit;
}
ZFMETHOD_DEFINE_0(P2Body, P2World *, p2_ownerWorld) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2WorldId implWorldId = b2Body_GetWorld(_ZFP_P2Body_d->implBodyId);
        ZFCoreAssert(B2_IS_NON_NULL(implWorldId));
        return (P2World *)(b2World_GetUserData(implWorldId));
    }
    else {
        return zfnull;
    }
}

ZFMETHOD_DEFINE_1(P2Body, void, p2_shape
        , ZFMP_IN(P2Shape *, shape)
        ) {
    ZFCoreAssert(shape != zfnull && shape->p2_ownerBody() == zfnull);
    this->p2_shapeList()->add(shape);
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        _ZFP_P2ShapePrivate::shapeCreate(this, shape);
        _ZFP_P2BodyImplMassUpdateRequest(this);
    }
}
ZFMETHOD_DEFINE_0(P2Body, zfindex, p2_shapeCount) {
    return this->p2_shapeList()->count();
}
ZFMETHOD_DEFINE_1(P2Body, P2Shape *, p2_shapeAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->p2_shapeList()->get(index);
}
static void _ZFP_P2Body_shapeDetach(ZF_IN P2Body *owner, ZF_IN P2Shape *item) {
    if(B2_IS_NON_NULL(item->_ZFP_P2Shape_d->implShapeId)) {
        b2DestroyShape(item->_ZFP_P2Shape_d->implShapeId, zffalse);
        item->_ZFP_P2Shape_d->implShapeId = b2_nullShapeId;
        _ZFP_P2BodyImplMassUpdateRequest(owner);
    }
}
ZFMETHOD_DEFINE_1(P2Body, zfautoT<P2Shape>, p2_shapeRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2Shape> item = this->p2_shapeList()->removeAndGet(index);
    _ZFP_P2Body_shapeDetach(this, item);
    return item;
}
ZFMETHOD_DEFINE_1(P2Body, zfautoT<P2Shape>, p2_shapeRemove
        , ZFMP_IN(P2Shape *, shape)
        ) {
    zfindex index = this->p2_shapeList()->find(shape);
    if(index == zfindexMax()) {
        return zfnull;
    }
    zfautoT<P2Shape> item = this->p2_shapeList()->removeAndGet(index);
    _ZFP_P2Body_shapeDetach(this, item);
    return item;
}
ZFMETHOD_DEFINE_0(P2Body, void, p2_shapeRemoveAll) {
    ZFArray *l = this->p2_shapeList();
    for(zfindex i = 0; i < l->count(); ++i) {
        P2Shape *item = l->get(i);
        _ZFP_P2Body_shapeDetach(this, item);
    }
    l->removeAll();
}

ZFMETHOD_DEFINE_1(P2Body, P2Shape *, p2_shapeFind
        , ZFMP_IN(const zfstring &, shapeId)
        ) {
    if(shapeId) {
        ZFArray *shapeList = this->p2_shapeList();
        for(zfindex i = 0; i < shapeList->count(); ++i) {
            P2Shape *item = shapeList->get(i);
            if(item->p2_shapeId() == shapeId) {
                return item;
            }
        }
    }
    return zfnull;
}
ZFMETHOD_DEFINE_1(P2Body, P2Joint *, p2_jointFind
        , ZFMP_IN(const zfstring &, jointId)
        ) {
    if(jointId) {
        for(zfimplhashmap<P2Joint *, zfbool>::iterator it = _ZFP_P2Body_d->bodyRefJointList.begin(); it != _ZFP_P2Body_d->bodyRefJointList.end(); ++it) {
            P2Joint *item = it->first;
            if(item->p2_jointId() == jointId) {
                return item;
            }
        }
    }
    return zfnull;
}

ZFMETHOD_DEFINE_0(P2Body, zfautoT<ZFContainer>, p2_refJointList) {
    zfobj<ZFHashSet> ret;
    for(zfimplhashmap<P2Joint *, zfbool>::iterator it = _ZFP_P2Body_d->bodyRefJointList.begin(); it != _ZFP_P2Body_d->bodyRefJointList.end(); ++it) {
        ret->add(it->first);
    }
    return ret;
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zfstring, p2_bodyId) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Body_d->implBodyId)
            , "not allowed to change bodyId after adding to world, body: %s, new bodyId: %s"
            , this
            , propertyValue
            );
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, P2BodyType, p2_type) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2BodyType implType;
        switch(propertyValue) {
            case v_P2BodyType::e_Static:
                implType = b2_staticBody;
                break;
            case v_P2BodyType::e_Kinematic:
                implType = b2_kinematicBody;
                break;
            case v_P2BodyType::e_Dynamic:
                implType = b2_dynamicBody;
                break;
            default:
                ZFCoreCriticalShouldNotGoHere();
                return;
        }
        b2Body_SetType(_ZFP_P2Body_d->implBodyId, implType);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zfbool, p2_enable) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        if(propertyValue) {
            b2Body_Enable(_ZFP_P2Body_d->implBodyId);
        }
        else {
            b2Body_Disable(_ZFP_P2Body_d->implBodyId);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zfbool, p2_sleepEnable) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_EnableSleep(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zfbool, p2_bullet) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetBullet(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zfbool, p2_rotationFixed) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetFixedRotation(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zffloat, p2_gravityScale) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetGravityScale(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, ZFUIPoint, p2_position) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetTransform(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(propertyValue), b2Body_GetTransform(_ZFP_P2Body_d->implBodyId).q);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, ZFUIPoint, p2_positionVelocity) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetLinearVelocity(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zffloat, p2_positionDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetLinearDamping(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zffloat, p2_rotation) {
    propertyValue = fmodf(fmodf(propertyValue, 360.0f) + 360.0f, 360.0f);

    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetTransform(_ZFP_P2Body_d->implBodyId, b2Body_GetTransform(_ZFP_P2Body_d->implBodyId).p, b2RotFromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zffloat, p2_rotationVelocity) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetAngularVelocity(_ZFP_P2Body_d->implBodyId, b2RadFromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zffloat, p2_rotationDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetAngularDamping(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}

ZFMETHOD_DEFINE_0(P2Body, ZFUIRect, p2_AABB) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2AABBToZF(b2Body_ComputeAABB(_ZFP_P2Body_d->implBodyId));
    }
    else {
        return ZFUIRectZero();
    }
}
ZFMETHOD_DEFINE_0(P2Body, zffloat, p2_mass) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2Body_GetMass(_ZFP_P2Body_d->implBodyId);
    }
    else {
        return 0;
    }
}
ZFMETHOD_DEFINE_0(P2Body, ZFUIPoint, p2_centerOfMass) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2Vec2ToZF(b2Body_GetLocalCenterOfMass(_ZFP_P2Body_d->implBodyId));
    }
    else {
        return ZFUIPointZero();
    }
}
ZFMETHOD_DEFINE_0(P2Body, zffloat, p2_rotationInertia) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2Body_GetRotationalInertia(_ZFP_P2Body_d->implBodyId);
    }
    else {
        return 0;
    }
}
ZFMETHOD_DEFINE_0(P2Body, ZFUIPoint, p2_positionCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2Vec2ToZF(b2Body_GetPosition(_ZFP_P2Body_d->implBodyId));
    }
    else {
        return ZFUIPointZero();
    }
}
ZFMETHOD_DEFINE_0(P2Body, ZFUIPoint, p2_positionVelocityCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2Vec2ToZF(b2Body_GetLinearVelocity(_ZFP_P2Body_d->implBodyId));
    }
    else {
        return ZFUIPointZero();
    }
}
ZFMETHOD_DEFINE_0(P2Body, zffloat, p2_rotationCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2RotToZF(b2Body_GetRotation(_ZFP_P2Body_d->implBodyId));
    }
    else {
        return 0;
    }
}
ZFMETHOD_DEFINE_0(P2Body, zffloat, p2_rotationVelocityCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2RadToZF(b2Body_GetAngularVelocity(_ZFP_P2Body_d->implBodyId));
    }
    else {
        return 0;
    }
}

ZFMETHOD_DEFINE_0(P2Body, zfbool, sleeping) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return !b2Body_IsAwake(_ZFP_P2Body_d->implBodyId);
    }
    else {
        return zffalse;
    }
}
ZFMETHOD_DEFINE_1(P2Body, void, sleeping
        , ZFMP_IN(zfbool, v)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetAwake(_ZFP_P2Body_d->implBodyId, !v);
    }
}
ZFMETHOD_DEFINE_3(P2Body, void, p2_moveTo
        , ZFMP_IN(zftimet, duration)
        , ZFMP_IN(const ZFUIPoint &, position)
        , ZFMP_IN_OPT(zffloat, rotation, -1)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Transform implTransform;
        implTransform.p = b2Vec2FromZF(position);
        if(rotation >= 0) {
            implTransform.q = b2RotFromZF(rotation);
        }
        else {
            implTransform.q = b2Body_GetRotation(_ZFP_P2Body_d->implBodyId);
        }
        b2Body_SetTargetTransform(_ZFP_P2Body_d->implBodyId, implTransform, duration);
    }
}

ZFMETHOD_DEFINE_3(P2Body, void, p2_applyPositionForceInWorld
        , ZFMP_IN(const ZFUIPoint &, force)
        , ZFMP_IN(const ZFUIPoint &, worldPosition)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyForce(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(force), b2Vec2FromZF(worldPosition), wakeup);
    }
}
ZFMETHOD_DEFINE_2(P2Body, void, p2_applyPositionForce
        , ZFMP_IN(const ZFUIPoint &, force)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyForceToCenter(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(force), wakeup);
    }
}
ZFMETHOD_DEFINE_2(P2Body, void, p2_applyRotationForce
        , ZFMP_IN(zffloat, force)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyTorque(_ZFP_P2Body_d->implBodyId, force, wakeup);
    }
}

ZFMETHOD_DEFINE_3(P2Body, void, p2_applyPositionImpulseInWorld
        , ZFMP_IN(const ZFUIPoint &, impulse)
        , ZFMP_IN(const ZFUIPoint &, worldPosition)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyLinearImpulse(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(impulse), b2Vec2FromZF(worldPosition), wakeup);
    }
}
ZFMETHOD_DEFINE_2(P2Body, void, p2_applyPositionImpulse
        , ZFMP_IN(const ZFUIPoint &, impulse)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyLinearImpulseToCenter(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(impulse), wakeup);
    }
}
ZFMETHOD_DEFINE_2(P2Body, void, p2_applyRotationImpulse
        , ZFMP_IN(zffloat, impulse)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyTorque(_ZFP_P2Body_d->implBodyId, impulse, wakeup);
    }
}

void P2Body::objectOnInit(void) {
    zfsuper::objectOnInit();
    _ZFP_P2Body_d = zfpoolNew(_ZFP_P2BodyPrivate);
}
void P2Body::objectOnDealloc(void) {
    ZFCoreAssert(B2_IS_NULL(_ZFP_P2Body_d->implBodyId));
    zfpoolDelete(_ZFP_P2Body_d);
    zfsuper::objectOnDealloc();
}

// ============================================================
ZFOBJECT_REGISTER(P2Unit)

ZFMETHOD_DEFINE_0(P2Unit, P2World *, p2_ownerWorld) {
    return _ZFP_P2Unit_d->ownerWorld;
}

ZFPROPERTY_ON_ATTACH_DEFINE(P2Unit, P2Body *, p2_body) {
    ZFCoreAssertWithMessageTrim(_ZFP_P2Unit_d->ownerWorld == zfnull
            , "not allowed to change body of unit after added to world, unit: %s, body: %s"
            , this
            , propertyValue
            );
    if(propertyValue != zfnull) {
        _ZFP_P2BodyAttach(this, propertyValue);
    }
    else {
        _ZFP_P2BodyDetach(propertyValue);
    }
}
ZFPROPERTY_ON_DETACH_DEFINE(P2Unit, P2Body *, p2_body) {
    _ZFP_P2BodyDetach(propertyValue);
}

ZFMETHOD_DEFINE_1(P2Unit, void, p2_part
        , ZFMP_IN(P2Body *, v)
        ) {
    _ZFP_P2BodyAttach(this, v);
    this->p2_partList()->add(v);
}
ZFMETHOD_DEFINE_0(P2Unit, zfindex, p2_partCount) {
    return this->p2_partList()->count();
}
ZFMETHOD_DEFINE_1(P2Unit, P2Body *, p2_partAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->p2_partList()->get(index);
}
ZFMETHOD_DEFINE_1(P2Unit, zfautoT<P2Body>, p2_partRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2Body> item = this->p2_partList()->removeAndGet(index);
    _ZFP_P2BodyDetach(item);
    return item;
}
ZFMETHOD_DEFINE_1(P2Unit, zfautoT<P2Body>, p2_partRemove
        , ZFMP_IN(P2Body *, part)
        ) {
    zfindex index = this->p2_partList()->find(part);
    if(index == zfindexMax()) {
        return zfnull;
    }
    zfautoT<P2Body> item = this->p2_partList()->removeAndGet(index);
    _ZFP_P2BodyDetach(item);
    return item;
}
ZFMETHOD_DEFINE_0(P2Unit, void, p2_partRemoveAll) {
    ZFArray *partList = this->p2_partList();
    for(zfindex i = partList->count() - 1; i != zfindexMax(); --i) {
        P2Body *item = partList->get(i);
        _ZFP_P2BodyDetach(item);
    }
    this->p2_partList()->removeAll();
}

ZFMETHOD_DEFINE_1(P2Unit, void, p2_joint
        , ZFMP_IN(P2Joint *, v)
        ) {
    _ZFP_P2JointAttach(this, v);
    this->p2_jointList()->add(v);
}
ZFMETHOD_DEFINE_0(P2Unit, zfindex, p2_jointCount) {
    return this->p2_jointList()->count();
}
ZFMETHOD_DEFINE_1(P2Unit, P2Joint *, p2_jointAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->p2_jointList()->get(index);
}
ZFMETHOD_DEFINE_1(P2Unit, zfautoT<P2Joint>, p2_jointRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2Joint> item = this->p2_jointList()->removeAndGet(index);
    _ZFP_P2JointDetach(item);
    return item;
}
ZFMETHOD_DEFINE_1(P2Unit, zfautoT<P2Joint>, p2_jointRemove
        , ZFMP_IN(P2Joint *, joint)
        ) {
    zfindex index = this->p2_jointList()->find(joint);
    if(index == zfindexMax()) {
        return zfnull;
    }
    zfautoT<P2Joint> item = this->p2_jointList()->removeAndGet(index);
    _ZFP_P2JointDetach(item);
    return item;
}
ZFMETHOD_DEFINE_0(P2Unit, void, p2_jointRemoveAll
        ) {
    ZFArray *jointList = this->p2_jointList();
    for(zfindex i = jointList->count() - 1; i != zfindexMax(); --i) {
        P2Joint *item = jointList->get(i);
        _ZFP_P2JointDetach(item);
    }
    jointList->removeAll();
}

ZFMETHOD_DEFINE_1(P2Unit, P2Body *, p2_bodyFind
        , ZFMP_IN(const zfstring &, bodyId)
        ) {
    if(bodyId) {
        if(this->p2_body() && this->p2_body()->p2_bodyId() == bodyId) {
            return this->p2_body();
        }
        ZFArray *partList = this->p2_partList();
        for(zfindex i = partList->count() - 1; i != zfindexMax(); --i) {
            P2Body *part = partList->get(i);
            if(part->p2_bodyId() == bodyId) {
                return part;
            }
        }
    }
    return zfnull;
}
ZFMETHOD_DEFINE_1(P2Unit, P2Shape *, p2_shapeFind
        , ZFMP_IN(const zfstring &, shapeId)
        ) {
    if(shapeId) {
        if(this->p2_body()) {
            P2Shape *shape = this->p2_body()->p2_shapeFind(shapeId);
            if(shape) {
                return shape;
            }
        }
        ZFArray *partList = this->p2_partList();
        for(zfindex i = partList->count() - 1; i != zfindexMax(); --i) {
            P2Body *part = partList->get(i);
            P2Shape *shape = part->p2_shapeFind(shapeId);
            if(shape) {
                return shape;
            }
        }
    }
    return zfnull;
}
ZFMETHOD_DEFINE_1(P2Unit, P2Joint *, p2_jointFind
        , ZFMP_IN(const zfstring &, jointId)
        ) {
    if(jointId) {
        ZFArray *jointList = this->p2_jointList();
        for(zfindex i = jointList->count() - 1; i != zfindexMax(); --i) {
            P2Joint *joint = jointList->get(i);
            if(joint->p2_jointId() == jointId) {
                return joint;
            }
        }
        for(zfimplhashmap<P2Joint *, zfbool>::iterator it = _ZFP_P2Unit_d->unitRefJointList.begin(); it != _ZFP_P2Unit_d->unitRefJointList.end(); ++it) {
            P2Joint *joint = it->first;
            if(joint->p2_jointId() == jointId) {
                return joint;
            }
        }
    }
    return zfnull;
}

ZFMETHOD_DEFINE_0(P2Unit, zfautoT<ZFContainer>, p2_refJointList) {
    zfobj<ZFHashSet> ret;
    for(zfimplhashmap<P2Joint *, zfbool>::iterator it = _ZFP_P2Unit_d->unitRefJointList.begin(); it != _ZFP_P2Unit_d->unitRefJointList.end(); ++it) {
        ret->add(it->first);
    }
    return ret;
}

void P2Unit::objectOnInit(void) {
    zfsuper::objectOnInit();
    _ZFP_P2Unit_d = zfpoolNew(_ZFP_P2UnitPrivate);
}
void P2Unit::objectOnDealloc(void) {
    ZFCoreAssert(_ZFP_P2Unit_d->ownerWorld == zfnull);
    zfpoolDelete(_ZFP_P2Unit_d);
    zfsuper::objectOnDealloc();
}

// ============================================================
ZFOBJECT_REGISTER(P2UnitVisibilityEvent)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2UnitVisibilityEvent, ZFCoreArray<P2Unit *>, p2_unitEnterList)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2UnitVisibilityEvent, ZFCoreArray<P2Unit *>, p2_unitExitList)

ZFTYPEID_ACCESS_ONLY_DEFINE_UNCOMPARABLE(P2BodyMoveEventData, P2BodyMoveEventData)
ZFOUTPUT_TYPE_DEFINE(P2BodyMoveEventData, {
    zfstringAppend(s
            , "<Move %s %s %s>"
            , v.p2_position
            , v.p2_rotation
            , v.p2_body
            );
})
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2BodyMoveEventData, P2Body *, p2_body)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2BodyMoveEventData, ZFUIPoint, p2_position)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2BodyMoveEventData, zffloat, p2_rotation)

ZFOBJECT_REGISTER(P2BodyMoveEvent)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2BodyMoveEvent, ZFCoreArray<P2BodyMoveEventData>, p2_moveEventList)

ZFTYPEID_ACCESS_ONLY_DEFINE_UNCOMPARABLE(P2ContactEventData, P2ContactEventData)
ZFOUTPUT_TYPE_DEFINE(P2ContactEventData, {
    zfstringAppend(s
            , "<Contact %s %s>"
            , v.p2_shape0
            , v.p2_shape1
            );
})
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2ContactEventData, P2Shape *, p2_shape0)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2ContactEventData, P2Shape *, p2_shape1)

ZFOBJECT_REGISTER(P2ContactEvent)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2ContactEvent, ZFCoreArray<P2ContactEventData>, p2_contactEnterList)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2ContactEvent, ZFCoreArray<P2ContactEventData>, p2_contactExitList)

ZFTYPEID_ACCESS_ONLY_DEFINE_UNCOMPARABLE(P2SensorEventData, P2SensorEventData)
ZFOUTPUT_TYPE_DEFINE(P2SensorEventData, {
    zfstringAppend(s
            , "<Sensor %s %s>"
            , v.p2_shape0
            , v.p2_shape1
            );
})
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2SensorEventData, P2Shape *, p2_shape0)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2SensorEventData, P2Shape *, p2_shape1)

ZFOBJECT_REGISTER(P2SensorEvent)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2SensorEvent, ZFCoreArray<P2SensorEventData>, p2_sensorEnterList)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2SensorEvent, ZFCoreArray<P2SensorEventData>, p2_sensorExitList)

// ============================================================
ZFOBJECT_REGISTER(P2World)

ZFPROPERTY_ON_UPDATE_DEFINE(P2World, ZFUIPoint, p2_gravity) {
    if(B2_IS_NON_NULL(_ZFP_P2World_d->implWorldId)) {
        b2World_SetGravity(_ZFP_P2World_d->implWorldId, b2Vec2FromZF(propertyValue));
    }
}

ZFMETHOD_DEFINE_1(P2World, void, p2_visibleArea
        , ZFMP_IN(const ZFUIRect &, v)
        ) {
    _ZFP_P2World_d->visibleArea = v;
}
ZFMETHOD_DEFINE_0(P2World, const ZFUIRect &, p2_visibleArea) {
    return _ZFP_P2World_d->visibleArea;
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2World, ZFUIPoint, p2_UIOffset) {
    if(propertyValue != propertyValueOld) {
        this->p2impl_UIOffsetUpdate.execute(ZFArgs().sender(this));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2World, ZFUIPoint, p2_UIScale) {
    if(propertyValue != propertyValueOld) {
        this->p2impl_UIScaleUpdate.execute(ZFArgs().sender(this));
    }
}

ZFMETHOD_DEFINE_0(P2World, void, p2_start) {
    if(!_ZFP_P2World_d->implTimer) {
        zfself *owner = this;
        ZFLISTENER_1(impl
                , zfself *, owner
                ) {
            _ZFP_P2WorldImplStep(owner);
        } ZFLISTENER_END()
        _ZFP_P2World_d->implTimer = impl;
        ZFGlobalTimerAttach(impl);
    }
}
ZFMETHOD_DEFINE_0(P2World, void, p2_stop) {
    if(_ZFP_P2World_d->implTimer) {
        ZFGlobalTimerDetach(_ZFP_P2World_d->implTimer);
        _ZFP_P2World_d->implTimer = zfnull;
    }
}
ZFMETHOD_DEFINE_0(P2World, zfbool, p2_started) {
    return _ZFP_P2World_d->implTimer;
}
ZFMETHOD_DEFINE_0(P2World, void, p2_manualStep) {
    if(_ZFP_P2World_d->implTimer) {
        ZFGlobalTimerDetach(_ZFP_P2World_d->implTimer);
        _ZFP_P2World_d->implTimer = zfnull;
    }
    _ZFP_P2WorldImplStep(this);
}

ZFMETHOD_DEFINE_1(P2World, void, p2_unit
        , ZFMP_IN(P2Unit *, unit)
        ) {
    _ZFP_P2UnitAttach(this, unit);
    this->p2_unitList()->add(unit);
}
ZFMETHOD_DEFINE_0(P2World, zfindex, p2_unitCount) {
    return this->p2_unitList()->count();
}
ZFMETHOD_DEFINE_1(P2World, P2Unit *, p2_unitAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->p2_unitList()->get(index);
}
ZFMETHOD_DEFINE_1(P2World, zfautoT<P2Unit>, p2_unitRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2Unit> item = this->p2_unitList()->removeAndGet(index);
    _ZFP_P2UnitDetach(item);
    return item;
}
ZFMETHOD_DEFINE_1(P2World, zfautoT<P2Unit>, p2_unitRemove
        , ZFMP_IN(P2Unit *, unit)
        ) {
    zfindex index = this->p2_unitList()->find(unit);
    if(index == zfindexMax()) {
        return zfnull;
    }
    zfautoT<P2Unit> item = this->p2_unitList()->removeAndGet(index);
    _ZFP_P2UnitDetach(item);
    return item;
}
ZFMETHOD_DEFINE_0(P2World, void, p2_unitRemoveAll) {
    ZFArray *unitList = this->p2_jointList();
    for(zfindex i = unitList->count() - 1; i != zfindexMax(); --i) {
        P2Unit *item = unitList->get(i);
        _ZFP_P2UnitDetach(item);
    }
    unitList->removeAll();
}

ZFMETHOD_DEFINE_1(P2World, void, p2_joint
        , ZFMP_IN(P2Joint *, v)
        ) {
    _ZFP_P2JointAttach(this, v);
    this->p2_jointList()->add(v);
}
ZFMETHOD_DEFINE_0(P2World, zfindex, p2_jointCount) {
    return this->p2_jointList()->count();
}
ZFMETHOD_DEFINE_1(P2World, P2Joint *, p2_jointAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->p2_jointList()->get(index);
}
ZFMETHOD_DEFINE_1(P2World, zfautoT<P2Joint>, p2_jointRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2Joint> item = this->p2_jointList()->removeAndGet(index);
    _ZFP_P2JointDetach(item);
    return item;
}
ZFMETHOD_DEFINE_1(P2World, zfautoT<P2Joint>, p2_jointRemove
        , ZFMP_IN(P2Joint *, joint)
        ) {
    zfindex index = this->p2_jointList()->find(joint);
    if(index == zfindexMax()) {
        return zfnull;
    }
    zfautoT<P2Joint> item = this->p2_jointList()->removeAndGet(index);
    _ZFP_P2JointDetach(item);
    return item;
}
ZFMETHOD_DEFINE_0(P2World, void, p2_jointRemoveAll
        ) {
    ZFArray *jointList = this->p2_jointList();
    for(zfindex i = jointList->count() - 1; i != zfindexMax(); --i) {
        P2Joint *item = jointList->get(i);
        _ZFP_P2JointDetach(item);
    }
    jointList->removeAll();
}

ZFMETHOD_DEFINE_1(P2World, P2Unit *, p2_unitFind
        , ZFMP_IN(const zfstring &, unitId)
        ) {
    if(unitId) {
        ZFArray *unitList = this->p2_unitList();
        for(zfindex i = unitList->count() - 1; i != zfindexMax(); --i) {
            P2Unit *item = unitList->get(i);
            if(item->p2_unitId() == unitId) {
                return item;
            }
        }
    }
    return zfnull;
}
ZFMETHOD_DEFINE_1(P2World, P2Body *, p2_bodyFind
        , ZFMP_IN(const zfstring &, bodyId)
        ) {
    if(bodyId) {
        zfimplhashmap<zfstring, P2Body *>::iterator it = _ZFP_P2World_d->bodyIdMap.find(bodyId);
        if(it != _ZFP_P2World_d->bodyIdMap.end()) {
            return it->second;
        }
    }
    return zfnull;
}
ZFMETHOD_DEFINE_1(P2World, P2Shape *, p2_shapeFind
        , ZFMP_IN(const zfstring &, shapeId)
        ) {
    if(shapeId) {
        ZFArray *unitList = this->p2_unitList();
        for(zfindex i = unitList->count() - 1; i != zfindexMax(); --i) {
            P2Unit *item = unitList->get(i);
            P2Shape *shape = item->p2_shapeFind(shapeId);
            if(shape) {
                return shape;
            }
        }
    }
    return zfnull;
}
ZFMETHOD_DEFINE_1(P2World, P2Joint *, p2_jointFind
        , ZFMP_IN(const zfstring &, jointId)
        ) {
    if(jointId) {
        ZFArray *jointList = this->p2_jointList();
        for(zfindex i = jointList->count() - 1; i != zfindexMax(); --i) {
            P2Joint *joint = jointList->get(i);
            if(joint->p2_jointId() == jointId) {
                return joint;
            }
        }

        ZFArray *unitList = this->p2_unitList();
        for(zfindex i = unitList->count() - 1; i != zfindexMax(); --i) {
            P2Unit *item = unitList->get(i);
            P2Joint *joint = item->p2_jointFind(jointId);
            if(joint) {
                return joint;
            }
        }
    }
    return zfnull;
}

zfclassLikePOD _ZFP_P2World_overlapTestContext {
public:
    ZFListener callback;
    ZFArgs zfargs;
};
static bool _ZFP_P2World_overlapTestCb(b2ShapeId shapeId, void *context) {
    _ZFP_P2World_overlapTestContext &t = *(_ZFP_P2World_overlapTestContext *)context;
    t.zfargs.param0((P2Shape *)b2Shape_GetUserData(shapeId));
    t.callback.execute(t.zfargs);
    return !t.zfargs.eventFiltered();
}
ZFMETHOD_DEFINE_4(P2World, zfauto, p2_overlapTest
        , ZFMP_IN(const ZFUIRect &, rect)
        , ZFMP_IN(const ZFListener &, callback)
        , ZFMP_IN_OPT(zfflags, filterMask, P2FilterMaskAll())
        , ZFMP_IN_OPT(zfflags, filterCategory, P2FilterMaskAll())
        ) {
    _ZFP_P2World_overlapTestContext context;
    b2QueryFilter implFilter;
    implFilter.maskBits = (uint64_t)filterMask;
    implFilter.categoryBits = (uint64_t)filterCategory;
    b2World_OverlapAABB(
            _ZFP_P2World_d->implWorldId
            , b2AABBFromZF(rect)
            , implFilter
            , _ZFP_P2World_overlapTestCb
            , &context
            );
    return context.zfargs.result();
}
static float _ZFP_P2World_rayTestCb(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void *context) {
    _ZFP_P2World_overlapTestContext &t = *(_ZFP_P2World_overlapTestContext *)context;
    t.zfargs.param0((P2Shape *)b2Shape_GetUserData(shapeId));
    t.callback.execute(t.zfargs);
    if(t.zfargs.eventFiltered()) {
        return 0;
    }
    else {
        return 1;
    }
}
ZFMETHOD_DEFINE_5(P2World, zfauto, p2_rayTest
        , ZFMP_IN(const ZFUIPoint &, src)
        , ZFMP_IN(const ZFUIPoint &, direction)
        , ZFMP_IN(const ZFListener &, callback)
        , ZFMP_IN_OPT(zfflags, filterMask, P2FilterMaskAll())
        , ZFMP_IN_OPT(zfflags, filterCategory, P2FilterMaskAll())
        ) {
    _ZFP_P2World_overlapTestContext context;
    b2QueryFilter implFilter;
    implFilter.maskBits = (uint64_t)filterMask;
    implFilter.categoryBits = (uint64_t)filterCategory;
    b2World_CastRay(
            _ZFP_P2World_d->implWorldId
            , b2Vec2FromZF(src)
            , b2Vec2FromZF(direction)
            , implFilter
            , _ZFP_P2World_rayTestCb
            , &context
            );
    return context.zfargs.result();
}

void P2World::objectOnInit(void) {
    zfsuper::objectOnInit();
    _ZFP_P2World_d = zfpoolNew(_ZFP_P2WorldPrivate);

    b2WorldDef implWorldDef = b2DefaultWorldDef();
    implWorldDef.userData = this;
    implWorldDef.gravity = b2Vec2FromZF(this->p2_gravity());
    _ZFP_P2World_d->implWorldId = b2CreateWorld(&implWorldDef);
}
void P2World::objectOnDealloc(void) {
    b2DestroyWorld(_ZFP_P2World_d->implWorldId);
    zfpoolDelete(_ZFP_P2World_d);
    zfsuper::objectOnDealloc();
}
void P2World::objectOnDeallocPrepare(void) {
    this->p2_stop();

    // recursively cleanup internal state
    // impl would be destroyed by b2DestroyWorld
    {
        zfclassNotPOD CleanupJoint {
        public:
            static void a(ZF_IN P2Joint *joint) {
                joint->_ZFP_P2Joint_d->implJointId = b2_nullJointId;
            }
        };
        zfclassNotPOD CleanupBody {
        public:
            static void a(ZF_IN P2World *world, ZF_IN P2Body *body) {
                if(B2_IS_NON_NULL(body->_ZFP_P2Body_d->implBodyId)) {
                    world->p2impl_bodyRemove.execute(ZFArgs()
                            .sender(world)
                            .param0(body)
                            );
                }
                body->_ZFP_P2Body_d->implBodyId = b2_nullBodyId;
                body->_ZFP_P2Body_d->ownerUnit = zfnull;
                body->_ZFP_P2Body_d->bodyRefJointList.clear();
            }
        };

        ZFArray *worldJointList = this->p2_jointList();
        for(zfindex iJoint = worldJointList->count() - 1; iJoint != zfindexMax(); --iJoint) {
            CleanupJoint::a(worldJointList->get(iJoint));
        }
        worldJointList->removeAll();

        ZFArray *unitList = this->p2_unitList();
        for(zfindex iUnit = unitList->count() - 1; iUnit != zfindexMax(); --iUnit) {
            P2Unit *unit = unitList->get(iUnit);
            if(unit->p2_body()) {
                CleanupBody::a(this, unit->p2_body());
            }
            ZFArray *partList = unit->p2_partList();
            for(zfindex iPart = partList->count() - 1; iPart != zfindexMax(); --iPart) {
                CleanupBody::a(this, partList->get(iPart));
            }
            partList->removeAll();
            ZFArray *unitJointList = unit->p2_jointList();
            for(zfindex iJoint = unitJointList->count() - 1; iJoint != zfindexMax(); --iJoint) {
                CleanupJoint::a(unitJointList->get(iJoint));
            }
            unitJointList->removeAll();
            unit->_ZFP_P2Unit_d->unitRefJointList.clear();
            unit->_ZFP_P2Unit_d->ownerWorld = zfnull;
            unit->p2_body(zfnull);
        }
        unitList->removeAll();

        _ZFP_P2World_d->pendingBody.clear();
        _ZFP_P2World_d->pendingJoint.clear();
    }

    zfsuper::objectOnDeallocPrepare();
}

ZFMETHOD_FUNC_DEFINE_3(ZFUIPoint, P2World_toLocalPosition
        , ZFMP_IN(const ZFUIPoint &, relPosition)
        , ZFMP_IN(zffloat, relRotation)
        , ZFMP_IN(ZFUIPoint, worldPosition)
        ) {
    return b2Vec2ToZF(b2InvTransformPoint(b2TransformFromZF(relPosition, relRotation), b2Vec2FromZF(worldPosition)));
}
ZFMETHOD_FUNC_DEFINE_3(ZFUIPoint, P2World_toWorldPosition
        , ZFMP_IN(const ZFUIPoint &, relPosition)
        , ZFMP_IN(zffloat, relRotation)
        , ZFMP_IN(ZFUIPoint, localPosition)
        ) {
    return b2Vec2ToZF(b2TransformPoint(b2TransformFromZF(relPosition, relRotation), b2Vec2FromZF(localPosition)));
}

// ============================================================
static void _ZFP_P2JointAttach(ZF_IN ZFObject *jointOwner, ZF_IN P2Joint *joint) {
    ZFCoreAssertWithMessageTrim(joint->_ZFP_P2Joint_d->jointOwner == zfnull
            , "joint already attached to owner, joint: %s, owner: %s"
            , joint
            , joint->_ZFP_P2Joint_d->jointOwner
            );
    joint->_ZFP_P2Joint_d->jointOwner = jointOwner;
    P2World *ownerWorld = joint->p2_ownerWorld();
    if(ownerWorld) {
        ownerWorld->_ZFP_P2World_d->pendingJoint[joint];
    }
}
static void _ZFP_P2JointDetach(ZF_IN P2Joint *joint) {
    if(B2_IS_NON_NULL(joint->_ZFP_P2Joint_d->implJointId)) {
        joint->p2_ownerBody0()->_ZFP_P2Body_d->bodyRefJointList.erase(joint);
        joint->p2_ownerBody1()->_ZFP_P2Body_d->bodyRefJointList.erase(joint);
        b2DestroyJoint(joint->_ZFP_P2Joint_d->implJointId);
    }
    P2World *ownerWorld = joint->p2_ownerWorld();
    if(ownerWorld) {
        ownerWorld->_ZFP_P2World_d->pendingJoint.erase(joint);
    }
    joint->_ZFP_P2Joint_d->jointOwner = zfnull;
}
static void _ZFP_P2BodyAttach(ZF_IN P2Unit *ownerUnit, ZF_IN P2Body *body) {
    ZFCoreAssertWithMessageTrim(body->_ZFP_P2Body_d->ownerUnit == zfnull
            , "body already attached to unit, body: %s, unit: %s"
            , body
            , ownerUnit
            );
    body->_ZFP_P2Body_d->ownerUnit = ownerUnit;
    if(ownerUnit->_ZFP_P2Unit_d->ownerWorld) {
        ownerUnit->_ZFP_P2Unit_d->ownerWorld->_ZFP_P2World_d->pendingBody[body];
    }
}
static void _ZFP_P2BodyDetach(ZF_IN P2Body *body) {
    P2Unit *ownerUnit = body->_ZFP_P2Body_d->ownerUnit;
    ZFCoreAssert(ownerUnit != zfnull);
    P2World *ownerWorld = ownerUnit->_ZFP_P2Unit_d->ownerWorld;
    if(ownerWorld) {
        ownerWorld->_ZFP_P2World_d->pendingBody.erase(body);
        if(B2_IS_NON_NULL(body->_ZFP_P2Body_d->implBodyId)) {
            ownerWorld->p2impl_bodyRemove.execute(ZFArgs()
                    .sender(ownerWorld)
                    .param0(body)
                    );

            zfimplhashmap<P2Joint *, zfbool> &bodyRefJointList = body->_ZFP_P2Body_d->bodyRefJointList;
            for(zfimplhashmap<P2Joint *, zfbool>::iterator it = bodyRefJointList.begin(); it != bodyRefJointList.end(); ++it) {
                P2Joint *joint = it->first;
                ownerWorld->_ZFP_P2World_d->pendingJoint.erase(joint);

                P2Body *owenrBody2 = joint->p2_ownerBody0();
                if(owenrBody2 == body) {
                    owenrBody2 = joint->p2_ownerBody1();
                }
                owenrBody2->_ZFP_P2Body_d->bodyRefJointList.erase(joint);

                joint->p2_ownerBody0()->p2_ownerUnit()->_ZFP_P2Unit_d->unitRefJointList.erase(joint);
                joint->p2_ownerBody1()->p2_ownerUnit()->_ZFP_P2Unit_d->unitRefJointList.erase(joint);
            }

            ZFArray *shapeList = body->p2_shapeList();
            for(zfindex i = shapeList->count() - 1; i != zfindexMax(); --i) {
                P2Shape *shape = shapeList->get(i);
                shape->_ZFP_P2Shape_d->implShapeId = b2_nullShapeId;
            }

            b2DestroyBody(body->_ZFP_P2Body_d->implBodyId);
            body->_ZFP_P2Body_d->implBodyId = b2_nullBodyId;
        }
    }
    body->_ZFP_P2Body_d->ownerUnit = zfnull;
}
static void _ZFP_P2UnitAttach(ZF_IN P2World *ownerWorld, ZF_IN P2Unit *unit) {
    ZFCoreAssertWithMessageTrim(unit->_ZFP_P2Unit_d->ownerWorld == zfnull
            , "unit already attached to world, unit: %s, world: %s"
            , unit
            , unit->_ZFP_P2Unit_d->ownerWorld
            );
    unit->_ZFP_P2Unit_d->ownerWorld = ownerWorld;
    ownerWorld->_ZFP_P2World_d->pendingBody[unit->p2_body()];
    ZFArray *partList = unit->p2_partList();
    for(zfindex i = partList->count() - 1; i != zfindexMax(); --i) {
        ownerWorld->_ZFP_P2World_d->pendingBody[partList->get(i)];
    }
    ZFArray *jointList = unit->p2_jointList();
    for(zfindex i = jointList->count() - 1; i != zfindexMax(); --i) {
        ownerWorld->_ZFP_P2World_d->pendingJoint[jointList->get(i)];
    }
}
static void _ZFP_P2UnitDetach(ZF_IN P2Unit *unit) {
    _ZFP_P2BodyDetach(unit->p2_body());
    ZFArray *partList = unit->p2_partList();
    for(zfindex i = partList->count() - 1; i != zfindexMax(); --i) {
        _ZFP_P2BodyDetach(partList->get(i));
    }
    ZFArray *jointList = unit->p2_jointList();
    for(zfindex i = jointList->count() - 1; i != zfindexMax(); --i) {
        _ZFP_P2JointDetach(jointList->get(i));
    }
}

static void _ZFP_P2BodyImplMassUpdateRequest(ZF_IN P2Body *body) {
    P2World *ownerWorld = body->p2_ownerWorld();
    if(ownerWorld) {
        ownerWorld->_ZFP_P2World_d->pendingBody[body];
    }
    body->_ZFP_P2Body_d->massNeedUpdate = zftrue;
}

// ============================================================
static void _ZFP_P2BodyImplCreate(ZF_IN P2World *world, ZF_IN P2Body *body) {
    b2BodyDef implBodyDef = b2DefaultBodyDef();
    implBodyDef.userData = body;
    switch(body->p2_type()) {
        case v_P2BodyType::e_Static:
            implBodyDef.type = b2_staticBody;
            break;
        case v_P2BodyType::e_Kinematic:
            implBodyDef.type = b2_kinematicBody;
            break;
        case v_P2BodyType::e_Dynamic:
            implBodyDef.type = b2_dynamicBody;
            break;
        default:
            ZFCoreCriticalShouldNotGoHere();
            break;
    }
    implBodyDef.isEnabled = body->p2_enable();
    implBodyDef.enableSleep = body->p2_sleepEnable();
    implBodyDef.isBullet = body->p2_bullet();
    implBodyDef.fixedRotation = body->p2_rotationFixed();
    implBodyDef.gravityScale = body->p2_gravityScale();
    implBodyDef.position = b2Vec2FromZF(body->p2_position());
    implBodyDef.linearVelocity = b2Vec2FromZF(body->p2_positionVelocity());
    implBodyDef.linearDamping = body->p2_positionDamping();
    implBodyDef.rotation = b2RotFromZF(body->p2_rotation());
    implBodyDef.angularVelocity = b2RadFromZF(body->p2_rotationVelocity());
    implBodyDef.angularDamping = body->p2_rotationDamping();
    body->_ZFP_P2Body_d->implBodyId = b2CreateBody(world->_ZFP_P2World_d->implWorldId, &implBodyDef);

    ZFArray *shapeList = body->p2_shapeList();
    for(zfindex i = 0; i < shapeList->count(); ++i) {
        P2Shape *shape = shapeList->get(i);
        _ZFP_P2ShapePrivate::shapeCreate(body, shape);
    }
}
static void _ZFP_P2WorldImplStep_pendingBody(ZF_IN P2World *world) {
    zfimplhashmap<P2Body *, zfbool> &pendingBody = world->_ZFP_P2World_d->pendingBody;
    for(zfimplhashmap<P2Body *, zfbool>::iterator it = pendingBody.begin(); it != pendingBody.end(); ++it) {
        P2Body *body = it->first;
        if(B2_IS_NULL(body->_ZFP_P2Body_d->implBodyId)) {
            _ZFP_P2BodyImplCreate(world, body);
        }
        b2Body_ApplyMassFromShapes(body->_ZFP_P2Body_d->implBodyId);
    }
}
static void _ZFP_P2WorldImplStep_pendingJoint(ZF_IN P2World *world) {
    zfimplhashmap<P2Joint *, zfbool> &pendingJoint = world->_ZFP_P2World_d->pendingJoint;
    for(zfimplhashmap<P2Joint *, zfbool>::iterator it = pendingJoint.begin(); it != pendingJoint.end(); ++it) {
        P2Joint *joint = it->first;
        if(!joint->p2_bodyId0()) {
            ZFLogTrim("P2Joint: p2_bodyId0 not set, joint: %s", joint);
            continue;
        }
        if(!joint->p2_bodyId1()) {
            ZFLogTrim("P2Joint: p2_bodyId1 not set, joint: %s", joint);
            continue;
        }
        P2Body *ownerBody0 = zfnull;
        P2Body *ownerBody1 = zfnull;
        if(joint->_ZFP_P2Joint_d->jointOwner == world) {
            ownerBody0 = world->p2_bodyFind(joint->p2_bodyId0());
            ownerBody1 = world->p2_bodyFind(joint->p2_bodyId1());
        }
        else {
            P2Unit *unit = zfcast(P2Unit *, joint->_ZFP_P2Joint_d->jointOwner);
            if(unit) {
                ownerBody0 = unit->p2_bodyFind(joint->p2_bodyId0());
                ownerBody1 = unit->p2_bodyFind(joint->p2_bodyId1());
            }
        }
        if(ownerBody0 == zfnull) {
            ZFLogTrim("P2Joint: no body found, bodyId0: %s, owner: %s, joint: %s", joint->p2_bodyId0(), joint, joint->_ZFP_P2Joint_d->jointOwner);
            continue;
        }
        if(ownerBody1 == zfnull) {
            ZFLogTrim("P2Joint: no body found, bodyId1: %s, owner: %s, joint: %s", joint->p2_bodyId1(), joint, joint->_ZFP_P2Joint_d->jointOwner);
            continue;
        }
        _ZFP_P2JointPrivate::jointCreate(ownerBody0, ownerBody1, joint);
    }
    pendingJoint.clear();
}

static void _ZFP_P2WorldImplStep_events(ZF_IN P2World *world) {
    b2BodyEvents implBodyEvents = b2World_GetBodyEvents(world->_ZFP_P2World_d->implWorldId);
    ZFCoreArray<P2BodyMoveEventData> &moveEventList = world->_ZFP_P2World_d->bodyMoveEvent->p2_moveEventList;
}
static void _ZFP_P2WorldImplStep_visibleUnitsUpdate(ZF_IN P2World *world) {
}
static void _ZFP_P2WorldImplStep(ZF_IN P2World *world) {
    world->p2impl_stepPrev.execute(ZFArgs().sender(world));
    _ZFP_P2WorldImplStep_pendingBody(world);
    _ZFP_P2WorldImplStep_pendingJoint(world);
    b2World_Step(world->_ZFP_P2World_d->implWorldId, 1.0f / ZFGlobalTimerInterval(), 4);
    _ZFP_P2WorldImplStep_events(world);
    _ZFP_P2WorldImplStep_visibleUnitsUpdate(world);
    world->p2impl_stepPost.execute(ZFArgs().sender(world));
}

ZF_NAMESPACE_GLOBAL_END

