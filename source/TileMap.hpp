#ifndef TILEMAP_HPP
#define TILEMAP_HPP

#include <functional>
#include <fstream>
#include <vector>

#include "Vector.hpp"


template<typename T>
class TileMap {
public:

    TileMap(int cols, int rows, int tile_width, int tile_height)
        : m_cols(cols)
        , m_rows(rows)
        , m_tile_width(tile_width)
        , m_tile_height(tile_height) {
        m_map.resize(m_cols * m_rows, T());
    }

    virtual ~TileMap() = default;

    void setTile(int x, int y, T value) {
        assert(isTileInBounds(Vector(x, y)));
        m_map[x + y * m_cols] = value;
    }

    const T& getTile(int x, int y) const {
        assert(isTileInBounds(Vector(x, y)));
        return m_map[x + y * m_cols];
    }

    const T& getCell(const Vector& point) const {
        return getCell(point.x, point.y);
    }

    void clear(T value = T()) {
        for (int i = 0; i < m_cols * m_rows; ++i) {
            m_map[i] = value;
        }
    }

    int cols() const {
        return m_cols;
    }

    int rows() const {
        return m_rows;
    }

    Vector getTileFromPointCoordinates(const Vector& point) const {
        return Vector((int)point.x / m_tile_width, (int)point.y / m_tile_height);
    }

    Vector getPointCoordinatesFromTile(const Vector& tile) const {
        return Vector((int)tile.x * m_tile_width, (int)tile.y * m_tile_height);
    }

    Rect getRenderBounds() const {
        return Rect(0, 0, m_cols * m_tile_width, m_rows * m_tile_height);
    }

    bool isTileInBounds(const Vector& tile) const {
        return ((tile.x >= 0) && (tile.y >= 0) &&
                (tile.x < m_cols) && (tile.y < m_rows));
    }

private:
    int m_cols = 0;
    int m_rows = 0;
    int m_tile_width = 0;
    int m_tile_height = 0;
    std::vector<T> m_map;
};

#endif // TILEMAP_HPP
