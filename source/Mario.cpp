#include "Mario.h"
#include "Blocks.h"
#include "Items.h"
#include "SuperMarioGame.h"
#include "Enemies.h"
#include <cmath>

CMarioBullet::CMarioBullet(const Vector& pos, const Vector& direction)
{
	m_direction = direction;
	setPosition(pos);
	m_speed = direction*bullet_speed;
	m_animator.create("fly", *MarioGame().textureManager().get("Mario"), { { 0,0,16,16 },{ 16,0,16,16 },{ 16,0,-16,16 },{ 16,16,16,-16 } }, 0.01f);
	m_animator.create("splash", *MarioGame().textureManager().get("Mario"),  Vector(31,0), Vector(16,16), 3,1, 0.02f, AnimType::forward_backward_cycle);
}

void CMarioBullet::draw(sf::RenderWindow* render_window) 
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CMarioBullet::setState(State state)
{
	m_state = state;

	if (m_state == State::fly)
		m_animator.play("fly");
	else if (m_state == State::splash)
		m_animator.play("splash");

	m_timer = 0;
}

void CMarioBullet::start()
{
	m_blocks = getParent()->findObjectByName<CBlocks>("Blocks");
}

void CMarioBullet::update(int delta_time) 
{
	m_timer += delta_time;
	if (m_timer > 3000)
		getParent()->removeObject(this);


	if (m_state == State::fly)
	{
		m_speed += Vector::down*gravity_force * delta_time;
		move(m_speed*delta_time);
		Vector block = m_blocks->toBlockCoordinates(getPosition());
		if (m_blocks->isBlockInBounds(block) && m_blocks->isCollidableBlock(block.x, block.y))
		{
			Rect block_bounds = m_blocks->getBlockBounds(block.x, block.y);
            float dw = std::min(std::abs(block_bounds.left() - getPosition().x), std::abs(block_bounds.right() - getPosition().x));
            float dh = std::min(std::abs(block_bounds.top() - getPosition().y), std::abs(block_bounds.bottom() - getPosition().y));
			if (dw < dh && (dw > 4 && dh > 4))     // kick side (and angles - r=4)
				setState(State::splash);
			else                                   // kick top or bottom
				m_speed.y = -0.35f;
		}

		auto enemies = getParent()->findObjectsByType<CEnemy>();

		for (auto enemy : enemies)
			if (enemy->getBounds().isContain(getPosition()))
			{
				enemy->fired(NULL);
				setState(State::splash);
				break;
			}
	}
	else
	{
		if (m_timer > 250)
			getParent()->removeObject(this);
	}
	m_animator.update(delta_time);
}

//-------------------------------------------------------------------------------------------------

CMario::CMario()
{
	m_blocks = NULL;
	const sf::Texture& texture = *MarioGame().textureManager().get("Mario");
	addObject(m_animator = new Animator());
	m_animator->create("idle_big", texture, { 0,32,32,64 });
	m_animator->create("walk_big", texture, { 32, 32 }, { 32, 64 }, 3, 1, 0.01f);
	m_animator->create("swim_big", texture, { 321, 32 }, { 40, 64 }, 3, 1, 0.01f);
	m_animator->create("jump_big", texture, { 160,32,32,64 });
	m_animator->create("slip_big", texture, { 128,32,32,64 });
	m_animator->create("seat_big", texture, { 192,52,32,44 });
	m_animator->create("climb_big", texture, Vector(256, 32), Vector(32, 64), 2, 1, 0.01f);
	m_animator->create("growing", texture, { { 0,32,32,64 }, { 0,96,32,32 } }, 0.01f );
	m_animator->setSpriteOffset("growing", 1, { 0, 32 });
	m_animator->create("demoting", texture, { { 401,32,40,64 },{ 288,96,32,32 } }, 0.01f);
	m_animator->setSpriteOffset("demoting", 1, { 4, 32 });
	m_animator->create("firing", texture, { { 0,32,32,64 },{ 224,32,32,64 } }, 0.01f);
	m_animator->create("idle_small", texture, { 0,96,32,32 });
	m_animator->create("walk_small", texture, { 32, 96 }, { 32, 32 }, 3, 1, 0.01f);
	m_animator->create("swim_small", texture,   { 288, 96 }, { 32, 32 }, 3, 1, 0.01f);
	m_animator->create("jump_small", texture, { 160,96,32,32 });
	m_animator->create("slip_small", texture, { 128,96,32,32 });
	m_animator->create("seat_small", texture, { 192,96,32,32 });
	m_animator->create("climb_small", texture, Vector(224, 96), Vector(32, 32), 2, 1, 0.01f);
	m_animator->create("shoot", texture, { 224,32,32,64 });
	m_animator->create("died", texture, { 192,96,32,32 });
	setRank(MarioRank::small);
	setState(MarioState::normal);
	m_fire_pallete.create({ sf::Color(64,64,128), sf::Color(64,96,192), sf::Color(160,32,0), sf::Color(192,0,64), sf::Color(224,32,64)},
	                      { sf::Color(64,128,0),sf::Color(96,160,0),sf::Color(192,192,128),sf::Color(224,224,128), sf::Color(255,251,240) });
	m_black_pallete.create({ sf::Color(64,64,128), sf::Color(64,96,192), sf::Color(160,32,0), sf::Color(192,0,64), sf::Color(224,32,64) },
	                       { sf::Color(128,64,0),sf::Color(160,96,0),sf::Color(20,20,10),sf::Color(30,30,20), sf::Color(0,0,0) });
	m_animator->play("idle_small");
	m_animator->get("seat_big")->setOrigin(-Vector::up*12);
}

