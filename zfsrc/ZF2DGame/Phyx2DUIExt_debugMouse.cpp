#include "Phyx2DUIExt.h"

ZF_NAMESPACE_GLOBAL_BEGIN

zfclass _ZFP_I_P2DebugMouseCtrl : zfextend ZFUIView {
    ZFOBJECT_DECLARE(_ZFP_I_P2DebugMouseCtrl, ZFUIView)
private:
    zfautoT<P2Body> _target;
    zfautoT<P2JointMouse> _moveCtrl;
    zfautoT<P2Body> _moveHelper;
    ZFUIPoint _moveValueBak;
    zffloat _rotateValueBak;
    zffloat _cs; // center view's size
    ZFUISize _ms; // move view's size
    zffloat _rs; // rotate view's size
    zffloat _ox; // center of this view's position when mouse down
    zffloat _oy;
    zffloat _dx; // mouse down position relative to (_ox, _oy)
    zffloat _dy;
    zfautoT<ZFUIImageView> _move_c; // center image
    zfautoT<ZFUIImageView> _move_l; //     rotate -90
    zfautoT<ZFUIImageView> _move_t; // main move image
    zfautoT<ZFUIImageView> _move_r; //     rotate 90
    zfautoT<ZFUIImageView> _move_b; //     rotate 180
    zfautoT<ZFUIImageView> _rotate_lt; //     rotate -90
    zfautoT<ZFUIImageView> _rotate_rt; // main rotate image
    zfautoT<ZFUIImageView> _rotate_rb; //     rotate 90
    zfautoT<ZFUIImageView> _rotate_lb; //     rotate 180
    zfautoT<ZFUIImageView> _hl; // current highlighted view
    zfautoT<ZFUIImage> _hlImgBak; // highlighted view's original image
    enum {
        _Ctrl_move_x = 1 << 0,
        _Ctrl_move_y = 1 << 1,
        _Ctrl_rotate = 1 << 2,
    };
    zfflags _ctrl;
public:
    void targetPosUpdate(void) {
        if(_target) {
            _move_c->layoutMeasure(ZFUISizeInvalid(), ZFUISizeParamWrapWrap());
            _move_t->layoutMeasure(ZFUISizeInvalid(), ZFUISizeParamWrapWrap());
            const ZFUISize &cs = _move_c->layoutMeasuredSize();
            zffloat size = zfmMax(cs.width, cs.height) + _move_t->layoutMeasuredSize().height * 2;
            ZFUIPoint center = P2UIPointFromWorld(_target->p2_ownerWorld(), _target->p2_positionCur());
            if(this->layoutParam() == zfnull) {
                this->layoutParam(this->layoutParamCreate());
            }
            this->layoutParam()->margin(center.x - size / 2, center.y - size / 2, 0, 0);
            this->viewSizeMin(ZFUISizeCreate(size));
        }
    }

    P2Body *target(void) {
        return _target;
    }
    void target(ZF_IN P2Body *target) {
        if(_target == target) {
            return;
        }
        if(_target) {
            _moveCtrlDetach();
            _rotateCtrlDetach();
            _target = zfnull;

            _move_c->removeFromParent();
            _move_c = zfnull;
            _move_l->removeFromParent();
            _move_l = zfnull;
            _move_t->removeFromParent();
            _move_t = zfnull;
            _move_r->removeFromParent();
            _move_r = zfnull;
            _move_b->removeFromParent();
            _move_b = zfnull;

            _rotate_lt->removeFromParent();
            _rotate_lt = zfnull;
            _rotate_rt->removeFromParent();
            _rotate_rt = zfnull;
            _rotate_rb->removeFromParent();
            _rotate_rb = zfnull;
            _rotate_lb->removeFromParent();
            _rotate_lb = zfnull;

            _hl = zfnull;
            _hlImgBak = zfnull;
        }
        _target = target;
        if(!_target) {
            return;
        }
        _moveCenterViewUpdate();
        _rotateViewUpdate(_rotate_lt, -90);
        _rotateViewUpdate(_rotate_rt, 0);
        _rotateViewUpdate(_rotate_rb, 90);
        _rotateViewUpdate(_rotate_lb, 180);
        _moveViewUpdate(_move_l, -90);
        _moveViewUpdate(_move_t, 0);
        _moveViewUpdate(_move_r, 90);
        _moveViewUpdate(_move_b, 180);
    }
    // x, y : mouse pos relative to world's left top
    // return: whether touch successfully dispatched, when false, the ctrl view should be removed
    zfbool update(ZF_IN ZFUIMouseAction action, ZF_IN zffloat x, ZF_IN zffloat y) {
        if(!_target) {
            return zffalse;
        }
        switch(action) {
            case v_ZFUIMouseAction::e_Down:
                {
                    ZFUIRect viewRect = ZFUIViewUtil::viewRectToParent(this, zfcast(ZFUIView *, _target->p2_ownerWorld()));
                    _ox = viewRect.x + viewRect.width / 2;
                    _oy = viewRect.y + viewRect.height / 2;
                    _dx = x - _ox;
                    _dy = y - _oy;
                }
                if(_hl) {
                    _hl->image(_hlImgBak);
                    _hlImgBak = zfnull;
                    _hl = zfnull;
                }
                _touchViewDetect(_hl, _hlImgBak, _dx, _dy);
                if(!_hl) {
                    return zffalse;
                }
                if(ZFBitTest(_ctrl, _Ctrl_move_x | _Ctrl_move_y)) {
                    _moveCtrlAttach();
                }
                if(ZFBitTest(_ctrl, _Ctrl_rotate)) {
                    _rotateCtrlAttach();
                }
                break;
            case v_ZFUIMouseAction::e_Move:
                if(!_hl) {
                    return zffalse;
                }
                if(ZFBitTest(_ctrl, _Ctrl_move_x | _Ctrl_move_y)) {
                    if(ZFBitTestAll(_ctrl, _Ctrl_move_x | _Ctrl_move_y)) {
                        _moveCtrlUpdate(x - (_ox + _dx), y - (_oy + _dy));
                    }
                    else if(ZFBitTestAll(_ctrl, _Ctrl_move_x)) {
                        _moveCtrlUpdate(x - (_ox + _dx), 0);
                    }
                    else if(ZFBitTestAll(_ctrl, _Ctrl_move_y)) {
                        _moveCtrlUpdate(0, y - (_oy + _dy));
                    }
                }
                if(ZFBitTest(_ctrl, _Ctrl_rotate)) {
                    _rotateCtrlUpdate(x - _ox, y - _oy);
                }
                break;
            case v_ZFUIMouseAction::e_Up:
            case v_ZFUIMouseAction::e_Cancel:
                if(_hl) {
                    _hl->image(_hlImgBak);
                    _hlImgBak = zfnull;
                    _hl = zfnull;
                }
                _moveCtrlDetach();
                break;
            default:
                break;
        }
        return zftrue;
    }
private:
    void _moveCenterViewUpdate(void) {
        _move_c = zfobj<ZFUIImageView>();
        this->child(_move_c);
        _move_c->image(zfres("ZF2DGame/P2DebugMouse_center.png"));
    }
    void _moveViewUpdate(ZF_IN zfautoT<ZFUIImageView> &v, ZF_IN zffloat rotate) {
        v = zfobj<ZFUIImageView>();
        this->child(v);
        v->image(zfres("ZF2DGame/P2DebugMouse_move.png"));
        v->rotateZ(rotate);
    }
    void _rotateViewUpdate(ZF_IN zfautoT<ZFUIImageView> &v, ZF_IN zffloat rotate) {
        v = zfobj<ZFUIImageView>();
        this->child(v);
        v->image(zfres("ZF2DGame/P2DebugMouse_rotate.png"));
        v->rotateZ(rotate);
    }

    void _touchViewDetect(ZF_OUT zfautoT<ZFUIImageView> &v, ZF_OUT zfautoT<ZFUIImage> &imgN, ZF_IN zffloat dx, ZF_IN zffloat dy) {
        zffloat rc = _cs / 2;
        zffloat rt = _cs / 2 + _ms.height;
        if(dx >= -rt && dx <= -rc) {
            if(dy >= -rt && dy <= -rc) {
                v = _rotate_lt;
                v->image(zfres("ZF2DGame/P2DebugMouse_rotate_hl.png"));
                imgN = zfres("ZF2DGame/P2DebugMouse_rotate.png");
                _ctrl = _Ctrl_rotate;
            }
            else if(dy >= -rc && dy <= rc) {
                v = _move_l;
                v->image(zfres("ZF2DGame/P2DebugMouse_move_hl.png"));
                imgN = zfres("ZF2DGame/P2DebugMouse_move.png");
                _ctrl = _Ctrl_move_x;
            }
            else if(dy >= rc && dy <= rt) {
                v = _rotate_lb;
                v->image(zfres("ZF2DGame/P2DebugMouse_rotate_hl.png"));
                imgN = zfres("ZF2DGame/P2DebugMouse_rotate.png");
                _ctrl = _Ctrl_rotate;
            }
        }
        else if(dx >= -rc && dx <= rc) {
            if(dy >= -rt && dy <= -rc) {
                v = _move_t;
                v->image(zfres("ZF2DGame/P2DebugMouse_move_hl.png"));
                imgN = zfres("ZF2DGame/P2DebugMouse_move.png");
                _ctrl = _Ctrl_move_y;
            }
            else if(dy >= -rc && dy <= rc) {
                v = _move_c;
                v->image(zfres("ZF2DGame/P2DebugMouse_center_hl.png"));
                imgN = zfres("ZF2DGame/P2DebugMouse_center.png");
                _ctrl = _Ctrl_move_x | _Ctrl_move_y;
            }
            else if(dy >= rc && dy <= rt) {
                v = _move_b;
                v->image(zfres("ZF2DGame/P2DebugMouse_move_hl.png"));
                imgN = zfres("ZF2DGame/P2DebugMouse_move.png");
                _ctrl = _Ctrl_move_y;
            }
        }
        else if(dx >= rc && dx <= rt) {
            if(dy >= -rt && dy <= -rc) {
                v = _rotate_rt;
                v->image(zfres("ZF2DGame/P2DebugMouse_rotate_hl.png"));
                imgN = zfres("ZF2DGame/P2DebugMouse_rotate.png");
                _ctrl = _Ctrl_rotate;
            }
            else if(dy >= -rc && dy <= rc) {
                v = _move_r;
                v->image(zfres("ZF2DGame/P2DebugMouse_move_hl.png"));
                imgN = zfres("ZF2DGame/P2DebugMouse_move.png");
                _ctrl = _Ctrl_move_x;
            }
            else if(dy >= rc && dy <= rt) {
                v = _rotate_rb;
                v->image(zfres("ZF2DGame/P2DebugMouse_rotate_hl.png"));
                imgN = zfres("ZF2DGame/P2DebugMouse_rotate.png");
                _ctrl = _Ctrl_rotate;
            }
        }
    }

    void _moveCtrlAttach(void) {
        P2World *world = _target->p2_ownerWorld();

        _moveValueBak = _target->p2_positionCur();

        _moveHelper = zfobj<ZFUIView>();
        world->p2_unit(_moveHelper);
        _moveHelper->p2_type(v_P2BodyType::e_Static);
        _moveHelper->p2_position(_moveValueBak);

        _moveCtrl = zfobj<P2JointMouse>();
        world->p2_joint(_moveCtrl);
        _moveCtrl->p2_body0(_moveHelper);
        _moveCtrl->p2_body1(_target);
        _moveCtrl->p2_position(_moveValueBak);

        _target->p2_sleeping(zffalse);
    }
    void _moveCtrlDetach(void) {
        if(_moveCtrl) {
            _moveCtrl->p2_removeFromParent();
            _moveCtrl = zfnull;
        }
        if(_moveHelper) {
            _moveHelper->p2_removeFromParent();
            _moveHelper = zfnull;
        }
        P2WorldView *worldView = zfcast(P2WorldView *, _target->p2_ownerWorld());
        if(worldView && worldView->debugMousePositionAlign() > 0) {
            ZFUIPoint t = _target->p2_positionCur();
            if(ZFBitTest(_ctrl, _Ctrl_move_x)) {
                _valueAlign(t.x, worldView->debugMousePositionAlign());
            }
            if(ZFBitTest(_ctrl, _Ctrl_move_y)) {
                _valueAlign(t.y, worldView->debugMousePositionAlign());
            }
            if(t != _target->p2_positionCur()) {
                _target->p2_position(t);
            }
        }
        _target->p2_positionVelocity(ZFUIPointZero());
    }
    void _moveCtrlUpdate(ZF_IN zffloat dx, ZF_IN zffloat dy) {
        if(!_target) {
            return;
        }
        P2World *world = _target->p2_ownerWorld();
        ZFUIPoint t = _moveValueBak;
        if(ZFBitTest(_ctrl, _Ctrl_move_x)) {
            t.x += dx / world->p2_UIScale();
        }
        if(ZFBitTest(_ctrl, _Ctrl_move_y)) {
            t.y -= dy / world->p2_UIScale();
        }
        {
            P2WorldView *worldView = zfcast(P2WorldView *, world);
            if(worldView && worldView->debugMousePositionAlign() > 0) {
                if(ZFBitTest(_ctrl, _Ctrl_move_x)) {
                    _valueAlign(t.x, worldView->debugMousePositionAlign());
                }
                if(ZFBitTest(_ctrl, _Ctrl_move_y)) {
                    _valueAlign(t.y, worldView->debugMousePositionAlign());
                }
            }
        }
        _moveCtrl->p2_position(t);
    }

    void _rotateCtrlAttach(void) {
        _rotateValueBak = _target->p2_rotationCur();
    }
    void _rotateCtrlDetach(void) {
        _rotateValueBak = P2_MAX();
        P2WorldView *worldView = zfcast(P2WorldView *, _target->p2_ownerWorld());
        if(worldView && worldView->debugMouseRotationAlign() > 0) {
            zffloat t = _target->p2_rotationCur();
            _valueAlign(t, worldView->debugMouseRotationAlign());
            if(t != _target->p2_rotationCur()) {
                _target->p2_rotation(t);
            }
        }
        _target->p2_rotationVelocity(0);
    }
    void _rotateCtrlUpdate(ZF_IN zffloat dx, ZF_IN zffloat dy) {
        if((dx == 0 && dy == 0) || (dx == _dx && dy == _dy)) {
            return;
        }
        zffloat ux = -dx, uy = -dy;
        zffloat vx = -_dx, vy = -_dy;
        zffloat rot = -zfm_atan2(ux * vy - uy * vx, ux * vx + uy * vy) * 180 / zfm_PI();
        zffloat t = _rotateValueBak + rot;
        {
            P2WorldView *worldView = zfcast(P2WorldView *, _target->p2_ownerWorld());
            if(worldView && worldView->debugMouseRotationAlign() > 0) {
                _valueAlign(t, worldView->debugMouseRotationAlign());
            }
        }
        _target->p2_rotation(t);
    }

    void _valueAlign(ZF_IN_OUT zffloat &v, ZF_IN zffloat align) {
        zffloat t = zfm_fmod(v, align);
        zffloat d = align / 5;
        if(t >= 0) {
            if(t <= d) {
                v -= t;
            }
            else if(t >= align - d) {
                v += align - t;
            }
        }
        else {
            if(t >= -d) {
                v -= t;
            }
            else if(t <= -(align - d)) {
                v += align + t;
            }
        }
    }

protected:
    zfoverride
    virtual void objectOnInitFinish(void) {
        zfsuper::objectOnInitFinish();
        this->viewUIEnableTree(zffalse);
    }
    zfoverride
    virtual void objectOnDeallocPrepare(void) {
        this->target(zfnull);
        zfsuper::objectOnDeallocPrepare();
    }
    virtual void viewFrame(ZF_IN const ZFUIRect &v) {
        zfsuper::viewFrame(v); // zfzfzf
    }
    zfoverride
    virtual void layoutOnMeasure(
            ZF_OUT ZFUISize &ret
            , ZF_IN const ZFUISize &sizeHint
            , ZF_IN const ZFUISizeParam &sizeParam
            ) {
        if(!_target) {
            return;
        }
        _move_c->layoutMeasure(ZFUISizeInvalid(), ZFUISizeParamWrapWrap());
        _move_t->layoutMeasure(ZFUISizeInvalid(), ZFUISizeParamWrapWrap());
        const ZFUISize &cs = _move_c->layoutMeasuredSize();
        ret.width = ret.height = zfmMax(cs.width, cs.height) + _move_t->layoutMeasuredSize().height * 2;
    }
    zfoverride
    virtual void layoutOnLayout(ZF_IN const ZFUIRect &bounds) {
        if(!_target) {
            return;
        }
        _move_c->layoutMeasure(ZFUISizeInvalid(), ZFUISizeParamWrapWrap());
        _move_t->layoutMeasure(ZFUISizeInvalid(), ZFUISizeParamWrapWrap());
        _rotate_rt->layoutMeasure(ZFUISizeInvalid(), ZFUISizeParamWrapWrap());
        _cs = zfmMax(_move_c->layoutMeasuredSize().width, _move_c->layoutMeasuredSize().height);
        _ms = _move_t->layoutMeasuredSize();
        _rs = zfmMax(_rotate_rt->layoutMeasuredSize().width, _rotate_rt->layoutMeasuredSize().height);
        ZFUIPoint c = ZFUIRectGetCenter(bounds);

        _move_c->viewFrame(ZFUIRectCreate(
                    c.x - _cs / 2
                    , c.y - _cs / 2
                    , _cs
                    , _cs
                    ));
        _move_l->viewFrame(ZFUIRectCreate(
                    c.x - _cs / 2 - _ms.height / 2 - _ms.width / 2
                    , c.y - _ms.height / 2
                    , _ms.width
                    , _ms.height
                    ));
        _move_r->viewFrame(ZFUIRectCreate(
                    c.x + _cs / 2 + _ms.height / 2 - _ms.width / 2
                    , c.y - _ms.height / 2
                    , _ms.width
                    , _ms.height
                    ));
        _move_t->viewFrame(ZFUIRectCreate(
                    c.x - _ms.width / 2
                    , c.y - _cs / 2 - _ms.height
                    , _ms.width
                    , _ms.height
                    ));
        _move_b->viewFrame(ZFUIRectCreate(
                    c.x - _ms.width / 2
                    , c.y + _cs / 2
                    , _ms.width
                    , _ms.height
                    ));

        _rotate_lt->viewFrame(ZFUIRectCreate(
                    c.x - _cs / 2 - _rs
                    , c.y - _cs / 2 - _rs
                    , _rs
                    , _rs
                    ));
        _rotate_rt->viewFrame(ZFUIRectCreate(
                    c.x + _cs / 2
                    , c.y - _cs / 2 - _rs
                    , _rs
                    , _rs
                    ));
        _rotate_rb->viewFrame(ZFUIRectCreate(
                    c.x + _cs / 2
                    , c.y + _cs / 2
                    , _rs
                    , _rs
                    ));
        _rotate_lb->viewFrame(ZFUIRectCreate(
                    c.x - _cs / 2 - _rs
                    , c.y + _cs / 2
                    , _rs
                    , _rs
                    ));
    }
};

