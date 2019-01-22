#include "Blocks.h"
#include "SuperMarioGame.h"
#include "Pickups.h"
#include "Enemies.h"
#include <unordered_set>
#include <SFML/Graphics.hpp>
//-----------------------------------------------------------------------------------------------------------------------------------------------

int AbstractBlock::block_size = 0;
float AbstractBlock::timer = 0;
 
void AbstractBlock::killEnemiesAbove(CMario* mario)
{
	Rect block_rect = m_blocks->getBlockBounds((int)position.x, (int)position.y - 1).bordered(-4);

	auto func = [block_rect,mario](CGameObject* obj)
	 {
		if (obj->getBounds().isIntersect(block_rect))
		{
			if (obj->isTypeOf<CEnemy>())
			{
				auto enemy = obj->castTo<CEnemy>();
				if (enemy->isAlive())
					enemy->kickFromBottom(mario);
			}
			else if (obj->isTypeOf<CMushroom>())
			{
				auto mushroom = obj->castTo<CMushroom>();
				mushroom->addImpulse(Vector::up*0.4f);				
			}
			else  if (obj->isTypeOf<CCoin>())
				obj->castTo<CCoin>()->kick();
		}
	 };

	m_blocks->getParent()->foreachObject(func);
}

AbstractBlock::AbstractBlock(int id) :m_id(id)
{


}

AbstractBlock::~AbstractBlock()
{

};

void AbstractBlock::draw(sf::RenderWindow* render_window)
{

};

void AbstractBlock::update(int delta_time)
{

}

void AbstractBlock::kick(CMario* mario)
{

}

void AbstractBlock::setPosition(const Vector& pos)
{
	position = pos;
}

bool AbstractBlock::isColliable() const
{
	return m_colliable;
}

void AbstractBlock::setParent(CBlocks* blocks)
{
	m_blocks = blocks;
}

int AbstractBlock::code() const
{
	return m_id;
}

bool AbstractBlock::isInvisible() const
{
	return m_invisible;
}

void AbstractBlock::setInvisible(bool value)
{
	m_invisible = value;
	m_colliable = !value;
	m_current_sprite = &m_kicked_sprite;
}

CSpriteSheet AbstractBlock::m_question_block_sprite = CSpriteSheet();
CSpriteSheet AbstractBlock::m_water_sprite = CSpriteSheet();
CSpriteSheet AbstractBlock::m_lava_sprite = CSpriteSheet();
CSpriteSheet AbstractBlock::m_kicked_sprite = CSpriteSheet();
CSpriteSheet AbstractBlock::m_bricket_sprite = CSpriteSheet();

//------------------------------------------------------------------------------------------------------------------------------------------------

CStaticBlock::CStaticBlock(int id): AbstractBlock(id)
{
	if (m_sprite_sheet.empty())
		 m_sprite_sheet.load(*MarioGame().textureManager().get("Tiles"), Vector(0,0), {32, 32 }, 8, 12);
	 cur_sprite = &m_sprite_sheet[id-1];
  
	m_colliable = id <= 40;

	if (id == 14 || id == 22)
		m_colliable = false;
}

void CStaticBlock::draw(sf::RenderWindow* render_window)
{
	cur_sprite->setPosition(position*block_size);
	render_window->draw(*cur_sprite);
};

void CStaticBlock::kick(CMario* mario)
{
	MarioGame().playSound("bump");
}

CSpriteSheet CStaticBlock::m_sprite_sheet = CSpriteSheet();
CBricksBlock::CBricksBlock():AbstractBlock(BRICK_TILE_CODE)
{
	m_sprite_sheet.load(*MarioGame().textureManager().get("Tiles"), { { 32,0,32,32 } });
}

void CBricksBlock::draw(sf::RenderWindow* render_window)
{
	m_sprite_sheet.setPosition(position*block_size + Vector::up*kicked_value);
	m_sprite_sheet.draw(render_window);		 
};

