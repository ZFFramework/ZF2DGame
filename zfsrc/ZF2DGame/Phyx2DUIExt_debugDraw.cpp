#include "Phyx2DUIExt.h"
#include "../../zfsrc_ext/ZFImpl/_repo/box2d/box2d/box2d.h"
#include <cmath>

ZF_NAMESPACE_GLOBAL_BEGIN

zfclass _ZFP_I_P2DebugDrawBody : zfextend ZFUIView {
    ZFOBJECT_DECLARE_WITH_CUSTOM_CTOR(_ZFP_I_P2DebugDrawBody, ZFUIView)
public:
    zffloat p2_UIScale;
    ZFCoreArray<zfautoT<ZFUIView> > shapeDebugList;
    zfautoT<ZFUIView> massCenterDebug;
protected:
    _ZFP_I_P2DebugDrawBody(void)
    : p2_UIScale(-1)
    , shapeDebugList()
    , massCenterDebug()
    {
    }
};
zfclass _ZFP_I_P2DebugDrawBodyCenter : zfextend ZFUIView {
    ZFOBJECT_DECLARE(_ZFP_I_P2DebugDrawBodyCenter, ZFUIView)
};
zfclass _ZFP_I_P2DebugDrawShape : zfextend ZFUIView {
    ZFOBJECT_DECLARE(_ZFP_I_P2DebugDrawShape, ZFUIView)
};
zfclass _ZFP_I_P2DebugDrawJoint : zfextend ZFUIView {
    ZFOBJECT_DECLARE(_ZFP_I_P2DebugDrawJoint, ZFUIView)
};

