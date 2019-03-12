#include "Enemies.h"

#include "Mario.h"
#include "Blocks.h"
#include "SuperMarioGame.h"
#include <cmath>

void CEnemy::fired(CMario* mario)
{
	kickFromBottom(mario);
}

void CEnemy::checkNextTileUnderFoots()
{
	if (m_speed.y == 0)
	{
		Vector own_center = getBounds().center();
		Vector opposite_vector = math::sign(m_speed.x)*Vector::right;
		bool is_next_under_foot = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center + 20*opposite_vector + 32*Vector::down));
		bool is_prev_under_foot = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center - 60*opposite_vector + 32*Vector::down));
		bool is_prev_back = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center - 50*opposite_vector));
		bool is_next_back = m_blocks->isCollidableBlock(m_blocks->toBlockCoordinates(own_center + 50*opposite_vector));

		if ((!is_next_under_foot && !is_prev_back) && (is_next_under_foot || is_prev_under_foot))
			m_speed.x = -m_speed.x;
	}
}

void CEnemy::checkCollideOtherCharasters()
{
	auto enemies = getParent()->findObjectsByType<CEnemy>();
	for (auto enemy : enemies)
		if (enemy != this && enemy->isAlive() && enemy->getBounds().isIntersect(getBounds()))
		{
			bool is_enemy_in_bullet_state_also = enemy->isInBulletState();
			enemy->kickFromBottom(nullptr);
			if (is_enemy_in_bullet_state_also)
			{
				kickFromBottom(nullptr);
				break;
			}
		}
}

void CEnemy::updateCollision(float delta_time)
{
	m_collision_tag = ECollisionTag::none;
	setPosition(m_blocks->collsionResponse(getBounds(), m_speed, delta_time, m_collision_tag));
}

void CEnemy::updatePhysics(float delta_time, float gravity)
{
	m_speed += Vector::down*gravity * delta_time;
	move(delta_time*m_speed);
}

void CEnemy::checkFallUndergound()
{
	if (getPosition().y > 1000)
		getParent()->removeObject(this);
}

CMario* CEnemy::mario()
{
	return m_mario;
}

void CEnemy::addScoreToPlayer(int score)
{
	MarioGame().addScore(score, getBounds().center());
}

void CEnemy::start()
{
	m_mario = getParent()->findObjectByName<CMario>("Mario");
	m_blocks = getParent()->findObjectByName<CBlocks>("Blocks");
	assert(m_mario && m_blocks);
}

void CEnemy::update(int delta_time)
{
	checkFallUndergound();
}

//-----------------------------------------------------------------------------------------------------------------------------------------------

CGoomba::CGoomba()
{
	setSize({ 32,32 });
	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("walk", texture, { { 0,0,32,32 },{ 32,0,32,32 } }, 0.005f);
	m_animator.create("cramped", texture, { 64,0,32,32 });
	m_animator.create("fall", texture, { 0,32,32,-32 });
	m_animator.setSpriteOffset("cramped", 0, { 0,8 });
}

void CGoomba::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CGoomba::update(int delta_time)
{
	CEnemy::update(delta_time);

	switch (m_state)
	{
		case (State::Deactivated):
		{
			auto scene = getParent()->castTo<CMarioGameScene>();
			Rect camera_rect = scene->cameraRect();
			if (std::abs(getPosition().x - camera_rect.center().x) < camera_rect.width() / 2)
				setState(State::Normal);
			break;
		}
		case (State::Normal):
		{
			updatePhysics(delta_time, gravity_force);
			updateCollision(delta_time);
			if ((m_collision_tag & ECollisionTag::floor) || (m_collision_tag & ECollisionTag::cell))
				m_speed.y = 0;
			if ((m_collision_tag & ECollisionTag::left) || (m_collision_tag & ECollisionTag::right))
				m_speed.x = -m_speed.x;
			break;
		}
		case (State::Died):
		{
			updatePhysics(delta_time, gravity_force);
			break;
		}
		case(State::Cramped):
		{
			m_timer += delta_time;
			if (m_timer > 3000)
				getParent()->removeObject(this);
			break;
		}
	}

	switch (m_state)
	{
		case(State::Normal):
		{
			m_animator.play("walk");
			m_animator.update(delta_time);
			break;
		};
		case(State::Cramped): { m_animator.play("cramped"); break; };
		case(State::Died): { m_animator.play("fall"); break; };
	}
}

void CGoomba::kickFromTop(CMario* mario)
{
	setState(State::Cramped);
}

void CGoomba::kickFromBottom(CMario* mario)
{
	if (m_state != Died)
	{
		setState(State::Died);
		m_speed += 0.4f*Vector::up;
	}
}

void CGoomba::touchSide(CMario* mario)
{
	if (m_state == State::Normal)
		mario->reciveDamage();
}

bool CGoomba::isAlive() const
{
	return m_state == Normal;
}

