#include <cmath>

#include "Blocks.hpp"
#include "Enemies.hpp"

#include "Items.hpp"
#include "SuperMarioGame.hpp"
#include "Mario.hpp"

namespace {

constexpr float WALK_SPEED     = 0.2f;
constexpr float RUN_SPEED      = 0.3f;
constexpr float JUMP_FORCE     = 0.175f;
constexpr float JUMP_TIME      = 50.f;
constexpr float ACCELERATION   = 0.001f;
constexpr float FRICTION_FORCE = 0.0005f;
constexpr float GRAVITY_FORCE  = 0.0015f;
constexpr float CLIMB_SPEED    = 0.1f;
constexpr int INNINCIBLE_TIME  = 11000;   //<1 ms
constexpr int FIRE_RATE        = 400;     //<! bullet per ms
constexpr float JUMP_RATE      = 250.f;   //<! jump per ms
constexpr float SWIM_RATE      = 200.f;   //<! jump per ms
constexpr int SPAWN_PROTECTION_TIME = 3000; //<! ms

const Vector SMALL_MARIO_SIZE  = { 31,32 };
const Vector BIG_MARIO_SIZE    = { 31,64 };
const Vector SEATED_MARIO_SIZE = SMALL_MARIO_SIZE;


void processTimer(float& timer, float delta_time)
{
    if (timer)
    {
        timer -= delta_time;

        if (timer < 0)
        {
            timer = 0;
        }
    }
}

} // anonymous namespace

MarioBullet::MarioBullet(const Vector& pos, const Vector& direction) {
    m_direction = direction;
    setPosition(pos);
    m_speed = direction * SPEED;
    m_animator.create("fly", *MARIO_GAME.textureManager().get("Mario"),
        { { 0,0,16,16 },{ 16,0,16,16 },{ 16,0,-16,16 },{ 16,16,16,-16 } }, 0.01f);
    m_animator.create("splash", *MARIO_GAME.textureManager().get("Mario"),
        Vector(31,0), Vector(16,16), 3,1, 0.02f, AnimType::FORWARD_BACKWARD_CYCLE);
}

void MarioBullet::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void MarioBullet::setState(State state) {
    m_state = state;

    switch (m_state)
    {
    case State::FLY:
        m_animator.play("fly");
        break;
    case State::SPLASH:
        m_animator.play("splash");
        break;
    }

    m_timer = 0;
}

void MarioBullet::onStarted() {
    m_blocks = getParent()->findChildObjectByType<Blocks>();
}

void MarioBullet::update(int delta_time)  {
    m_timer += delta_time;

    if (m_timer > 3000) {
        removeLater();
    }

    if (m_state == State::FLY) {
        m_speed += Vector::DOWN * GRAVITY_FORCE * delta_time;
        move(m_speed * delta_time);
        Vector block = m_blocks->toBlockCoordinates(getPosition());
        if (m_blocks->isBlockInBounds(block) && m_blocks->isCollidableBlock(block)) {
            Rect block_bounds = m_blocks->getBlockBounds(block);
            float dw = std::min(std::abs(block_bounds.left() - getPosition().x), std::abs(block_bounds.right() - getPosition().x));
            float dh = std::min(std::abs(block_bounds.top() - getPosition().y), std::abs(block_bounds.bottom() - getPosition().y));

            if ((dw < dh) && (dw > 4 && dh > 4)) {
                // kick side (and angles - r=4)
                setState(State::SPLASH);
                MARIO_GAME.playSound("bump");
            } else {
                // // kick top or bottom
                m_speed.y = -0.35f; // small jump
            }
        }

        auto enemies = getParent()->findChildObjectsByType<Enemy>();
        for (auto enemy : enemies) {
            if (enemy->getBounds().isContain(getPosition())) {
                enemy->takeDamage(DamageType::SHOOT, nullptr);
                setState(State::SPLASH);
                break;
            }
        }
    } else {
        if (m_timer > 250) {
            removeLater();
        }
    }

    m_animator.update(delta_time);
}

//---------------------------------------------------------------------------
//! Mario
//---------------------------------------------------------------------------

