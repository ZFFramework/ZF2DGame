#include "Phyx2DUIExt.h"

ZF_NAMESPACE_GLOBAL_BEGIN

ZFCLASS_EXTEND(P2WorldView, P2World)
ZFCLASS_EXTEND(ZFUIView, P2Body)

zfclassNotPOD _ZFP_P2WorldViewPrivate : zfextend P2WorldImpl {
public:
    typedef enum {
        TileUpdateByUI,
        TileUpdateBySpeed,
        TileUpdateByOffset,
    } TileUpdateMode;
public:
    P2World *world;
    P2WorldView *worldView;
    ZFUIView *container;
    zffloat UIScalePrev;
    TileUpdateMode tileUpdateMode;
    ZFUIPoint tileUpdateParam; // tileIsUpdateBySpeed's speed, or tileIsUpdateByOffset's offset

public:
    _ZFP_P2WorldViewPrivate(void)
    : world(zfnull)
    , worldView(zfnull)
    , container(zfobjAlloc(ZFUIView))
    , UIScalePrev(1)
    , tileUpdateMode(TileUpdateByUI)
    , tileUpdateParam()
    {
    }
    ~_ZFP_P2WorldViewPrivate(void) {
        zfobjRelease(container);
    }

public:
    zfoverride
    virtual void bodyAdd(ZF_IN P2World *world, ZF_IN P2Body *body) {
        ZFUIView *bodyView = zfcast(ZFUIView *, body);
        if(bodyView) {
            container->child(bodyView);
            _bodyPosUpdate(body, bodyView, body->p2_position(), body->p2_rotation());
        }
    }
    zfoverride
    virtual void bodyRemove(ZF_IN P2World *world, ZF_IN P2Body *body) {
        ZFUIView *bodyView = zfcast(ZFUIView *, body);
        if(bodyView) {
            bodyView->removeFromParent();
        }
    }
    zfoverride
    virtual void bodyMoveEvent(ZF_IN P2World *world, ZF_IN P2BodyMoveEvent *event) {
        for(zfindex i = event->p2_moveEventList.count() - 1; i != zfindexMax(); --i) {
            P2BodyMoveEventData const &data = event->p2_moveEventList[i];
            ZFUIView *bodyView = zfcast(ZFUIView *, data.p2_body);
            if(bodyView) {
                _bodyPosUpdate(data.p2_body, bodyView, data.p2_position, data.p2_rotation);
            }
        }
    }
    zfoverride
    virtual void UIUpdate(ZF_IN P2World *world) {
        ZFCoreArray<zfautoT<ZFUIView> > childList = container->childArray();
        for(zfindex i = childList.count() - 1; i != zfindexMax(); --i) {
            ZFUIView *bodyView = childList[i];
            P2Body *body = zfcast(P2Body *, bodyView);
            if(body) {
                _bodyPosUpdate(body, bodyView, body->p2_positionCur(), body->p2_rotationCur());
            }
        }

        const ZFCoreArray<zfautoT<P2ScreenTileView> > &tilesBg = worldView->tileBg();
        const ZFCoreArray<zfautoT<P2ScreenTileView> > &tilesFg = worldView->tileBg();
        if(!tilesBg.isEmpty() || !tilesFg.isEmpty()) {
            switch(this->tileUpdateMode) {
                case TileUpdateByUI: {
                    ZFUIPoint offset;
                    offset.x = world->p2_UIOffset().x * world->p2_UIScale();
                    offset.y = 0 - world->p2_UIOffset().y * world->p2_UIScale();
                    for(zfindex i = tilesBg.count() - 1; i != zfindexMax(); --i) {
                        tilesBg[i]->tileOffset(offset);
                    }
                    for(zfindex i = tilesFg.count() - 1; i != zfindexMax(); --i) {
                        tilesFg[i]->tileOffset(offset);
                    }
                    break;
                }
                case TileUpdateBySpeed:
                    for(zfindex i = tilesBg.count() - 1; i != zfindexMax(); --i) {
                        tilesBg[i]->tileOffsetStep(this->tileUpdateParam.x, this->tileUpdateParam.y);
                    }
                    for(zfindex i = tilesFg.count() - 1; i != zfindexMax(); --i) {
                        tilesFg[i]->tileOffsetStep(this->tileUpdateParam.x, this->tileUpdateParam.y);
                    }
                    break;
                case TileUpdateByOffset: {
                    for(zfindex i = tilesBg.count() - 1; i != zfindexMax(); --i) {
                        tilesBg[i]->tileOffset(this->tileUpdateParam);
                    }
                    for(zfindex i = tilesFg.count() - 1; i != zfindexMax(); --i) {
                        tilesFg[i]->tileOffset(this->tileUpdateParam);
                    }
                    break;
                }
                default:
                    ZFCoreCriticalShouldNotGoHere();
                    break;
            }
        }
    }
private:
    void _bodyPosUpdate(ZF_IN P2Body *body, ZF_IN ZFUIView *bodyView, ZF_IN const ZFUIPoint &position, ZF_IN zffloat rotation) {
        bodyView->UIScale(body->p2_ownerUnit()->p2_unitScale());
        bodyView->rotateZ(rotation);
        bodyView->viewFrame(P2UIRectFromBody(
                    worldView->height()
                    , world->p2_UIScale()
                    , world->p2_UIOffset()
                    , position
                    , body->p2_AABBLocal()
                    , rotation
                    , body->p2_centerOfMass()
                    ));
    }
};

