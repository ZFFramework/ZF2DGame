#include "TileView.h"

#include <cmath> // for fmodf

ZF_NAMESPACE_GLOBAL_BEGIN

ZFOBJECT_REGISTER(TileView)

ZFPROPERTY_ON_UPDATE_DEFINE(TileView, zfanyT<ZFUIImage>, tile) {
    if(propertyValue != propertyValueOld) {
        this->drawRequest();
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(TileView, ZFUISize, tileScale) {
    if(propertyValue != propertyValueOld) {
        this->drawRequest();
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(TileView, ZFUISize, tileSpace) {
    if(propertyValue != propertyValueOld) {
        this->drawRequest();
    }
}
ZFMETHOD_DEFINE_0(TileView, const ZFUIPoint &, tileOffset) {
    return _tileOffset;
}
ZFMETHOD_DEFINE_1(TileView, void, tileOffset
        , ZFMP_IN(const ZFUIPoint &, v)
        ) {
    if(_tileOffset != v) {
        _tileOffset = v;
        this->drawRequest();
    }
}
ZFMETHOD_DEFINE_2(TileView, void, tileOffsetStep
        , ZFMP_IN(zffloat, x)
        , ZFMP_IN(zffloat, y)
        ) {
    this->tileOffset(ZFUIPointCreate(_tileOffset.x + x, _tileOffset.y + y));
}

void TileView::onDraw(void) {
    zfsuper::onDraw();
    zfautoT<ZFUIImage> tile = this->tile();
    if(tile) {
        tile = tile->imageState();
    }
    if(!tile || ZFUISizeIsEmpty(tile->imageSize())) {
        return;
    }
    const ZFUIRect &viewFrame = this->viewFrame();
    ZFUIRect tileRect;
    {
        const ZFUISize &tileScale = this->tileScale();
        if(tileScale.width == 0) {
            tileRect.width = viewFrame.width;
        }
        else if(tileScale.width < 0) {
            tileRect.width = -1;
        }
        else {
            tileRect.width = tile->imageSize().width * tileScale.width;
        }
        if(tileScale.height == 0) {
            tileRect.height = viewFrame.height;
        }
        else if(tileScale.height < 0) {
            if(tileRect.width > 0) {
                tileRect.height = tile->imageSize().height * tileRect.width / tile->imageSize().width;
            }
            else {
                ZFUIRect t = ZFUIScaleTypeApply(v_ZFUIScaleType::e_FillCenter, viewFrame, tile->imageSize());
                tileRect.width = t.width;
                tileRect.height = t.height;
            }
        }
        else {
            tileRect.height = tile->imageSize().height * tileScale.height;
        }
        if(tileRect.width < 0 && tileRect.height > 0) {
            tileRect.width = tile->imageSize().width * tileRect.height / tile->imageSize().height;
        }
        if(tileRect.width <= 0 || tileRect.height <= 0) {
            return;
        }
    }
    ZFUIPoint tileOffset = this->tileOffset();
    tileOffset.x = fmodf(tileOffset.x * this->tileOffsetScale().width, viewFrame.width);
    if(tileOffset.x > this->tileSpace().width) {
        tileOffset.x -= tileRect.width + this->tileSpace().width;
    }
    tileOffset.y = fmodf(tileOffset.y * this->tileOffsetScale().height, viewFrame.height);
    if(tileOffset.y > this->tileSpace().height) {
        tileOffset.y -= tileRect.height + this->tileSpace().height;
    }

    void *ctx = ZFUIDraw::beginForView(this);
    for(tileRect.x = tileOffset.x; tileRect.x < viewFrame.width; tileRect.x += tileRect.width) {
        for(tileRect.y = tileOffset.y; tileRect.y < viewFrame.height; tileRect.y += tileRect.height) {
            ZFUIDraw::drawImage(ctx, tile, ZFUIRectZero(), tileRect);
        }
    }
    ZFUIDraw::endForView(ctx);
}

ZF_NAMESPACE_GLOBAL_END