void CMario::reciveDamage()
{
	if (!m_spawn_timer && !m_invincible_mode)
	{
		if (m_rank != MarioRank::small)
			setState(MarioState::demoting);
		else
			setState(MarioState::died);
	}
}

bool CMario::isSmall() const
{
	return getRank() == MarioRank::small;
}

void CMario::promote()
{
	if (m_rank != MarioRank::fire)
	 setState(MarioState::promoting);
}

void CMario::playAnimation(const std::string& name)
{
	m_animator->play(name);
}

void CMario::setRank(MarioRank rank)
{
	switch (rank)
	{
		case(MarioRank::small):
		{
			setSize(small_mario_size);
			if (m_rank != MarioRank::small)
			move(small_mario_size - big_mario_size);
			m_seated = false;
			m_animator->setPallete(NULL);
			m_animator->play("idle_small");
			break;
		}
		case(MarioRank::big):
		{
			setSize(big_mario_size);
			move(small_mario_size - big_mario_size);
			m_animator->setPallete(NULL);
			m_animator->play("idle_big");
			break;
		}
		case(MarioRank::fire):
		{
			m_animator->setPallete(&m_fire_pallete);
			if (!m_seated)
			{
				setSize(big_mario_size);
				m_animator->play("idle_big");
			}
			break;
		}
	}
	m_rank = rank;
}

MarioRank CMario::getRank() const
{
	return m_rank;
}

void CMario::draw(sf::RenderWindow* render_window)
{
	m_animator->setPosition(getPosition());
	m_animator->draw(render_window);
}

void CMario::kickBlocksProcessing()
{
	if (m_collision_tag & ECollisionTag::cell && !(m_collision_tag & ECollisionTag::floor))
	{
		Vector block_left = m_blocks->toBlockCoordinates(getBounds().leftTop() + Vector::up * 16, false);
		Vector block_right = m_blocks->toBlockCoordinates(getBounds().rightTop() + Vector::up * 16, false);

		bool block_left_exist = m_blocks->isBlockInBounds(block_left) && (m_blocks->isCollidableBlock(block_left) || m_blocks->isInvizibleBlock(block_left));
		bool block_right_exist = m_blocks->isBlockInBounds(block_right) && (m_blocks->isCollidableBlock(block_right) || m_blocks->isInvizibleBlock(block_right));

		if (block_left_exist || block_right_exist)
		{
			if (block_left_exist ^ block_right_exist) //only one block touched
			{
				if (block_left_exist)
				 m_blocks->kickBlock(block_left.x, block_left.y, this);
				else
				 m_blocks->kickBlock(block_right.x, block_right.y, this);
			}
			else                //Kick closer block between both touched blocks
			{
				Rect block_left_rect = m_blocks->getBlockBounds(block_left);
				Rect block_right_rect = m_blocks->getBlockBounds(block_right);

				float mario_X = getBounds().center().x;

                if (std::abs(block_left_rect.center().x - mario_X) < std::abs(block_right_rect.center().x - mario_X))
					m_blocks->kickBlock(block_left.x, block_left.y, this);
				else
					m_blocks->kickBlock(block_right.x, block_right.y, this);
			}
		}
	}
}