Mario::Mario() {
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Mario");
    addChild(m_animator = new Animator());
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
    setRank(MarioRank::SMALL);
    setState(State::NORMAL);
    m_fire_pallete.create({ sf::Color(64,64,128), sf::Color(64,96,192), sf::Color(160,32,0), sf::Color(192,0,64), sf::Color(224,32,64)},
                          { sf::Color(64,128,0), sf::Color(96,160,0),sf::Color(192,192,128),sf::Color(224,224,128), sf::Color(255,251,240) });
    m_black_pallete.create({ sf::Color(64,64,128), sf::Color(64,96,192), sf::Color(160,32,0), sf::Color(192,0,64), sf::Color(224,32,64) },
                           { sf::Color(128,64,0), sf::Color(160,96,0),sf::Color(20,20,10),sf::Color(30,30,20), sf::Color(0,0,0) });
    m_animator->play("idle_small");
    m_animator->get("seat_big")->setOrigin(-Vector::UP*12);
}

bool Mario::isAlive() const {
    return (m_state != State::DIED);
}

void Mario::touch(Character* character) {
    (void)character;
}

void Mario::takeDamage(DamageType damageType, Character* attacker) {
    (void)damageType;
    (void)attacker;

    if (m_spawn_timer || m_invincible_mode) {
        return;
    }

    setState(isSmall() ? State::DIED
                       : State::DEMOTING);
}

void Mario::kill() {
    setState(State::DIED);
}

bool Mario::isSmall() const {
    return (getRank() == MarioRank::SMALL);
}

void Mario::promote() {
    if (m_rank != MarioRank::FIRE) {
        setState(State::PROMOTING);
    }
}

void Mario::playAnimation(AnimType animType, float speed) {
    const std::string rank_suffix = isSmall() ? "_small"
                                              : "_big";
    std::string animName;

    switch (animType) {
    case AnimType::IDLE:
        animName = "idle" + rank_suffix;
        break;
    case AnimType::WALK:
        animName =  "walk" + rank_suffix;
        break;
    case AnimType::SWIM:
        animName =  "swim" + rank_suffix;
        break;
    case AnimType::JUMP:
        animName =  "jump" + rank_suffix;
        break;
    case AnimType::SLIP:
        animName = "slip" + rank_suffix;
        break;
    case AnimType::SEAT:
        animName = "seat" + rank_suffix;
        break;
    case AnimType::CLIMB:
        animName = "climb" + rank_suffix;
        break;
    case AnimType::GROWING:
        animName = "growing";
        break;
    case AnimType::DEMOTING:
        animName = "demoting";
        break;
    case AnimType::FIRING:
        animName = "firing";
        break;
    case AnimType::SHOOT:
        animName = "shoot";
        break;
    case AnimType::DIED:
        animName = "died";
        break;
    default:
        // Unknown animation
        assert(false);
        break;
    }
 
    m_animator->play(animName);
 
    if (speed >= 0) {
        m_animator->setSpeed(animName, speed);
    }
}

void Mario::setPallete(PalleteType pallete) {
    switch (pallete) {
    case PalleteType::NORMAL:
        m_animator->setPallete(nullptr);
        break;
    case PalleteType::BLACK:
        m_animator->setPallete(&m_black_pallete);
        break;
    case PalleteType::FIRE:
        m_animator->setPallete(&m_fire_pallete);
        break;
    default:
        // Unknown pallete
        assert(false);
        break;
    }
}

void Mario::setRank(MarioRank rank) {
    switch (rank) {
    case MarioRank::SMALL:
        setSize(SMALL_MARIO_SIZE);

        if (m_rank != MarioRank::SMALL) {
            move(SMALL_MARIO_SIZE - BIG_MARIO_SIZE);
        }

        m_seated = false;
        setPallete(PalleteType::NORMAL);
        break;
    case MarioRank::BIG:
        setSize(BIG_MARIO_SIZE);
        move(SMALL_MARIO_SIZE - BIG_MARIO_SIZE);
        setPallete(PalleteType::NORMAL);
        break;
    case MarioRank::FIRE:
        setPallete(PalleteType::FIRE);
        if (!m_seated) {
            setSize(BIG_MARIO_SIZE);
        }
        break;
    }
    m_rank = rank;
}

MarioRank Mario::getRank() const {
    return m_rank;
}

void Mario::draw(sf::RenderWindow* render_window) {
    m_animator->setPosition(getPosition());
    m_animator->draw(render_window);
}

