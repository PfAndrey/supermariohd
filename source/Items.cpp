#include "Items.h"
#include "Enemies.h"
#include "Blocks.h"
#include "Mario.h"
#include "SuperMarioGame.h"
#include <cmath>

//-----------------------------------------------------------------------------------------------------------------------------------------------
void CPlatform::collsionResponse(CMario* mario,   ECollisionTag& collision_tag, int delta_time)
{
	mario->setPosition(::collsionResponse(mario->getBounds(), mario->getSpeed(), this->getBounds(), this->getSpeedVector(), delta_time, collision_tag));
}

CMoveablePlatform::CMoveablePlatform()
{
	m_platform_type = PlatformType::no_init;
	m_amplitude = 0;
}

void CMoveablePlatform::draw(sf::RenderWindow* render_window)
{
	m_sprtite.setPosition(getPosition());
	render_window->draw(m_sprtite);
}

void CMoveablePlatform::update(int delta_time)
{
	switch (m_platform_type)
	{
	  case(PlatformType::elevator):
	  {
		  static const int bottom = getParent()->findObjectByType<CBlocks>()->height();
		  if (m_speed.y > 0 && getPosition().y > m_bottom)
			  setPosition({ m_center.x,-16.f });
		  if (m_speed.y < 0 && getPosition().y < 0)
			  setPosition({ m_center.x,m_bottom + 16.f });
		  break;
	  }
	  case(PlatformType::vertical):
	  case(PlatformType::horizontal):
	  {
		  int k = 1;
		  if ((m_platform_type == PlatformType::horizontal && getPosition().x > m_center.x) ||
			  (m_platform_type == PlatformType::vertical && getPosition().y > m_center.y))
			  k = -1;
		  m_timer += delta_time;
		  m_speed += k*m_orientation*m_acceleration*delta_time;
		  break;
	  }
	}
 
	move(m_speed*delta_time);
}

Vector CMoveablePlatform::getSpeedVector()
{
	return 2 * m_speed;
}

void CMoveablePlatform::start()
{
	const int height = 16;
	m_mario = getParent()->findObjectByName<CMario>("Mario");
	
	//read properties
	m_platform_type = (PlatformType)getProperty("Orientation").asInt();
	m_amplitude = getProperty("Amplitude").asInt();
	m_period_time = getProperty("Period").asFloat() * 1000;
	m_size = getProperty("width").asFloat();
	m_center = getPosition();
	assert(m_platform_type != PlatformType::no_init);
	m_sprtite.setTexture(*MarioGame().textureManager().get("Items"));
	m_sprtite.setTextureRect({ 0,0,m_size,height });
	setSize({ m_size, height });
	m_timer = 0;// m_amplitude + getProperty("Phase").asInt() / (m_max_speed*speed_koef);

	switch (m_platform_type)
	{
	  case(PlatformType::elevator):
	  {
		m_bottom = getParent()->findObjectByType<CBlocks>()->height();
		m_speed.y = m_bottom / m_period_time;
		break;
	  }
	  case(PlatformType::horizontal):
	  case(PlatformType::vertical):
	  {
		  m_orientation = (m_platform_type == PlatformType::horizontal) ? Vector::right : Vector::down;
		  m_acceleration = 2 * (m_amplitude) / pow(m_period_time * 0.25f, 2);
		  m_speed = m_acceleration * (m_period_time * 0.25f)*m_orientation;
		  setPosition(m_center);
		  break;
	  }
	  case(PlatformType::skate):
	  {
		  m_sprtite.setTextureRect({ 112,0,m_size,height });
		  m_orientation = Vector::right;
		  m_speed = Vector::zero;
		  break;
	  }
	} 
	if (getProperty("Phase").asInt() != 0)
		m_speed = -m_speed;
}

void CMoveablePlatform::collsionResponse(CMario* mario, ECollisionTag& collision_tag, int delta_time)
{
 	if (m_platform_type == PlatformType::elevator)
    {
	 	if (mario->getSpeed().y >= 0 && getBounds().top() > abs(getBounds().top() - mario->getBounds().bottom()) < 8)
	 		CPlatform::collsionResponse(mario, collision_tag, delta_time);
 	}
	else
			CPlatform::collsionResponse(mario, collision_tag, delta_time);

	if (collision_tag & ECollisionTag::floor)
	{
		if (m_platform_type == PlatformType::elevator)
		{
			auto mario_pos = mario->getPosition();
			mario->setPosition(Vector(mario_pos.x, getPosition().y - mario->getBounds().size().y) +
				delta_time * getSpeedVector() +
				(m_speed.y < 0 ? 2 * Vector::down : 3 * Vector::up));
		}
		else if (m_platform_type == PlatformType::skate)
		{
			if (m_speed == Vector::zero)
				m_speed = Vector::right*SKATE_SPEED;
		}
		else
		{
			if (m_speed.y < 0)
				mario->move({ 0.f,-getPosition().y + mario->getBounds().bottom() - 2 });
		}
	}
 
}

