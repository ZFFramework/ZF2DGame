/**
 * @file P2TileView.h
 * @brief tile view util
 */

#ifndef _ZFI_P2TileView_h_
#define _ZFI_P2TileView_h_

#include "Phyx2D.h"
ZF_NAMESPACE_GLOBAL_BEGIN

zfclassFwd _ZFP_P2TileViewPrivate;
/**
 * @brief tile view
 *
 * auto render tiles by tileImage,
 * auto create polygon shapes according tiles
 */
zfclass ZFLIB_ZF2DGame P2TileView : zfextend ZFUIDrawableView {
    ZFOBJECT_DECLARE(P2TileView, ZFUIDrawableView)

public:
    /**
     * @brief the size of each tile
     *
     * used to automatically create #P2Body::p2_shape,
     * must be updated before #tileCreate,
     * set to 0 to disable auto create of #P2Body::p2_shape
     */
    ZFPROPERTY_ASSIGN(ZFUISize, tileSize, ZFUISizeCreate(1))
    /**
     * @brief the tile image
     *
     * the tile image must with this section layout:
     * @code
     *    outer corner  | inner corner
     *   +----+----+----+----+----+
     *   | lt | t  | rt | rb | lb |
     *   +----+----+----+----+----+
     *   | l  | c  | r  | rt | lt |
     *   +----+----+----+----+----+
     *   | lb | b  | rb |    |    |
     *   +----+----+----+----+----+
     * @endcode
     */
    ZFPROPERTY_RETAIN(zfanyT<ZFUIImage>, tileImage)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfanyT<ZFUIImage>, tileImage)

    /** @brief tile width, must not manually modify */
    ZFPROPERTY_ASSIGN(zfuint, tileWidth)
    /** @brief tile height, must not manually modify */
    ZFPROPERTY_ASSIGN(zfuint, tileHeight)
    /**
     * @brief index of empty tiles, must not manually modify
     *
     * index ordered from left bottom (left to right, bottom to top)
     */
    ZFPROPERTY_ASSIGN(ZFCoreArray<zfindex>, tileHoles)

public:
    /**
     * @brief create the tile view
     *
     * you must ensure the tileHoles logical valid\n
     * \n
     * #tileShapeUpdate would be automatically called to update shapes
     */
    ZFMETHOD_DECLARE_3(void, tileCreate
            , ZFMP_IN(zfuint, tileWidth)
            , ZFMP_IN(zfuint, tileHeight)
            , ZFMP_IN_OPT(const ZFCoreArray<zfindex> &, tileHoles, zfnull)
            )
    /**
     * @brief set tile on or off, at specified postion
     *
     * x and y may exceeds range [0, width_or_height),
     * the #tileWidth, #tileHeight, #tileHoles would be updated automatically
     * \n
     * -  when new tile set on below position 0 causing width expanded, xOffset would be set to negative value
     * -  when tile set off below position 0 causing width shrinked, xOffset would be set to positive value
     *
     * note, modifying tile is expensive,
     * usually designed for editor only,
     * and #tileShapeUpdate must be explicitly called if necessary
     */
    ZFMETHOD_DECLARE_5(void, tileSet
            , ZFMP_IN(zfint, x)
            , ZFMP_IN(zfint, y)
            , ZFMP_IN(zfbool, on)
            , ZFMP_OUT_OPT(zfint *, xOffset, zfnull)
            , ZFMP_OUT_OPT(zfint *, yOffset, zfnull)
            )
    /** @brief get tile at specified postion */
    ZFMETHOD_DECLARE_2(zfbool, tileGet
            , ZFMP_IN(zfuint, x)
            , ZFMP_IN(zfuint, y)
            )

    /**
     * @brief explicitly update #P2Body::p2_shape according to current tiles
     *
     * note:
     * -  #P2Body::p2_shapeRemoveAll would be called before updating shapes
     * -  shapes' origin point would be placed at left bottom of this view
     *
     * see also #tileSize
     */
    ZFMETHOD_DECLARE_0(void, tileShapeUpdate)

protected:
    zfoverride
    virtual void objectOnInit(void);
    zfoverride
    virtual void objectOnDealloc(void);
    zfoverride
    virtual void onDraw(void);

private:
    _ZFP_P2TileViewPrivate *d;
};

/**
 * @brief util to merge tiles polygon
 *
 * tiles contains each tile (x, y) ordered from l2r b2t
 * (e.g. x0,y0, x1,y1, ...)\n
 * result contains each merged polygons' vertex, clockwise
 */
ZFMETHOD_FUNC_DECLARE_2(ZFLIB_ZF2DGame, ZFCoreArray<ZFCoreArray<zfuint> >, P2TilePolygonMerge
        , ZFMP_IN(const ZFCoreArray<zfuint> &, tiles)
        , ZFMP_IN_OPT(zfuint, maxVertex, P2ShapePolygon::p2_maxVertex())
        )

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_P2TileView_h_