void Mario::kickBlocksProcessing() {
    if (!(m_collision_tag & ECollisionTag::CELL) || // not touch cell
        (m_collision_tag & ECollisionTag::FLOOR) || // touch floor
        (m_speed.y <= 0)) {                         // fall down
        return;
    }

    Vector block_left  = m_blocks->toBlockCoordinates(getBounds().leftTop()  + Vector::UP * 16);
    Vector block_right = m_blocks->toBlockCoordinates(getBounds().rightTop() + Vector::UP * 16);

    bool block_left_exist  = m_blocks->isBlockInBounds(block_left)  && ((m_blocks->isCollidableBlock(block_left))  || m_blocks->isInvizibleBlock(block_left));
    bool block_right_exist = m_blocks->isBlockInBounds(block_right) && ((m_blocks->isCollidableBlock(block_right)) || m_blocks->isInvizibleBlock(block_right));

    if (block_left_exist ^ block_right_exist) { // only one block touched
        if (block_left_exist) {
            m_blocks->hitBlock(block_left.x, block_left.y, this);
        } else {
            m_blocks->hitBlock(block_right.x, block_right.y, this);
        }
    } else if (block_left_exist || block_right_exist) { // two blocks touched
        Rect block_left_rect = m_blocks->getBlockBounds(block_left);
        Rect block_right_rect = m_blocks->getBlockBounds(block_right);
        float mario_X = getBounds().center().x;

        if (std::abs(block_left_rect.center().x - mario_X) < std::abs(block_right_rect.center().x - mario_X)) {
            m_blocks->hitBlock(block_left.x, block_left.y, this);
        } else {
            m_blocks->hitBlock(block_right.x, block_right.y, this);
        }
    }
}

void Mario::kickEnemiesProcessing(float delta_time) {
    if (m_spawn_timer) {
        return;
    }

    auto enemies = getParent()->findChildObjectsByType<Enemy>();

    for (auto enemy : enemies) {
        if (!enemy->isAlive() || !getBounds().isIntersect(enemy->getBounds()))
        {
            continue;
        }

        if (m_invincible_mode) {
            enemy->takeDamage(DamageType::HIT_FROM_BELOW, this);
            continue;
        }

        if (isInWater()) {
            enemy->touch(this);
            continue;
        }

        Vector hitDirection = enemy->getBounds().center() - getBounds().center();

        if ((hitDirection.y > 0) && (hitDirection.y > hitDirection.x)) {
            enemy->takeDamage(DamageType::HIT_FROM_ABOVE, this);
            move(-m_speed * delta_time);
            m_speed.y = -0.3f;
        } else {
            enemy->touch(this);
        }
    }
}

const Vector& Mario::getInputDirection() const {
    return m_input_direction;
}

void Mario::standUp() {
    if (m_seated) {
        setSize(BIG_MARIO_SIZE);
        move(SEATED_MARIO_SIZE - BIG_MARIO_SIZE);
        m_seated = false;
    }
}

void Mario::seatDown() {
    if (!m_seated) {
        setSize(SEATED_MARIO_SIZE);
        move(BIG_MARIO_SIZE - SEATED_MARIO_SIZE);
        m_seated = true;
    }
}

bool Mario::isSeated() const {
    return m_seated;
}

void Mario::syncState(Mario* otherMario) {
    setRank(otherMario->getRank());
}

void Mario::inputProcessing(float delta_time) {
    // Timers
    processTimer(m_jump_timer, delta_time);
    processTimer(m_fire_timer, delta_time);

    // Get inputs
    auto& input_manager = MARIO_GAME.inputManager();
    m_input_direction = input_manager.getXYAxis();
    const bool pressed_fire = input_manager.isButtonPressed("Fire");
    const bool pressed_jump = input_manager.isButtonPressed("Jump");

    // Movements
    switch (m_env_state) {
    case EnvState::NORMAL:
        if (!m_seated && m_input_direction.x) {
            if (isGrounded()) {
                // sync direction when mario touch the ground only
                m_direction.x = m_input_direction.x;
            }

            addImpulse(Vector(m_input_direction.x, 0.f) * ACCELERATION * delta_time * (isGrounded() ? 1.0f : 0.7f));
            m_x_max_speed = (pressed_fire ? RUN_SPEED
                                          : WALK_SPEED);
        }
    break;
    case EnvState::WATER:
        if (!isSeated() && m_input_direction.x) {
            m_direction.x = m_input_direction.x;
            addImpulse(Vector(m_input_direction.x, 0.f) * ACCELERATION * delta_time);
            m_x_max_speed = (isGrounded() ? WALK_SPEED
                                          : RUN_SPEED);
        }
    break;
    case EnvState::LADDER:
        if (((m_input_direction == Vector::UP)   && (getPosition().y > m_used_ladder->getPosition().y)) ||
            ((m_input_direction == Vector::DOWN) && (getBounds().bottom() <= m_used_ladder->getBounds().bottom()))) {
            move(CLIMB_SPEED * m_input_direction * delta_time);
        }
        break;
    }

    // Jumping
    if (pressed_jump && canJump()) {
        jump();
    }

    // Seat-down / stand-up
    if (!isSmall() && !isClimbing() && isGrounded()) {
        if (!m_seated && (m_input_direction.y > 0)) {
            seatDown();
        } else if (m_seated && (m_input_direction.y <= 0)) {
            standUp();
        }
    }

    // Firing
    if (canFire() && input_manager.isButtonDown("Fire")) {
        fire();
    }
}

