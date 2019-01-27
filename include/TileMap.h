#pragma once

#ifndef TILEMAP_H
#define TILEMAP_H

#include "Geometry.h"
#include <vector>
#include <functional>
#include <fstream>

const static Vector directions[] = { Vector::zero,Vector::left, Vector::up,Vector::down,Vector::right };

template<typename T>
class TileMap
{
public:
	TileMap(int width, int height)
	{
		m_width = width;
		m_height = height;
		m_map = new T*[width];
		for (int x = 0; x < width; ++x)
		{
			m_map[x] = new T[height];
		    
	     	for (int i = 0; i < height; ++i)
				m_map[x][i] = T();
		}
	}
	~TileMap()
	{
		for (int x = 0; x < m_width; ++x)
			delete m_map[x];
		delete m_map;
	}
	inline void setCell(int x, int y, T value)
	{
		assert(x < m_width && y < m_height && x >= 0 && y >= 0);
		m_map[x][y] = value;
	}
	inline const T& getCell(int x, int y) const
	{
		assert(x < m_width && y < m_height && x >= 0 && y >= 0);
		return m_map[x][y];
	}
	inline const T& getCell(const Vector& point) const
	{
		assert(point.x < m_width && point.y < m_height && point.x >= 0 && point.y >= 0);
		return m_map[(int)point.x][(int)point.y];
	}
	void clear(T value = T())
	{
		for (int x = 0; x < m_width; ++x)
			for (int y = 0; y < m_height; ++y)
				m_map[x][y] = value;
	}
	inline int width() const
	{
		return m_width;
	}
	inline int height() const
	{
		return m_height;
	}
	void fillRect(int x1, int y1, int width, int height, T value)
	{
		for (int x = x1; x < x1 + width; ++x)
			for (int y = y1; y < y1 + height; ++y)
				setCell(x, y, value);
	}
	void loadFromString(std::map<char, T> dictionary, const std::string& str)
	{
		clear();
		assert(width()*height() == str.length());
		int i = 0;
		for (size_t y = 0; y < height(); ++y)
			for (size_t x = 0; x < width(); ++x)
				setCell(x, y, dictionary[str[i++]]);
	}
	void loadFromString(std::function<T(char)> fabric, const std::string& str)
	{

		clear();

		assert(width()*height() == str.length());
		int i = 0;
		for (int y = 0; y < height(); ++y)
			for (int x = 0; x < width(); ++x)
				setCell(x, y, fabric(str[i++]));
	}
	void loadFromFile(std::map<char, T> dictionary, const std::string& FilePath)
	{
		std::ifstream file;

		file.open(FilePath);

		if (!file.is_open())
			throw std::runtime_error("Can't load file: " + FilePath);


		std::string str;
		for (int y = 0; y < height(); ++y)
		{
			std::getline(file, str);
			assert(str.length() == m_width);
			for (int x = 0; x < m_width; ++x)
			{
				assert(dictionary.find(str[x]) != dictionary.end());
				m_map[x][y] = dictionary[str[x]];
			}
		}
	}
	bool inBounds(const Vector& cell) const
	{
		return cell.x >= 0 && cell.y >= 0 && cell.x < m_width && cell.y < m_height;
	}
	std::vector<Vector> getCells(T cell_type)
	{
		std::vector<Vector> cells;

		for (int x = 0; x < m_width; ++x)
			for (int y = 0; y < m_height; ++y)
				if (m_map[x][y] == cell_type)
					cells.emplace_back(x, y);
		return 	cells;
	}
	std::vector<std::pair<Vector,T>> getCells(const Rect& rect)
	{
		std::vector<std::pair<Vector, T>> cells;
		for (int x = rect.left(); x < rect.right(); ++x)
			for (int y = rect.top(); y < rect.bottom(); ++y)
				cells.push_back(std::make_pair<>({ x,y }, getCell(x, y)));
		return cells;
	}
	Vector traceLine(const Vector& start_cell, const Vector& direction, const std::function<bool(T)>& allowed_cell)
	{
		Vector curr_cell = floor(start_cell);
		if (!allowed_cell(getCell(curr_cell)))
			return curr_cell;
		
		//assert(allowed_cell(getCell(curr_cell)));

		if (direction == Vector::zero)
			return curr_cell;

		while (inBounds(curr_cell) && allowed_cell(getCell(curr_cell)))
			curr_cell += direction;

		curr_cell += -direction;
		assert(allowed_cell(getCell(curr_cell)));
		return curr_cell;
	}
	Vector getCell(const Vector& start_cell, const Vector& direction, int length)
	{
		Vector cur_cell = start_cell;
		for (int i = 0; i < length; ++i)
			cur_cell +=  direction;
		return cur_cell;
	}
	std::vector<Vector> getNeighborNodes(const Vector& start_cell, const T& allowedCellType)
	{
		Vector curr_cell;
		std::vector<Vector> nodes;

		for (int i = 1; i < 5; ++i)
		{
			curr_cell = start_cell;
			while ((curr_cell == start_cell || getCellDegree(curr_cell, allowedCellType) < 3) && inBounds(curr_cell + directions[i]) && getCell(curr_cell + directions[i]) == allowedCellType)
				curr_cell += directions[i];
			if (curr_cell != start_cell && curr_cell.x != 0 && curr_cell.x != m_width - 1)
				nodes.push_back(curr_cell);
		}

		return nodes;
	}
	TileMap& operator=(const TileMap& other_map)
	{
		for (int x = 0; x < m_width; ++x)
			for (int y = 0; y < m_height; ++y)
				setCell(x, y, other_map.getCell(x, y));
		m_width = other_map.m_width;
		m_height = other_map.m_height;
		return *this;
	}

private:
	T** m_map;
	int m_height,m_width;
};

#endif





