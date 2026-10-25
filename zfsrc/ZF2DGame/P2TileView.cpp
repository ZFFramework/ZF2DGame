#include "P2TileView.h"
#include "Phyx2D.h"

ZF_NAMESPACE_GLOBAL_BEGIN

zfclassPOD _ZFP_P2TileViewDrawData {
public:
    /*
     * tileImage's sliced index
     *
     * the tileImage's original section layout:
     *    outer corner  | inner corner
     *   +----+----+----+----+----+
     *   | lt | t  | rt | rb | lb |
     *   +----+----+----+----+----+
     *   | l  | c  | r  | rt | lt |
     *   +----+----+----+----+----+
     *   | lb | b  | rb |    |    |
     *   +----+----+----+----+----+
     *
     * each section would further be splitted to 4 slices,
     * to achieve 47 tiles logic (all possible combinations for 2d tiles),
     * the sx/sy index ordered by l2r and t2b within the tileImage
     *       0 1   2 3   4 5   6 7   8 9   x
     *     +-----+-----+-----+-----+-----+->
     *   0 | lt  |  t  |  rt |     |     |
     *   1 |     |     |     |  rb | lb  |
     *     +-----+-----+-----+-----+-----+
     *   2 |  l  |  c  |  r  |  rt | lt  |
     *   3 |     |     |     |     |     |
     *     +-----+-----+-----+-----+-----+
     *   4 |     |     |     |     |     |
     *   5 | lb  |  b  |  rb |     |     |
     *     +-----+-----+-----+-----+-----+
     *     |
     *   y v
     */
    zfuint sx;
    zfuint sy;

    /*
     * the grid xy in owner tile view,
     * each tile would be splitted to 4 grid,
     * it's the xy of the splitted grids,
     * ordered by l2r and b2t
     *
     *   y ^
     *     |
     *     +-----+-----+-----+
     *   3 |  3  |  4  |  5  |
     *   2 |     |     |     |
     *     +-----+-----+-----+
     *   1 |  0  |  1  |  2  |
     *   0 |     |     |     |
     *     +-----+-----+-----+->
     *       0 1   2 3   4 5   x
     */
    zfuint gx;
    zfuint gy;
};
ZFCORE_POD_DECLARE_NO_COMPARER(_ZFP_P2TileViewDrawData)
ZFOUTPUT_TYPE(_ZFP_P2TileViewDrawData, {
    zfstringAppend(s, "(%s,%s %s,%s)"
            , v.sx, v.sy
            , v.gx, v.gy
            );
})
zfclassNotPOD _ZFP_P2TileViewPrivate {
public:
    zfbool *cache; // cacheWidth * cacheHeight, from l2r b2t
    zfuint cacheWidth;
    zfuint cacheHeight;
    ZFCoreArray<_ZFP_P2TileViewDrawData> drawData;
public:
    _ZFP_P2TileViewPrivate(void)
    : cache(zfnull)
    , cacheWidth(0)
    , cacheHeight(0)
    {
    }
    ~_ZFP_P2TileViewPrivate(void) {
        if(this->cache) {
            zffree(this->cache);
        }
    }
public:
    zfbool &tile(ZF_IN zfuint x, ZF_IN zfuint y) {
        return this->cache[x + y * this->cacheWidth];
    }
    void dataUpdate(
            ZF_IN P2TileView *owner
            , ZF_OUT_OPT zfint *xOffset = zfnull
            , ZF_OUT_OPT zfint *yOffset = zfnull
            );
    void dataExpand(
            ZF_IN P2TileView *owner
            , ZF_IN zfint x
            , ZF_IN zfint y
            , ZF_OUT zfint *xOffset
            , ZF_OUT zfint *yOffset
            );
    void rebuild(ZF_IN P2TileView *owner) {
        this->rebuildShape(owner);
        this->rebuildDrawData(owner);
        owner->drawRequest();
    }
    void rebuildShape(ZF_IN P2TileView *owner);
    void rebuildDrawData(ZF_IN P2TileView *owner);
};

ZFOBJECT_REGISTER(P2TileView)

ZFPROPERTY_ON_UPDATE_DEFINE(P2TileView, zfanyT<ZFUIImage>, tileImage) {
    this->drawRequest();
}

