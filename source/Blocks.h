#ifndef BLOCKS_H
#define BLOCKS_H

#include "TileMap.h"
#include "GameEngine.h"

#include "SuperMarioGame.h"


enum EBlock { empty = 0, ground, armor,brick, question };

class CMario;
class CBlocks;

class AbstractBlock
{
public:
	AbstractBlock(int id);
	virtual ~AbstractBlock();
	virtual void draw(sf::RenderWindow* render_window);
	virtual void update(int delta_time);
	virtual void kick(CMario* mario);
	void setPosition(const Vector& pos);
	bool isColliable() const;
	bool isInvisible() const;
	void setParent(CBlocks* blocks);
	int code() const;
	enum {
		BRICK_TILE_CODE = 2, 
		MONEY_TILE_CODE = 3, 
		COIN_TILE_CODE = 11, 
		MUSHROOM_TILE_CODE = 12,
		STAR_CODE = 13, 
		LADDER_TILE_CODE = 17,
		BRIDGE_TILE_CODE = 18, 
		CHAIN_TILE_CODE = 81,
		INIZ_UP_TILE_CODE = 20, 
		INIZ_COIN_CODE = 19, 
		LIVE_UP_CODE = 21,
		BRICK_LADDER_TILE_CODE = 26,
		INVIZ_LADDER_TILE_CODE = 27,
		BRICK_MUSHROOM_TILE_CODE = 28,
	};
protected:
	void setInvisible(bool value);
	void killEnemiesAbove(CMario* mario);
	bool m_colliable = true;
	bool m_invisible = false;
	friend class CBlocks;
	static int block_size;
	static float timer;
	CBlocks* m_blocks = NULL;
	Vector position;
	static CSpriteSheet m_question_block_sprite;
	static CSpriteSheet m_water_sprite;
	static CSpriteSheet m_lava_sprite;
	static CSpriteSheet m_kicked_sprite;
	static CSpriteSheet m_bricket_sprite;
	CSpriteSheet* m_current_sprite = NULL;
	int m_id;
};

class CStaticBlock : public AbstractBlock
{
public:
	CStaticBlock(int id);
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void kick(CMario* mario) override;
private:
	friend class CBlocks;
	static CSpriteSheet m_sprite_sheet;
	sf::Sprite* cur_sprite = NULL;
};

class CBricksBlock : public AbstractBlock
{
public:
	CBricksBlock();
	virtual void draw(sf::RenderWindow* render_window);
	virtual void kick(CMario* mario) override;
	virtual void update(int delta_time);
private:
	CSpriteSheet m_sprite_sheet;
	float kicked_value = 0;
	int kicked_dir = 0;
};

class CMoneyBox : public AbstractBlock
{
public:
	CMoneyBox();
	virtual void draw(sf::RenderWindow* render_window);
	virtual void kick(CMario* mario) override;
	virtual void update(int delta_time);
private:
	CSpriteSheet m_sprite_sheet;
	int m_coin_left = 5;
	float kicked_value = 0;
	int kicked_dir = 0;
};

class CBlocks : public CGameObject
{
private:
	Vector kicked_block;
	Rect view_rect;
	bool m_night_view_filter = false;
	TileMap<AbstractBlock*>* m_map;
	sf::RectangleShape m_shape;
	float m_timer = 0;
	int m_block_size = 32;
	sf::Shader m_night_view_filter_shader;
    std::vector<AbstractBlock*> m_remove_later_list;
public:
	CBlocks(int cols, int rows, int block_size);
	~CBlocks();
	Vector blockSize() const;
	AbstractBlock* getBlock(int x, int y);
	int rows() const;
	int cols() const;
	int width() const;
	int height() const;
	Vector toBlockCoordinates(const Vector& pixel, bool rounded = false) const;
	Vector toPixelCoordinates(const Vector& block) const;
	Rect getBlockBounds(int x, int y) const;
	Rect getBlockBounds(const Vector& pos) const;
	Vector traceLine(const Vector& start_cell, const Vector& direction);
	bool isBlockInBounds(int x, int y) const;
	bool isBlockInBounds(const Vector& block) const;
	std::vector<Vector> getBridgeBlocks();
	void draw(sf::RenderWindow* render_window) override;
	void update(int delta_time) override;
	void clearBlock(int x, int y);
	void kickBlock(int x, int y, CMario* mario);
	void enableNightViewFilter();
	void loadFromString(const std::string& data, std::function<AbstractBlock*(char)> fabric);
	bool isCollidableBlock(int x, int y) const;
	bool isCollidableBlock(const Vector& block) const;
	bool isInvizibleBlock(const Vector& block) const;
	Vector collsionResponse(const Rect& body_rect, const Vector& body_speed, float delta_time, ECollisionTag& collision_tag);
};

template <typename T>
class CQuestionBlock : public AbstractBlock
{
public:
	CQuestionBlock(int id, bool isInvis = false, bool isBrickStyle = false) :AbstractBlock(id)
	{
		setInvisible(isInvis);
		m_current_sprite = &m_question_block_sprite;

		if (isBrickStyle)
			m_current_sprite = &m_bricket_sprite;
	}
	void draw(sf::RenderWindow* render_window)
	{
		if (!m_invisible)
		{
			m_current_sprite->setPosition(position*block_size + Vector::up*kicked_value);
			m_current_sprite->draw(render_window);
		}
	}
	void kick(CMario* mario)
	{
		if (!kicked)
		{
			kicked_value = 1;
			kicked_dir = 0;
			kicked = true;
			CGameObject* object = NULL;
			Vector pos = (position + Vector::up)*block_size;
			object = (CGameObject*)getObject(pos, mario);
			m_blocks->getParent()->addObject(object);
			killEnemiesAbove(mario);
		}
		CMarioGame::instance()->playSound("bump");
	}
	void update(int delta_time)
	{
		if (kicked_value != 0)
		{
			if (kicked_dir == 0)
			{
				kicked_value += delta_time / 10;
				if (kicked_value > 15)
				{
					m_current_sprite = &m_kicked_sprite;
					kicked_dir = 1;
				}
			}
			if (kicked_dir == 1)
			{
				kicked_value -= delta_time / 10;
				if (kicked_value < 0)
					kicked_value = 0;
			}
		}
	}
protected:
	T* getObject(const Vector& pos, CMario* mario)
	{
		return new T(pos, mario);
	}
	float kicked_value = 0;
	bool kicked = false;
	int kicked_dir = 0;
	friend class CBlocks;
};

class CBackground : public CGameObject
{
public:
	CBackground();
	virtual void draw(sf::RenderWindow* render_window);
private:
	void start() override;
	void onActivated() override;
	sf::Sprite m_background;
};

class COneBrick : public CGameObject
{
 public:
	 COneBrick(const Vector& pos, const Vector& speed_vector);
	 void draw(sf::RenderWindow* render_window) override;
	 void update(int delta_time) override;
 private:
	 Vector m_speed;
	 CSpriteSheet m_sprite_sheet;
	 int m_timer = 0;
};

class CTwistedCoin : public CGameObject
{
public:
	CTwistedCoin(const Vector& pos, CMario* mario);
	void draw(sf::RenderWindow* render_window) override;
	void update(int delta_time) override;
private:
	Vector m_speed;
	Animator m_animator;
	int m_timer = 0;
};

#endif