void Mario::animationProcessing(float delta_time) {
    switch(m_env_state) {
    case EnvState::NORMAL:
        if (isSeated()) {
            playAnimation(AnimType::SEAT);
        }
        else if ((getRank() == MarioRank::FIRE) && (m_fire_timer > 100)) {
            playAnimation(AnimType::SHOOT);
        }
        else if (m_speed.y && !isGrounded()) {
            playAnimation(AnimType::JUMP);
        }
        else if (m_speed.x) {
            m_animator->flipX(m_direction.x < 0);

            if (m_direction.x != math::sign(m_speed.x)) {
                playAnimation(AnimType::SLIP);
            } else {
                playAnimation(AnimType::WALK, 0.02f * (std::abs(m_speed.x) / WALK_SPEED));
            }
        } else {
            playAnimation(AnimType::IDLE);
        }
        break;
    case EnvState::WATER:
        if (isSeated())
        {
            playAnimation(AnimType::SEAT);
        } else if (m_speed.y) {
            playAnimation(AnimType::SWIM, 0.02f * (std::abs(m_speed.x) / WALK_SPEED));
        } else if (m_speed.x) {
            m_animator->flipX(m_direction.x < 0);

            if (m_direction.x != math::sign(m_speed.x)) {
                playAnimation(AnimType::SLIP);
            } else {
                playAnimation(AnimType::WALK, 0.02f * (std::abs(m_speed.x) / WALK_SPEED));
            }
        } else {
            playAnimation(AnimType::IDLE);
        }
        break;
    case EnvState::LADDER:
        playAnimation(AnimType::CLIMB, (m_input_direction.y ? 0.005f : 0.f));
        break;
    }

    // Damaged flashing
    if (m_spawn_timer) {
        m_spawn_timer -= delta_time;
        auto color = (int(m_spawn_timer / 80) % 2) ? sf::Color::White
                                                   : sf::Color::Transparent;
        m_animator->setColor(color);

        if (m_spawn_timer < 0) {
            m_spawn_timer = 0;
            m_animator->setColor(sf::Color::White);
        }
    }
}

void Mario::physicProcessing(float delta_time) {
    if (isClimbing()) {
        return;
    }

    auto effectiveGravity = isInWater() ? 0.25 * GRAVITY_FORCE
                                        : GRAVITY_FORCE;
    auto maxSpeed = isInWater() ? m_x_max_speed / 2
                                : m_x_max_speed;

    if (isGrounded()) {
        // Ground friction force
        addImpulse(Vector::LEFT * math::sign(m_speed.x) * FRICTION_FORCE * delta_time);
    } else {
        // Gravity force
        addImpulse(Vector::DOWN * effectiveGravity * delta_time);
    }

    // Speed limits
    if (std::abs(m_speed.x) < 0.01f) {
        m_speed.x = 0.f;
    }

    m_speed.x = math::clamp(m_speed.x, -maxSpeed, maxSpeed);

    move(m_speed * delta_time);

    if (isInWater() && (getPosition().y < 0)) {
        m_speed.y = 0.1f;
    }
}

const Vector& Mario::getSpeed() const {
    return m_speed;
}

void Mario::update(int delta_time) {
    GameObject::update(delta_time);
    m_current_state->update(delta_time);
}