ZFMETHOD_DEFINE_3(P2TileView, void, tileCreate
        , ZFMP_IN(zfuint, tileWidth)
        , ZFMP_IN(zfuint, tileHeight)
        , ZFMP_IN_OPT(const ZFCoreArray<zfindex> &, tileHoles, zfnull)
        ) {
    if(tileWidth == 0 || tileHeight == 0) {
        if(!tileHoles.isEmpty()) {
            ZFCoreCriticalMessageTrim(
                    "[P2TileView] invalid param, tileCreate(%s, %s, %s)"
                    , tileWidth
                    , tileHeight
                    , tileHoles
                    );
            return;
        }
        this->tileWidth(0);
        this->tileHeight(0);
        this->tileHoles().removeAll();
        d->dataUpdate(this);
        d->rebuild(this);
    }
    else {
        this->tileWidth(tileWidth);
        this->tileHeight(tileHeight);
        this->tileHoles(tileHoles);
        d->dataUpdate(this);
        d->rebuild(this);
    }
}
ZFMETHOD_DEFINE_5(P2TileView, void, tileSet
        , ZFMP_IN(zfint, x)
        , ZFMP_IN(zfint, y)
        , ZFMP_IN(zfbool, on)
        , ZFMP_OUT_OPT(zfint *, xOffset, zfnull)
        , ZFMP_OUT_OPT(zfint *, yOffset, zfnull)
        ) {
    if(x < 0 || y < 0) {
        if(on) {
            d->dataExpand(this, x, y, xOffset, yOffset);
            d->rebuild(this);
        }
    }
    else if(x >= (zfint)d->cacheWidth || y >= (zfint)d->cacheHeight) {
        if(on) {
            d->dataExpand(this, x, y, xOffset, yOffset);
            d->rebuild(this);
        }
    }
    else {
        if(d->tile(x, y) != on) {
            d->tile(x, y) = on;
            zfindex index = x + y * this->tileWidth();
            if(on) {
                this->tileHoles().removeElement(index);
            }
            else {
                if(!this->tileHoles().isContain(index)) {
                    this->tileHoles().add(index);
                    this->tileHoles().sort();
                }
            }
            d->dataUpdate(this, xOffset, yOffset);
            d->rebuild(this);
        }
    }
}
ZFMETHOD_DEFINE_2(P2TileView, zfbool, tileGet
        , ZFMP_IN(zfuint, x)
        , ZFMP_IN(zfuint, y)
        ) {
    if(x < this->tileWidth() && y < this->tileHeight()) {
        return d->tile(x, y);
    }
    else {
        return zffalse;
    }
}

ZFMETHOD_DEFINE_0(P2TileView, void, tileShapeUpdate) {
    d->rebuildShape(this);
}

void P2TileView::objectOnInit(void) {
    zfsuper::objectOnInit();
    d = zfpoolNew(_ZFP_P2TileViewPrivate);
}
void P2TileView::objectOnDealloc(void) {
    zfpoolDelete(d);
    zfsuper::objectOnDealloc();
}
void P2TileView::onDraw(void) {
    zfsuper::onDraw();
    if(this->width() <= 0 || this->height() <= 0
            || d->drawData.isEmpty()
            || d->cacheWidth <= 0
            || d->cacheHeight <= 0
            || this->tileImage() == zfnull
            ) {
        return;
    }
    zfautoT<ZFUIImage> img = this->tileImage()->imageState();
    ZFUIRect srcFrame;
    srcFrame.width = img->imageSize().width / 5 / 2;
    srcFrame.height = img->imageSize().height / 3 / 2;
    ZFUIRect dstFrame;
    dstFrame.width = this->width() / d->cacheWidth / 2;
    dstFrame.height = this->height() / d->cacheHeight / 2;
    zffloat height = this->height();
    void *ctx = ZFUIDraw::beginForView(this);
    for(zfindex i = d->drawData.count() - 1; i != zfindexMax(); --i) {
        _ZFP_P2TileViewDrawData &data = d->drawData[i];
        srcFrame.x = data.sx * srcFrame.width;
        srcFrame.y = data.sy * srcFrame.height;
        dstFrame.x = data.gx * dstFrame.width;
        dstFrame.y = height - (data.gy + 1) * dstFrame.height;
        ZFUIDraw::drawImage(
                ctx
                , img
                , srcFrame
                , dstFrame
                );
    }
    ZFUIDraw::endForView(ctx);
}