ZFOBJECT_REGISTER(P2WorldView)

ZFPROPERTY_ON_UPDATE_DEFINE(P2WorldView, ZFCoreArray<zfautoT<P2ScreenTileView> >, tileBg) {
    for(zfindex i = propertyValueOld.count() - 1; i != zfindexMax(); --i) {
        P2ScreenTileView *child = propertyValueOld[i];
        if(child) {
            child->removeFromParent();
        }
    }
    for(zfindex i = propertyValue.count() - 1; i != zfindexMax(); --i) {
        P2ScreenTileView *child = propertyValue[i];
        if(child) {
            this->internalImplView(child, 0);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2WorldView, ZFCoreArray<zfautoT<P2ScreenTileView> >, tileFg) {
    for(zfindex i = propertyValueOld.count() - 1; i != zfindexMax(); --i) {
        P2ScreenTileView *child = propertyValueOld[i];
        if(child) {
            child->removeFromParent();
        }
    }
    for(zfindex i = 0; i < propertyValue.count(); ++i) {
        P2ScreenTileView *child = propertyValue[i];
        if(child) {
            this->internalImplView(child);
        }
    }
}

ZFMETHOD_DEFINE_0(P2WorldView, void, tileUpdateByUI) {
    d->tileUpdateMode = _ZFP_P2WorldViewPrivate::TileUpdateByUI;
}
ZFMETHOD_DEFINE_0(P2WorldView, zfbool, tileIsUpdateByUI) {
    return d->tileUpdateMode == _ZFP_P2WorldViewPrivate::TileUpdateByUI;
}

ZFMETHOD_DEFINE_1(P2WorldView, void, tileSpeed
        , ZFMP_IN(const ZFUIPoint &, v)
        ) {
    d->tileUpdateMode = _ZFP_P2WorldViewPrivate::TileUpdateBySpeed;
    d->tileUpdateParam = v;
}
ZFMETHOD_DEFINE_0(P2WorldView, ZFUIPoint, tileSpeed) {
    if(d->tileUpdateMode == _ZFP_P2WorldViewPrivate::TileUpdateBySpeed) {
        return d->tileUpdateParam;
    }
    else {
        return ZFUIPointZero();
    }
}
ZFMETHOD_DEFINE_0(P2WorldView, zfbool, tileIsUpdateBySpeed) {
    return d->tileUpdateMode == _ZFP_P2WorldViewPrivate::TileUpdateBySpeed;
}

ZFMETHOD_DEFINE_1(P2WorldView, void, tileOffset
        , ZFMP_IN(const ZFUIPoint &, v)
        ) {
    d->tileUpdateMode = _ZFP_P2WorldViewPrivate::TileUpdateByOffset;
    d->tileUpdateParam = v;
}
ZFMETHOD_DEFINE_0(P2WorldView, ZFUIPoint, tileOffset) {
    if(d->tileUpdateMode == _ZFP_P2WorldViewPrivate::TileUpdateByOffset) {
        return d->tileUpdateParam;
    }
    else {
        return ZFUIPointZero();
    }
}
ZFMETHOD_DEFINE_0(P2WorldView, zfbool, tileIsUpdateByOffset) {
    return d->tileUpdateMode == _ZFP_P2WorldViewPrivate::TileUpdateByOffset;
}

void P2WorldView::objectOnInit(void) {
    zfsuper::objectOnInit();
    d = zfpoolNew(_ZFP_P2WorldViewPrivate);
}
void P2WorldView::objectOnDealloc(void) {
    zfpoolDelete(d);
    zfsuper::objectOnDealloc();
}
void P2WorldView::objectOnInitFinish(void) {
    zfsuper::objectOnInitFinish();
    d->world = zfcast(P2World *, this);
    d->world->p2impl = d;
    d->worldView = this;
    this->internalImplView(d->container)->sizeFill();
    d->UIScalePrev = d->world->p2_UIScale();
}
void P2WorldView::objectOnDeallocPrepare(void) {
    d->container->removeFromParent();
    d->world = zfnull;
    d->worldView = zfnull;
    zfcast(P2World *, this)->p2impl = zfnull;
    zfsuper::objectOnDeallocPrepare();
}

void P2WorldView::layoutOnLayout(ZF_IN const ZFUIRect &bounds) {
    zfsuper::layoutOnLayout(bounds);
    const ZFUIRect &viewFrame = this->viewFrame();
    d->world->p2_UISize(ZFUISizeCreate(viewFrame.width, viewFrame.height));
}

// ============================================================
ZFMETHOD_FUNC_DEFINE_2(void, P2UIRectFromBodyT
        , ZFMP_OUT(ZFUIRect &, rect)
        , ZFMP_IN(P2Body *, body)
        ) {
    if(body == zfnull || body->p2_ownerWorld() == zfnull) {
        rect = ZFUIRectZero();
    }
    else {
        P2World *world = body->p2_ownerWorld();
        P2UIRectFromBodyT(
                rect
                , zfcast(ZFUIView *, world)->height()
                , world->p2_UIScale()
                , world->p2_UIOffset()
                , body->p2_positionCur()
                , body->p2_AABBLocal()
                , body->p2_rotationCur()
                , body->p2_centerOfMass()
                );
    }
}
ZFMETHOD_FUNC_DEFINE_1(ZFUIRect, P2UIRectFromBody
        , ZFMP_IN(P2Body *, body)
        ) {
    ZFUIRect ret;
    P2UIRectFromBodyT(ret, body);
    return ret;
}

ZFMETHOD_FUNC_DEFINE_8(void, P2UIRectFromBodyT
        , ZFMP_OUT(ZFUIRect &, rect)
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, position)
        , ZFMP_IN(const ZFUIRect &, aabbLocal)
        , ZFMP_IN_OPT(zffloat, rotation, 0)
        , ZFMP_IN_OPT(const ZFUIPoint &, centerOfMass, ZFUIPointZero())
        ) {
    zffloat r = (zffloat)(rotation * zfm_PI() / 180);
    zffloat cr = zfm_cos(r);
    zffloat sr = zfm_sin(r);
    zffloat x = position.x + centerOfMass.x * cr + centerOfMass.y * sr - centerOfMass.x;
    zffloat y = position.y - centerOfMass.x * sr + centerOfMass.y * cr - centerOfMass.y;
    zffloat w = aabbLocal.width;
    zffloat h = aabbLocal.height;
    zffloat dx = w / 2 - (centerOfMass.x - aabbLocal.x);
    zffloat dy = h / 2 - (centerOfMass.y - aabbLocal.y);

    rect.x = (x + (dx * cr + dy * sr) - w / 2 + worldUIOffset.x) * worldUIScale;
    rect.y = worldUIHeight + (-(y + (-dx * sr + dy * cr)) - h / 2 - worldUIOffset.y) * worldUIScale;
    rect.width = w * worldUIScale;
    rect.height = h * worldUIScale;
}
ZFMETHOD_FUNC_DEFINE_7(ZFUIRect, P2UIRectFromBody
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, position)
        , ZFMP_IN(const ZFUIRect &, aabbLocal)
        , ZFMP_IN_OPT(zffloat, rotation, 0)
        , ZFMP_IN_OPT(const ZFUIPoint &, centerOfMass, ZFUIPointZero())
        ) {
    ZFUIRect rect;
    P2UIRectFromBodyT(
            rect
            , worldUIHeight
            , worldUIScale
            , worldUIOffset
            , position
            , aabbLocal
            , rotation
            , centerOfMass
            );
    return rect;
}