void CBricksBlock::update(int delta_time)
{
	if (kicked_value != 0)
	{
		if (kicked_dir == 0)
		{
			kicked_value += delta_time / 10;

			if (kicked_value > 15)
				kicked_dir = 1;
		}
		if (kicked_dir == 1)
		{
			kicked_value -= delta_time / 10;
			if (kicked_value < 0)
				kicked_value = 0;
		}
	}
}

void CBricksBlock::kick(CMario* mario)
{
	int x = (int)position.x;
	int y = (int)position.y;
	
	if (!mario->isSmall()) // crash box
	{
            m_blocks->addObject(new COneBrick(Vector(x, y)*block_size, Vector(-0.05f, -0.2f)));
            m_blocks->addObject(new COneBrick(Vector(x + 0.5f, (float)y)*block_size, Vector(0.05f, -0.2f)));
            m_blocks->addObject(new COneBrick(Vector((float)x, y + 0.5f)*block_size, Vector(-0.05f, -0.1f)));
            m_blocks->addObject(new COneBrick(Vector(x + 0.5f, y + 0.5f)*block_size, Vector(0.05f, -0.1f)));
			killEnemiesAbove(mario);
            m_blocks->clearBlock(x, y);
			MarioGame().addScore(50);
			MarioGame().playSound("breakblock");
	}
	else                                        // just kick box
	{
			if (kicked_value == 0)
			{
				killEnemiesAbove(mario);
		 		kicked_value = 1;
		 		kicked_dir = 0;
				MarioGame().playSound("bump");
			}
	}		
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

CMoneyBox::CMoneyBox():AbstractBlock(MONEY_TILE_CODE)
{
	m_sprite_sheet.load(*MarioGame().textureManager().get("Tiles"), { { 32,0,32,32 }, { 0,32,32,32 } });
}

void CMoneyBox::draw(sf::RenderWindow* render_window)
{
	m_sprite_sheet.setPosition(position*block_size + Vector::up*kicked_value);
	m_sprite_sheet.draw(render_window);
};

void CMoneyBox::update(int delta_time)
{
	if (kicked_value != 0)
	{
		if (kicked_dir == 0)
		{
			kicked_value += delta_time / 10;

			if (kicked_value > 15)
				kicked_dir = 1;
		} 
		if (kicked_dir == 1)
		{
			kicked_value -= delta_time / 10;
			if (kicked_value < 0)
				kicked_value = 0;
		}
	}
}

void CMoneyBox::kick(CMario* mario)
{
	if (kicked_value == 0 && m_coin_left !=0 )
	{
		kicked_value = 1;
		kicked_dir = 0;
		Vector pos = (position + Vector::up)*block_size;
		m_blocks->addObject(new CTwistedCoin(pos, mario));
		--m_coin_left;
		if (m_coin_left == 0)
			m_sprite_sheet.setSpriteIndex(1);
		MarioGame().addScore(100, m_blocks->toPixelCoordinates(position));
		MarioGame().addCoin();
		MarioGame().playSound("bump");
	}
	killEnemiesAbove(mario);
}

//------------------------------------------------------------------------------------------------------------------

CBackground::CBackground()
{
	m_background.setTextureRect({ 0,0,1280,720 });
}

void CBackground::draw(sf::RenderWindow* render_window)
{
	Rect cameraRect = getParent()->castTo<CMarioGameScene>()->cameraRect();
	m_background.setPosition(cameraRect.leftTop());
	render_window->draw(m_background);
}

void CBackground::onActivated()
 {
	 m_background.setTexture(*MarioGame().textureManager().get(getProperty("Picture").asString()));
	 moveToBack();
 }

void CBackground::start()  
{
	if (getProperty("NightViewFilter").isValid() && getProperty("NightViewFilter").asBool() == true)
	 	getParent()->findObjectByType<CBlocks>()->enableNightViewFilter();
}

//------------------------------------------------------------------------------------------------------------------

CBlocks::CBlocks(int cols, int rows, int block_size): m_block_size(block_size)
{
	setName("Blocks");
	AbstractBlock::block_size = block_size;
	m_shape.setSize(blockSize());
	m_shape.setFillColor(sf::Color::Red);
	m_map = new TileMap<AbstractBlock*>(cols, rows);
	m_map->clear(NULL);
	auto texture = MarioGame().textureManager().get("AnimTiles");
	AbstractBlock::m_question_block_sprite.load(*texture, { 0,0 }, { 32,32 }, 4, 1);
	AbstractBlock::m_question_block_sprite.setAnimType(AnimType::manual);
	AbstractBlock::m_water_sprite.load(*texture, { 0,32 }, { 32,32 }, 4, 1);
	AbstractBlock::m_water_sprite.setAnimType(AnimType::manual);
	AbstractBlock::m_lava_sprite.load(*texture, { 0,64 }, { 32,32 }, 4, 1);
	AbstractBlock::m_lava_sprite.setAnimType(AnimType::manual);
	AbstractBlock::m_kicked_sprite.load(*MarioGame().textureManager().get("Tiles"), { { 0,32,32,32 } });
	AbstractBlock::m_bricket_sprite.load(*MarioGame().textureManager().get("Tiles"), { { 32,0,32,32 } });

	static const sf::String frag_shader =
			"#version 120\n"\
			"uniform sampler2D texture;\n"\
			"void main()"\
			"{"\
			"vec4 color = texture2D(texture, gl_TexCoord[0].xy);"\
			"float middle = (color.r + color.g)/2.f;"\
			"gl_FragColor = vec4(color.g,middle,middle,color.a);"\
			"}";
	m_night_view_filter_shader.loadFromMemory(frag_shader, sf::Shader::Fragment);
	m_night_view_filter_shader.setUniform("texture", sf::Shader::CurrentTexture);	 
}

void CBlocks::loadFromString(const std::string& data, std::function<AbstractBlock*(char)> fabric)
{
	m_map->loadFromString(fabric, data);
	for (int x = 0; x < m_map->width(); ++x)
		for (int y = 0; y < m_map->height(); ++y)
		{
			AbstractBlock* block = m_map->getCell(x, y);
			if (block)
			{
				block->setPosition({ x, y });
				block->setParent(this);
			}
		}
}

void CBlocks::enableNightViewFilter()
{
	m_night_view_filter = true;
}

void CBlocks::draw(sf::RenderWindow* render_window)
{
	static std::unordered_set<int> filter_except = {82,83,50,58,62,64};

	if (m_night_view_filter)
		sf::Shader::bind(&m_night_view_filter_shader);

	Rect cameraRect = getParent()->castTo<CMarioGameScene>()->cameraRect();
	const float block_size = blockSize().x;
	Vector center = render_window->getView().getCenter();
	Vector size = render_window->getView().getSize();
	view_rect = Rect(toBlockCoordinates(center - size / 2), toBlockCoordinates(size)).getIntersection(Rect(0, 0, cols(), rows()));

	for (int x = view_rect.left(); x < view_rect.right(); ++x)
		for (int y = view_rect.top(); y < view_rect.bottom(); ++y)
		{
			AbstractBlock* block = m_map->getCell(x, y);
            if (block == nullptr)
				continue;
			bool skip_fiter = m_night_view_filter && filter_except.count(block->m_id);

			if (skip_fiter)
				sf::Shader::bind(NULL);
			block->draw(render_window); 
			if (skip_fiter)
				sf::Shader::bind(&m_night_view_filter_shader);
		}

	CGameObject::draw(render_window);

	if (m_night_view_filter)
		sf::Shader::bind(NULL);
}

void CBlocks::update(int delta_time)
{
    if (!m_remove_later_list.empty())
    {
        for (auto object : m_remove_later_list)
            delete object;
        m_remove_later_list.clear();
    }

	CGameObject::update(delta_time);

	m_timer += delta_time / 200.f;
	AbstractBlock::timer = m_timer;
	AbstractBlock::m_question_block_sprite.setSpriteIndex(int(m_timer) % 4);
	CStaticBlock::m_sprite_sheet[49] = AbstractBlock::m_water_sprite[int(m_timer) % 4];
	CStaticBlock::m_sprite_sheet[81] = AbstractBlock::m_lava_sprite [int(m_timer) % 4];

	for (int x = view_rect.left(); x < view_rect.right(); ++x)
		for (int y = view_rect.top(); y < view_rect.bottom(); ++y)
		{
			AbstractBlock* block = m_map->getCell(x, y);
			if (block)
				block->update(delta_time);
		}
}

Rect CBlocks::getBlockBounds(int x, int y) const
{
	return Rect(x*m_block_size, y*m_block_size, m_block_size, m_block_size);
}

Rect CBlocks::getBlockBounds(const Vector& pos) const
{
	return Rect(pos.x*m_block_size, pos.y*m_block_size, m_block_size, m_block_size );
}

Vector CBlocks::traceLine(const Vector& start_cell, const Vector& direction)
{
	return m_map->traceLine(start_cell, direction, [](AbstractBlock* block) {return !(block && block->isColliable()); });
}

Vector CBlocks::blockSize() const
{
	return{ m_block_size,m_block_size };
}

AbstractBlock* CBlocks::getBlock(int x, int y)
{
	return m_map->getCell(x, y);
}

void CBlocks::clearBlock(int x, int y)
{
    m_remove_later_list.push_back(m_map->getCell(x, y));
    m_map->setCell(x, y, nullptr);
}

void CBlocks::kickBlock(int x, int y, CMario* mario)
{
	AbstractBlock* block = getBlock(x, y);
	if (block)
		block->kick(mario);
	if (block && block->isInvisible())
		block->setInvisible(false);
}

int CBlocks::rows() const
{
	return m_map->height();
}

int CBlocks::cols() const
{
	return m_map->width();
}

int CBlocks::width() const
{
	return cols()*m_block_size;
}

int CBlocks::height() const
{
	return rows()*m_block_size;
}

bool CBlocks::isCollidableBlock(int x, int y) const
{
	AbstractBlock* block = m_map->getCell(x, y);
	if (block == NULL)
	 return false;
  return block->isColliable();
}

bool CBlocks::isCollidableBlock(const Vector& block) const
{
	AbstractBlock* b = m_map->getCell(block.x, block.y);
	if (b == NULL)
		return false;
	return b->isColliable();
}

bool CBlocks::isInvizibleBlock(const Vector& block) const
{
	AbstractBlock* b = m_map->getCell(block.x, block.y);
	if (b == NULL)
		return false;
	return b->isInvisible();
}

Vector CBlocks::toBlockCoordinates(const Vector& pixel, bool rounded) const
{
	if (rounded)
		return round(pixel / m_block_size);
	return  pixel / m_block_size;
}

Vector CBlocks::toPixelCoordinates(const Vector& block) const
{
	return  block*m_block_size;
}

bool CBlocks::isBlockInBounds(int x, int y) const
{
	if (x >= 0 && y >= 0 && x < cols() && y < rows())
		return true;
	return false;
}

bool CBlocks::isBlockInBounds(const Vector& block) const
{
	if (block.x >= 0 && block.y >= 0 && block.x < cols() && block.y < rows())
		return true;
	return false;
}

Vector CBlocks::collsionResponse(const Rect& body_rect, const Vector& body_speed, float delta_time, ECollisionTag& collision_tag)
{
	CBlocks* blocks = this;
	Vector  own_size = body_rect.size();
	float tile_size = blocks->blockSize().x;
	Vector new_pos = body_rect.leftTop();

	// Y axis
	new_pos.x -= body_speed.x*delta_time;
	for (int x = new_pos.x / tile_size; x < (new_pos.x + own_size.x) / tile_size; ++x)
		for (int y = new_pos.y / tile_size; y < (new_pos.y + own_size.y) / tile_size; ++y)
			if (blocks->isBlockInBounds(x, y) && (blocks->isCollidableBlock(x, y) || (blocks->isInvizibleBlock({ x,y }) && body_speed.y < 0)))
			{
				if (body_speed.y == 0 && body_rect.bottom() > (y+1) * tile_size) //fix for "body in the restricted area" collision bug
				{
					collision_tag |= ECollisionTag::cell;
					collision_tag |= ECollisionTag::floor;
					return new_pos + Vector::right*2; //push avay body onto right side
				}
				if (body_speed.y >= 0)
				{
					new_pos.y = y * tile_size - own_size.y;
					collision_tag |= ECollisionTag::floor;
				}
				else if (body_speed.y < 0)
				{
					new_pos.y = y * tile_size + tile_size;
					collision_tag |= ECollisionTag::cell;
				}
				goto stopYLoop;
			}
	stopYLoop:;
	new_pos.x += body_speed.x*delta_time;

	// X axis
	for (int x = new_pos.x / tile_size; x < (new_pos.x + own_size.x) / tile_size; ++x)
		for (int y = new_pos.y / tile_size; y < (new_pos.y + own_size.y) / tile_size; ++y)
			if (blocks->isBlockInBounds(x, y) && blocks->isCollidableBlock(x, y))
			{
				if (body_speed.x > 0)
				{
					new_pos.x = x * tile_size - own_size.x;
					collision_tag |= ECollisionTag::left;

				}
				else if (body_speed.x <  0)
				{
					new_pos.x = x * tile_size + tile_size;
					collision_tag |= ECollisionTag::right;
                }
				goto stopXLoop;
			}
	stopXLoop:;
	return new_pos;
}

CBlocks::~CBlocks()
{

	for (int x = 0; x < m_map->width(); ++x)
		for (int y = 0; y < m_map->height(); ++y)
			delete m_map->getCell(x, y);
	delete m_map;

}

std::vector<Vector> CBlocks::getBridgeBlocks()
{
	std::vector<Vector> bridge_cells;

		for (int x = 0; x < m_map->width(); x++)
			for (int y = 0; y < m_map->height(); y++)
				if (m_map->getCell(x, y) && (m_map->getCell(x, y)->code() == AbstractBlock::BRIDGE_TILE_CODE ||
					                         m_map->getCell(x, y)->code() == AbstractBlock::CHAIN_TILE_CODE))
					bridge_cells.emplace_back(x, y);

		//std::reverse(bridge_cells.begin(), bridge_cells.end());
		return bridge_cells;
}

//---------------------------------------------------------------------------------------------------------------

COneBrick::COneBrick(const Vector& pos, const Vector& speed_vector)
{
	setPosition(pos);
	m_speed = speed_vector;
	m_sprite_sheet.load(*MarioGame().textureManager().get("Items"), { { 96,0,16,16 }, { 96,16,16,-16 } });

	m_sprite_sheet.setAnimType(AnimType::forward_cycle);
	m_sprite_sheet.setSpeed(0.005f);
}

void COneBrick::draw(sf::RenderWindow* render_window)
{
	m_sprite_sheet.setPosition(getPosition());
    m_sprite_sheet.draw(render_window);
}

void COneBrick::update(int delta_time) 
{
	m_sprite_sheet.update(delta_time);
	m_speed.y += delta_time*0.0005f; //gravity;
	move(m_speed*delta_time);
	m_timer += delta_time;
	if (m_timer > 3000)
		getParent()->removeObject(this);
}

//-----------------------------------------------------------------------------------

CTwistedCoin::CTwistedCoin(const Vector& pos, CMario* mario)
{
	setPosition(pos);
	m_speed = Vector::up*0.05f;
	m_speed.y = -0.20f;
	m_animator.create("twist", *MarioGame().textureManager().get("Items"), Vector(0, 84), Vector(32, 32), 4, 1, 0.01f);
	m_animator.create("shine", *MarioGame().textureManager().get("Items"), Vector(0, 116), Vector(40, 32), 5, 1, 0.01f, AnimType::forward);
	m_animator.get("shine")->setOrigin(Vector(4, 0));
	MarioGame().addScore(100, pos);
	MarioGame().addCoin();
}

void CTwistedCoin::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CTwistedCoin::update(int delta_time)
{
	if (m_timer == 0)
	{
		MarioGame().playSound("coin");
	}
	else if (m_timer < 700)
	{
		m_speed.y += delta_time*0.0005f; //gravity;
		move(m_speed*delta_time);
	}
	else if (m_timer < 1200)
	{
		m_animator.play("shine");
	}
	else
	{
		getParent()->removeObject(this);
	}

	m_timer += delta_time;
	m_animator.update(delta_time);
}