void CGoomba::setState(State state)
{
	m_state = state;
	switch (m_state)
	{
		case State::Normal:
		{
			m_speed.x = m_run_speed;
			break;
		}
		case State::Cramped:
		{
			m_speed.x = 0;
			addScoreToPlayer(100);
			MarioGame().playSound("stomp");
			break;
		}
		case State::Died:
		{
			m_speed.x = 0;
			addScoreToPlayer(100);
			MarioGame().playSound("kick");
			break;
		}
	}
	m_timer = 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------

CKoopa::CKoopa()
{
	setSize({ 32, 48 });
	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("walk", texture, { { 0,32,32,48 },{ 32,32,32,48 } }, 0.005f);
	m_animator.create("flying", texture, { 224,32 }, { 32, 48 }, 2, 1, 0.005f);
	m_animator.create("climb", texture, { { 64,48,32,32 },{ 192,48,32,32 } }, 0.005f);
	m_animator.create("hidden", texture, { 64,48,32,32 });
	m_animator.create("bullet", texture, { 64, 48 }, { 32, 32 }, 4, 1, 0.01f);
	m_animator.create("fall", texture, { 0,80, 32, -48 });
}

void  CKoopa::kickFromTop(CMario* mario)
{
	switch (m_state)
	{
		case CKoopa::Levitating:
		case CKoopa::Jumping:
		{
			m_speed.y = 0;
			setState(State::Normal);
			addScoreToPlayer(100);
			MarioGame().playSound("stomp");
			break;
		}
		case CKoopa::Normal:
		{
			if (m_speed.y == 0)
			{
				setState(State::Hidden);
				addScoreToPlayer(100);
				MarioGame().playSound("stomp");
			}
			break;
		}
		case CKoopa::Hidden:
		case CKoopa::Climb:
		{
			setState(State::Bullet);
			m_speed.x = (mario->getBounds().center().x > this->getBounds().center().x) ? -std::abs(m_run_speed) * 6 : std::abs(m_run_speed) * 6;
			addScoreToPlayer(400);
			MarioGame().playSound("kick");
			break;
		}
		case CKoopa::Bullet:
		{
			setState(Hidden);
			break;
		}
	}
}

void  CKoopa::kickFromBottom(CMario* mario)
{
	if (m_state != State::Died)
	{
		setState(State::Died);

		addScoreToPlayer(500);
		MarioGame().playSound("kick");
	}
}

bool  CKoopa::isAlive() const
{
	return m_state != Died;
}

void CKoopa::touchSide(CMario* mario)
{
	if (m_state == State::Hidden || m_state == State::Climb)
	{
		setState(State::Bullet);
        m_speed.x = (mario->getBounds().center().x > this->getBounds().center().x) ? -std::abs(m_run_speed) * 6 : std::abs(m_run_speed) * 6;
		move(14 * Vector::right*math::sign(m_speed.x));
		addScoreToPlayer(400);
		MarioGame().playSound("kick");
	}
	else if (m_state == State::Normal || m_state == State::Bullet)
		mario->reciveDamage();
}

void CKoopa::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CKoopa::setState(State state)
{
	m_timer = 0;
	if (m_state != state)
	{
		m_state = state;
		switch (m_state)
		{
		case (State::Jumping):
		{
			m_animator.play("flying");
			m_speed.x = m_run_speed;
			break;
		}
		case (State::Levitating):
		{
			m_animator.play("flying");
			m_speed.x = 0;
			m_initial_pos = getPosition();
			break;
		}
		case (State::Normal):
		{
			m_speed.x = m_run_speed;
			m_animator.play("walk");

			move(getBounds().size() - full_size);
			setSize(full_size);

			break;
		}
		case (State::Hidden):
		{
			move(getBounds().size() - hidden_size);
			setSize(hidden_size);
			m_speed.x = 0;
			m_animator.play("hidden");
			break;
		}
		case (State::Bullet):
		{
			move(getBounds().size() - hidden_size);
			setSize(hidden_size);
			m_animator.play("bullet");
			break;
		}
		case (State::Climb):
		{
			move(getBounds().size() - hidden_size);
			setSize(hidden_size);
			m_animator.play("climb");
			break;
		}
		case (State::Died):
		{
			m_speed.y = -0.4f;
			m_animator.play("fall");
			break;
		}
		}
		m_timer = 0;
	}
}

void CKoopa::update(int delta_time)
{
	CEnemy::update(delta_time);

	if (m_state == State::Deactivated)
	{
		Rect camera_rect = getParent()->castTo<CMarioGameScene>()->cameraRect();
        if (std::abs(getPosition().x - camera_rect.center().x) < camera_rect.width() / 2)
			if (m_flying_mode == 1)
				setState(State::Jumping);
			else if (m_flying_mode == 2)
				setState(State::Levitating);
			else
				setState(State::Normal);
	}
	else
	{
		if (m_state != State::Levitating)
			updatePhysics(delta_time, gravity_force);
		if (m_state != State::Died && m_state != State::Levitating)
		{
			updateCollision(delta_time);

			if ((m_collision_tag & ECollisionTag::left) || (m_collision_tag & ECollisionTag::right))
				m_speed.x = -m_speed.x;

			if (m_collision_tag & ECollisionTag::floor)
				if (m_state == State::Jumping)
					m_speed.y = -0.4f;
				else
					m_speed.y = 0.f;
			if (m_collision_tag & ECollisionTag::cell)
				m_speed.y = 0;
		}

		switch (m_state)
		{
			case(State::Normal):
			{
				checkNextTileUnderFoots();
				break;
			}
			case(State::Jumping):
			{
				m_speed += Vector::up*delta_time*gravity_force / 2; //anti-grav force
				break;
			}
			case(State::Levitating):
			{
				m_timer += delta_time;
				setPosition(m_initial_pos + Vector::up*sin(m_timer / 800.f) * 50);
				break;
			}
			case(State::Hidden):
			{
				m_timer += delta_time;
				if (m_timer > 3000)
					setState(State::Climb);
				break;
			}
			case(State::Climb):
			{
				m_timer += delta_time;
				if (m_timer > 5000)
					setState(State::Normal);
				break;
			}
			case(State::Bullet):
			{
				checkCollideOtherCharasters();
				break;
			}
		}
	}

	if (m_state == State::Bullet && ((m_collision_tag & ECollisionTag::left) || (m_collision_tag & ECollisionTag::right)))
		getParent()->castTo<CMarioGameScene>()->playSoundAtPoint("bump", getBounds().center());
	
	m_animator.flipX(m_speed.x > 0);
	m_animator.update(delta_time);
}

bool CKoopa::isInBulletState() const
{
	return m_state == State::Bullet;
}

void CKoopa::onActivated()
{
	m_flying_mode = getProperty("Flying").asInt();
}

//--------------------------------------------------------------------------------------------------------------

CBuzzyBeetle::CBuzzyBeetle()
{
	setSize({ 32, 32 });
	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("walk", texture, { { 96,0,32,32 },{ 128,0,32,32 } }, 0.005f);
	m_animator.create("hidden", texture, { 160,0,32,32 });
	m_animator.create("bullet", texture, { 160, 0 }, { 32, 32 }, 4, 1, 0.01f);
	m_animator.create("fall", texture, { 96,32, 32, -32 });
	setState(State::Normal);
}

void  CBuzzyBeetle::kickFromTop(CMario* mario)
{
	if (m_state == Normal)
	{
		if (m_speed.y == 0)
		{
			setState(State::Hidden);
			addScoreToPlayer(100);
			MarioGame().playSound("stomp");
		}
	}
	else if (m_state == Hidden)
	{
		setState(State::Bullet);
        m_speed.x = (mario->getBounds().center().x > this->getBounds().center().x) ? -std::abs(m_run_speed) * 6 : std::abs(m_run_speed) * 6;
		addScoreToPlayer(400);
		MarioGame().playSound("kick");
	}
	else if (m_state == Bullet)
		setState(Hidden);
	m_timer = 0;
}

void  CBuzzyBeetle::kickFromBottom(CMario* mario)
{
	if (m_state != Died)
	{
		setState(State::Died);
		m_speed.y = -0.4f;
		m_timer = 0;
		addScoreToPlayer(800);
		MarioGame().playSound("kick");
	}
}

bool  CBuzzyBeetle::isAlive() const
{
	return m_state != Died;
}

void CBuzzyBeetle::touchSide(CMario* mario)
{
	if (m_state == State::Hidden)
	{
		setState(State::Bullet);
        m_speed.x = (mario->getBounds().center().x > this->getBounds().center().x) ? -std::abs(m_run_speed) * 6 : std::abs(m_run_speed) * 6;
		move(14 * Vector::right*math::sign(m_speed.x));
		MarioGame().playSound("kick");
	}
	else if (m_state == State::Normal || m_state == State::Bullet)
		mario->reciveDamage();
}

void CBuzzyBeetle::fired(CMario* mario)
{
	//nothing happen
}

void CBuzzyBeetle::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CBuzzyBeetle::setState(State state)
{
	m_state = state;
	switch (m_state)
	{
	case (State::Normal):
	{
		m_speed.x = m_run_speed;
		m_animator.play("walk");
		break;
	}
	case (State::Hidden):
	{
		m_speed.x = 0;
		m_animator.play("hidden");
		break;
	}
	case (State::Bullet):
	{
		m_speed.x = 6 * m_run_speed;
		m_animator.play("bullet");
		break;
	}
	case (State::Died):
	{
		m_speed.y = -0.4f;
		m_animator.play("fall");
		break;
	}
	}
	m_timer = 0;
}

void CBuzzyBeetle::update(int delta_time)
{
	CEnemy::update(delta_time);

	switch (m_state)
	{
	case(State::Deactivated):
	{
		auto scene = getParent()->castTo<CMarioGameScene>();
		Rect camera_rect = scene->cameraRect();
        if (std::abs(getPosition().x - camera_rect.center().x) < camera_rect.width() / 2)
			setState(Normal);
		break;
	}
	case(State::Normal):
	{
		updatePhysics(delta_time, gravity_force);
		updateCollision(delta_time);

		if ((m_collision_tag & ECollisionTag::left) || (m_collision_tag & ECollisionTag::right))
		{
			m_speed.x = -m_speed.x;
		}
		if ((m_collision_tag & ECollisionTag::floor) || (m_collision_tag & ECollisionTag::cell))
			m_speed.y = 0;

		checkNextTileUnderFoots();
		break;
	}
	case(State::Hidden):
	{
		m_timer += delta_time;
		if (m_timer > 5000)
		{
			setState(State::Normal);
			m_timer = 0;
		}
		updatePhysics(delta_time, gravity_force);
		updateCollision(delta_time);

		if ((m_collision_tag & ECollisionTag::left) || (m_collision_tag & ECollisionTag::right))
			m_speed.x = -m_speed.x;
		if ((m_collision_tag & ECollisionTag::floor) || (m_collision_tag & ECollisionTag::cell))
			m_speed.y = 0;

		break;
	}
	case(State::Bullet):
	{
		updatePhysics(delta_time, gravity_force);
		updateCollision(delta_time);

		if ((m_collision_tag & ECollisionTag::left) || (m_collision_tag & ECollisionTag::right))
		{
			m_speed.x = -m_speed.x;
			getParent()->castTo<CMarioGameScene>()->playSoundAtPoint("bump", getBounds().center());
		}
		if ((m_collision_tag & ECollisionTag::floor) || (m_collision_tag & ECollisionTag::cell))
			m_speed.y = 0;

		checkCollideOtherCharasters();
		break;
	}
	case(State::Died):
	{
		updatePhysics(delta_time, gravity_force);
		break;
	}
	}
	m_animator.flipX(m_speed.x > 0);
	m_animator.update(delta_time);

}

bool CBuzzyBeetle::isInBulletState() const
{
	return m_state == State::Bullet;
}

//---------------------------------------------------------------------------------------------------------------

CHammer::CHammer(CMario* target)
{
	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("fly", texture, Vector(96, 112), Vector(32, 32), 4, 1, 0.01f);
	m_animator.create("in_hand", texture, { 96,112,32,32 });
	m_animator.play("in_hand");
	m_target = target;
	setSize({ 32,32 });
}

void CHammer::update(int delta_time)
{
	if (m_state == in_hand)
	{
	}
	else if (m_state == fly)
	{
		m_animator.update(delta_time);
		m_speed += Vector::down*gravity_force * delta_time;
		move(m_speed*delta_time);
		if (getPosition().y > 1000)
			getParent()->removeObject(this);
		if (m_target->getBounds().isContain(getBounds().center()))
			m_target->reciveDamage();
	}
};

void  CHammer::throwAway(const Vector& speed)
{
	m_speed = speed;
	m_animator.play("fly");
	m_state = fly;
}

void  CHammer::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

//---------------------------------------------------------------------------------------------------------------

CHammerBro::CHammerBro()
{
	setSize({ 32, 44 });
	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("died", texture, { 96,160 + 48,32,-48 });
	m_animator.create("walk", texture, Vector(96, 160), Vector(32, 48), 2, 1, 0.005f);
	m_animator.create("walk_with_hammer", texture, Vector(160, 160), Vector(32, 48), 2, 1, 0.005f);
	m_animator.play("walk_with_hammer");
	m_speed.x = m_run_speed;
}

void CHammerBro::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CHammerBro::update(int delta_time)
{
	CEnemy::update(delta_time);
	if (m_speed.y == 0)
		m_animator.update(delta_time);

	switch (m_state)
	{
	case(State::Normal):
	{
		//Physic processing
		updatePhysics(delta_time, gravity_force / 2);
		if (m_collision_on)
		{
			updateCollision(delta_time);
			if ((m_collision_tag & ECollisionTag::left) || (m_collision_tag & ECollisionTag::right))
				m_speed.x = -m_speed.x;
			if ((m_collision_tag & ECollisionTag::floor) || (m_collision_tag & ECollisionTag::cell))
				m_speed.y = 0;
		}

		//Walk processing
        if (std::abs(getPosition().x - m_center_x) > walk_amlitude)
			m_speed.x = -m_speed.x;
		m_direction = (m_mario->getPosition().x < getPosition().x) ? Vector::left : Vector::right;
		m_animator.flipX(m_direction == Vector::right);

		// Jump processing
		m_jump_timer += delta_time;
		if (m_jump_timer > jump_rate)
		{
			if (m_jump_direction == Vector::up && !isCanJumpUp() && isCanJumpDown())
				m_jump_direction = Vector::down;
			if (m_jump_direction == Vector::down && !isCanJumpDown())
				m_jump_direction = Vector::up;

			if (m_jump_direction == Vector::up)  //jump up
				m_speed += Vector::up*0.5;
			else                                   //jump-off down
			{
				m_speed += Vector::up*0.25;
				m_drop_off_height = getPosition().y + getBounds().height() + 32.f;
			}

			m_collision_on = false;               //turn off collision check for moving through walls
			m_jump_timer = 0;
		}

		if (!m_collision_on)                      //turn on collision check for take ground
		{
			if (m_jump_direction == Vector::up)
			{
				if (m_speed.y > 0)
					m_collision_on = true;
			}
			else
			{
				if (getPosition().y > m_drop_off_height)
					m_collision_on = true;
			}
		}

		//Fire processing
		m_fire_timer += delta_time;
		if (m_fire_timer < fire_rate / 2)  //just walk
		{

		}
		else if (m_fire_timer < fire_rate) //get hammer in hand
		{
			if (!m_hummer)
			{
				m_hummer = new CHammer(m_mario);
				getParent()->addObject(m_hummer);
				m_animator.play("walk_with_hammer");
			}
			const Vector hand_off_set = { -3 * m_direction.x, -22.f };
			m_hummer->setPosition(getPosition() + hand_off_set);
		}
		else                      //throw hummer
		{
			m_hummer->throwAway({ m_direction.x*0.15f, -0.55f });
			m_hummer = NULL;
			m_fire_timer = 0;
			m_animator.play("walk");
		};
		break;
	}
	case(State::Died):
	{
		updatePhysics(delta_time, gravity_force);
		break;
	}
	}
}

void CHammerBro::setState(State state)
{
	m_state = state;
	if (m_state == State::Died)
	{
		m_animator.play("died");
		m_speed.y = 0;
		if (m_hummer)
		{
			getParent()->removeObject(m_hummer);
			m_hummer = NULL;
		}
		addScoreToPlayer(1000);
		MarioGame().playSound("kick");
	}
}

void CHammerBro::onActivated()
{
	m_center_x = getPosition().x;
	m_fire_timer = rand() % 500;
	m_jump_timer = rand() % int(jump_rate / 2);
}

void CHammerBro::kickFromTop(CMario* mario)
{
	setState(State::Died);
}

void CHammerBro::kickFromBottom(CMario* mario)
{
	setState(State::Died);
}

void CHammerBro::touchSide(CMario* mario)
{
	if (m_state == State::Normal)
		mario->reciveDamage();
}

bool CHammerBro::isAlive() const
{
	return m_state != State::Died;
}

bool CHammerBro::isCanJumpUp() const
{
	Vector begin_point = m_blocks->toBlockCoordinates(getBounds().center(), true);
	Vector end_point = m_blocks->traceLine(begin_point, Vector::up);
	if (end_point == begin_point)
		return false;
	if (end_point.y == 0) return false;
    if (std::abs(end_point.y - begin_point.y) == 2 && !m_blocks->isCollidableBlock(end_point + Vector::up * 2))
		return true;
	return false;
}

bool CHammerBro::isCanJumpDown() const
{
	Vector begin_point = m_blocks->toBlockCoordinates(getBounds().center()) + Vector::down * 2;
	if (!m_blocks->isCollidableBlock(begin_point))
		return true;
	return false;
}

//----------------------------------------------------------------------------------------------------------------

CSpinny::CSpinny(const Vector& position, const Vector& speed, const Vector& walk_direction)
{
	setPosition(position);
	m_speed = speed;
	m_walk_direction = walk_direction;
	setSize({ 31, 32 });

	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("walk", texture, Vector(64, 80), Vector(32, 32), 2, 1, 0.005f);
	m_animator.create("egg", texture, Vector(128, 80), Vector(32, 32), 2, 1, 0.005f);
	m_animator.create("died", texture, { 64,80 + 32,32,-32 });

	setState(State::Egg);
}

void CSpinny::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CSpinny::setState(State state)
{
	m_state = state;

	if (m_state == State::Normal)
	{
		m_animator.play("walk");
		m_speed.x = m_run_speed;
		if (m_walk_direction == Vector::right)
			m_speed = -m_speed;
	}
	else if (m_state == State::Died)
	{
		m_animator.play("died");
		addScoreToPlayer(400);
		MarioGame().playSound("kick");
	}
	else if (m_state == State::Egg)
	{
		m_animator.play("egg");
	}
}

void CSpinny::update(int delta_time)
{
	CEnemy::update(delta_time);
	m_animator.update(delta_time);

	switch (m_state)
	{
	case (State::Egg):
	{
		updatePhysics(delta_time, gravity_force / 2);
		updateCollision(delta_time);
		if ((m_collision_tag & ECollisionTag::left) || (m_collision_tag & ECollisionTag::right))
			m_speed.x = -m_speed.x;
		if ((m_collision_tag & ECollisionTag::floor) || (m_collision_tag & ECollisionTag::cell))
		{
			m_speed.y = 0;
			setState(State::Normal);
		}
		break;
	}
	case (State::Normal):
	{
		updatePhysics(delta_time, gravity_force / 2);
		updateCollision(delta_time);
		if ((m_collision_tag & ECollisionTag::left) || (m_collision_tag & ECollisionTag::right))
			m_speed.x = -m_speed.x;
		if ((m_collision_tag & ECollisionTag::floor) || (m_collision_tag & ECollisionTag::cell))
			m_speed.y = 0;
		m_animator.flipX(m_speed.x > 0);
		break;
	}
	case (State::Died):
	{
		updatePhysics(delta_time, gravity_force / 2);
		break;
	}
	}

}

void CSpinny::kickFromTop(CMario* mario)
{
	mario->reciveDamage();
}

void CSpinny::kickFromBottom(CMario* mario)
{
	setState(State::Died);
}

void CSpinny::touchSide(CMario* mario)
{
	mario->reciveDamage();
}

bool CSpinny::isAlive() const
{
	return m_state != State::Died;
}

//---------------------------------------------------------------------------------------------------------------

CLakity::CLakity()
{
	setName("Lakity");
	setSize({ 32, 48 });
	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("fire", texture, { 0,128,32,48 });
	m_animator.create("fly", texture, { 32,128,32,48 });
	m_animator.create("died", texture, { 32,128 + 48,32,-48 });
	setState(State::Normal);
}

void CLakity::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CLakity::update(int delta_time)
{
	CEnemy::update(delta_time);

	switch (m_state)
	{
	case (State::Normal):
	{
		//move porcessing
		float diff_x = m_mario->getPosition().x - getPosition().x;
        m_speed.x += math::sign(diff_x)*sqrt(std::abs(diff_x)) / 4000;
		m_speed.x = math::clamp(m_speed.x, -0.35f, 0.35f);

		move(m_speed*delta_time);

		//fire processing
		m_fire_timer += delta_time;
		if (m_fire_timer > fire_rate)
		{
			Vector fly_direction = (m_mario->getPosition().x > getPosition().x) ? Vector::right : Vector::left;
			CSpinny* spinny = new CSpinny(getPosition() - Vector(0, 10), Vector(-0.05f*fly_direction.x, -0.2f), fly_direction);
			getParent()->addObject(spinny);
			m_fire_timer = 0;
			m_animator.play("fly");
		}
		if (m_fire_timer > fire_rate*0.8f)
			m_animator.play("fire");

		break;
	}
	case (State::RunAway):
	{
		move(m_speed*delta_time);
		m_died_timer += delta_time;
		if (m_died_timer > 2000)
			getParent()->removeObject(this);
		break;
	}
	case (State::Died):
	{
		updatePhysics(delta_time, gravity_force / 2);
		break;
	}
	}
}

void CLakity::kickFromTop(CMario* mario)
{
	setState(State::Died);
}

void CLakity::kickFromBottom(CMario* mario)
{
	setState(State::Died);
}

void CLakity::touchSide(CMario* mario)
{
	mario->reciveDamage();
}

bool CLakity::isAlive() const
{
	return m_state != State::Died;
}

void CLakity::setState(State state)
{
	m_state = state;
	if (m_state == State::Died)
	{
		m_animator.play("died");
		m_speed = Vector::zero;
		addScoreToPlayer(1200);
		MarioGame().playSound("kick");
	}
	else if (m_state == State::Normal)
		m_animator.play("fly");
}

void CLakity::runAway(const Vector& run_direction)
{
	m_speed.x = run_direction.x * 0.2f;
	setState(State::RunAway);
}

void CLakity::start()
{
	CEnemy::start();
}

//----------------------------------------------------------------------------------------------------------------

CCheepCheep::CCheepCheep(const Vector& initial_pos, const Vector& initial_speed)
{
	setSize({ 32, 32 });
	m_speed = initial_speed;
	setPosition(initial_pos);
	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("fly", texture, Vector(0, 176), Vector(32, 32), 2, 1, 0.005f);
	m_animator.create("died", texture, { 0,176 + 32,32,-32 });
	setState(State::Normal);
}

CCheepCheep::CCheepCheep()
{
	setSize({ 32, 32 });
	m_speed = Vector::left*0.05f;
	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("fly", texture, Vector(0, 176), Vector(32, 32), 2, 1, 0.005f);
	m_animator.create("died", texture, { 0,176 + 32,32,-32 });
	setState(State::Underwater);
}

void CCheepCheep::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CCheepCheep::update(int delta_time)
{
	CEnemy::update(delta_time);
	switch (m_state)
	{
		case (State::Normal):
		{
			updatePhysics(delta_time, gravity_force*0.4f);
			m_animator.update(delta_time);
			break;
		}
		case (State::Underwater):
		{
			if (std::abs(mario()->getPosition().x - getPosition().x) < MarioGame().screenSize().x / 2)
			{
				move(m_speed*delta_time);
				m_animator.update(delta_time);
			}
			break;
		}
		case (State::Died):
		{
			updatePhysics(delta_time, gravity_force);
			break;
		}
	}

}

void CCheepCheep::kickFromTop(CMario* mario)
{
	setState(State::Died);
}

void CCheepCheep::kickFromBottom(CMario* mario)
{
	setState(State::Died);
}

void CCheepCheep::touchSide(CMario* mario)
{
	mario->reciveDamage();
}

bool CCheepCheep::isAlive() const
{
	return m_state != State::Died;
}

void CCheepCheep::setState(State state)
{
	m_state = state;
	if (m_state == State::Normal)
	{
		m_animator.play("fly");
		m_animator.flipX(m_speed.x > 0);
	}
	else  if (m_state == State::Died)
	{
		m_speed = Vector::zero;
		m_animator.play("died");
		addScoreToPlayer(200);
		MarioGame().playSound("kick");
	}
}

//---------------------------------------------------------------------------------------------------------------

CBlooper::CBlooper()
{
	setSize({ 32, 48 });
	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("zig", texture, { 224,161,32,48 });
	m_animator.create("zag", texture, { 256,161,32,48 });
	m_animator.create("died", texture, { 224,161 + 48,32,-48 });
}

void CBlooper::enterState(State state)
{
	m_state = state;
	switch (state)
	{
	case (CBlooper::Zig):
	{
		m_speed = -Vector(1, 1)*0.15f;
		m_animator.play("zig");
		m_delay_time = 400;
		break;
	}
	case (CBlooper::Zag):
	{
		m_speed = Vector::down*0.05f;
		m_animator.play("zag");
		m_delay_time = 1200;
		break;
	}
	case (CBlooper::Died):
	{
		m_animator.play("died");
		m_speed = Vector::down*0.2f;
		break;
	}
	}
}

void CBlooper::update(int delta_time)
{
	CEnemy::update(delta_time);
    if (std::abs(mario()->getPosition().x - getPosition().x) < MarioGame().screenSize().x / 2)
	{
		switch (m_state)
		{
		case (CBlooper::Zig):
		{
			m_delay_time -= delta_time;
			if (m_delay_time < 0)
				enterState(State::Zag);
			break;
		}
		case (CBlooper::Zag):
		{
			m_delay_time -= delta_time;
			if (m_delay_time < 0)
				enterState(State::Zig);
			break;
		}
		}
		move(delta_time*m_speed);
		m_animator.update(delta_time);
	}
}

void CBlooper::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CBlooper::kickFromTop(CMario* mario)
{
	mario->reciveDamage();
}

void CBlooper::kickFromBottom(CMario* mario)
{
	enterState(State::Died);
}

void CBlooper::touchSide(CMario* mario)
{
	mario->reciveDamage();
}

bool CBlooper::isAlive() const
{
	return m_state != State::Died;
}

//----------------------------------------------------------------------------------------------------------------

CBulletBill::CBulletBill(const Vector& initial_pos, const Vector& initial_speed)
{
	setSize({ 32, 32 });
	m_speed = initial_speed;
	setPosition(initial_pos);
	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("fly", texture, Vector(64, 112), Vector(32, 32), 1, 3, 0.005f);
	m_animator.create("died", texture, { 64,176 + 32,32,-32 });
	setState(State::Normal);
}

void CBulletBill::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CBulletBill::update(int delta_time)
{
	CEnemy::update(delta_time);

	switch (m_state)
	{
	case (State::Normal):
	{
		updatePhysics(delta_time, 0);
		m_animator.update(delta_time);
		int camera_width = getParent()->castTo<CMarioGameScene>()->cameraRect().width();
		int distance_to_mario = std::abs(getPosition().x - mario()->getPosition().x);
		if (distance_to_mario > camera_width)
			getParent()->removeObject(this);
		break;
	}
	case (State::Died):
	{
		updatePhysics(delta_time, gravity_force);
		break;
	}
	}
}

void CBulletBill::kickFromTop(CMario* mario)
{
	setState(State::Died);
}

void CBulletBill::kickFromBottom(CMario* mario)
{
	setState(State::Died);
}

void CBulletBill::touchSide(CMario* mario)
{
	mario->reciveDamage();
}

bool CBulletBill::isAlive() const
{
	return m_state != State::Died;
}

void CBulletBill::setState(State state)
{
	m_state = state;
	if (m_state == State::Normal)
	{
		m_animator.play("fly");
		m_animator.flipX(m_speed.x > 0);
	}
	else  if (m_state == State::Died)
	{
		m_speed = Vector::zero;
		m_animator.play("died");
		addScoreToPlayer(1000);
		MarioGame().playSound("kick");
	}
}

void CBulletBill::start()
{
	CEnemy::start();
}

//------------------------------------------------------------------------------------------------------------------

CPiranhaPlant::CPiranhaPlant()
{
	m_sprite.setTexture(*MarioGame().textureManager().get("Enemies"));
	m_sprite.setTextureRect({ 0,0,0,0 });

	setSize(size);
}

void CPiranhaPlant::kickFromTop(CMario* mario)
{
	if (m_dead_zone)
		mario->reciveDamage();
}

void CPiranhaPlant::kickFromBottom(CMario* mario)
{
	getParent()->removeObject(this);
	addScoreToPlayer(800);
	MarioGame().playSound("kick");
}

bool CPiranhaPlant::isAlive() const
{
	return true;
}

void CPiranhaPlant::touchSide(CMario* mario)
{
	if (m_dead_zone)
		mario->reciveDamage();
}

void CPiranhaPlant::fired(CMario* mario)
{
	getParent()->removeObject(this);
	addScoreToPlayer(800);
	MarioGame().playSound("kick");
}

void CPiranhaPlant::draw(sf::RenderWindow* render_window)
{
	m_sprite.setPosition(getPosition());
	render_window->draw(m_sprite);
}

void CPiranhaPlant::update(int delta_time)
{
	CEnemy::update(delta_time);

	int sprite_index = 0;
	float height = 0;
	m_timer += delta_time;

	m_dead_zone = (m_timer > period_time*0.25 && m_timer < 3 * period_time);
		
	if (m_timer < period_time) //None
	{
		//too close to mario ===> no appear
		static const int distance = 20;
		if ((mario()->getBounds().center() - getBounds().center()).length() < 100)
			hideInTube();
		height = 0;
	}
	else  if (m_timer < 1.25 * period_time) //Appearing
	{
		sprite_index = 1;
		height = ((m_timer - period_time) / (0.25f*period_time)) * size.y;
	}
	else  if (m_timer < 3 * period_time)   //in full size 
	{
		sprite_index = int(m_timer / 100) % 2;
		height = size.y;
	}
	else  if (m_timer < 3.25 * period_time)  //Hiding
	{
		sprite_index = 0;
		height = (1 - ((m_timer - 3 * period_time) / (0.25f*period_time))) * size.y;
	}
	else 
		m_timer = 0;


	setSize({ size.x, height });
	move({ 0.f,m_buttom - getPosition().y - height });
	m_sprite.setTextureRect({ 32 * sprite_index,80,(int)size.x, (int)height });
}

void CPiranhaPlant::hideInTube()
{
	m_timer = 0;
	setSize({ size.x, 0.f });
	move({ 0.f,m_buttom - getPosition().y });
}

void CPiranhaPlant::onActivated()
{
	m_buttom = getPosition().y + size.y;
}

//-----------------------------------------------------------------------------------------------------------------

CPodoboo::CPodoboo()
{
	m_shape.setRadius(16);
	m_shape.setFillColor(sf::Color::Red);
	setSize({ 32,32 });
	const sf::Texture& texture = *MarioGame().textureManager().get("Enemies");
	m_animator.create("up", texture, Vector(192, 80), Vector(32, 32), 3, 1, 0.005f);
	m_animator.create("down", texture, Vector(192, 112), Vector(32, -32), 3, 1, 0.005f);
}

void CPodoboo::kickFromTop(CMario* mario)
{
	mario->reciveDamage();
}

void CPodoboo::kickFromBottom(CMario* mario)
{
	//nothing
}

bool CPodoboo::isAlive() const
{
	return true;
}

void CPodoboo::touchSide(CMario* mario)
{
	mario->reciveDamage();
}

void CPodoboo::fired(CMario* mario)
{
	//nothing
}

void CPodoboo::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CPodoboo::update(int delta_time)
{
	CEnemy::update(delta_time);
	m_timer += delta_time;

	if (getPosition().y > m_center.y)
		m_speed += Vector::up*m_acceleration*delta_time;
	else
		m_speed += Vector::down*m_acceleration*delta_time;

	if (m_timer > period_time) //synchronization
	{
		setPosition(m_center);
		m_speed = Vector::up*m_acceleration * period_time*0.25;
		m_timer = 0;
	}

	m_animator.update(delta_time);
	move(m_speed*delta_time);
	m_animator.play((m_speed.y < 0) ? "down" : "up");

}

void CPodoboo::onActivated()
{
	m_center = getPosition();// -Vector(16, 16);
	m_acceleration = amplitude / (period_time*period_time*0.25f*0.25f);
	m_speed = Vector::up*m_acceleration * period_time*0.25f;
}

//----------------------------------------------------------------------------------------------------------------

CLakitySpawner::CLakitySpawner()
{

}

void CLakitySpawner::update(int delta_time)
{
	CGameObject::update(delta_time);

	m_lakity_checker_timer += delta_time;
	if (m_lakity_checker_timer > check_interval)
	{
		m_lakity = getParent()->findObjectByName<CLakity>("Lakity");
		Rect camera_rect = getParent()->castTo<CMarioGameScene>()->cameraRect();


		if (!m_lakity)  	// Add Lakity to the scene processing
		{
			if (getBounds().isContainByX(m_mario->getPosition()))
			{
				m_lakity = new CLakity();
				m_lakity->setPosition(camera_rect.left() - 32, 64);
				getParent()->addObject(m_lakity);
			}
		}
		m_lakity_checker_timer = 0;
	}

	if (m_lakity)              // Throw Lakity from the scene processing
	{
		Rect camera_rect = getParent()->castTo<CMarioGameScene>()->cameraRect();
		if (m_lakity->getPosition().x > getBounds().right() + camera_rect.size().x / 2)
		{
			m_lakity->runAway(Vector::left);
			m_lakity = NULL;
		}
		else if (m_lakity->getPosition().x < getBounds().left() - camera_rect.size().x / 2)
		{
			m_lakity->runAway(Vector::right);
			m_lakity = NULL;
		}
	}

}

void CLakitySpawner::onActivated()
{
	setSize({ getProperty("width").asFloat(), getProperty("height").asFloat() });
}

void CLakitySpawner::start()
{
	m_mario = getParent()->findObjectByName<CMario>("Mario");
}

//-----------------------------------------------------------------------------------------------------------------

CCheepCheepSpawner::CCheepCheepSpawner()
{

}

void CCheepCheepSpawner::update(int delta_time)
{
	m_spawn_timer += delta_time;
	if (m_spawn_timer > spawn_interval && getBounds().isContainByX(m_mario->getPosition()))
	{
		int camera_width = getParent()->castTo<CMarioGameScene>()->cameraRect().width();
		int x = rand() % int(camera_width) - camera_width / 2 + m_mario->getPosition().x;
		Vector direction = (m_mario->getPosition().x < x) ? Vector::left : Vector::right;
		getParent()->addObject(new CCheepCheep({ x, m_map_height + 32 }, { direction.x*spawn_speed.x, spawn_speed.y }));
		m_spawn_timer = 0;
	}
}

void CCheepCheepSpawner::onActivated()
{
	setSize({ getProperty("width").asFloat(), getProperty("height").asFloat() });
}

void CCheepCheepSpawner::start()
{
	m_map_height = getParent()->findObjectByName<CBlocks>("Blocks")->height();
	m_mario = getParent()->findObjectByName<CMario>("Mario");
}

//-------------------------------------------------------------------------------------------------------------

CBulletBillSpawner::CBulletBillSpawner()
{
	m_spawn_timer = rand() % spawn_interval;
}

void CBulletBillSpawner::update(int delta_time)
{
	int camera_width = getParent()->castTo<CMarioGameScene>()->cameraRect().width();
	m_spawn_timer += delta_time;
	bool is_mario_close_enough = std::abs(m_mario->getPosition().x - getPosition().x) < camera_width;
	bool is_time_to_push = m_spawn_timer > spawn_interval;
	bool is_bullet_bill_beyond_tiled_map = isBulletBillBeyondTiledMap();
    if (is_time_to_push && (is_mario_close_enough || is_bullet_bill_beyond_tiled_map))
	{
		Vector direction = (m_mario->getPosition().x < getPosition().x) ? Vector::left : Vector::right;
		Vector pos = getPosition();

		if (is_bullet_bill_beyond_tiled_map)
		{
			int k = (pos.x < m_mario->getPosition().x ? -1 : 1);
			pos.x = m_mario->getPosition().x + k*MarioGame().screenSize().x / 2;
		}

		getParent()->addObject(new CBulletBill(pos, direction*bullet_speed));
		m_spawn_timer = 0;
		if (is_bullet_bill_beyond_tiled_map)
			m_spawn_timer = -4000 - rand() % 8000;

		MarioGame().playSound("fireworks");
	}
}

void CBulletBillSpawner::onActivated()
{

}

bool CBulletBillSpawner::isBulletBillBeyondTiledMap() const
{
	return getPosition().x < 0 || getPosition().x > m_blocks_width;
}

void CBulletBillSpawner::start()
{
	m_mario = getParent()->findObjectByName<CMario>("Mario");
	m_blocks_width = getParent()->findObjectByName<CBlocks>("Blocks")->width();
	if (isBulletBillBeyondTiledMap())
		m_spawn_timer = - rand() % 5000;
}

//----------------------------------------------------------------------------------------------------------------

CFireball::CFireball(const Vector& Position, const Vector& SpeedVector)
{
	auto texture = MarioGame().textureManager().get("Bowser");
	m_animator.create("fire", *texture, { 0,364 }, { 32,36 }, 4, 1, 0.01f, AnimType::forward_backward_cycle);
	m_speed = SpeedVector;
	setPosition(Position);
	m_animator.flipX(SpeedVector.x < 0);
	m_animator.get("fire")->setOrigin({ 16,18 });
}

void CFireball::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CFireball::update(int delta_time)
{
	m_animator.update(delta_time);
	m_life_timer -= delta_time;
	if (m_life_timer < 0)
		getParent()->removeObject(this);
	move(delta_time*m_speed);

	if (m_mario->getBounds().isContain(getPosition()))
		m_mario->reciveDamage();
}

void  CFireball::start()
{
	m_mario = getParent()->findObjectByName<CMario>("Mario");
}

//----------------------------------------------------------------------------------------------------------------

CBowser::CBowser()
{
	setSize({ 84,80 });
	auto texture = MarioGame().textureManager().get("Bowser");
	m_animator.create("walk", *texture, { 0,0 }, { 84,80 }, 6, 1, anim_speed, AnimType::forward_cycle);
	m_animator.create("died", *texture, { 0,80,84,-80 });
	m_animator.create("turn", *texture, { 381,122 }, { 74,85 }, 2, 1, anim_speed / 2, AnimType::forward_stop);
	m_animator.create("middle_fire", *texture, { 0,167 }, { 91,100 }, 4, 1, anim_speed, AnimType::forward_stop);
	m_animator.create("land_fire", *texture, { 0,267 }, { 92,97 }, 6, 1, anim_speed, AnimType::forward_stop);
	m_animator.create("pre_jump", *texture, { 0,80 }, { 91,79 }, 2, 1, anim_speed, AnimType::forward_stop);
	m_animator.create("up_jump", *texture, { 182,80,84,87 });
	m_animator.create("down_jump", *texture, { 266,80, 84,87 });
	m_animator.get("middle_fire")->setOrigin(Vector::down * 16);
	m_animator.get("land_fire")->setOrigin(Vector::down * 16);
	m_animator.get("turn")->setOrigin(Vector::down * 5);
	m_speed.x = m_run_speed;
}
void CBowser::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void  CBowser::noBridge()
{
	enterState(State::no_bridge);
}

void CBowser::enterState(State state)
{
	if (state == m_state)
		return;
	switch (state)
	{
		case(State::walk):
		{
			m_speed.x = m_old_speed.x;
			m_animator.play("walk");
			m_delay_timer = 2000;
			break;
		}
		case(State::turn):
		{
			m_animator.play("turn");
			m_delay_timer = 400;
			break;
		}
		case(State::pre_jump):
		{
			m_old_speed.x = m_speed.x;
			m_old_speed.y = -1;
			m_speed.x = 0;
			m_animator.play("pre_jump");
			m_delay_timer = 300;
			break;
		}
		case(State::jump):
		{
			m_speed = 0.4f*Vector::up;
			break;
		}
		case(State::middle_fire):
		{
			m_old_speed.x = m_speed.x;
			m_speed.x = 0;
			m_animator.play("middle_fire");
			m_delay_timer = 500;
			break;
		}
		case(State::land_fire):
		{
			m_old_speed.x = m_speed.x;
			m_speed.x = 0;
			m_animator.play("land_fire");
			m_delay_timer = 700;
			break;
		}
		case(State::no_bridge):
		{
			m_animator.play("walk");
			m_animator.get("walk")->setSpeed(anim_speed * 2.5f);
			m_delay_timer = 1000;
			break;
		}
		case(State::fall):
		{
			m_animator.get("walk")->setSpeed(0);
			MarioGame().playSound("bowser_falls");
			m_speed = Vector::zero;
			break;
		}
		case(State::died):
		{
			m_animator.play("died");
			MarioGame().playSound("bowser_falls");
			m_speed = Vector::zero;
			break;
		}
	}
	m_state = state;
}

void CBowser::update(int delta_time)
{
	CEnemy::update(delta_time);
	m_animator.update(delta_time);
	if (m_state == State::walk || m_state == State::jump)
	{
		updatePhysics(delta_time, gravity_force / 2);
		updateCollision(delta_time);
		if ((m_collision_tag & ECollisionTag::left) || (m_collision_tag & ECollisionTag::right))
			m_speed.x = -m_speed.x;
		if ((m_collision_tag & ECollisionTag::floor) || (m_collision_tag & ECollisionTag::cell))
			m_speed.y = 0;
	}

	switch (m_state)
	{
		case(State::walk):
		{
			if (std::abs(getPosition().x - m_center_x) > c_walk_amlitude)
			{
				m_speed.x = -m_speed.x;
				move({ (float)(c_walk_amlitude - std::abs(getPosition().x - m_center_x))*math::sign(-m_speed.x),0.f });
			}
			auto old_direction = m_direction;
			m_direction = (m_mario->getPosition().x < getPosition().x) ? Vector::left : Vector::right;

			if (old_direction != m_direction)
			{
				enterState(State::turn);
				break;
			}

			m_animator.flipX(m_direction == Vector::right);

			m_delay_timer -= delta_time;
			if (m_delay_timer < 0)
			{
				int d = rand() % 3;
				if (d == 0) enterState(State::pre_jump);
				if (d == 1) enterState(State::middle_fire);
				if (d == 2) enterState(State::land_fire);
			}
			break;
		}
		case(State::turn):
		{
			m_delay_timer -= delta_time;
			if (m_delay_timer < 0)
				enterState(State::walk);
			break;
		}
		case(State::jump):
		{
			if (m_speed.y > 0)
				m_animator.play("down_jump");
			else if (m_speed.y < 0)
				m_animator.play("up_jump");
			if (m_speed.y > 0 && m_old_speed.y < 0) //jump peak
			{

				getParent()->addObject(new CFireball(getBounds().center() + m_direction * 50 + Vector::down * 20, m_direction*0.13f));
				MarioGame().playSound("bowser_fire");
			}
			m_old_speed.y = m_speed.y;
			if (m_collision_tag & ECollisionTag::floor)
				enterState(State::walk);
			break;
		}
		case(State::pre_jump):
		{
			m_delay_timer -= delta_time;
			if (m_delay_timer < 0)
				enterState(State::jump);
			break;
		}
		case(State::land_fire):
		{
			m_delay_timer -= delta_time;
			if (m_delay_timer < 0)
			{
				getParent()->addObject(new CFireball(getBounds().center() + m_direction * 50 + Vector::down * 10, m_direction*0.13f));
				MarioGame().playSound("bowser_fire");
				enterState(State::walk);
			}
			break;
		}
		case(State::middle_fire):
		{
			m_delay_timer -= delta_time;
			if (m_delay_timer < 0)
			{
				getParent()->addObject(new CFireball(getBounds().center() + m_direction * 50 + Vector::up * 10, m_direction*0.13f));
				MarioGame().playSound("bowser_fire");
				enterState(State::walk);
			}
			break;
		}
		case(State::no_bridge):
		{
			m_delay_timer -= delta_time;
			if (m_delay_timer < 0)
				enterState(State::fall);
			break;
		}
		case(State::died):
		case(State::fall):
		{
			updatePhysics(delta_time, gravity_force / 2);
			break;
		}
	}
}

void CBowser::onActivated()
{
	m_center_x = getPosition().x;
}

void CBowser::kickFromTop(CMario* mario)
{
	mario->reciveDamage();
}

void CBowser::kickFromBottom(CMario* mario)
{
	m_lives--;
	if (m_lives < 0)
		enterState(State::died);
}

void CBowser::touchSide(CMario* mario)
{
	mario->reciveDamage();
}

bool CBowser::isAlive() const
{
	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------