void Mario::setState(State state) {
    if (m_current_state) {
        m_current_state->onLeave();
        delete m_current_state;
    }

    switch (state) {
    case State::NORMAL:
        m_current_state = new NormalMarioState();
        break;
    case State::PROMOTING:
        m_current_state = new PromotingMarioState();
        break;
    case State::DEMOTING:
        m_current_state = new DemotingMarioState();
        break;
    case State::DIED:
        m_current_state = new DiedMarioState();
        break;
    }

    m_current_state->setMario(this);
    m_current_state->onEnter();
}

void Mario::setState(IMarioState* state)
{
    if (m_current_state) {
        m_current_state->onLeave();
        delete m_current_state;
    }

    m_current_state = state;
    m_current_state->setMario(this);
    m_current_state->onEnter();
}

void Mario::fire() {
    Vector pos = getBounds().center() + 25 * m_direction + 8 * Vector::UP;
    MARIO_GAME.spawnObject<MarioBullet>(pos, m_direction);
    MARIO_GAME.playSound("fireball");
    m_fire_timer = FIRE_RATE;
}

bool Mario::canFire() const {
    return !m_fire_timer &&
           (m_rank == MarioRank::FIRE) &&
           !isSeated() &&
           !isClimbing();
}

void Mario::jump() {
    switch (m_env_state) {
    case EnvState::NORMAL:
        if (!m_jump_timer) {
            addImpulse(1.5 * Vector::UP * JUMP_FORCE);
            m_jump_timer = JUMP_RATE;
            MARIO_GAME.playSound("jump_super");
        } else {
            // prolong jump
            addImpulse(Vector::UP * JUMP_FORCE * (m_jump_timer / JUMP_RATE));
        }
        break;
    case EnvState::WATER:
        m_jump_timer = SWIM_RATE;
        addImpulse(0.7 * Vector::UP * JUMP_FORCE);
        MARIO_GAME.playSound("squish");
        break;
    case EnvState::LADDER:
        addImpulse(1.5f * Vector::UP * JUMP_FORCE);
        setUnclimb();
        MARIO_GAME.playSound("jump_super");
        break;
    }
}

bool Mario::canJump() const {
    switch (m_env_state) {
    case EnvState::NORMAL:
        return (!m_jump_timer && isGrounded()) ||        // from ground
               (m_jump_timer > (JUMP_RATE - JUMP_TIME)); // prolong jump
    case EnvState::WATER:
        return !m_jump_timer;
    case EnvState::LADDER:
        return true;
    default:
        return false;
    }
}

void Mario::addImpulse(const Vector& speed) {
    m_speed += speed;
}

bool Mario::isGrounded() const {
    return (m_collision_tag & ECollisionTag::FLOOR);
}

bool Mario::isClimbing() const {
    return (m_env_state == EnvState::LADDER);
}

void Mario::setUnclimb() {
    m_env_state = isInWater() ? EnvState::WATER
                              : EnvState::NORMAL;
}

void Mario::setInvincibleMode(bool value) {
    m_invincible_mode = value;
    m_invincible_timer = INNINCIBLE_TIME;
}

void Mario::collisionProcessing(float delta_time) {
    m_collision_tag = ECollisionTag::NONE;
    setPosition(m_blocks->collsionResponse(getBounds(), m_speed, delta_time, m_collision_tag));

    auto items = getParent()->findChildObjectsByType<Item>(true);

    for (auto item : items) {
        if (item->getBounds().isIntersect(getBounds())) {
            item->collsionResponse(this, m_collision_tag, delta_time);
        }
    }

    if (!isClimbing() && (m_input_direction == Vector::UP)) {
        auto ladders = getParent()->findChildObjectsByType<Ladder>();
        for (auto ladder : ladders) {
            if (ladder->getBounds().isIntersect(getBounds())) {
                m_used_ladder = ladder;
                m_env_state = EnvState::LADDER;

                int x_ladder = ladder->getBounds().center().x;
                if (x_ladder > getBounds().center().x) {
                    setPosition(x_ladder - getBounds().width(), getPosition().y);
                    m_animator->flipX(false);
                } else {
                    setPosition(x_ladder, getPosition().y);
                    m_animator->flipX(true);
                }
                m_speed = Vector::ZERO;
                break;
            }
        }
    }

    if (m_collision_tag & ECollisionTag::X_AXIS) {
        m_speed.x = 0;
    }

    if (m_collision_tag & ECollisionTag::CELL) {
        m_speed.y = 0.1f;
    }

    if (m_collision_tag & ECollisionTag::FLOOR) {
        m_speed.y = 0.f;
    }
}