zfclass _ZFP_I_P2DebugMouse : zfextend ZFUIView {
    ZFOBJECT_DECLARE(_ZFP_I_P2DebugMouse, ZFUIView)
public:
    P2World *world;
private:
    zfautoT<_ZFP_I_P2DebugMouseCtrl> _ctrl;
protected:
    zfoverride
    virtual void objectOnDeallocPrepare(void) {
        _ctrlDetach();
        zfsuper::objectOnDeallocPrepare();
    }
    zfoverride
    virtual void viewEventOnMouseEvent(ZF_IN ZFUIMouseEvent *mouseEvent) {
        if(_ctrl && !_ctrl->update(mouseEvent->mouseAction, mouseEvent->mousePoint.x, mouseEvent->mousePoint.y)) {
            _ctrlDetach();
        }
        if(!_ctrl && mouseEvent->mouseAction == v_ZFUIMouseAction::e_Down) {
            ZFLISTENER_0(impl) {
                zfargs.eventFiltered(zftrue);
                zfargs.result(zfargs.param0());
            } ZFLISTENER_END()
            zfautoT<P2Shape> touched = world->p2_overlapTest(
                    P2UIRectToAABB(world, ZFUIRectCreate(mouseEvent->mousePoint, ZFUISizeZero()))
                    , impl
                    );
            if(touched) {
                _ctrlAttach(touched->p2_ownerBody());
            }
        }
    }
private:
    void _ctrlAttach(ZF_IN P2Body *body) {
        _ctrl = zfobj<_ZFP_I_P2DebugMouseCtrl>();
        _ctrl->target(body);
        this->child(_ctrl);
        _ctrl->targetPosUpdate();

        this->world->observerAdd(P2World::E_P2BodyMoveEvent(), ZFCallbackForFunc(zfself::_bodyOnMove));
        this->world->observerAdd(P2World::E_P2UIUpdate(), ZFCallbackForFunc(zfself::_uiOnUpdate));
    }
    void _ctrlDetach(void) {
        if(_ctrl) {
            _ctrl->removeFromParent();
            _ctrl->target(zfnull);
            _ctrl = zfnull;
        }

        this->world->observerRemove(P2World::E_P2BodyMoveEvent(), ZFCallbackForFunc(zfself::_bodyOnMove));
        this->world->observerRemove(P2World::E_P2UIUpdate(), ZFCallbackForFunc(zfself::_uiOnUpdate));
    }
private:
    static void _bodyOnMove(ZF_IN const ZFArgs &zfargs) {
        _ZFP_I_P2DebugMouse *d = zfargs.sender()->objectTag(zftext("_ZFP_I_P2DebugMouse"));
        if(d && d->_ctrl) {
            P2Body *target = d->_ctrl->target();
            if(target) {
                P2BodyMoveEvent *event = zfargs.param0();
                for(zfindex i = event->p2_moveEventList.count() - 1; i != zfindexMax(); --i) {
                    if(event->p2_moveEventList[i].p2_body == target) {
                        d->_ctrl->targetPosUpdate();
                        break;
                    }
                }
            }
        }
    }
    static void _uiOnUpdate(ZF_IN const ZFArgs &zfargs) {
        _ZFP_I_P2DebugMouse *d = zfargs.sender()->objectTag(zftext("_ZFP_I_P2DebugMouse"));
        if(d && d->_ctrl) {
            d->_ctrl->targetPosUpdate();
        }
    }
};