// ============================================================
void _ZFP_P2TileViewPrivate::dataUpdate(
        ZF_IN P2TileView *owner
        , ZF_OUT_OPT zfint *xOffset /* = zfnull */
        , ZF_OUT_OPT zfint *yOffset /* = zfnull */
        ) {
    this->cacheWidth = owner->tileWidth();
    this->cacheHeight = owner->tileHeight();
    zfindex count = this->cacheWidth * this->cacheHeight;
    this->cache = (zfbool *)zfrealloc(this->cache, sizeof(zfbool) * count);
    zfmemset(this->cache, 1, sizeof(zfbool) * count);
    for(zfindex i = owner->tileHoles().count() - 1; i != zfindexMax(); --i) {
        zfindex index = owner->tileHoles()[i];
        if(index < count) {
            this->cache[index] = zffalse;
        }
    }
    if(xOffset) {*xOffset = 0;}
    if(yOffset) {*yOffset = 0;}

    zfint x0 = 0;
    zfint x1 = this->cacheWidth - 1;
    zfint y0 = 0;
    zfint y1 = this->cacheHeight - 1;
    while(y0 <= y1) {
        zfbool exist = zffalse;
        for(zfint ix = x0; ix <= x1; ++ix) {
            if(this->cache[ix + y0 * this->cacheWidth]) {
                exist = zftrue;
                break;
            }
        }
        if(exist) {
            break;
        }
        else {
            ++y0;
        }
    }
    while(y0 <= y1) {
        zfbool exist = zffalse;
        for(zfint ix = x0; ix <= x1; ++ix) {
            if(this->cache[ix + y1 * this->cacheWidth]) {
                exist = zftrue;
                break;
            }
        }
        if(exist) {
            break;
        }
        else {
            --y1;
        }
    }
    while(x0 <= x1) {
        zfbool exist = zffalse;
        for(zfint iy = y0; iy <= y1; ++iy) {
            if(this->cache[x0 + iy * this->cacheWidth]) {
                exist = zftrue;
                break;
            }
        }
        if(exist) {
            break;
        }
        else {
            ++x0;
        }
    }
    while(x0 <= x1) {
        zfbool exist = zffalse;
        for(zfint iy = y0; iy <= y1; ++iy) {
            if(this->cache[x1 + iy * this->cacheWidth]) {
                exist = zftrue;
                break;
            }
        }
        if(exist) {
            break;
        }
        else {
            --x1;
        }
    }
    if(x0 > x1 || y0 > y1) {
        zffree(this->cache);
        this->cacheWidth = 0;
        this->cacheHeight = 0;
        owner->tileWidth(0);
        owner->tileHeight(0);
        owner->tileHoles().removeAll();
        return;
    }
    else if(zftrue
            && x0 == 0 && x1 == (zfint)this->cacheWidth - 1
            && y0 == 0 && y1 == (zfint)this->cacheHeight - 1
            ) {
        return;
    }
    zfuint tmpWidth = x1 + 1 - x0;
    zfuint tmpHeight = y1 + 1 - y0;
    zfindex tmpCount = tmpWidth * tmpHeight;
    zfbool *tmp = (zfbool *)zfmalloc(sizeof(zfbool) * tmpCount);
    ZFCoreArray<zfindex> tmpTileHoles;
    for(zfint iy = y0; iy <= y1; ++iy) {
        for(zfint ix = x0; ix <= x1; ++ix) {
            zfindex index = ix + iy * tmpWidth;
            tmp[index] = this->cache[ix + iy * this->cacheWidth];
            if(!tmp[index]) {
                tmpTileHoles.add(index);
            }
        }
    }
    zffree(this->cache);
    this->cache = tmp;
    this->cacheWidth = tmpWidth;
    this->cacheHeight = tmpHeight;
    if(xOffset) {*xOffset = x0;}
    if(yOffset) {*yOffset = y0;}
    owner->tileWidth(tmpWidth);
    owner->tileHeight(tmpHeight);
    owner->tileHoles(tmpTileHoles);
}
void _ZFP_P2TileViewPrivate::dataExpand(
        ZF_IN P2TileView *owner
        , ZF_IN zfint x
        , ZF_IN zfint y
        , ZF_OUT zfint *xOffset
        , ZF_OUT zfint *yOffset
        ) {
    if(xOffset) {*xOffset = 0;}
    if(yOffset) {*yOffset = 0;}
    zfuint dx = 0;
    zfuint dy = 0;
    zfuint tmpWidth = this->cacheWidth;
    zfuint tmpHeight = this->cacheHeight;
    if(x < 0) {
        dx = -x;
        tmpWidth -= x;
        if(xOffset) {*xOffset = x;}
    }
    else if (x >= (zfint)this->cacheWidth) {
        tmpWidth = x + 1;
    }
    if(y < 0) {
        dy = -y;
        tmpHeight -= y;
        if(yOffset) {*yOffset = y;}
    }
    else if(y >= (zfint)this->cacheHeight) {
        tmpHeight = y + 1;
    }
    zfbool *tmp = (zfbool *)zfmalloc(sizeof(zfbool) * tmpWidth * tmpHeight);
    zfmemset(tmp, 0, sizeof(zfbool) * tmpWidth * tmpHeight);
    if(this->cache) {
        for(zfuint iy = 0; iy < tmpHeight; ++iy) {
            for(zfuint ix = 0; ix < tmpWidth; ++ix) {
                if(zftrue
                        && ix >= dx && ix - dx < this->cacheWidth
                        && iy >= dy && iy - dy < this->cacheHeight
                        ) {
                    tmp[ix + iy * tmpWidth] = this->cache[(ix - dx) + (iy - dy) * this->cacheWidth];
                }
            }
        }
    }
    tmp[(x + dx) * (y + dy) * tmpWidth] = zftrue;
    ZFCoreArray<zfindex> tileHoles;
    for(zfuint iy = 0; iy < tmpHeight; ++iy) {
        for(zfuint ix = 0; ix < tmpWidth; ++ix) {
            zfindex index = ix + iy * tmpWidth;
            if(!tmp[index]) {
                tileHoles.add(index);
            }
        }
    }

    zfbool *cacheTmp = this->cache;
    this->cache = tmp;
    this->cacheWidth = tmpWidth;
    this->cacheHeight = tmpHeight;
    if(cacheTmp) {
        zffree(cacheTmp);
    }
    owner->tileWidth(tmpWidth);
    owner->tileHeight(tmpHeight);
    owner->tileHoles(tileHoles);
}