zfclass _ZFP_I_P2DebugDraw : zfextend ZFUIView {
    ZFOBJECT_DECLARE(_ZFP_I_P2DebugDraw, ZFUIView)

public:
    static void bodyOnAttach(ZF_IN const ZFArgs &zfargs) {
        _ZFP_I_P2DebugDraw *d = zfargs.sender()->objectTag(zftext("_ZFP_P2DebugDraw_world"));
        if(d) {
            d->bodyDebugAttach(zfargs.param0());
        }
    }
    static void bodyOnDetach(ZF_IN const ZFArgs &zfargs) {
        _ZFP_I_P2DebugDraw *d = zfargs.sender()->objectTag(zftext("_ZFP_P2DebugDraw_world"));
        if(d) {
            d->bodyDebugDetach(zfargs.param0());
        }
    }
    static void jointOnAttach(ZF_IN const ZFArgs &zfargs) {
        _ZFP_I_P2DebugDraw *d = zfargs.sender()->objectTag(zftext("_ZFP_P2DebugDraw_world"));
        if(d) {
            d->jointDebugAttach(zfargs.param0());
        }
    }
    static void jointOnDetach(ZF_IN const ZFArgs &zfargs) {
        _ZFP_I_P2DebugDraw *d = zfargs.sender()->objectTag(zftext("_ZFP_P2DebugDraw_world"));
        if(d) {
            d->jointDebugDetach(zfargs.param0());
        }
    }
    static void bodyOnMove(ZF_IN const ZFArgs &zfargs) {
        _ZFP_I_P2DebugDraw *d = zfargs.sender()->objectTag(zftext("_ZFP_P2DebugDraw_world"));
        if(d) {
            P2BodyMoveEvent *event = zfargs.param0();
            ZFCoreArray<P2Joint *> jointsUpdated;
            for(zfindex i = event->p2_moveEventList.count() - 1; i != zfindexMax(); --i) {
                ZFArray *jointList = event->p2_moveEventList[i].p2_body->p2_refJointList();
                for(zfindex i = jointList->count() - 1; i != zfindexMax(); --i) {
                    P2Joint *joint = jointList->get(i);
                    if(!jointsUpdated.isContain(joint)) {
                        jointsUpdated.add(joint);
                        d->jointDebugAttach(joint);
                    }
                }
            }
        }
    }
    static void uiOnUpdate(ZF_IN const ZFArgs &zfargs) {
        P2World *world = zfargs.sender();
        if(world->p2_UIScaleChanged()) {
            _ZFP_I_P2DebugDraw *d = world->objectTag(zftext("_ZFP_P2DebugDraw_world"));
            if(d) {
                d->updateAll(world);
            }
        }
    }
public:
    void bodyDebugAttach(ZF_IN P2Body *body) {
        if(!body->p2_implAttached()) {
            return;
        }
        P2World *world = body->p2_ownerWorld();
        if(!world) {
            return;
        }
        _ZFP_I_P2DebugDrawBody *v = body->objectTag(zftext("_ZFP_P2DebugDraw_body"));
        if(!v) {
            zfobj<_ZFP_I_P2DebugDrawBody> t;
            v = t;
            body->objectTag(zftext("_ZFP_P2DebugDraw_body"), v);
            zfcast(ZFUIView *, body)->internalFgView(v)->sizeFill();
            v->bgColor(ZFUIColorRandom(0.1f));
        }
        else {
            if(v->p2_UIScale == world->p2_UIScale()) {
                return;
            }
        }

        zffloat s = world->p2_UIScale();
        v->p2_UIScale = s;
        ZFUIRect bodyAABB = body->p2_AABBLocal();

        {
            ZFArray *shapeList = body->p2_shapeList();
            while(v->shapeDebugList.count() > shapeList->count()) {
                v->shapeDebugList.removeLastAndGet()->removeFromParent();
            }
            while(v->shapeDebugList.count() < shapeList->count()) {
                zfobj<_ZFP_I_P2DebugDrawShape> sv;
                sv->bgColor(ZFUIColorRandom(0.4f));
                v->child(sv);
                v->shapeDebugList.add(sv);
            }
            for(zfindex i = 0; i < shapeList->count(); ++i) {
                P2Shape *shape = shapeList->get(i);
                ZFUIRect shapeAABB = shape->p2_AABBLocal();
                v->shapeDebugList[i]->viewFrame(ZFUIRectCreate(
                            (shapeAABB.x - bodyAABB.x) * s
                            , (bodyAABB.y + bodyAABB.height - (shapeAABB.y + shapeAABB.height)) * s
                            , shapeAABB.width * s
                            , shapeAABB.height * s
                            ));
            }
        }

        {
            if(!v->massCenterDebug) {
                zfobj<_ZFP_I_P2DebugDrawBodyCenter> sv;
                v->massCenterDebug = sv;
                sv->bgColor(ZFUIColorRandom(0.8f));
                v->internalFgView(sv);
            }
            ZFUIPoint centerOfMass = body->p2_centerOfMass();
            zffloat size = 8;
            v->massCenterDebug->viewFrame(ZFUIRectCreate(
                        (centerOfMass.x - bodyAABB.x) * s - size / 2
                        , (bodyAABB.y + bodyAABB.height - centerOfMass.y) * s - size / 2
                        , size
                        , size
                        ));
        }
    }
    void bodyDebugDetach(ZF_IN P2Body *body) {
        zfautoT<ZFUIView> v = body->objectTagRemoveAndGet(zftext("_ZFP_P2DebugDraw_body"));
        if(v) {
            v->removeFromParent();
        }
    }
    void jointDebugAttach(ZF_IN P2Joint *joint) {
        if(!joint->p2_implAttached()) {
            return;
        }
        P2World *world = joint->p2_ownerWorld();
        if(!world) {
            return;
        }
        _ZFP_I_P2DebugDraw *d = world->objectTag(zftext("_ZFP_P2DebugDraw_world"));
        if(!d) {
            return;
        }
        ZFUIView *v = joint->objectTag(zftext("_ZFP_P2DebugDraw_joint"));
        if(!v) {
            zfobj<_ZFP_I_P2DebugDrawJoint> t;
            v = t;
            joint->objectTag(zftext("_ZFP_P2DebugDraw_joint"), v);
            d->internalFgView(v)->sizeFill();
            v->bgColor(ZFUIColorRandom(0.4f));
        }
        ZFUIPoint pos0 = joint->p2_ownerBody0()->p2_positionCur();
        ZFUIPoint pos1 = joint->p2_ownerBody1()->p2_positionCur();
        zffloat w = (zffloat)sqrt(pow(pos1.x - pos0.x, 2) + pow(pos1.y - pos0.y, 2));
        zffloat h = 3;
        zffloat x = (pos0.x + pos1.x) / 2;
        zffloat y = (pos0.y + pos1.y) / 2;
        zffloat s = world->p2_UIScale();
        v->viewFrame(ZFUIRectCreate(
                    (x - w / 2) * s
                    , (y - h / 2) * s
                    , w
                    , h
                    ));
        v->rotateZ(360 - atan2(pos1.y - pos0.y, pos1.x - pos0.x) * 180 / B2_PI);
    }
    void jointDebugDetach(ZF_IN P2Joint *joint) {
        zfautoT<ZFUIView> v = joint->objectTagRemoveAndGet(zftext("_ZFP_P2DebugDraw_joint"));
        if(v) {
            v->removeFromParent();
        }
    }
public:
    void updateAll(ZF_IN P2World *world) {
        ZFArray *unitList = world->p2_unitList();
        for(zfindex iUnit = unitList->count() - 1; iUnit != zfindexMax(); --iUnit) {
            P2Unit *unit = unitList->get(iUnit);
            this->bodyDebugAttach(unit->p2_body());
            ZFArray *partList = unit->p2_partList();
            for(zfindex iPart = partList->count() - 1; iPart != zfindexMax(); --iPart) {
                this->bodyDebugAttach(partList->get(iPart));
            }
            this->jointListDebugAttach(unit->p2_jointList());
        }

        this->jointListDebugAttach(world->p2_jointList());
    }
    void jointListDebugAttach(ZF_IN ZFArray *jointList) {
        for(zfindex iJoint = jointList->count() - 1; iJoint != zfindexMax(); --iJoint) {
            this->jointDebugAttach(jointList->get(iJoint));
        }
    }
    void jointListDebugDetach(ZF_IN ZFArray *jointList) {
        for(zfindex iJoint = jointList->count() - 1; iJoint != zfindexMax(); --iJoint) {
            this->jointDebugDetach(jointList->get(iJoint));
        }
    }
};