void CMario::kickEnemiesProcessing(float delta_time)
{
	if (m_spawn_timer == 0)
	{
		auto enemies = getParent()->findObjectsByType<CEnemy>();
		for (auto enemy : enemies)
		{
			if (enemy->isAlive() && getBounds().isIntersect(enemy->getBounds()))
			{
				if (m_invincible_mode)
					enemy->kickFromBottom(this);
				else
				{
					if (!m_in_water)
					{
						Rect intersection = getBounds().getIntersection(enemy->getBounds());
						if (intersection.height() / 8 < intersection.width() &&  // touch from above
							intersection.bottom() == getBounds().bottom())
						{
							enemy->kickFromTop(this);
							move(-m_speed * delta_time);
							m_speed.y = -0.3f;
						}
						else                                                     /// touch from side
							enemy->touchSide(this);
					}
					else enemy->touchSide(this);
				}
			}
		}
	}

}

Vector CMario::getInputDirection()
{
	return m_input_direcition;
}

void CMario::standUp()
{
	if (m_seated)
	{
		setSize(big_mario_size);
		move(seated_mario_size - big_mario_size);
		m_seated = false;
	}
}

void CMario::seatDown()
{
	if (!m_seated)
	{
		setSize(seated_mario_size);
		move(big_mario_size - seated_mario_size);
		m_seated = true;
	}
}

bool CMario::isSeated() const
{
	return m_seated;
}

void CMario::inputProcessing(float delta_time)
{
	auto& input_manager = MarioGame().inputManager();

	m_input_direcition = input_manager.getXYAxis();
 
	if (!m_climb) //normal
	{
		// walk-run
		m_x_max_speed = (input_manager.isButtonPressed("Fire") && !m_jumped && !m_speed.y) ? run_speed : walk_speed;

		// left-right
		if (!m_seated || m_jumped)
		{ 
			float k = 1.f;
			if (m_jumped) 
				k = 0.7f;
			if ((m_input_direcition.x<0)   && !(m_collision_tag & ECollisionTag::left))
				addImpulse(Vector::left*(acceleration*delta_time*k));
			if ((m_input_direcition.x>0)  && !(m_collision_tag & ECollisionTag::right))
				addImpulse(Vector::right*(acceleration*delta_time*k));

			if (m_grounded && m_input_direcition.x)
				m_direction.x = m_input_direcition.x;
		}

		// seat-down / stand-up
		if (m_grounded && m_rank != MarioRank::small)
		{
			if (m_input_direcition.y > 0 && !m_seated)
				seatDown();
			else if (m_input_direcition.y <= 0 && m_seated)
				standUp();
			
		}

		if (m_grounded)
			m_jumped = false;

		//fire 
		if (input_manager.isButtonDown("Fire") && !m_seated && m_fire_timer > fire_rate && m_rank == MarioRank::fire)
		{
			fire();
			m_fire_timer = 0;
		}
		m_fire_timer += delta_time;
	}
	else  // climbing
	{
		if (!m_input_direcition.x)
		{
			if (m_input_direcition == Vector::up)
			{
				if (getPosition().y >= m_used_ladder->getPosition().y)
				  move(climb_speed*Vector::up*delta_time);
			}
			else if (m_input_direcition == Vector::down)
			{
				if (getBounds().bottom() <= m_used_ladder->getBounds().bottom())
				  move(climb_speed*Vector::down*delta_time);
			}
		} 
		else 
		{
			addImpulse({ run_speed*m_input_direcition.x,-0.3f });
			m_climb = false;
		}
 
		if (m_collision_tag & ECollisionTag::floor)
			m_climb = false;
	}

	m_jumping_timer += delta_time;

	if (m_jump_timer)
	{
		m_jump_timer -= delta_time;
		if (m_jump_timer < 0)
			m_jump_timer = 0;
	}

	// jump
	if (!m_in_water)
	{
		if (input_manager.isButtonPressed("Jump") && !(m_collision_tag & ECollisionTag::cell))
		{
			if (m_grounded && !m_jump_timer)
			{
				MarioGame().playSound("jump_super");
				m_grounded = m_climb = false;
				m_jumped = true;
				m_jumping_timer = 0.65f*jump_time;
				addImpulse(1.5*Vector::up*jump_force);
				m_jump_timer = jump_rate;
			}
			else if (m_jumping_timer < jump_time)
	    		addImpulse(Vector::up*jump_force* (1 - m_jumping_timer/jump_time));
		}
		else
			m_jumping_timer = jump_time;
	}
	else
	{
		if (input_manager.isButtonPressed("Jump"))
		{
			if (m_grounded)
				m_jumped = true;
			if (m_jumping_timer > 100)
			{
				addImpulse(0.5*Vector::up*jump_force);
				m_jumping_timer = 0;
			}
		}
	}

	//jump off from ladder
	if (input_manager.isButtonPressed("Jump") && m_climb && !m_input_direcition.y)
	{
		addImpulse(0.4f*Vector::up*jump_force);
		m_grounded = m_climb = false;
		m_jumped = true;
	}
}