// ============================================================
ZFMETHOD_FUNC_DEFINE_3(void, P2UIPointFromWorldT
        , ZFMP_OUT(ZFUIPoint &, pos)
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIPoint &, worldPos)
        ) {
    if(world == zfnull) {
        pos = ZFUIPointZero();
    }
    else {
        P2UIPointFromWorldT(
                pos
                , zfcast(ZFUIView *, world)->height()
                , world->p2_UIScale()
                , world->p2_UIOffset()
                , worldPos
                );
    }
}
ZFMETHOD_FUNC_DEFINE_2(ZFUIPoint, P2UIPointFromWorld
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIPoint &, worldPos)
        ) {
    ZFUIPoint pos;
    P2UIPointFromWorldT(
            pos
            , world
            , worldPos
            );
    return pos;
}

ZFMETHOD_FUNC_DEFINE_3(void, P2UIPointToWorldT
        , ZFMP_OUT(ZFUIPoint &, worldPos)
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIPoint &, pos)
        ) {
    if(world == zfnull) {
        worldPos = ZFUIPointZero();
    }
    else {
        P2UIPointToWorldT(
                worldPos
                , zfcast(ZFUIView *, world)->height()
                , world->p2_UIScale()
                , world->p2_UIOffset()
                , pos
                );
    }
}
ZFMETHOD_FUNC_DEFINE_2(ZFUIPoint, P2UIPointToWorld
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIPoint &, pos)
        ) {
    ZFUIPoint worldPos;
    P2UIPointToWorldT(
            worldPos
            , world
            , pos
            );
    return worldPos;
}