void Mario::setSpeed(const Vector& speed) {
    m_speed = speed;
}

void Mario::onStarted() {
    bool in_water = (getProperty("InWater").isValid() && getProperty("InWater").asBool());
    m_env_state = in_water ? EnvState::WATER
                           : EnvState::NORMAL;

    if (getProperty("SpawnDirection").isValid()) {
        Vector direction = Vector::fromString(getProperty("SpawnDirection").asString());
        if (direction != Vector::ZERO) {
            move(-direction * 0.03f * 2000);
            setState(new TransitionMarioState(direction * 0.03f, 2000));
        }
    }

    if (getProperty("StartScript").isValid()) {
        auto script = getProperty("StartScript").asString();
        if (script == "GoToPortal") {
            setState(new GoToPortalState());
        }
    }

    m_blocks = getParent()->findChildObjectByType<Blocks>();
}

bool Mario::isInWater() const {
    return (m_env_state == EnvState::WATER);
}
//---------------------------------------------------------------------------
//! IMarioState
//---------------------------------------------------------------------------
void IMarioState::setMario(Mario* mario) {
    m_mario = mario;
}

Mario* IMarioState::getMario() {
    return m_mario;
}

GameObject* IMarioState::getScene() {
    return m_mario->getParent();
}

void IMarioState::playAnimation(Mario::AnimType animType, float speed) {
    getMario()->playAnimation(animType, speed);
}

void IMarioState::setPallete(Mario::PalleteType pallete) {
    getMario()->setPallete(pallete);
}

void IMarioState::setMarioRank(MarioRank rank) {
    getMario()->setRank(rank);
}

void IMarioState::enableScene(bool value) {
    auto mario = getMario();
 
    for (auto obj : getScene()->getChilds()) {
        if (obj != mario) {
            if (value) {
                obj->enable();
            } else {
                obj->disable();
            }
        }
    }
}

void IMarioState::setMarioState(State state) {
    getMario()->setState(state);
}

void IMarioState::setMarioSpeed(const Vector& speed) {
    getMario()->m_speed = speed;
}

void IMarioState::marioPhysicProcessing(float delta_time) {
    getMario()->physicProcessing(delta_time);
}

void IMarioState::setMarioSpawnProtection() {
    getMario()->m_spawn_timer = SPAWN_PROTECTION_TIME;
}
//---------------------------------------------------------------------------
// ! PromotingMarioState
//---------------------------------------------------------------------------
void PromotingMarioState::onEnter() {
    m_promoting_timer = 1500;
    getMario()->m_animator->setColor(sf::Color::White);
    enableScene(false);

    if (getMario()->getRank() == MarioRank::SMALL) {
        setMarioRank(MarioRank::BIG);
        playAnimation(AnimType::GROWING);
    } else if (getMario()->getRank() == MarioRank::BIG) {
        setMarioRank(MarioRank::FIRE);
    }

    getMario()->show();
}
//---------------------------------------------------------------------------
void PromotingMarioState::onLeave() {
    enableScene(true);
    if (getMario()->getRank() == MarioRank::FIRE) {
        setPallete(PalleteType::FIRE);
    }
}
//---------------------------------------------------------------------------
void PromotingMarioState::update(int delta_time) {
    auto mario = getMario();

    if (mario->getRank() == MarioRank::FIRE) {
        int id = int(m_promoting_timer / 100) % 2;
        setPallete(id ? PalleteType::FIRE : PalleteType::NORMAL);
    }

    m_promoting_timer -= delta_time;

    if (m_promoting_timer < 0) {
        setMarioState(State::NORMAL);
    }
}
//---------------------------------------------------------------------------
// ! DiedMarioState
//---------------------------------------------------------------------------
void DemotingMarioState::onEnter() {
    enableScene(false);
    m_promoting_timer = 1500;
    setPallete(PalleteType::NORMAL);
    setMarioRank(MarioRank::SMALL);
    playAnimation(AnimType::DEMOTING);
    MARIO_GAME.playSound("pipe");
    getMario()->show();
}

void DemotingMarioState::onLeave() {
    enableScene(true);
    setMarioSpawnProtection();
}