void CMario::animationProcessing(float delta_time)
{
	std::string  rank_string = (m_rank == MarioRank::small) ? "_small" : "_big";

	if (m_climb)
	{
		playAnimation("climb" + rank_string);
		m_animator->setSpeed("climb" + rank_string, m_input_direcition.y?0.005f:0.f);
	}
	else if (m_in_water)
	{
		if (!m_speed.y)
		{
			if (m_speed.x)
			{
				playAnimation("walk" + rank_string);
				m_animator->flipX(m_speed.x < 0);

                m_animator->setSpeed("walk" + rank_string, 0.02f *(std::abs(m_speed.x) / walk_speed));

				if ((m_direction == Vector::left && m_speed.x > 0) ||
					(m_direction == Vector::right && m_speed.x < 0))
					playAnimation("slip" + rank_string);
			}
			else
				playAnimation("idle" + rank_string);
		}
		else
		{
			playAnimation("swim" + rank_string);
			m_animator->flipX(m_speed.x < 0);
            m_animator->setSpeed("swim" + rank_string, 0.02f *(std::abs(m_speed.x) / walk_speed));
		}
	}
	else // normal
	{
		if (getRank() == MarioRank::fire && m_fire_timer < 200 && !m_seated)
			playAnimation("shoot");
		else if (m_seated)
			playAnimation("seat" + rank_string);
		else if (m_jumped)
			playAnimation("jump" + rank_string);
		else if (m_grounded)
		{
			if (m_speed.x)
			{
				playAnimation("walk" + rank_string);
				m_animator->flipX(m_speed.x < 0);

                m_animator->setSpeed("walk" + rank_string, 0.02f *(std::abs(m_speed.x) / walk_speed));

				if ((m_direction == Vector::left && m_speed.x > 0) ||
					(m_direction == Vector::right && m_speed.x < 0))
					playAnimation("slip" + rank_string);
			}
			else
				playAnimation("idle" + rank_string);
		}
		else 
			m_animator->setSpeed("walk" + rank_string, 0.f);
	}

	//Damaged flashing
	if (m_spawn_timer)
	{
		m_spawn_timer -= delta_time;

		if (int(m_spawn_timer / 80) % 2)
			m_animator->setColor(sf::Color::White);
		else
			m_animator->setColor(sf::Color::Transparent);

		if (m_spawn_timer < 0)
		{
			m_animator->setColor(sf::Color::White);
			m_spawn_timer = 0;
		}
	}
}
 