ZFMETHOD_FUNC_DEFINE_5(void, P2UIPointFromWorldT
        , ZFMP_OUT(ZFUIPoint &, pos)
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, worldPos)
        ) {
    pos.x = (worldPos.x + worldUIOffset.x) * worldUIScale;
    pos.y = worldUIHeight - (worldPos.y + worldUIOffset.y) * worldUIScale;
}
ZFMETHOD_FUNC_DEFINE_4(ZFUIPoint, P2UIPointFromWorld
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, worldPos)
        ) {
    ZFUIPoint pos;
    P2UIPointFromWorldT(
            pos
            , worldUIHeight
            , worldUIScale
            , worldUIOffset
            , worldPos
            );
    return pos;
}

ZFMETHOD_FUNC_DEFINE_5(void, P2UIPointToWorldT
        , ZFMP_OUT(ZFUIPoint &, worldPos)
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, pos)
        ) {
    worldPos.x = pos.x / worldUIScale - worldUIOffset.x;
    worldPos.y = (worldUIHeight - pos.y) / worldUIScale - worldUIOffset.y;
}
ZFMETHOD_FUNC_DEFINE_4(ZFUIPoint, P2UIPointToWorld
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, pos)
        ) {
    ZFUIPoint worldPos;
    P2UIPointToWorldT(
            worldPos
            , worldUIHeight
            , worldUIScale
            , worldUIOffset
            , pos
            );
    return worldPos;
}

// ============================================================
ZFMETHOD_FUNC_DEFINE_3(void, P2UIRectFromAABBT
        , ZFMP_OUT(ZFUIRect &, rect)
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, aabb)
        ) {
    if(world == zfnull) {
        rect = ZFUIRectZero();
    }
    else {
        P2UIRectFromAABBT(
                rect
                , zfcast(ZFUIView *, world)->height()
                , world->p2_UIScale()
                , world->p2_UIOffset()
                , aabb
                );
    }
}
ZFMETHOD_FUNC_DEFINE_2(ZFUIRect, P2UIRectFromAABB
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, aabb)
        ) {
    ZFUIRect rect;
    P2UIRectFromAABBT(
            rect
            , world
            , aabb
            );
    return rect;
}

ZFMETHOD_FUNC_DEFINE_3(void, P2UIRectToAABBT
        , ZFMP_OUT(ZFUIRect &, aabb)
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, rect)
        ) {
    if(world == zfnull) {
        aabb = ZFUIRectZero();
    }
    else {
        P2UIRectToAABBT(
                aabb
                , zfcast(ZFUIView *, world)->height()
                , world->p2_UIScale()
                , world->p2_UIOffset()
                , rect
                );
    }
}
ZFMETHOD_FUNC_DEFINE_2(ZFUIRect, P2UIRectToAABB
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, rect)
        ) {
    ZFUIRect aabb;
    P2UIRectToAABBT(
            aabb
            , world
            , rect
            );
    return aabb;
}

ZFMETHOD_FUNC_DEFINE_5(void, P2UIRectFromAABBT
        , ZFMP_OUT(ZFUIRect &, rect)
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIRect &, aabb)
        ) {
    rect.width = aabb.width * worldUIScale;
    rect.height = aabb.height * worldUIScale;
    rect.x = (aabb.x + worldUIOffset.x) * worldUIScale;
    rect.y = worldUIHeight - (aabb.y + aabb.height + worldUIOffset.y) * worldUIScale;
}
ZFMETHOD_FUNC_DEFINE_4(ZFUIRect, P2UIRectFromAABB
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIRect &, aabb)
        ) {
    ZFUIRect rect;
    P2UIRectFromAABBT(
            rect
            , worldUIHeight
            , worldUIScale
            , worldUIOffset
            , aabb
            );
    return rect;
}

ZFMETHOD_FUNC_DEFINE_5(void, P2UIRectToAABBT
        , ZFMP_OUT(ZFUIRect &, aabb)
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIRect &, rect)
        ) {
    aabb.width = rect.width / worldUIScale;
    aabb.height = rect.height / worldUIScale;
    aabb.x = rect.x / worldUIScale - worldUIOffset.x;
    aabb.y = (worldUIHeight - (rect.y + rect.height)) / worldUIScale - worldUIOffset.y;
}
ZFMETHOD_FUNC_DEFINE_4(ZFUIRect, P2UIRectToAABB
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIRect &, rect)
        ) {
    ZFUIRect aabb;
    P2UIRectToAABBT(
            aabb
            , worldUIHeight
            , worldUIScale
            , worldUIOffset
            , rect
            );
    return aabb;
}

ZF_NAMESPACE_GLOBAL_END