void DemotingMarioState::update(int delta_time) {
    playAnimation(AnimType::DEMOTING);
    m_promoting_timer -= delta_time;
    if (m_promoting_timer < 0) {
        setMarioState(State::NORMAL);
    }
}
//---------------------------------------------------------------------------
// ! NormalMarioState
//---------------------------------------------------------------------------
void NormalMarioState::onEnter() {
}

void NormalMarioState::onLeave() {
}

void NormalMarioState::update(int delta_time) {
    auto mario = getMario();
    mario->inputProcessing(delta_time);
    mario->physicProcessing(delta_time);
    mario->collisionProcessing(delta_time);
    mario->kickBlocksProcessing();
    mario->kickEnemiesProcessing(delta_time);
    mario->animationProcessing(delta_time);

    //check if mario fall undergroud
    static int screen_height = MARIO_GAME.screenSize().y - 200;
    if (mario->getBounds().bottom() > screen_height) {
        if (mario->getParent()->castTo<MarioGameScene>()->getLevelName().substr(0,2) != "Sk") {
            setMarioState(State::DIED);
        }
        if (mario->m_invincible_mode) {
            MARIO_GAME.invincibleMode(false);
            MARIO_GAME.stopMusic();
        }
    }

    if (mario->m_invincible_mode) {
        PalleteType pallete = (int(mario->m_invincible_timer / 100) % 2) ? PalleteType::NORMAL
                                                                         : PalleteType::FIRE;

        mario->m_invincible_timer -= delta_time;
        if (mario->m_invincible_timer < 0) {
            pallete = (mario->getRank() == MarioRank::FIRE) ? PalleteType::FIRE
                                                            : PalleteType::NORMAL;
            mario->m_invincible_mode = false;
            MARIO_GAME.invincibleMode(false);
        }

        mario->setPallete(pallete);
    }
}
//---------------------------------------------------------------------------
// ! TransitionMarioState
//---------------------------------------------------------------------------
TransitionMarioState::TransitionMarioState(const Vector& speed, int time) {
    m_speed = speed;
    m_timer = time;
}

void TransitionMarioState::onEnter() {
    getMario()->setSpeed(m_speed.normalized() * WALK_SPEED / 2.f);
    getMario()->moveUnderTo(getScene()->findChildObjectByType<Blocks>());

    auto piranas = getScene()->findChildObjectsByType<PiranhaPlant>();
    for (auto pirana : piranas) {
        pirana->hideInTube();
    };

    if ((m_speed.y > 0) || (m_speed.x > 0)) {
        MARIO_GAME.playSound("pipe");
    }
}

void TransitionMarioState::onLeave() {
    getMario()->moveToFront();
    setMarioSpeed(Vector::ZERO);
}

void TransitionMarioState::update(int delta_time) {
    if (m_timer > 0) {
        m_timer -= delta_time;
        getMario()->move(delta_time * m_speed);
        getMario()->animationProcessing(delta_time);
    } else {
        setMarioState(State::NORMAL);
    }
}
//---------------------------------------------------------------------------
// !GoToCastleMarioState
//---------------------------------------------------------------------------
GoToCastleMarioState::GoToCastleMarioState() {
}

void GoToCastleMarioState::onEnter()
{
    MARIO_GAME.musicManager().stop();
    auto m_block = getScene()->findChildObjectByType<Blocks>();
    m_cell_y = m_block->getRenderBounds().height() - m_block->blockSize().y * 4 - (getMario()->isSmall() ? 2 : 34);
    getMario()->m_input_direction = Vector::ZERO;

    if (getMario()->isSeated()) {
        getMario()->standUp();
    }
}

void GoToCastleMarioState::onLeave() {
}

