#include "P2TileView.h"

#include "ZFCore/ZFSTLWrapper/zfstlvector.h"
#include "ZFCore/ZFSTLWrapper/zfstlmap.h"

ZF_NAMESPACE_GLOBAL_BEGIN

zfclassNotPOD _ZFP_P2TilePolygon {
public:
    static ZFCoreArray<ZFCoreArray<zfuint> > merge(
            ZF_IN const zfuint *tiles
            , ZF_IN zfindex tilesCount
            , ZF_IN zfindex maxVertex
            ) {
        return _merge(tiles, tilesCount, maxVertex);
    }
private:
    typedef zfstlpair<zfuint, zfuint> _Point;

    static zfstlsize _find(ZF_IN const zfstlvector<_Point> &a, ZF_IN const _Point &p) {
        for(zfstlsize i = 0; i < a.size(); ++i) {
            if(a[i].first == p.first && a[i].second == p.second) {
                return i;
            }
        }
        return (zfstlsize)-1;
    }
    static zfbool _exist(ZF_IN const zfstlvector<_Point> &a, ZF_IN const _Point &p) {
        for(zfstlsize i = 0; i < a.size(); ++i) {
            if(a[i].first == p.first && a[i].second == p.second) {
                return zftrue;
            }
        }
        return zffalse;
    }
    static void _uniqueAdd(ZF_IN_OUT zfstlvector<_Point> &a, ZF_IN const _Point &p) {
        if(!_exist(a, p)) {
            a.push_back(p);
        }
    }

    // merge points in same line
    static zfstlvector<_Point> _mergeLine(ZF_IN const zfstlvector<_Point> &raw) {
        zfuint n = (zfuint)raw.size();
        if(n <= 2) {
            return raw;
        }

        zfstlvector<_Point> ret;
        for(zfuint i = 0; i < n; ++i) {
            _Point prev = raw[(i - 1 + n) % n];
            _Point cur = raw[i];
            _Point next = raw[(i + 1) % n];
            zfint dx1 = cur.first - prev.first;
            zfint dy1 = cur.second - prev.second;
            zfint dx2 = next.first - cur.first;
            zfint dy2 = next.second - cur.second;
            if(dx1 * dy2 - dy1 * dx2 == 0 && dx1 * dx2 + dy1 * dy2 > 0) {
                continue;
            }
            ret.push_back(cur);
        }
        return ret;
    }

    static zfbool _isConvex(ZF_IN const zfstlvector<_Point> &polygon) {
        zfstlsize n = polygon.size();
        if(n < 3) {
            return zffalse;
        }
        zfbool hasPositive = zffalse;
        zfbool hasNegative = zffalse;
        for(zfstlsize i = 0; i < n; ++i) {
            const _Point &prev = polygon[(i + n - 1) % n];
            const _Point &cur = polygon[i];
            const _Point &next = polygon[(i + 1) % n];
            zfint dx1 = cur.first - prev.first;
            zfint dy1 = cur.second - prev.second;
            zfint dx2 = next.first - cur.first;
            zfint dy2 = next.second - cur.second;
            zfint cross = dx1 * dy2 - dy1 * dx2;
            if(cross > 0) {
                hasPositive = zftrue;
            }
            else if(cross < 0) {
                hasNegative = zftrue;
            }
            if(hasPositive && hasNegative) {
                return zffalse;
            }
        }
        return zftrue;
    }

    static zfbool _addEdge(ZF_IN_OUT zfstlmap<_Point, _Point> &edges, ZF_IN const _Point &from, ZF_IN const _Point &to) {
        if(edges.find(from) != edges.end()) {
            return zffalse;
        }
        else {
            edges[from] = to;
            return zftrue;
        }
    }

    static zfstlvector<zfstlvector<_Point> > _extractLoops(ZF_IN const zfstlvector<_Point> &cells) {
        zfstlvector<zfstlvector<_Point> > loops;
        if(cells.empty()) {
            return loops;
        }
        // <from, to> edge
        zfstlmap<_Point, _Point> edges;

        for(zfstlsize i = 0; i < cells.size(); ++i) {
            const _Point &c = cells[i];
            zfuint x = c.first;
            zfuint y = c.second;
            if(!_exist(cells, _Point(x - 1, y))) {
                if(!_addEdge(edges, _Point(x, y), _Point(x, y + 1))) {loops.clear(); return loops;}
            }
            if(!_exist(cells, _Point(x + 1, y))) {
                if(!_addEdge(edges, _Point(x + 1, y + 1), _Point(x + 1, y))) {loops.clear(); return loops;}
            }
            if(!_exist(cells, _Point(x, y - 1))) {
                if(!_addEdge(edges, _Point(x + 1, y), _Point(x, y))) {loops.clear(); return loops;}
            }
            if(!_exist(cells, _Point(x, y + 1))) {
                if(!_addEdge(edges, _Point(x, y + 1), _Point(x + 1, y + 1))) {loops.clear(); return loops;}
            }
        }

        zfstlvector<_Point> visited;
        for(zfstlmap<_Point, _Point>::iterator it = edges.begin(); it != edges.end(); ++it) {
            const _Point &start = it->first;
            if(_exist(visited, start)) {continue;}
            zfstlvector<_Point> raw;
            _Point cur = start;
            while(zftrue) {
                if(_exist(visited, cur)) {loops.clear(); return loops;}
                _uniqueAdd(visited, cur);
                raw.push_back(cur);
                zfstlmap<_Point, _Point>::iterator exist = edges.find(cur);
                if(exist == edges.end()) {loops.clear(); return loops;}
                cur = exist->second;
                if(cur == start) {break;}
            }
            zfstlvector<_Point> merged = _mergeLine(raw);
            if(!merged.empty()) {
                loops.push_back(merged);
            }
        }
        return loops;
    }

    static zfbool _isSibling(ZF_IN const zfstlvector<_Point> &a, const zfstlvector<_Point> &b) {
        for(zfstlsize i = 0; i < a.size(); ++i) {
            zfuint x = a[i].first;
            zfuint y = a[i].second;
            if (zffalse
                    || _exist(b, _Point(x + 1, y))
                    || _exist(b, _Point(x - 1, y))
                    || _exist(b, _Point(x, y + 1))
                    || _exist(b, _Point(x, y - 1))
                    ) {
                return zftrue;
            }
        }
        return zffalse;
    }

    static ZFCoreArray<ZFCoreArray<zfuint> > _merge(
            ZF_IN const zfuint *tiles
            , ZF_IN zfindex tilesCount
            , ZF_IN zfindex maxVertex
            ) {
        zfstlvector<zfstlvector<_Point> > blocks;
        for(ZF_IN zfindex i = 0; i < tilesCount; i += 2) {
            zfstlvector<_Point> t;
            t.push_back(_Point(tiles[i], tiles[i + 1]));
            blocks.push_back(t);
        }
        while(zftrue) {
            zfbool hasMerge = zffalse;
            for(zfstlsize i = 0; i < blocks.size() && !hasMerge; ++i) {
                for(zfstlsize j = i + 1; j < blocks.size(); ++j) {
                    if(!_isSibling(blocks[i], blocks[j])) {continue;}
                    zfstlvector<_Point> combined = blocks[i];
                    const zfstlvector<_Point> &toAdd = blocks[j];
                    for(zfstlsize t = 0; t < toAdd.size(); ++t) {
                        _uniqueAdd(combined, toAdd[t]);
                    }
                    zfstlvector<zfstlvector<_Point> > loops = _extractLoops(combined);
                    if (loops.size() == 1 && loops[0].size() <= (zfstlsize)maxVertex && _isConvex(loops[0])) {
                        blocks[i].swap(combined);
                        blocks.erase(blocks.begin() + j);
                        hasMerge = zftrue;
                        break;
                    }
                }
            }
            if(!hasMerge) {break;}
        }

        ZFCoreArray<ZFCoreArray<zfuint> > polygons;
        for(zfstlsize i = 0; i < blocks.size(); ++i) {
            const zfstlvector<_Point> &block = blocks[i];
            zfstlvector<zfstlvector<_Point> > loops = _extractLoops(block);
            if(loops.size() == 1 && loops[0].size() <= (zfstlsize)maxVertex && _isConvex(loops[0])) {
                ZFCoreArray<zfuint> polygon;
                const zfstlvector<_Point> &points = loops[0];
                for(zfstlsize t = 0; t < points.size(); ++t) {
                    polygon.add(points[t].first);
                    polygon.add(points[t].second);
                }
                polygons.add(polygon);
            }
            else {
                for(zfstlsize j = 0; j < block.size(); ++j) {
                    zfuint x = block[j].first;
                    zfuint y = block[j].second;
                    zfuint buf[] = {
                        x, y,
                        x + 1, y,
                        x + 1, y + 1,
                        x, y + 1,
                    };
                    ZFCoreArray<zfuint> polygon;
                    polygon.addFrom(buf, sizeof(buf) / sizeof(buf[0]));
                    polygons.add(polygon);
                }
            }
        }
        return polygons;
    }
};

ZFMETHOD_FUNC_DEFINE_2(ZFCoreArray<ZFCoreArray<zfuint> >, P2TilePolygonMerge
        , ZFMP_IN(const ZFCoreArray<zfuint> &, tiles)
        , ZFMP_IN_OPT(zfuint, maxVertex, P2ShapePolygon::p2_maxVertex())
        ) {
    ZFCoreAssert(!tiles.isEmpty());
    ZFCoreAssert((tiles.count() % 2) == 0);
    return _ZFP_P2TilePolygon::merge(tiles.arrayBuf(), tiles.count(), maxVertex);
}

ZF_NAMESPACE_GLOBAL_END