static void _ZFP_P2DebugMouseAttach(ZF_IN P2WorldView *worldView) {
    _ZFP_I_P2DebugMouse *d = worldView->objectTag("_ZFP_I_P2DebugMouse");
    if(!d) {
        zfobj<_ZFP_I_P2DebugMouse> t;
        t->world = zfcast(P2World *, worldView);
        worldView->objectTag("_ZFP_I_P2DebugMouse", t);
        worldView->internalFgView(t)->sizeFill();
    }
}
static void _ZFP_P2DebugMouseDetach(ZF_IN P2WorldView *worldView) {
    zfautoT<_ZFP_I_P2DebugMouse> d = worldView->objectTagRemoveAndGet("_ZFP_I_P2DebugMouse");
    if(d) {
        d->removeFromParent();
    }
}

ZFPROPERTY_ON_ATTACH_DEFINE(P2WorldView, zfbool, debugMouse) {
    if(propertyValue) {
        _ZFP_P2DebugMouseAttach(this);
    }
    else {
        _ZFP_P2DebugMouseDetach(this);
    }
}
ZFPROPERTY_ON_DETACH_DEFINE(P2WorldView, zfbool, debugMouse) {
    _ZFP_P2DebugMouseDetach(this);
}

ZF_NAMESPACE_GLOBAL_END