//-----------------------------------------------------------------------------------------------------------------------------------------------

CFallingPlatform::CFallingPlatform()
{

}

void CFallingPlatform::draw(sf::RenderWindow* render_window)
{
	m_sprtite.setPosition(getPosition());
	render_window->draw(m_sprtite);
}

void CFallingPlatform::setSpeed(const Vector& speed)
{
	m_speed = speed;
}

void CFallingPlatform::update(int delta_time)
{ 
	if (m_stayed)
	{
		m_speed += Vector::down * delta_time*0.008f;
		m_stayed = false;
		if (m_moving_callback)
			m_moving_callback();
	}
	else
	{
			m_speed += Vector::up*math::sign(m_speed.y)*0.001f*delta_time;
            if (std::abs(m_speed.y < 0.05)) m_speed = Vector::zero;
	}
	move(m_speed*delta_time);
}

void CFallingPlatform::collsionResponse(CMario* mario, ECollisionTag& collision_tag, int delta_time)
{
	CPlatform::collsionResponse(mario, collision_tag, delta_time);
	if (collision_tag & ECollisionTag::floor)
	{
		m_stayed = true;
		m_mario = mario;
	}
}

Vector CFallingPlatform::getSpeedVector()
{
	return m_speed;
}

void CFallingPlatform::onActivated()
{
	int m_size = getProperty("width").asFloat();
	Vector m_center = getPosition();
	setBounds(Rect(m_center - Vector(m_size, 16) / 2, Vector(m_size, 16)));
	m_sprtite.setTexture(*MarioGame().textureManager().get("Items"));
	m_sprtite.setTextureRect({ 0,0,m_size,16 });
}

void CFallingPlatform::start()
{

}

void CFallingPlatform::addImpulse(const Vector& speed)
{
	m_speed += speed;
}

