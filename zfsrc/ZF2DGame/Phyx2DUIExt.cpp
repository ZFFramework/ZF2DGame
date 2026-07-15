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
        container->child(bodyView);
        _bodyPosUpdate(body, bodyView, body->p2_position(), body->p2_rotation());
    }
    zfoverride
    virtual void bodyRemove(ZF_IN P2World *world, ZF_IN P2Body *body) {
        zfcast(ZFUIView *, body)->removeFromParent();
    }
    zfoverride
    virtual void bodyMoveEvent(ZF_IN P2World *world, ZF_IN P2BodyMoveEvent *event) {
        for(zfindex i = event->p2_moveEventList.count() - 1; i != zfindexMax(); --i) {
            P2BodyMoveEventData const &data = event->p2_moveEventList[i];
            _bodyPosUpdate(data.p2_body, zfcast(ZFUIView *, data.p2_body), data.p2_position, data.p2_rotation);
        }
    }
    zfoverride
    virtual void UIUpdate(ZF_IN P2World *world) {
        if(this->UIScalePrev != world->p2_UIScale()) {
            this->UIScalePrev = world->p2_UIScale();
            this->visibleAreaUpdate(worldView->viewFrame());
        }
        ZFCoreArray<zfautoT<ZFUIView> > childList = container->childArray();
        for(zfindex i = childList.count() - 1; i != zfindexMax(); --i) {
            ZFUIView *bodyView = childList[i];
            P2Body *body = zfcast(P2Body *, bodyView);
            if(body) {
                _bodyPosUpdate(body, bodyView, body->p2_positionCur(), body->p2_rotationCur());
            }
        }

        const ZFCoreArray<zfautoT<TileView> > &tilesBg = worldView->tileBg();
        const ZFCoreArray<zfautoT<TileView> > &tilesFg = worldView->tileBg();
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
public:
    void visibleAreaUpdate(ZF_IN const ZFUIRect &rect) {
        if(world) {
            world->p2_UIVisibleArea(ZFUIRectApplyMargin(ZFUIRectCreate(
                            0
                            , 0
                            , rect.width / world->p2_UIScale()
                            , rect.height / world->p2_UIScale()
                            )
                        , world->p2_UIVisibleAreaMargin()
                        ));
        }
    }
private:
    void _bodyPosUpdate(ZF_IN P2Body *body, ZF_IN ZFUIView *bodyView, ZF_IN const ZFUIPoint &position, ZF_IN zffloat rotation) {
        bodyView->rotateZ(360 - rotation);

        zffloat unitScale = body->p2_ownerUnit()->p2_unitScale();
        const ZFUIPoint &UIOffset = world->p2_UIOffset();
        zffloat UIScale = world->p2_UIScale();
        ZFUIPoint centerOfMass = ZFUIPointApplyScaleReversely(body->p2_centerOfMass(), unitScale);
        ZFUISize bodySize = ZFUISizeApplyScaleReversely(body->p2_bodySize(), unitScale);

        bodyView->UIScale(unitScale);
        bodyView->viewFrame(ZFUIRectCreate(
                    (position.x - UIOffset.x) * UIScale
                    , worldView->height() - (position.y - centerOfMass.y + bodySize.height / 2) * UIScale
                    , bodySize.width * UIScale
                    , bodySize.height * UIScale
                    ));
        bodyView->translateX((centerOfMass.x - bodySize.width / 2) * UIScale);
        bodyView->translateY(0 - (centerOfMass.y - bodySize.height / 2) * UIScale);
    }
};

ZFOBJECT_REGISTER(P2WorldView)

ZFPROPERTY_ON_UPDATE_DEFINE(P2WorldView, ZFCoreArray<zfautoT<TileView> >, tileBg) {
    for(zfindex i = propertyValueOld.count() - 1; i != zfindexMax(); --i) {
        TileView *child = propertyValueOld[i];
        if(child) {
            child->removeFromParent();
        }
    }
    for(zfindex i = propertyValue.count() - 1; i != zfindexMax(); --i) {
        TileView *child = propertyValue[i];
        if(child) {
            this->internalImplView(child, 0);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2WorldView, ZFCoreArray<zfautoT<TileView> >, tileFg) {
    for(zfindex i = propertyValueOld.count() - 1; i != zfindexMax(); --i) {
        TileView *child = propertyValueOld[i];
        if(child) {
            child->removeFromParent();
        }
    }
    for(zfindex i = 0; i < propertyValue.count(); ++i) {
        TileView *child = propertyValue[i];
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
    {
        const ZFUIRect &viewFrame = this->viewFrame();
        const ZFUIRect &viewFramePrev = this->viewFramePrev();
        if(viewFrame.width != viewFramePrev.width
                || viewFrame.height != viewFramePrev.height
                ) {
            d->world->p2_UIUpdateRequest();
        }
    }
    d->visibleAreaUpdate(bounds);
}

ZF_NAMESPACE_GLOBAL_END