static void _ZFP_P2DebugDrawAttach(ZF_IN P2WorldView *worldView, ZF_IN P2World *world) {
    _ZFP_I_P2DebugDraw *d = worldView->objectTag(zftext("_ZFP_P2DebugDraw_world"));
    if(d) {
        return;
    }
    else {
        zfobj<_ZFP_I_P2DebugDraw> t;
        d = t;
        worldView->objectTag(zftext("_ZFP_P2DebugDraw_world"), d);
        worldView->internalFgView(d)->sizeFill();
    }
    world->observerAdd(P2World::E_P2BodyAttach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnAttach));
    world->observerAdd(P2World::E_P2BodyDetach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnDetach));
    world->observerAdd(P2World::E_P2JointAttach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::jointOnAttach));
    world->observerAdd(P2World::E_P2JointDetach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::jointOnDetach));
    world->observerAdd(P2World::E_P2BodyMoveEvent(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnMove));
    world->observerAdd(P2World::E_P2UIUpdate(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::uiOnUpdate));

    d->updateAll(world);
}
static void _ZFP_P2DebugDrawDetach(ZF_IN P2WorldView *worldView, ZF_IN P2World *world) {
    zfautoT<_ZFP_I_P2DebugDraw> d = worldView->objectTagRemoveAndGet(zftext("_ZFP_P2DebugDraw_world"));
    if(!d) {
        return;
    }
    world->observerRemove(P2World::E_P2BodyAttach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnAttach));
    world->observerRemove(P2World::E_P2BodyDetach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnDetach));
    world->observerRemove(P2World::E_P2JointAttach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::jointOnAttach));
    world->observerRemove(P2World::E_P2JointDetach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::jointOnDetach));
    world->observerRemove(P2World::E_P2BodyMoveEvent(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnMove));
    world->observerRemove(P2World::E_P2UIUpdate(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::uiOnUpdate));

    {
        ZFArray *unitList = world->p2_unitList();
        for(zfindex iUnit = unitList->count() - 1; iUnit != zfindexMax(); --iUnit) {
            P2Unit *unit = unitList->get(iUnit);
            d->bodyDebugDetach(unit->p2_body());
            ZFArray *partList = unit->p2_partList();
            for(zfindex iPart = partList->count() - 1; iPart != zfindexMax(); --iPart) {
                d->bodyDebugDetach(partList->get(iPart));
            }
            d->jointListDebugDetach(unit->p2_jointList());
        }
    }
    d->jointListDebugDetach(world->p2_jointList());
}

ZFPROPERTY_ON_ATTACH_DEFINE(P2WorldView, zfbool, debugDraw) {
    if(propertyValue) {
        _ZFP_P2DebugDrawAttach(this, zfcast(P2World *, this));
    }
    else {
        _ZFP_P2DebugDrawDetach(this, zfcast(P2World *, this));
    }
}
ZFPROPERTY_ON_DETACH_DEFINE(P2WorldView, zfbool, debugDraw) {
    _ZFP_P2DebugDrawDetach(this, zfcast(P2World *, this));
}

ZF_NAMESPACE_GLOBAL_END