void CFallingPlatform::setMovingCallback(const std::function<void()>& func)
{
	m_moving_callback = func;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------

CPlatformSystem::CPlatformSystem()
{
	auto texture = MarioGame().textureManager().get("Items");
	for (auto& sprite : m_sprites)
		sprite.setTexture(*texture);
	m_sprites[0].setTextureRect({ 100 ,16,32,32 });
	m_sprites[1].setTextureRect({ 132 ,16,-32,32 });
	m_sprites[2].setTextureRect({ 132,16,32,32 }); //h
	m_sprites[3].setTextureRect({ 100,48,32,32 });
	m_sprites[4].setTextureRect({ 132,48,-32,32 });
}

void CPlatformSystem::onActivated()
{
	setSize({ getProperty("width").asFloat(), getProperty("height").asFloat() });
	m_left_platform = new CFallingPlatform();
	m_right_platform = new CFallingPlatform();
	float width = getBounds().width() / 3;
	m_left_platform->setProperty("width", Property(width));
	m_right_platform->setProperty("width", Property(width));
	m_left_platform->setPosition(getBounds().leftBottom() - Vector::up * 16 + Vector::right*width/2);
	m_right_platform->setPosition(getBounds().leftBottom() - Vector::up * 16 + Vector::right*2*width + Vector::right*width / 2);
	addObject(m_left_platform);
	addObject(m_right_platform);
	m_left_platform->setMovingCallback(std::bind(&CPlatformSystem::onLeftPlatformMove, this));
	m_right_platform->setMovingCallback(std::bind(&CPlatformSystem::onRightPlatformMove, this));
}

void CPlatformSystem::draw(sf::RenderWindow* render_window)
{
	CGameObject::draw(render_window);
	int width = getBounds().width() / 6 - 16;
	m_sprites[0].setPosition(getPosition() + Vector::right*width);
	render_window->draw(m_sprites[0]);
	m_sprites[1].setPosition(getBounds().rightTop() + Vector::left * 32 + Vector::left*width);
	render_window->draw(m_sprites[1]);
	int left_shoulder = m_left_platform->getPosition().y - getPosition().y - 32;
	drawLinearSprite_v(m_sprites[3], { (int)getPosition().x + width,(int)getPosition().y + 32,32,left_shoulder }, render_window);
	int right_shoulder = m_right_platform->getPosition().y - getPosition().y - 32;
	drawLinearSprite_v(m_sprites[4], { (int)getBounds().right() - (width + 32) ,(int)getPosition().y + 32,32,right_shoulder }, render_window);
	drawLinearSprite_h(m_sprites[2], { (int)getPosition().x + 32 + width, (int)getPosition().y, (int)getBounds().width() - 2*(32  + width), 32 }, render_window);
}

void CPlatformSystem::update(int delta_time)
{
	CGameObject::update(delta_time);
}

void CPlatformSystem::onLeftPlatformMove()
{
	if (m_left_platform->getPosition().y - getPosition().y > 2*getBounds().height() - 32)
		m_left_platform->setSpeed(Vector::zero);
	else
	{
		float wr = 2 * (getBounds().height() + getPosition().y) - m_left_platform->getPosition().y;
		m_right_platform->setPosition(m_right_platform->getPosition().x, + wr);
	}
}

void CPlatformSystem::onRightPlatformMove()
{
	if (m_right_platform->getPosition().y - getPosition().y > 2*getBounds().height() - 32)
		m_right_platform->setSpeed(Vector::zero);
	else
	{
		float wl = 2 * (getBounds().height()+ getPosition().y) - m_right_platform->getPosition().y;
		m_left_platform->setPosition(m_left_platform->getPosition().x,  + wl);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
CJumper::CJumper()
{
	 m_sprite_sheet = new CSpriteSheet();
	 m_sprite_sheet->load(*MarioGame().textureManager().get("Items"), { { 0,20,32,64 },{ 32,36,32,48 },{ 64,52,32,32 } });
	 addObject(m_sprite_sheet);
}

void CJumper::draw(sf::RenderWindow* render_window)
{
	m_sprite_sheet->setPosition(getPosition());
	m_sprite_sheet->draw(render_window);
}

void CJumper::update(int delta_time)
{
	if (m_zipped)
	{
		m_timer += delta_time;
		int new_state = int(m_timer / 100) % 4 + 1;
		if (new_state == 4)
		{
			new_state = 0;
			m_zipped = false;
		}
		if (new_state != m_state)
		{
			m_state = new_state;
			int index = m_state;
			if (index > 2) index = 1;
			Rect old_bounds = getBounds();
			old_bounds.setTop(m_bottom - 64 + 16 * index);
			setBounds(old_bounds);
			m_sprite_sheet->setSpriteIndex(index);

			if (m_state == 2)
				m_mario->addImpulse(Vector::up*0.5);
			
		}
	}
}

void  CJumper::collsionResponse(CMario* mario, ECollisionTag& collision_tag, int delta_time)
{
	 if (mario->getSpeed().y >= 0)
	 {
		 mario->setPosition(::collsionResponse(mario->getBounds(), mario->getSpeed(), getBounds(), Vector::zero, delta_time, collision_tag));
		 mario->setSpeed({ mario->getSpeed().x,0.f});
	 }

	 if (collision_tag == ECollisionTag::floor)
	 {
		 m_mario = mario;
		 if (m_state == 0)
		 {
			 m_timer = 0;
			 m_zipped = true;
		 }
	 }
}

void CJumper::onActivated() 
{
	setSize({ 32,64 });
	m_bottom = getBounds().bottom();
}

//-------------------------------------------------------------------------------------------------------------------------------------
CLadder::CLadder(const Vector& pos, CMario* mario)
{
	setBounds({ pos.x + 10.f, 0.f, 10.f,pos.y + 32.f });
	m_sprite.setTexture(*MarioGame().textureManager().get("Items"));
}

void CLadder::draw(sf::RenderWindow* render_window)
{
	int k = getBounds().height() / 32;
	int pos_x = getBounds().center().x - 32/2;

	for (int i = 0; i <= k; ++i)
	{
		m_sprite.setTextureRect({ 212,!i ? 0 : (64 - (i % 2) * 32),32,i == k?(int(getBounds().height()) % 32):32 });
		m_sprite.setPosition(pos_x, i * 32+getPosition().y);
		render_window->draw(m_sprite);
	}
}

void CLadder::update(int delta_time)
{
	CGameObject::update(delta_time);
	if (m_timer == 0)
		MarioGame().playSound("powerup_appears");
	m_timer += delta_time;
	float cur_height = m_timer*0.04f;
	if (cur_height < m_height)
		setBounds({ getPosition().x, m_bottom - cur_height,  m_width, cur_height });
}

void CLadder::onActivated()
{
	m_height = getBounds().height();
	m_width = getBounds().width();
	m_bottom = getPosition().y + m_height;
}

void CLadder::start()
{
	getParent()->findObjectByName<CMario>("Mario")->moveToFront();
}

//-------------------------------------------------------------------------------------------------------------------------------------
CFireBar::CFireBar()
{
    m_animator.create("fly", *MarioGame().textureManager().get("Mario"), { { 0,0,16,16 },{ 16,0,16,16 },{ 16,0,-16,16 },{ 16,16,16,-16 } }, 0.01f);
}

void CFireBar::CFireBar::draw(sf::RenderWindow* render_window)
{
	for (auto fire_pos : m_fire_pos)
	{
		m_animator.setPosition(fire_pos);
		m_animator.draw(render_window);
	}
}

void CFireBar::CFireBar::update(int delta_time)
{
	m_animator.update(delta_time);
  
	//rotation processing
	m_timer += delta_time;
	float angle = -m_speed* m_timer / 400;
	Vector rot((float)cos(angle), (float)sin(angle));
	for (size_t i=0; i < m_fire_pos.size(); ++i)
	 m_fire_pos[i] = getPosition() + rot * 16*i - Vector(4,4);

    //check collision with mario processing
	Rect own_bounds = Rect(m_fire_pos.front(), m_fire_pos.back() - m_fire_pos.front());
	own_bounds.normalize();
	auto mario_bounds = m_mario->getBounds();
	if (own_bounds.isIntersect(mario_bounds))
		for (auto fire_pos : m_fire_pos)
			if (mario_bounds.isContain(fire_pos + Vector(8, 8)))
			{
				m_mario->reciveDamage();
				break;
			}
}

void CFireBar::onActivated()
{
	int fires = getProperty("height").asFloat() / 16.f;
	m_fire_pos.resize(fires);
}

void CFireBar::start()
{
	m_speed = getProperty("Speed").asFloat();
	m_mario = getParent()->findObjectByName<CMario>("Mario");
}

//----------------------------------------------------------------------------------------------------------------------------------

CLevelPortal::CLevelPortal()
{

}

void CLevelPortal::goToLevel()
{
	MarioGame().loadLevel(m_level_name);
	if (m_show_status)
		MarioGame().showStatus();
}

void CLevelPortal::goToSublevel()
{
	if (m_came_back_portal)
	{
		m_mario->setUnclimb();
		m_mario->setPosition(m_came_back_portal->getBounds().center().x - m_mario->getBounds().width() / 2.f, m_came_back_portal->getBounds().bottom() - m_mario->getBounds().height());

	 
		if (m_came_back_portal->m_direction != Vector::zero)
		{
			m_mario->move(-m_came_back_portal->m_direction * TRANSITION_TIME * 0.03f);
			if (m_mario->getRank() != MarioRank::small)
				m_mario->move(-m_came_back_portal->m_direction * 32.f);
			m_mario->setState(new CTransitionMarioState(m_came_back_portal->m_direction*0.03f,  TRANSITION_TIME));
		}
 
		m_came_back_portal->m_used = true;
		getParent()->castTo<CMarioGameScene>()->setCameraOnTarget();
	}

	 MarioGame().loadSubLevel(m_sub_level_name);
}
void CLevelPortal::cameBackFromSublevel()
{
	MarioGame().unloadSubLevel();
}

void CLevelPortal::update(int delta_time)  
{	
     if (!m_used && getBounds().isContain(m_mario->getBounds()) &&
        (m_direction == Vector::zero || m_mario->getInputDirection() == m_direction) &&
        (m_mario->isGrounded() || m_direction == Vector::zero || m_mario->isClimbing()))
        {
			m_used = true;
			if (m_direction != Vector::zero)
				m_mario->setState(new CTransitionMarioState(m_direction*0.03f, TRANSITION_TIME));

			switch (m_portal_type)
			{
				 case(PortalType::enterLevel):
				 {
					MarioGame().timer().invoke(std::bind(&CLevelPortal::goToLevel, this), TRANSITION_TIME);
					break;
				 }
				 case(PortalType::enterSublevel):
				 {
					 MarioGame().timer().invoke(std::bind(&CLevelPortal::goToSublevel, this), (m_direction != Vector::zero) ? TRANSITION_TIME : 0);
					 break;
				 }
				 case(PortalType::comebackSublevel):
				 {
					 MarioGame().timer().invoke(std::bind(&CLevelPortal::cameBackFromSublevel, this), (m_direction != Vector::zero) ? TRANSITION_TIME : 0);
					 break;
				 }
			}
	    }
 }

void CLevelPortal::start()  
{
		setSize({ getProperty("width").asFloat(),getProperty("height").asFloat() });
		m_mario = getParent()->findObjectByName<CMario>("Mario");
		m_direction = toVector(getProperty("Direction").asString());
		
		if (getProperty("Level").isValid() && !getProperty("Level").asString().empty())
		{
			m_level_name = getProperty("Level").asString();
			m_portal_type = PortalType::enterLevel;
		} 
		else if (getProperty("SubLevel").isValid() && !getProperty("SubLevel").asString().empty())
		{
			m_sub_level_name = getProperty("SubLevel").asString();
			m_portal_type = PortalType::enterSublevel;
			if (getProperty("ComebackPortal").isValid() && !getProperty("ComebackPortal").asString().empty())
			{
				std::string came_back_portal_str = getProperty("ComebackPortal").asString();
				m_came_back_portal = getParent()->findObjectByName<CLevelPortal>(came_back_portal_str);
				assert(m_came_back_portal); //no cameback portal findeded for sublevel portal
			}
		}
		else
			m_portal_type = PortalType::comebackSublevel;

		if (getProperty("ShowStatus").isValid())
			m_show_status = getProperty("ShowStatus").asBool();
}

//------------------------------------------------------------------------------------------------------------------

void CEndLevelFlag::start() 
{
	setSize({ getProperty("width").asFloat(), getProperty("height").asFloat() });
	m_mario = getParent()->findObjectByName<CMario>("Mario");

	auto m_block = getParent()->findObjectByType<CBlocks>();
	cell_y = m_block->height() - m_block->blockSize().y * 4;
}
 
CEndLevelFlag::CEndLevelFlag()
{
	m_animator.create("base", *MarioGame().textureManager().get("Items"), { 0,180 }, { 32,32 }, 4, 1, 0.01f);
}

void CEndLevelFlag::draw(sf::RenderWindow* render_window) 
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CEndLevelFlag::update(int delta_time)
{
	CGameObject::update(delta_time);
	if (!m_touched)
	{
			  
		 if (m_mario->getBounds().right() > getPosition().x)
		 {
			m_mario->setState(new CGoToCastleMarioState());
			m_touched = true;
		 }
	}
	 else if (getPosition().y < cell_y)
			move(Vector::down*delta_time*0.25f);

	m_animator.update(delta_time);	   
}

//-------------------------------------------------------------------------------------------------------------

CEndLevelKey::CEndLevelKey()
{
	m_sprite.setTexture(*MarioGame().textureManager().get("Items"));
	m_sprite.setTextureRect({ 0,212,32,32 });
	setSize({ 32, 32 });
}
 
void CEndLevelKey::start()
{
	m_mario = getParent()->findObjectByType<CMario>();
	m_bowser = getParent()->findObjectByType<CBowser>();
	m_blocks = getParent()->findObjectByType<CBlocks>();
}
	  
void CEndLevelKey::draw(sf::RenderWindow* render_window)
{
	m_sprite.setPosition(getPosition());
	render_window->draw(m_sprite);
}

void CEndLevelKey::enterState(State state)
{
	m_state = state;
	switch (state)
	{
		    case(State::bridge_hiding):
		    {
				m_mario->disable();
				m_bowser->disable();
				CMarioGame::instance()->stopMusic();
				this->hide();
				m_bridge_blocks = m_blocks->getBridgeBlocks();
				m_delay_timer = 500;
				break;
		    }
			case(State::bowser_run):
			{
				m_bowser->enable();
				m_bowser->noBridge();
				m_delay_timer = 1000;
				break;
			}
			case(State::mario_go_to_princes):
			{
				m_mario->turnOn();
 				m_mario->setState( new CGoToPrincessState());
				break;
			}
			case(State::text_show):
			{
				getParent()->addObject(MarioGame().createText("Thank you Mario!",
					                   getParent()->castTo<CMarioGameScene>()->screenToPoint(MarioGame().screenSize() / 2) - Vector(100, 100)
				 	                   ));
				m_delay_timer = 1000;
				break;
			}
			case(State::go_to_next_level):
			{
				m_delay_timer = 5000;
				break;
			}
		  }
}

void CEndLevelKey::update(int delta_time)
{
		  switch (m_state)
		  {
		  case(State::play):
		  {
			  if (m_mario->getBounds().isIntersect(getBounds()))
			  {
				  if (getParent()->findObjectByName("Bowser"))
					  enterState(State::bridge_hiding);
				  else
				  {
					  CMarioGame::instance()->stopMusic();
					  this->hide();
					  CMarioGame::instance()->playSound("world_clear");
					  enterState(State::mario_go_to_princes);
				  }
			  }
			  break;
		  }

		  case(State::bridge_hiding):
		  {
			  m_delay_timer -= delta_time;
			  if (m_delay_timer < 0)
			  {
				  m_delay_timer = 100;
				  Vector block = m_bridge_blocks.back();
				  m_blocks->clearBlock(block.x, block.y);
				  m_bridge_blocks.pop_back();
				  if (m_bridge_blocks.empty())
					  enterState(State::bowser_run);
				  MarioGame().playSound("breakblock");
			  }
			  break;
		  }
		  case(State::bowser_run):
		  {
			  m_delay_timer -= delta_time;
			  if (m_delay_timer < 0)
			  {
				  enterState(State::mario_go_to_princes);
				  CMarioGame::instance()->playSound("world_clear");
			  }
			  break;
		  }
		  case(State::mario_go_to_princes):
		  {
			  if (m_mario->getSpeed().x == 0)
			  {
				  enterState(State::text_show);
			  }
			  break;
		  }

		  case(State::text_show):
		  {
			  m_delay_timer -= delta_time;

			  if (m_delay_timer < 0)
			  {
				  getParent()->addObject(MarioGame().createText("But our princess is in\n another castle!",
					  getParent()->castTo<CMarioGameScene>()->screenToPoint(MarioGame().screenSize() / 2) - Vector(120, 40)));
				  enterState(State::go_to_next_level);
			  }

			  break;
		  }

		  case(State::go_to_next_level):
		  {
			  m_delay_timer -= delta_time;
			  if (m_delay_timer < 0)
			  {
				  CLevelPortal* portal = getParent()->findObjectByType<CLevelPortal>();
				  assert(portal->getProperty("Level").isValid());
			      std::string next_level =  portal->getProperty("Level").asString();
				  MarioGame().loadLevel(next_level);
				  MarioGame().showStatus();
			  }
			  break;
		  }
		  }
}
//-------------------------------------------------------------------------------------------------------------

CCastleFlag::CCastleFlag()
{
		  m_animator.create("normal", *MarioGame().textureManager().get("Items"), { 0,148 }, {32,32},4,1,0.01f);
		  setSize({ 32,32 });
}
	  
void CCastleFlag::start()
{
	move(Vector::down*64); //hide for start
	m_pos_y = getPosition().y;
	moveUnderTo(getParent()->findObjectByType<CBlocks>());
	this->disable();
}

void CCastleFlag::liftUp()
{
	enable();
	m_pos_y -= 64;
}

void CCastleFlag::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CCastleFlag::update(int delta_time)
{
	if (getPosition().y > m_pos_y)
	  move(Vector::up *delta_time*0.03f);

	m_animator.update(delta_time);
}

//----------------------------------------------------------------------------------------------------------------------------------------------

CPrincess::CPrincess()
{
	setSize({ 32,64 });
	m_animator.create("stay", *MarioGame().textureManager().get("Items"), { 222,96,32,64 });
}

void CPrincess::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

//----------------------------------------------------------------------------------------------------------------------------------------------

void CTrigger::start()
{
	m_mario = getParent()->findObjectByName<CMario>("Mario");
}

void CTrigger::onActivated()
{
	setSize({ getProperty("width").asFloat(),getProperty("height").asFloat() });
}

void CTrigger::update(int delta_time)
{
	if (!m_trigered && getBounds().isContain(m_mario->getBounds()))
	{
		if (getProperty("EnableAction").isValid())
		{
			auto object_names = split(getProperty("EnableAction").asString(), ';');
			for (auto object_name : object_names)
			{
				auto object = getParent()->findObjectByName(object_name);
				if (object)
					object->show();
			}
		}
		m_trigered = true;
	}
}