void _ZFP_P2TileViewPrivate::rebuildShape(ZF_IN P2TileView *owner) {
    if(owner->tileSize().width <= 0 || owner->tileSize().height <= 0) {
        return;
    }
    P2Body *body = zfcast(P2Body *, owner);
    body->p2_shapeRemoveAll();

    ZFCoreArray<zfuint> tiles;
    for(zfuint y = 0; y < owner->tileHeight(); ++y) {
        for(zfuint x = 0; x < owner->tileWidth(); ++x) {
            if(owner->tileGet(x, y)) {
                tiles.add(x);
                tiles.add(y);
            }
        }
    }
    ZFCoreArray<ZFCoreArray<zfuint> > polygons = P2TilePolygonMerge(tiles);
    if(polygons.isEmpty()) {
        return;
    }

    ZFUISize const &tileSize = owner->tileSize();
    for(zfindex iPolygon = 0; iPolygon < polygons.count(); ++iPolygon) {
        ZFCoreArray<zfuint> const &points = polygons[iPolygon];
        ZFCoreArray<ZFUIPoint> shapePoints;
        for(zfindex i = 0; i < points.count(); i += 2) {
            shapePoints.add(ZFUIPointCreate(
                        points[i] * tileSize.width
                        , points[i + 1] * tileSize.height
                        ));
        }
        body->p2_shape(zfobj<P2ShapePolygon>()
                ->c_p2_polygon(shapePoints)
                );
    }
}

