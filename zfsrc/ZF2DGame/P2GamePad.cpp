#include "P2GamePad.h"

ZF_NAMESPACE_GLOBAL_BEGIN

ZFENUM_DEFINE(P2GamePadButton)

ZF_NAMESPACE_BEGIN(ZFGlobalEvent)
ZFEVENT_GLOBAL_REGISTER(P2GamePadChange)
ZF_NAMESPACE_END(ZFGlobalEvent)

// ============================================================
zfclassNotPOD _ZFP_P2GamePad {
public:
    enum {
        PAD_MAX = 8,
    };
    zfclassNotPOD Pad {
    public:
        ZFObserver ob;
        zfbool connected;
        zffloat value[v_P2GamePadButton::ZFEnumCount];
    public:
        void valueRemoveAll(void) {
            zfmemset(this->value, 0, sizeof(this->value));
        }
        zffloat &valueRef(ZF_IN P2GamePadButton btn) {
            return this->value[(zfindex)zfm_log2((zffloat)btn)];
        }
    };
public:
    static Pad *pads(void) {
        static Pad d[PAD_MAX];
        return d;
    }
    static ZFCoreArray<zfindex> &indexList(void) {
        static ZFCoreArray<zfindex> d;
        return d;
    }
    static Pad &changeState(ZF_IN zfindex padIndex, ZF_IN zfbool connected) {
        Pad &pad = pads()[padIndex];
        if(pad.connected != connected) {
            pad.connected = connected;
            ZFCoreArray<zfindex> &list = indexList();
            if(connected) {
                list.add(padIndex);
            }
            else {
                list.removeElement(padIndex);
                pad.valueRemoveAll();
            }
            ZFGlobalObserver().observerNotify(ZFGlobalEvent::E_P2GamePadChange());
        }
        return pad;
    }
};

ZF_GLOBAL_INITIALIZER_INIT_WITH_LEVEL(P2GamePadCleanup, ZFLevelZFFrameworkNormal) {
}
ZF_GLOBAL_INITIALIZER_DESTROY(P2GamePadCleanup) {
    _ZFP_P2GamePad::Pad *pads = _ZFP_P2GamePad::pads();
    for(zfindex i = 0; i < _ZFP_P2GamePad::PAD_MAX; ++i) {
        _ZFP_P2GamePad::Pad &pad = pads[i];
        pad.ob.observerRemoveAll();
        pad.connected = zffalse;
        pad.valueRemoveAll();
    }
}
ZF_GLOBAL_INITIALIZER_END(P2GamePadCleanup)

// ============================================================
ZFMETHOD_FUNC_DEFINE_0(zfindex, P2GamePadMax) {
    return _ZFP_P2GamePad::PAD_MAX;
}

ZFMETHOD_FUNC_DEFINE_1(ZFObserver, P2GamePadObserver
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    ZFCoreAssert(padIndex < _ZFP_P2GamePad::PAD_MAX);
    return _ZFP_P2GamePad::pads()[padIndex].ob;
}
ZFMETHOD_FUNC_DEFINE_2(zfbool, P2GamePadPressed
        , ZFMP_IN(P2GamePadButton, btn)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    ZFCoreAssert(padIndex < _ZFP_P2GamePad::PAD_MAX);
    return _ZFP_P2GamePad::pads()[padIndex].value[btn] != 0;
}
ZFMETHOD_FUNC_DEFINE_2(zffloat, P2GamePadValue
        , ZFMP_IN(P2GamePadButton, btn)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    ZFCoreAssert(padIndex < _ZFP_P2GamePad::PAD_MAX);
    return _ZFP_P2GamePad::pads()[padIndex].value[btn];
}

ZFMETHOD_FUNC_DEFINE_0(ZFCoreArray<zfindex>, P2GamePadIndexList) {
    return _ZFP_P2GamePad::indexList();
}

// ============================================================
ZFMETHOD_FUNC_DEFINE_3(void, P2GamePadImpl_notifyButtonValue
        , ZFMP_IN(zfindex, padIndex)
        , ZFMP_IN(P2GamePadButton, btn)
        , ZFMP_IN(zffloat, value)
        ) {
    _ZFP_P2GamePad::Pad &pad = (value
            ? _ZFP_P2GamePad::changeState(padIndex, zftrue)
            : _ZFP_P2GamePad::pads()[padIndex]
            );
    zffloat &valueRef = pad.valueRef(btn);
    if(valueRef != value) {
        valueRef = value;
        pad.ob.observerNotify(zfidentityInvalid(), ZFArgs()
                .param0(zfobj<v_P2GamePadButton>(btn))
                .param1(zfobj<v_zffloat>(value))
                );
    }
}
ZFMETHOD_FUNC_DEFINE_2(void, P2GamePadImpl_notifyPadChange
        , ZFMP_IN(zfindex, padIndex)
        , ZFMP_IN(zfbool, connected)
        ) {
    _ZFP_P2GamePad::changeState(padIndex, connected);
}

ZF_NAMESPACE_GLOBAL_END

