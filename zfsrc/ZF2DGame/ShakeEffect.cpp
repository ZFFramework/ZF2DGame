#include "ShakeEffect.h"

#include <cmath> // for sin and cos

ZF_NAMESPACE_GLOBAL_BEGIN

ZFOBJECT_REGISTER(ShakeEffect)

void ShakeEffect::aniTimerOnUpdate(ZF_IN zffloat progress) {
    zfsuper::aniTimerOnUpdate(progress);
    _positive = !_positive;
    ZFUIView *target = this->target();
    if(target == zfnull) {
        return;
    }
    zffloat direction = this->direction();
    if(direction == -1) {
        direction = (zffloat)zfmRand(0, 360);
    }
    else if(!_positive) {
        direction = direction + 180;
    }
    zffloat offset = this->offset();
    if(this->damping()) {
        offset *= (1 - progress);
    }
    target->translateX(offset * cos(direction));
    target->translateY(offset * sin(direction));
}

void ShakeEffect::aniOnStop(ZF_IN ZFResultType resultType) {
    ZFUIView *target = this->target();
    if(target) {
        target->translateX(0);
        target->translateY(0);
    }
    zfsuper::aniOnStop(resultType);
}

ZF_NAMESPACE_GLOBAL_END