void GoToCastleMarioState::update(int delta_time) {
    switch(m_state) {
    case State::START:
        getMario()->m_animator->flipX(false);
        playAnimation(AnimType::CLIMB, 0.005f);
        m_state = State::GO_DOWN;
        MARIO_GAME.playSound("flagpole");
        break;
    case State::GO_DOWN:
        if (getMario()->getPosition().y < m_cell_y) {
            getMario()->move(Vector::DOWN * delta_time * 0.25f);
        } else {
            m_state = State::OVERTURN;
            getMario()->m_animator->flipX(true);
            getMario()->move(Vector::RIGHT * 32.f);
            m_delay_timer = 500;
        }
        break;
    case State::OVERTURN:
        m_delay_timer -= delta_time;
        if (m_delay_timer < 0) {
            m_state = State::WALK;
            MARIO_GAME.playSound("stage_clear");
            playAnimation(AnimType::WALK, 0.003f);
            getMario()->m_animator->flipX(false);
            getMario()->m_input_direction = Vector::ZERO;
        }
        break;
    case State::WALK:
        getMario()->addImpulse(Vector::RIGHT*10.f);
        getMario()->physicProcessing(delta_time);
        getMario()->collisionProcessing(delta_time);

        for (auto portal : getScene()->findChildObjectsByType<LevelPortal>()) {
            if (portal->getBounds().isContainByX(getMario()->getPosition())) {
                m_state = State::WAIT;
                getMario()->hide();
                m_next_level = portal->getProperty("Level").asString();
                if (portal->getProperty("SubLevel").isValid()) {
                    m_next_sub_level = portal->getProperty("SubLevel").asString();
                }
                MARIO_GAME.setEndLevelStatus();
            }
        }
        break;
    case State::WAIT:
        if (MARIO_GAME.getGameTime() == 0) {
            m_delay_timer = 3500;
            m_state = State::NEXT_LEVEL;
            auto flag = getScene()->findChildObjectByType<CastleFlag>();
            assert(flag); // there is no castle flag in level
            flag->liftUp();
        }
        break;
    case State::NEXT_LEVEL:
        m_delay_timer -= delta_time;
        if (m_delay_timer < 0) {
            MARIO_GAME.loadLevel(m_next_level);
            if (!m_next_sub_level.empty())
                MARIO_GAME.loadSubLevel(m_next_sub_level);
            MARIO_GAME.showStatus();
            return;
        }
        break;
    };
 
    getMario()->m_animator->update(delta_time);
}
//---------------------------------------------------------------------------
// !GoToPortalMarioState
//---------------------------------------------------------------------------
GoToPortalState::GoToPortalState() {
}

void GoToPortalState::onEnter() {
    playAnimation(AnimType::WALK, 0.003f);
}

void GoToPortalState::onLeave() {
}
//---------------------------------------------------------------------------
void GoToPortalState::update(int delta_time) {
    getMario()->m_input_direction = Vector::RIGHT;
    getMario()->m_speed.x = WALK_SPEED * 0.6f;
    getMario()->move(getMario()->getSpeed() * delta_time);
    getMario()->collisionProcessing(delta_time);
    getMario()->m_collision_tag |= ECollisionTag::FLOOR;
}
//---------------------------------------------------------------------------
// !GoToPrincessMarioState
//---------------------------------------------------------------------------
GoToPrincessState::GoToPrincessState()
{
}

void GoToPrincessState::onEnter() {
    m_princess = getScene()->findChildObjectByType<Princess>();
    getMario()->m_input_direction = Vector::RIGHT;
    getMario()->m_direction = Vector::RIGHT;
    playAnimation(AnimType::WALK, 0.003f);
    getMario()->m_animator->flipX(false);

    if (getMario()->isSeated()) {
        getMario()->standUp();
    }
}

void GoToPrincessState::onLeave() {
}

void GoToPrincessState::update(int delta_time) {
    if (m_princess->getPosition().x - getMario()->getBounds().right() > 16) {
        getMario()->m_speed.x = WALK_SPEED * 0.6f;
        getMario()->physicProcessing(delta_time);
        getMario()->collisionProcessing(delta_time);
        if (getMario()->isGrounded()) {
            getMario()->m_animator->update(delta_time);
        }
    } else {
        playAnimation(AnimType::IDLE);
        getMario()->m_speed.x = 0;
    }
}
//---------------------------------------------------------------------------
// ! DiedMarioState
//---------------------------------------------------------------------------
void DiedMarioState::onEnter()  {
    enableScene(false);
    setMarioRank(MarioRank::SMALL);
    playAnimation(AnimType::DIED);
    setMarioSpeed(Vector::ZERO);
    getMario()->addImpulse(Vector::UP * 0.8f);
    MARIO_GAME.musicManager().stop();
    MARIO_GAME.playSound("mario_die");
}

void DiedMarioState::onLeave() {
}

void DiedMarioState::update(int delta_time) {
    marioPhysicProcessing(delta_time);
    m_delay_timer -= delta_time;

    if (m_delay_timer < 0)
    {
        MARIO_GAME.marioDied();
    }
}