void _ZFP_P2TileViewPrivate::rebuildDrawData(ZF_IN P2TileView *owner) {
    if(this->cache == zfnull) {
        this->drawData.removeAll();
        return;
    }
    this->drawData.resize((this->cacheWidth * this->cacheHeight - owner->tileHoles().count()) * 4);
    zfindex iDraw = 0;
    for(zfuint y = 0; y < this->cacheHeight; ++y) {
        for(zfuint x = 0; x < this->cacheWidth; ++x) {
            if(!this->tile(x, y)) {
                continue;
            }
            zfbool t_lt = (x > 0 && y < this->cacheHeight - 1) ? this->tile(x - 1, y + 1) : zffalse;
            zfbool t_t = (y < this->cacheHeight - 1) ? this->tile(x, y + 1) : zffalse;
            zfbool t_rt = (x < this->cacheWidth - 1 && y < this->cacheHeight - 1) ? this->tile(x + 1, y + 1) : zffalse;
            zfbool t_l = (x > 0) ? this->tile(x - 1, y) : zffalse;
            zfbool t_r = (x < this->cacheWidth - 1) ? this->tile(x + 1, y) : zffalse;
            zfbool t_lb = (x > 0 && y > 0) ? this->tile(x - 1, y - 1) : zffalse;
            zfbool t_b = (y > 0) ? this->tile(x, y - 1) : zffalse;
            zfbool t_rb = (x < this->cacheWidth - 1 && y > 0) ? this->tile(x + 1, y - 1) : zffalse;
            { // lt
                if(this->drawData.count() < iDraw) {this->drawData.add(_ZFP_P2TileViewDrawData());}
                _ZFP_P2TileViewDrawData &t = this->drawData[iDraw++];
                t.gx = x * 2;
                t.gy = y * 2 + 1;
                if(zffalse) {}
                else if(t_l && t_lt && t_t) {t.sx = 2; t.sy = 2;}
                else if(t_l && t_lt && !t_t) {t.sx = 2; t.sy = 0;}
                else if(t_l && !t_lt && t_t) {t.sx = 8; t.sy = 2;}
                else if(!t_l && t_lt && t_t) {t.sx = 0; t.sy = 2;}
                else if(t_l && !t_lt && !t_t) {t.sx = 2; t.sy = 0;}
                else if(!t_l && t_lt && !t_t) {t.sx = 0; t.sy = 0;}
                else if(!t_l && !t_lt && t_t) {t.sx = 0; t.sy = 2;}
                else if(!t_l && !t_lt && !t_t) {t.sx = 0; t.sy = 0;}
            }
            { // rt
                if(this->drawData.count() < iDraw) {this->drawData.add(_ZFP_P2TileViewDrawData());}
                _ZFP_P2TileViewDrawData &t = this->drawData[iDraw++];
                t.gx = x * 2 + 1;
                t.gy = y * 2 + 1;
                if(zffalse) {}
                else if(t_r && t_rt && t_t) {t.sx = 3; t.sy = 2;}
                else if(t_r && t_rt && !t_t) {t.sx = 3; t.sy = 0;}
                else if(t_r && !t_rt && t_t) {t.sx = 7; t.sy = 2;}
                else if(!t_r && t_rt && t_t) {t.sx = 5; t.sy = 2;}
                else if(t_r && !t_rt && !t_t) {t.sx = 3; t.sy = 0;}
                else if(!t_r && t_rt && !t_t) {t.sx = 5; t.sy = 0;}
                else if(!t_r && !t_rt && t_t) {t.sx = 5; t.sy = 2;}
                else if(!t_r && !t_rt && !t_t) {t.sx = 5; t.sy = 0;}
            }
            { // lb
                if(this->drawData.count() < iDraw) {this->drawData.add(_ZFP_P2TileViewDrawData());}
                _ZFP_P2TileViewDrawData &t = this->drawData[iDraw++];
                t.gx = x * 2;
                t.gy = y * 2;
                if(zffalse) {}
                else if(t_l && t_lb && t_b) {t.sx = 2; t.sy = 3;}
                else if(t_l && t_lb && !t_b) {t.sx = 2; t.sy = 5;}
                else if(t_l && !t_lb && t_b) {t.sx = 8; t.sy = 1;}
                else if(!t_l && t_lb && t_b) {t.sx = 0; t.sy = 3;}
                else if(t_l && !t_lb && !t_b) {t.sx = 2; t.sy = 5;}
                else if(!t_l && t_lb && !t_b) {t.sx = 0; t.sy = 5;}
                else if(!t_l && !t_lb && t_b) {t.sx = 0; t.sy = 3;}
                else if(!t_l && !t_lb && !t_b) {t.sx = 0; t.sy = 5;}
            }
            { // rb
                if(this->drawData.count() < iDraw) {this->drawData.add(_ZFP_P2TileViewDrawData());}
                _ZFP_P2TileViewDrawData &t = this->drawData[iDraw++];
                t.gx = x * 2 + 1;
                t.gy = y * 2;
                if(zffalse) {}
                else if(t_r && t_rb && t_b) {t.sx = 3; t.sy = 3;}
                else if(t_r && t_rb && !t_b) {t.sx = 3; t.sy = 5;}
                else if(t_r && !t_rb && t_b) {t.sx = 7; t.sy = 1;}
                else if(!t_r && t_rb && t_b) {t.sx = 5; t.sy = 3;}
                else if(t_r && !t_rb && !t_b) {t.sx = 3; t.sy = 5;}
                else if(!t_r && t_rb && !t_b) {t.sx = 5; t.sy = 5;}
                else if(!t_r && !t_rb && t_b) {t.sx = 5; t.sy = 3;}
                else if(!t_r && !t_rb && !t_b) {t.sx = 5; t.sy = 5;}
            }
        }
    }
    if(this->drawData.count() > iDraw) {
        this->drawData.remove(iDraw, zfindexMax());
    }
}

ZF_NAMESPACE_GLOBAL_END