void CMario::physicProcessing(float delta_time)
{
	if (m_climb)
		return;

	if (!m_in_water)
	{
		if (m_grounded)
		{
			addImpulse(-Vector::right*math::sign(m_speed.x)*friction_force*delta_time); //Ground friction force
		}
		else
			addImpulse(Vector::down*gravity_force * delta_time);                // Gravity force
		
																				// Speed limits				
        if (std::abs(m_speed.x) < 0.01f)
			m_speed.x = 0.f;
        m_speed.x = math::clamp(m_speed.x, -m_x_max_speed, m_x_max_speed);
		move(m_speed*delta_time);
	}
	else
	{
		if (m_grounded)
			addImpulse(-Vector::right*math::sign(m_speed.x)*friction_force*delta_time); //Ground friction force
		else
			addImpulse(0.25*Vector::down*gravity_force * delta_time);                // Gravity force

																				// Speed limits				
        if (std::abs(m_speed.x) < 0.01)
			m_speed.x = 0;
		m_speed.x = math::clamp(m_speed.x, -m_x_max_speed/2, m_x_max_speed/2);
		move(m_speed*delta_time);
		if (getPosition().y < 0)
			m_speed.y = 0.1f;

	}
}
 
Vector CMario::getSpeed() const
{
	return m_speed;
}

void CMario::update(int delta_time)
{
	if (!m_blocks)
		m_blocks = getParent()->findObjectByType<CBlocks>();
	assert(m_blocks);
	m_current_state->update(delta_time);
	CGameObject::update(delta_time);
}

void CMario::setState(MarioState state)
{
	if (m_current_state)
	{
		m_current_state->onLeave();
		delete m_current_state;
	}

	switch (state)
	{
		case(MarioState::normal):
		{
			m_current_state = new CNormalMarioState();
			break;
		}
		case(MarioState::promoting):
		{
			m_current_state = new CPromotingMarioState();
			break;
		}
		case(MarioState::demoting):
		{
			m_current_state = new CDemotingMarioState();
			break;
		}
		case(MarioState::died):
		{
			m_current_state = new CDiedMarioState();
			break;
		}
	}
 
	m_current_state->setMario(this);
	m_current_state->onEnter();
}

void CMario::setState(IMarioState* state)
{
	if (m_current_state)
	{
		m_current_state->onLeave();
		delete m_current_state;
	}
	m_current_state = state;
	m_current_state->setMario(this);
	m_current_state->onEnter();
}

void CMario::fire()
{
	Vector pos = getBounds().center() + m_direction * 25 + Vector::up*8;
	getParent()->addObject(new CMarioBullet(pos,m_direction));
	MarioGame().playSound("fireball");
}

void CMario::addImpulse(Vector _speed)
{
	m_speed += _speed;
}

bool CMario::isGrounded() const
{
	return m_grounded;
}

bool CMario::isClimbing() const
{
	return m_climb;
}

void CMario::setUnclimb()
{
	m_climb = false;
	m_jumped = false;
	m_grounded = false;
	m_speed = Vector::down*0.2f;
	playAnimation("jump_" + std::string(isSmall() ? "small" : "big"));
}

void CMario::setInvincibleMode(bool value)
{
	m_invincible_mode = value;
}

void CMario::collisionProcessing(float delta_time)
{
	m_collision_tag = ECollisionTag::none;
	setPosition(m_blocks->collsionResponse(this->getBounds(), m_speed, delta_time, m_collision_tag));
 
	auto items = getParent()->findObjectsByType<CItem>();
	CMario* mario = this;

	for (auto item : items)
		if (item->getBounds().isIntersect(mario->getBounds()))
			item->collsionResponse(this, m_collision_tag, delta_time);

	if (!m_climb && m_input_direcition.y)
	{
		auto ladders = getParent()->findObjectsByType<CLadder>();
		for (auto ladder : ladders)
			if (ladder->getBounds().isIntersect(mario->getBounds()))
			{
				m_used_ladder = ladder;
				m_climb = true;
				m_jumped = true;

				int x_ladder = ladder->getBounds().center().x;
				if (x_ladder > getBounds().center().x)
				{
					setPosition(x_ladder - getBounds().width(), getPosition().y);
					m_animator->flipX(false);
				}
				else
				{
					setPosition(x_ladder, getPosition().y);
					m_animator->flipX(true);
				}
				m_speed = Vector::zero;
				break;
			}
	}

	m_grounded = m_collision_tag & ECollisionTag::floor;

	if ((m_collision_tag & ECollisionTag::right) && m_input_direcition != Vector::left) m_speed.x = 0;
    if ((m_collision_tag & ECollisionTag::left) && m_input_direcition != Vector::right) m_speed.x = 0;

	if (m_collision_tag & ECollisionTag::cell) m_speed.y = 0.1f;
	if (m_collision_tag & ECollisionTag::floor) m_speed.y = 0.f;
	 
}
 
