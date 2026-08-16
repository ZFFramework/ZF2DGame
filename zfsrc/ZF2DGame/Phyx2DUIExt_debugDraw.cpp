#include "Phyx2DUIExt.h"
#include "../../zfsrc_ext/ZFImpl/_repo/box2d/box2d/box2d.h"
#include <cmath>

ZF_NAMESPACE_GLOBAL_BEGIN

zfclass _ZFP_I_P2DebugDraw : zfextend ZFUIView {
    ZFOBJECT_DECLARE(_ZFP_I_P2DebugDraw, ZFUIView)

public:
    static void bodyOnAttach(ZF_IN const ZFArgs &zfargs) {
        // zfzfzf
    }
    static void bodyOnDetach(ZF_IN const ZFArgs &zfargs) {
    }
    static void jointOnAttach(ZF_IN const ZFArgs &zfargs) {
    }
    static void jointOnDetach(ZF_IN const ZFArgs &zfargs) {
    }
    static void bodyOnMove(ZF_IN const ZFArgs &zfargs) {
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
        ZFUIView *v = body->objectTag(zftext("_ZFP_P2DebugDraw_body"));
        if(!v) {
            zfobj<ZFUIView> t;
            v = t;
            body->objectTag(zftext("_ZFP_P2DebugDraw_body"), v);
            zfcast(ZFUIView *, body)->internalFgView(v)->sizeFill();
            v->bgColor(ZFUIColorRandom(0.2f));
        }
        zffloat s = world->p2_UIScale();
        ZFUIRect bodyAABB = body->p2_AABBLocal();
        ZFArray *shapeList = body->p2_shapeList();
        for(zfindex i = 0; i < shapeList->count(); ++i) {
            P2Shape *shape = shapeList->get(i);
            zfobj<ZFUIView> sv;
            sv->bgColor(ZFUIColorRandom(0.4f));
            v->child(sv);
            ZFUIRect shapeAABB = shape->p2_AABBLocal();
            sv->viewFrame(ZFUIRectCreate(
                        shapeAABB.x * s
                        , (bodyAABB.height - (shapeAABB.y + shapeAABB.height)) * s
                        , shapeAABB.width * s
                        , shapeAABB.height * s
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
        _ZFP_I_P2DebugDraw *d = worldView->objectTag(zftext("_ZFP_P2DebugDraw_world"));
        if(!d) {
            return;
        }
        ZFUIView *v = joint->objectTag(zftext("_ZFP_P2DebugDraw_joint"));
        if(!v) {
            zfobj<ZFUIView> t;
            v = t;
            joint->objectTag(zftext("_ZFP_P2DebugDraw_joint"), v);
            d->internalFgView(v)->sizeFill();
            v->bgColor(ZFUIColorRandom(0.4f));
        }
        ZFUIPoint pos0 = _bodyCenter(joint->p2_ownerBody0());
        ZFUIPoint pos1 = _bodyCenter(joint->p2_ownerBody1());
        zffloat w = sqrt(pow(pos1.x - pos0.x, 2), pow(pos1.y - pos0.y, 2));
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
        v->rotateZ(360 - atan2(pos1.y - pos0.y, pos1.x, pos1.y) * 180 / B2_PI);
    }
    void jointDebugDetach(ZF_IN P2Joint *joint) {
        zfautoT<ZFUIView> v = joint->objectTagRemoveAndGet(zftext("_ZFP_P2DebugDraw_joint"));
        if(v) {
            v->removeFromParent();
        }
    }
public:
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
private:
    static ZFUIPoint _bodyCenter(ZF_IN P2Body *body) {
        // zfzfzf
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
    }
    worldView->observerAdd(P2World::E_P2BodyAttach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnAttach));
    worldView->observerAdd(P2World::E_P2BodyDetach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnDetach));
    worldView->observerAdd(P2World::E_P2JointAttach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::jointOnAttach));
    worldView->observerAdd(P2World::E_P2JointDetach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::jointOnDetach));
    worldView->observerAdd(P2World::E_P2BodyMoveEvent(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnMove));

    {
        ZFArray *unitList = world->p2_unitList();
        for(zfindex iUnit = unitList->count() - 1; iUnit != zfindexMax(); --iUnit) {
            P2Unit *unit = unitList->get(iUnit);
            d->bodyDebugAttach(unit->p2_body());
            ZFArray *partList = unit->p2_partList();
            for(zfindex iPart = partList->count() - 1; iPart != zfindexMax(); --iPart) {
                d->bodyDebugAttach(partList->get(iPart));
            }
            d->jointListDebugAttach(unit->p2_jointList());
        }
    }
    d->jointListDebugAttach(world->p2_jointList());
}
static void _ZFP_P2DebugDrawDetach(ZF_IN P2WorldView *worldView, ZF_IN P2World *world) {
    zfautoT<_ZFP_I_P2DebugDraw> d = worldView->objectTagRemoveAndGet(zftext("_ZFP_P2DebugDraw_world"));
    if(!d) {
        return;
    }
    worldView->observerRemove(P2World::E_P2BodyAttach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnAttach));
    worldView->observerRemove(P2World::E_P2BodyDetach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnDetach));
    worldView->observerRemove(P2World::E_P2JointAttach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::jointOnAttach));
    worldView->observerRemove(P2World::E_P2JointDetach(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::jointOnDetach));
    worldView->observerRemove(P2World::E_P2BodyMoveEvent(), ZFCallbackForFunc(_ZFP_I_P2DebugDraw::bodyOnMove));

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

