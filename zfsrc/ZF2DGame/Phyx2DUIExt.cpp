#include "Phyx2DUIExt.h"

ZF_NAMESPACE_GLOBAL_BEGIN

ZFCLASS_EXTEND(P2WorldView, P2World)
ZFCLASS_EXTEND(ZFUIView, P2Body)

zfclassNotPOD _ZFP_P2WorldViewPrivate : zfextend P2WorldImpl {
public:
    P2World *world;
    P2WorldView *worldView;
    zffloat UIScalePrev;

public:
    _ZFP_P2WorldViewPrivate(void)
    : world(zfnull)
    , worldView(zfnull)
    , UIScalePrev(1)
    {
    }

public:
    zfoverride
    virtual void bodyAdd(ZF_IN P2World *world, ZF_IN P2Body *body) {
        ZFUIView *bodyView = zfcast(ZFUIView *, body);
        worldView->internalImplView(bodyView);
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
        ZFCoreArray<zfautoT<ZFUIView> > childList = worldView->internalImplViewArray();
        for(zfindex i = childList.count() - 1; i != zfindexMax(); --i) {
            ZFUIView *bodyView = childList[i];
            P2Body *body = zfcast(P2Body *, bodyView);
            if(body) {
                _bodyPosUpdate(body, bodyView, body->p2_positionCur(), body->p2_rotationCur());
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

        const ZFUIPoint &implOffset = world->p2_UIOffset();
        zffloat implScale = world->p2_UIScale();
        ZFUIPoint implCenter = body->p2_centerOfMass();
        ZFUISize implSize = body->p2_bodySize();

        bodyView->viewFrame(ZFUIRectCreate(
                    (position.x - implOffset.x) * implScale
                    , worldView->height() - (position.y - implCenter.y + implSize.height / 2) * implScale
                    , implSize.width * implScale
                    , implSize.height * implScale
                    ));
        bodyView->translateX((implCenter.x - implSize.width / 2) * implScale);
        bodyView->translateY(0 - (implCenter.y - implSize.height / 2) * implScale);
    }
};

ZFOBJECT_REGISTER(P2WorldView)

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
    d->UIScalePrev = d->world->p2_UIScale();
}
void P2WorldView::objectOnDeallocPrepare(void) {
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