void CMario::setSpeed(const Vector& _speed)
{
	m_speed = _speed;
}

void CMario::setGrounded(bool val)
{
	m_grounded = val;
}

void CMario::onActivated() 
{
	if (getProperty("InWater").isValid() && getProperty("InWater").asBool() == true)
		m_in_water = true;
}

void CMario::start()
{
	if (getProperty("SpawnDirection").isValid())
	{
		Vector direction = toVector(getProperty("SpawnDirection").asString());
		if (direction != Vector::zero)
		{
			move(-direction*0.03f * 2000);
			setState(new CTransitionMarioState(direction*0.03f, 2000));
		}
	}

    if (getProperty("StartScript").isValid())
	{
        auto script = getProperty("StartScript").asString();
        if (script == "GoToPortal")
        {
			setState(new CGoToPortalState());
        }
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------
void IMarioState::setMario(CMario* mario)
{
	m_mario = mario;
}

CMario* IMarioState::mario()
{
	return m_mario;
}

void IMarioState::playAnimation(const std::string& anim_name)
{
	mario()->playAnimation(anim_name);
}

void IMarioState::setMarioRank(const MarioRank& rank)
{
	mario()->setRank(rank);
}

void IMarioState::setMarioNormalPallete()
{
	mario()->m_animator->setPallete(NULL);
}

void IMarioState::setMarioFirePallete()
{
	mario()->m_animator->setPallete(&(mario()->m_fire_pallete));
}

void  IMarioState::enableScene(bool value)
{
	auto m = mario();
	mario()->getParent()->foreachObject([m, value](CGameObject* obj) {if (obj != m) if (value) obj->enable(); else  obj->disable(); });
}

void  IMarioState::setMarioState(const MarioState& state)
{
	mario()->setState(state);
}

void IMarioState::setMarioSpawnProtection()
{
	mario()->m_spawn_timer = 3000;
}

//-----------------------------------------------------------------------------------------------------------------------------------------

void CPromotingMarioState::onEnter()
{
	m_promoting_timer = 1500;
	mario()->m_animator->setColor(sf::Color::White);
	enableScene(false);
	if (mario()->getRank() ==  MarioRank::small)
	{
		setMarioRank(MarioRank::big);
		playAnimation("growing");
	}
	else  if (mario()->getRank() == MarioRank::big)
		setMarioRank(MarioRank::fire);
		
	mario()->show();
}

void CPromotingMarioState::onLeave()
{
	enableScene(true);
	if (mario()->getRank() == MarioRank::fire)
		setMarioFirePallete();
}

void CPromotingMarioState::update(int delta_time)
{
	auto _mario = mario();
	if (_mario->getRank() == MarioRank::fire)
		_mario->m_animator->setPallete((int(m_promoting_timer / 100) % 2) ? &(_mario->m_fire_pallete) : NULL);
	m_promoting_timer -= delta_time;
	if (m_promoting_timer < 0)
		_mario->setState(MarioState::normal);
} 

//---------------------------------------------------------------------------------------------------------------------------------------------
 
void CDemotingMarioState::onEnter()
{
	enableScene(false);
	m_promoting_timer = 1500;
	setMarioNormalPallete();
	setMarioRank(MarioRank::small);
	playAnimation("demoting");
	CMarioGame::instance()->playSound("pipe");
	mario()->show();
}

void CDemotingMarioState::onLeave()
{
	enableScene(true);
	setMarioSpawnProtection();
}

void CDemotingMarioState::update(int delta_time)
{
	playAnimation("demoting");
	m_promoting_timer -= delta_time;
	if (m_promoting_timer < 0)
		mario()->setState(MarioState::normal);
}
 
//----------------------------------------------------------------------------------------------------------------------------------------------

void CNormalMarioState::onEnter()
{
}

void CNormalMarioState::onLeave()
{
}

void CNormalMarioState::update(int delta_time)
{
	auto _mario = mario();
	_mario->inputProcessing(delta_time);
	_mario->physicProcessing(delta_time);
	_mario->collisionProcessing(delta_time);
	_mario->kickBlocksProcessing();
	_mario->kickEnemiesProcessing(delta_time);
	_mario->animationProcessing(delta_time);
		
	//check if mario fall undergroud
	static int screen_height = CMarioGame::instance()->screenSize().y - 200;
	if (_mario->getBounds().bottom() > screen_height)
	{
		if (_mario->getParent()->castTo<CMarioGameScene>()->getLevelName().substr(0,2) != "Sk")
			setMarioState(MarioState::died);
		if (_mario->m_invincible_mode)
		{
			CMarioGame::instance()->invincibleMode(false);
			CMarioGame::instance()->stopMusic();
		}
	}

	if (_mario->m_invincible_mode)
	{
		_mario->m_invincible_timer -= delta_time;
		_mario->m_animator->setPallete((int(_mario->m_invincible_timer / 100) % 2) ? &(_mario->m_black_pallete) : &(_mario->m_fire_pallete));
		if (_mario->m_invincible_timer < 0)
		{
			_mario->m_animator->setPallete((_mario->getRank() == MarioRank::fire)?&_mario->m_fire_pallete:nullptr);
			_mario->m_invincible_mode = false;
			CMarioGame::instance()->invincibleMode(false);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------

CTransitionMarioState::CTransitionMarioState(const Vector&  speed, int time)
{
		m_speed = speed;
		m_timer = time;
}

void CTransitionMarioState::onEnter() 
{
		 mario()->setSpeed(m_speed.normalized()*mario()->walk_speed/2.f);
		 mario()->moveUnderTo(mario()->getParent()->findObjectByType<CBlocks>());

		auto piranas_plant = mario()->getParent()->findObjectsByType<CPiranhaPlant>();
		for (auto& pirana : piranas_plant)
			pirana->hideInTube();

		if (m_speed.y>0 || m_speed.x>0)
			MarioGame().playSound("pipe");
}

void CTransitionMarioState::onLeave() 
{
		mario()->moveToFront();
		mario()->setSpeed(Vector::zero);
}

void CTransitionMarioState::update(int delta_time)
{
		if (m_timer > 0)
		{
			m_timer -= delta_time;
			mario()->move(delta_time*m_speed);
		    mario()->animationProcessing(delta_time);
		}
		else
			mario()->setState(MarioState::normal);
}

//----------------------------------------------------------------------------------------------------------------------------------------------

CGoToCastleMarioState::CGoToCastleMarioState()
{

}

void CGoToCastleMarioState::onEnter()
{
	CMarioGame::instance()->musicManager().stop();
	auto m_block = mario()->getParent()->findObjectByType<CBlocks>();
	m_cell_y = m_block->height() - m_block->blockSize().y * 4 - (mario()->isSmall()?2:34);
	mario()->m_input_direcition = Vector::zero;
	if (mario()->isSeated())
		mario()->standUp();
}

void CGoToCastleMarioState::onLeave() 
{ 

}

void CGoToCastleMarioState::update(int delta_time) 
{
    switch(m_state)
    {
        case(State::start):
        {
            mario()->m_animator->flipX(false);
            auto anim_name = std::string("climb_") + (mario()->isSmall() ? "small" : "big");
            mario()->playAnimation(anim_name);
            mario()->m_animator->setSpeed(anim_name, 0.005f);
            m_state = State::go_down;
            MarioGame().playSound("flagpole");
            break;
        }
        case(State::go_down):
        {
            if (mario()->getPosition().y < m_cell_y)
                mario()->move(Vector::down*delta_time*0.25f);
            else
            {
                m_state = State::overturn;
                mario()->m_animator->flipX(true);
                mario()->move(Vector::right * 32.f);
                m_delay_timer = 500;
            }
            break;
        }
        case(State::overturn):
        {
            m_delay_timer -= delta_time;
            if (m_delay_timer < 0)
            {
                m_state = State::walk;
				CMarioGame::instance()->playSound("stage_clear");
                auto anim_name = std::string("walk_") + (mario()->isSmall() ? "small" : "big");
                mario()->playAnimation(anim_name);
                mario()->m_animator->setSpeed(anim_name, 0.003f);
                mario()->m_animator->flipX(false);
                mario()->m_input_direcition = Vector::zero;
            }
            break;
        }
        case(State::walk):
        {
            mario()->addImpulse(Vector::right*10.f);
            mario()->physicProcessing(delta_time);
            mario()->collisionProcessing(delta_time);
            auto portals = mario()->getParent()->findObjectsByType<CLevelPortal>();
            for (auto& portal : portals)
                if (portal->getBounds().isContainByX(mario()->getPosition()))
                {
                    m_state = State::wait;
                    mario()->hide();
                    m_next_level = portal->getProperty("Level").asString();
                    if (portal->getProperty("SubLevel").isValid())
                        m_next_sub_level = portal->getProperty("SubLevel").asString();
                    MarioGame().setEndLevelStatus();
                }
            break;
        }
        case(State::wait):
        {
            if (MarioGame().getGameTime() == 0)
            {
                m_delay_timer = 3500;
                m_state = State::next_level;
				auto flag = mario()->getParent()->findObjectByType<CCastleFlag>();
				assert(flag); // there is no castle flag in level
				flag->liftUp();
            }
            break;
        }
        case(State::next_level):
        {
            m_delay_timer -= delta_time;
            if (m_delay_timer < 0)
            {
                MarioGame().loadLevel(m_next_level);
                if (!m_next_sub_level.empty())
                    MarioGame().loadSubLevel(m_next_sub_level);
                MarioGame().showStatus();
                return;
            }
            break;
        }
    };
		mario()->m_animator->update(delta_time);
}

//----------------------------------------------------------------------------------------------------------------------------------------------

CGoToPortalState::CGoToPortalState()
{

}

void CGoToPortalState::onEnter() 
{
		auto anim_name = std::string("walk_") + (mario()->isSmall() ? "small" : "big");
		mario()->playAnimation(anim_name);
		mario()->m_animator->setSpeed(anim_name, 0.003f);

}

void CGoToPortalState::onLeave() 
{

	
}

void CGoToPortalState::update(int delta_time) 
{
    mario()->m_input_direcition = Vector::right;
    mario()->m_speed.x = mario()->walk_speed*0.6f;
    mario()->move(mario()->getSpeed()*delta_time);
    //mario()->physicProcessing(delta_time);
    mario()->collisionProcessing(delta_time);
    mario()->m_collision_tag |= ECollisionTag::right;
    mario()->setGrounded();
}

//----------------------------------------------------------------------------------------------------------------------------------------------

CGoToPrincessState::CGoToPrincessState()
{

}

void CGoToPrincessState::onEnter()
{
	m_princess = mario()->getParent()->findObjectByType<CPrincess>();
	mario()->m_input_direcition = Vector::right;
	mario()->m_direction = Vector::right;
	auto anim_name = std::string("walk_") + (mario()->isSmall() ? "small" : "big");
	mario()->playAnimation(anim_name);
	mario()->m_animator->setSpeed(anim_name, 0.003f);
	mario()->m_animator->flipX(false);
	if (mario()->isSeated())
		mario()->standUp();
}

void CGoToPrincessState::onLeave()
{

}

void CGoToPrincessState::update(int delta_time)
{
	if (m_princess->getPosition().x - mario()->getBounds().right() > 16)
	{
		mario()->m_speed.x = mario()->walk_speed*0.6f;
		mario()->physicProcessing(delta_time);
		mario()->collisionProcessing(delta_time);
		if (mario()->isGrounded())
				mario()->m_animator->update(delta_time);
	}
	else
	{
		mario()->playAnimation("idle_" + std::string(mario()->isSmall() ? "small" : "big"));
		mario()->m_speed.x = 0;
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------

void CDiedMarioState::onEnter() 
{
	enableScene(false);
    mario()->setRank(MarioRank::small);
	mario()->m_animator->play("died");
	mario()->m_speed = Vector::zero;
	mario()->addImpulse(Vector::up * 0.8f);
	mario()->setGrounded(false);
	mario()->m_jumped = false;
	CMarioGame::instance()->musicManager().stop();
	CMarioGame::instance()->playSound("mario_die");
}

void CDiedMarioState::onLeave()
{
	
}

void CDiedMarioState::update(int delta_time)
{
	auto _mario = mario();
	_mario->physicProcessing(delta_time);
	m_delay_timer -= delta_time;
	if (m_delay_timer < 0)
	{
		CMarioGame::instance()->marioDied();
	}
}
