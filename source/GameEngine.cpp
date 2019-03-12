
#include "GameEngine.h"
#include <assert.h>


#define SWITCH_STR(A) switch(hash_str((A)))
#define CASE_STR(A) case(hash_str((A)))

std::vector<std::string> split(const std::string &s, char delim)
{
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, delim))
		elems.push_back(item);
	return elems;
}

float toFloat(const std::string& str)
{
    return std::stof(str);
}

int toInt(const std::string& str)
{
    return std::stoi(str);
}

bool toBool(const std::string& str)
{
    if (str == "true" || str == "True" || str == "TRUE")
        return true;
    return false;
}

namespace math
{
int sign(float value)
{
    return (value > 0) - (value < 0);
}
}
void drawLinearSprite_v(sf::Sprite sprite, const sf::Rect<int>& draw_area, sf::RenderWindow* render_window)
{
    if (!draw_area.height)
        return;

    int k = draw_area.height / std::abs(sprite.getTextureRect().height);
    int off_set = draw_area.height % std::abs(sprite.getTextureRect().height);

    for (int i = 0; i < k; ++i)
    {
        sprite.setPosition(draw_area.left, i * 32.f + draw_area.top);
        render_window->draw(sprite);
    }
    auto new_rect = sprite.getTextureRect();
    new_rect.height = off_set;
    sprite.setTextureRect(new_rect);
    sprite.setPosition(draw_area.left, k * 32.f + draw_area.top);
    render_window->draw(sprite);
}

void drawLinearSprite_h(sf::Sprite sprite, const sf::Rect<int>& draw_area, sf::RenderWindow* render_window)
{
    if (!draw_area.width)
        return;

    int k = draw_area.width / std::abs(sprite.getTextureRect().width);
    int off_set = draw_area.width % std::abs(sprite.getTextureRect().width);

    for (int i = 0; i < k; ++i)
    {
        sprite.setPosition(i * 32.f + draw_area.left, draw_area.top);
        render_window->draw(sprite);
    }
    auto new_rect = sprite.getTextureRect();
    new_rect.width = off_set;
    sprite.setTextureRect(new_rect);
    sprite.setPosition(k * 32.f + draw_area.left, draw_area.top);
    render_window->draw(sprite);
}

//----------------------------------------------------------------------------------------------

Property::Property()
{
    m_type = Type::NoInit;
}

Property::~Property()
{
	if (m_type == Type::String)
		delete string_data;
}

Property::Property(const Property& property)
{
	m_type = property.m_type;
	switch (m_type)
	{
		case(Type::Float):
		{
			float_data = property.float_data;
			break;
		}
		case(Type::Int):
		{
			int_data = property.int_data;
			break;
		}
		case(Type::Bool):
		{
			bool_data = property.bool_data;
			break;
		}
		case(Type::String):
		{
			string_data = new std::string(*property.string_data);
			break;
		}
		default:break;
	}
}

Property& Property::operator=(const Property& property)
{
	m_type = property.m_type;
	switch (m_type)
	{
		case(Type::Float):
		{
			float_data = property.float_data;
			break;
		}
		case(Type::Int):
		{
			int_data = property.int_data;
			break;
		}
		case(Type::Bool):
		{
			bool_data = property.bool_data;
			break;
		}
		case(Type::String):
		{
			string_data = new std::string(*property.string_data);
			break;
		}
		default:break;
	}
	return *this;
}

Property::Property(Property&& property)
{
	m_type = property.m_type;
	switch (m_type)
	{
	case(Type::Float):
	{
		float_data = property.float_data;
		break;
	}
	case(Type::Int):
	{
		int_data = property.int_data;
		break;
	}
	case(Type::Bool):
	{
		bool_data = property.bool_data;
		break;
	}
	case(Type::String):
	{
		string_data = property.string_data;
		property.string_data = nullptr;
		break;
	}
	default:break;
	}
}

Property& Property::operator=(Property&& property)
{
	m_type = property.m_type;
	switch (m_type)
	{
	case(Type::Float):
	{
		float_data = property.float_data;
		break;
	}
	case(Type::Int):
	{
		int_data = property.int_data;
		break;
	}
	case(Type::Bool):
	{
		bool_data = property.bool_data;
		break;
	}
	case(Type::String):
	{
		string_data = property.string_data;
		property.string_data = nullptr;
		break;
	}
	default:break;
	}
	return *this;
}

Property::Property(bool bool_value)
{
    m_type = Type::Bool;
    bool_data = bool_value;
}

Property::Property(int int_value)
{
    m_type = Type::Int;
    int_data = int_value;
}

Property::Property(const std::string& string_value)
{
    m_type = Type::String;
    string_data = new std::string(string_value);
}

Property::Property(float float_value)
{
    m_type = Type::Float;
    float_data = float_value;
}

bool Property::asBool() const
{
    assert(m_type == Type::Bool);
    return bool_data;
}
int Property::asInt() const
{
    assert(m_type == Type::Int);
    return int_data;
}
float Property::asFloat() const
{
    assert(m_type == Type::Float);
    return float_data;
}
const std::string& Property::asString() const
{
    assert(m_type == Type::String);
    return *string_data;
}
bool Property::isValid() const
{
    return m_type != Type::NoInit;
}

//----------------------------------------------------------------------------------------------

void CEventManager::pushEvent(const sf::Event& event)
{
    for (auto& sub : m_subcribes)
        sub->events(event);
}

void CEventManager::subscribe(CGameObject* object)
{
    m_subcribes.push_back(object);
}

void CEventManager::unsubcribe(CGameObject* object)
{
    auto it = std::find(m_subcribes.begin(), m_subcribes.end(), object);
    if (it != m_subcribes.end())
        m_subcribes.erase(it);
}

//-----------------------------------------------------------------------------------------------

CInputManager::CInputManager()
{
    m_keys_prev_ptr = &m_keys_prev;
    m_keys_now_ptr = &m_keys_now;
	m_jsk_btns_prev_ptr = &m_jsk_btns_prev;
	m_jsk_btns_now_ptr = &m_jsk_btns_now;

	for (auto& ax : m_axis_keys)
		ax = sf::Keyboard::Key::Unknown;
}

void CInputManager::registerKey(const sf::Keyboard::Key& key)
{
    m_keys_prev.insert(std::make_pair(key, false));
    m_keys_now.insert(std::make_pair(key, false));
}

void CInputManager::unregisterKey(const sf::Keyboard::Key& key)
{
    m_keys_prev.erase(m_keys_prev.find(key));
    m_keys_now.erase(m_keys_now.find(key));
}

void CInputManager::registerJoysticButton(int index)
{
	m_jsk_btns_prev.insert(std::make_pair(index, false));
	m_jsk_btns_now.insert(std::make_pair(index, false));
}

bool CInputManager::isKeyJustPressed(const sf::Keyboard::Key& key) const
{
    if (m_keys_prev_ptr->count(key))
        if (!(*m_keys_prev_ptr)[key] && (*m_keys_now_ptr)[key])
            return true;
    return false;
}

bool CInputManager::isKeyJustReleased(const sf::Keyboard::Key& key) const
{
    if (m_keys_prev_ptr->count(key))
        if ((*m_keys_prev_ptr)[key] && !(*m_keys_now_ptr)[key])
            return true;
    return false;
}

bool CInputManager::isKeyPressed(const sf::Keyboard::Key& key) const
{
    if (m_keys_now_ptr->count(key))
        return (*m_keys_now_ptr)[key];
    return false;
}

Vector CInputManager::getXYAxis() const
{
	Vector value;
	if (sf::Joystick::isConnected(0))
	{
		value.x = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) / 100.f;
		value.y = -sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) / 100.f;
		value.x = math::sens(value.x, 0.5f);
		value.y = math::sens(value.y, 0.5f);
	}
 
	if (m_axis_keys[0] != -1 && sf::Keyboard::isKeyPressed(m_axis_keys[0])) value.y = -1;
	if (m_axis_keys[1] != -1 && sf::Keyboard::isKeyPressed(m_axis_keys[1])) value.x = 1;
	if (m_axis_keys[2] != -1 && sf::Keyboard::isKeyPressed(m_axis_keys[2])) value.y = 1;
	if (m_axis_keys[3] != -1 && sf::Keyboard::isKeyPressed(m_axis_keys[3])) value.x = -1;
 
	return value;
}

bool CInputManager::isButtonPressed(const std::string& button) const
{
	if (m_btn_to_key.count(button) && isKeyPressed(m_btn_to_key.at(button)))
		return true;
	if (m_jsk_btn_to_key.count(button) && isJoystickButtonPressed(m_jsk_btn_to_key.at(button)))
		return true;
	return false;
}

bool CInputManager::isButtonDown(const std::string& button) const
{
	if (m_btn_to_key.count(button) && isKeyJustPressed(m_btn_to_key.at(button)))
		return true;
	if (m_jsk_btn_to_key.count(button) && isJoystickButtonJustPressed(m_jsk_btn_to_key.at(button)))
		return true;
	return false;
}

bool CInputManager::isButtonUp(const std::string& button) const
{
	if (m_btn_to_key.count(button) && isKeyJustReleased(m_btn_to_key.at(button)))
		return true;
	if (m_jsk_btn_to_key.count(button) && isJoystickButtonJustPressed(m_jsk_btn_to_key.at(button)))
		return true;
	return false;
}

sf::Keyboard::Key CInputManager::toKey(const std::string& str)
{
	if (str.length() == 1 && str[0] >= 'A' && str[0] <= 'Z')
		return static_cast<sf::Keyboard::Key>(str[0] - 65);

		static const std::map<std::string, sf::Keyboard::Key> key_map =
		{
		{ "Left", sf::Keyboard::Left },
		{ "Right", sf::Keyboard::Right },
		{ "Up", sf::Keyboard::Up },
		{ "Down", sf::Keyboard::Down },
		{ "Space", sf::Keyboard::Space },
		{ "LShift", sf::Keyboard::LShift },
		{ "Enter", sf::Keyboard::Enter },
		{ "Return", sf::Keyboard::Return },
		};

		if (key_map.find(str) != key_map.end())
			return key_map.at(str);
}

void CInputManager::setupButton(const std::string& button, const std::vector<std::string>& keys)
{
	static const std::unordered_map<std::string, int> special_keys = 
	{
		{ "Vertical-",0},
		{ "Horizontal+",1 },
		{ "Vertical+",2 },
		{ "Horizontal-",3 }
	};

	if (special_keys.count(button))
	{
		m_axis_keys[special_keys.at(button)] = toKey(keys[0]);
		return;
	}

	for (auto key : keys)
	{
		if (key.front() == '[' && key.back() == ']') //joystick btn
		{
			int index = toInt(key.substr(1, key.length() - 2));
			registerJoysticButton(index);
			m_jsk_btn_to_key[button] = index;
		}
		else 
		{
			sf::Keyboard::Key pkey = toKey(key);
			registerKey(pkey);
			m_btn_to_key[button] = pkey;
		}
	}
}

void CInputManager::update(int delta_time)
{
    std::swap(m_keys_now_ptr, m_keys_prev_ptr);
	for (auto& key : *m_keys_now_ptr)
		key.second = sf::Keyboard::isKeyPressed(key.first);

	std::swap(m_jsk_btns_now, m_jsk_btns_prev);
	for (auto& btn : *m_jsk_btns_now_ptr)
		btn.second = sf::Joystick::isButtonPressed(0, btn.first);
}

bool CInputManager::isJoystickButtonPressed(int index) const
{
	if (m_jsk_btns_now_ptr->count(index))
		return (*m_jsk_btns_now_ptr)[index];
	return false;
}

bool CInputManager::isJoystickButtonJustPressed(int index) const
{
	if (m_jsk_btns_prev_ptr->count(index))
		if (!(*m_jsk_btns_prev_ptr)[index] && (*m_jsk_btns_now_ptr)[index])
			return true;
	return false;
}

bool CInputManager::isJoystickButtonJustReleased(int index) const
{
	if (m_jsk_btns_prev_ptr->count(index))
		if ((*m_jsk_btns_prev_ptr)[index] && !(*m_jsk_btns_now_ptr)[index])
			return true;
	return false;
}

//-----------------------------------------------------------------------------------------------
const Vector& CGameObject::getPosition() const
{
    return m_pos;
}

void CGameObject::setPosition(const Vector& point)
{
    onPositionChanged(point, m_pos);
    m_pos = point;
}

void CGameObject::setPosition(float x, float y)
{
    setPosition(Vector(x, y));
}

void CGameObject::move(const Vector& point)
{
    m_pos += point;
}

void CGameObject::setSize(const Vector& size)
{
    m_size = size;
}

Rect CGameObject::getBounds() const
{
    return Rect(m_pos,m_size);
}

void CGameObject::setBounds(const Rect& rect)
{
    m_pos = rect.leftTop();
    m_size = rect.size();
}

CGameObject::CGameObject()
{
    m_enable = m_visible = true;
    m_parent = NULL;
}

void CGameObject::setParent(CGameObject* game_object)
{
    m_parent = game_object;
}

CGameObject* CGameObject::getParent() const
{
    return m_parent;
}

void CGameObject::update(int delta_time)
{
    if (isEnabled())
    {
        for (auto& obj : m_objects)
            if (!obj->m_started)
            {
                obj->m_started = true;
                obj->start();
            }

        for (auto& obj : m_objects)
            if (obj->isEnabled())
                obj->update(delta_time);
    }
}

void CGameObject::start()
{

}

void CGameObject::setName(const std::string& name)
{
    m_name = name;
}

const std::string&  CGameObject::getName() const
{
    return m_name;
}

void CGameObject::setProperty(const std::string& name, const Property& property)
{
    m_properties[name] = property;
    onPropertySet(name);
};

Property CGameObject::getProperty(const std::string& name) const
{
    onPropertyGet(name);
    return const_cast<CGameObject*>(this)->m_properties[name];
};

void CGameObject::disable()
{
    m_enable = false;
}

void CGameObject::enable()
{
    m_enable = true;
}

bool CGameObject::isEnabled() const
{
    return m_enable;
}

void CGameObject::hide()
{
    m_visible = false;
}

void CGameObject::show()
{
    m_visible = true;
}

bool CGameObject::isVisible() const
{
    return m_visible;
}

void CGameObject::turnOn()
{
    show();
    enable();
}
void CGameObject::turnOff()
{
    hide();
    disable();
}

CGameObject* CGameObject::addObject(CGameObject* object)
{
    m_objects.push_back(object);
    object->setParent(this);
    object->onActivated();
    if (m_started)
    {
        object->m_started = true;
        object->start();
    }
    return object;
}

CGameObject* CGameObject::findObjectByName(const std::string& name)
{
    auto it = std::find_if(m_objects.begin(), m_objects.end(), [this, &name](const CGameObject* obj) -> bool { return obj->getName() == name;  });
    if (it != m_objects.end())
        return *it;
    return nullptr;
}

CGameObject::~CGameObject()
{
    for (auto& obj : m_objects)
        delete obj;
    m_objects.clear();
}

void CGameObject::draw(sf::RenderWindow* window)
{
    if (isVisible())
        for (auto& obj : m_objects)
            if (obj->isVisible())
                obj->draw(window);
}

void CGameObject::postDraw(sf::RenderWindow* window)
{
    if (isVisible())
        for (auto& obj : m_objects)
            if (obj->isVisible())
                obj->postDraw(window);
}

void CGameObject::foreachObject(std::function<void(CGameObject*)> predicate)
{
    for (auto& obj : m_objects)
        predicate(obj);
}
void CGameObject::foreachObject(std::function<void(CGameObject*, bool& )> predicate)
{
    bool need_break = false;
    for (auto& obj : m_objects)
    {
        predicate(obj, need_break);
        if (need_break)
            break;
    }
}

void CGameObject::removeObject(CGameObject* object)
{
    auto action = [this, object]() //remove object later
    {
        auto it = std::find(m_objects.begin(), m_objects.end(), object);
        assert(it != m_objects.end());
        m_objects.erase(it);
        delete object;
    };
    m_preupdate_actions.push_back(action);
}

void CGameObject::onPropertySet(const std::string& name)
{
    if (name == "x")
        setPosition(m_properties["x"].asFloat(),getPosition().y);
    else if (name == "y")
        setPosition(getPosition().x, m_properties["y"].asFloat());
    else if (name == "name")
        setName(m_properties["name"].asString());

}

void CGameObject::onPropertyGet(const std::string& name) const
{

}

void CGameObject::moveToBack()
{
    if (getParent())
    {
        auto move_to_back_action = [this]()
        {
            auto list = &(getParent()->m_objects);
            auto it = std::find(list->begin(), list->end(), this);
            assert(*it == this);
            auto tmp = *it;
            it = list->erase(it);
            list->push_front(tmp);
        };
        m_preupdate_actions.push_back(move_to_back_action);
    }
}

void CGameObject::moveToFront()
{
    if (getParent())
    {
        auto move_to_front_action = [this]()
        {
            auto list = &(getParent()->m_objects);
            auto it = std::find(list->begin(), list->end(), this);
            assert(*it == this);
            auto tmp = *it;
            it = list->erase(it);
            list->push_back(tmp);
        };

        m_preupdate_actions.push_back(move_to_front_action);
    }
}

void CGameObject::moveUnderTo(CGameObject* obj)
{
    if (getParent())
    {
        auto move_under_action = [this,obj]()
        {
            auto list = &(getParent()->m_objects);
            auto this_obj = std::find(list->begin(), list->end(), this);
            auto other_obj = std::find(list->begin(), list->end(), obj);
            assert(this_obj != list->end() && other_obj != list->end());
            list->erase(this_obj);
            list->insert(other_obj,this);
        };

        m_preupdate_actions.push_back(move_under_action);
    }
}

void CGameObject::clear()
{
    for (auto object : m_objects)
        delete object;
    m_objects.clear();
}

std::vector<std::function<void()>> CGameObject::m_preupdate_actions = std::vector<std::function<void()>>();

void CGameObject::invokePreupdateActions()
{
    for (auto& action : m_preupdate_actions)
        action();
    m_preupdate_actions.clear();
}

//---------------------------------------------------------------------------------------------------------

void Timer::invoke(const std::function <void()>& func, int delay)
{
	m_invoke_list.push_back(std::make_pair<>(m_time + delay, func));
}

void Timer::update(int delta_time)
{
	if (!m_paused)
	{
		m_time += delta_time;
		for (auto it = m_invoke_list.begin(); it != m_invoke_list.end(); )
		{
			if (m_time > it->first)
			{
				it->second();
				it = m_invoke_list.erase(it);
			}
			else
				++it;
		}
	}
}

void Timer::pause()
{
	m_paused = true;
}

void Timer::play()
{
	m_paused = false;
}

//---------------------------------------------------------------------------------------------------------

void CGame::init()
{

}

CGame::~CGame()
{

}

CGame::CGame(const std::string& name, const Vector& screen_size)
{
    m_root_object = new CGameObject();
    m_root_object->setName(name);
    m_screen_size = screen_size;
}

void CGame::updateStats(const sf::Time time)
{
    if (time > m_max_time)
        m_max_time = time;
    if (time < m_min_time)
        m_min_time = time;

    static int k = 0;

    if (++k % 60 == 0)
    {
        std::cout << "min: " << m_min_time.asMilliseconds() << " max: " << m_max_time.asMilliseconds() << std::endl;
        m_min_time = sf::seconds(3600);
        m_max_time = sf::Time::Zero;
    }
}

void CGame::setClearColor(const sf::Color& color)
{
    m_clear_color = color;
}

void CGame::run()
{
    m_window = new sf::RenderWindow(sf::VideoMode(m_screen_size.x, m_screen_size.y), m_root_object->getName());
    init();

    sf::Event event;
    sf::Clock clock;
    sf::Time acumulator = sf::Time::Zero;
    const sf::Time ups = sf::seconds(1.f / 60.f);
    //m_window->setFramerateLimit(60);
    //m_window->setVerticalSyncEnabled(true);

    while (true)   // game loop
    {

        while (m_window->pollEvent(event))
        {
            if (event.type == sf::Event::EventType::Closed)
            {
                m_window->close();
                exit(0);
            }
            eventManager().pushEvent(event);
        }

        sf::Time elapsedTime = clock.restart();
        acumulator += elapsedTime;
        //updateStats(elapsedTime);

        while (acumulator > ups)
        {
            acumulator -= ups;
            inputManager().update(ups.asMilliseconds());
            update(ups.asMilliseconds());
            sf::sleep(sf::milliseconds(10));
        }

        m_window->clear(m_clear_color);
        draw(m_window);
        m_window->display();
    }
}

CGameObject*  CGame::getRootObject()
{
    return m_root_object;
}

void  CGame::draw(sf::RenderWindow* render_window)
{
    m_root_object->draw(render_window);
    m_root_object->postDraw(render_window);
}

void CGame::update(int delta_time)
{
    CGameObject::invokePreupdateActions(); //remove obj, change z-oreder, etc
    m_root_object->update(delta_time);
}

CTextureManager&  CGame::textureManager()
{
    return m_texture_manager;
}

CFontManager&  CGame::fontManager()
{
    return m_font_manager;
}

CSoundManager&  CGame::soundManager()
{
    return m_sound_manager;
}

CEventManager&  CGame::eventManager()
{
    return m_event_manager;
}

CInputManager&  CGame::inputManager()
{
    return m_input_manager;
}

CMusicManager& CGame::musicManager()
{
    return m_music_manager;
}

void CGame::playMusic(const std::string& name)
{
    m_music_manager.play(name);
}

void CGame::stopMusic()
{
    m_music_manager.stop();
}

void  CGame::playSound(const std::string& name)
{
    const int SOUND_BUFFER_SIZE = 40;
    int i = 0;
    while (m_sounds_buf[i].getStatus() == sf::Sound::Playing)
    {
        if (i >= SOUND_BUFFER_SIZE)
            break;
        ++i;
    }

    if (i >= SOUND_BUFFER_SIZE)
		throw std::runtime_error("sound buffer overflow");
    m_sounds_buf[i].setBuffer(*soundManager().get(name));
    m_sounds_buf[i].play();
}

Vector  CGame::screenSize() const
{
    return Vector((int)m_window->getSize().x, (int)m_window->getSize().y);
}

//---------------------------------------------------------------------------------------------------------

CSpriteSheet::CSpriteSheet()
{
    setName("SpriteSheet");
    m_speed = 0.03f;
    m_current_sprite = NULL;
    setAnimType(AnimType::manual);
    m_index = 0;
}

void CSpriteSheet::load(const sf::Texture& texture, const std::vector<sf::IntRect>& rects)
{
    m_sprites.clear();
    for (auto& rect : rects)
        m_sprites.emplace_back(texture, rect);
    setSpriteIndex(0);
}

void CSpriteSheet::load(const sf::Texture& texture, const Vector& off_set, const Vector& size, int cols, int rows)
{
    m_sprites.clear();
    for (int y = 0; y < rows; ++y)
        for (int x = 0; x < cols; ++x)
            m_sprites.emplace_back(texture, sf::IntRect(x*std::abs(size.x) + off_set.x, y*std::abs(size.y) + off_set.y,size.x,size.y));
    setSpriteIndex(0);
    setAnimType(AnimType::forward);
}

void CSpriteSheet::reset()
{
    m_index = 0;
}

bool CSpriteSheet::empty() const
{
    return m_sprites.empty();
}

void CSpriteSheet::setSpriteIndex(int index)
{
    assert(index >= 0 && index < m_sprites.size());
    m_current_sprite = &m_sprites[index];
}
sf::Sprite* CSpriteSheet::currentSprite()
{
    return m_current_sprite;
}

void  CSpriteSheet::setAnimType(AnimType type)
{
    m_anim_type = type;
    //	if (type == AnimType::forward)
    m_index = 0;
}

void CSpriteSheet::draw(sf::RenderWindow* wnd)
{
    switch (m_anim_type)
    {
		case(AnimType::manual):
		{
			break;
		}
		case(AnimType::forward_backward_cycle):
		{
			int size = m_sprites.size();
			int current_slide = int(m_index) % (size * 2);
			if (current_slide > size - 1)
				current_slide = 2 * size - 1 - current_slide;
			setSpriteIndex(current_slide);
			break;
		}
		case(AnimType::forward_cycle):
		{
			int current_slide = int(m_index) % m_sprites.size();
			setSpriteIndex(current_slide);
			break;
		}
		case(AnimType::forward_stop):
		{
			int current_slide = int(m_index);
			if (current_slide < m_sprites.size())
				setSpriteIndex(current_slide);
			break;
		}
		case(AnimType::forward):
		{
			int current_slide = int(m_index);
			if (current_slide < m_sprites.size())
				setSpriteIndex(current_slide);
			else
				return;
			break;
		}
    }
    wnd->draw(*m_current_sprite);
}

void CSpriteSheet::setPosition(sf::Vector2f pos)
{
    m_position = pos;
    for (auto& sprite : m_sprites)
        sprite.setPosition(m_position + m_torigin);
}

void CSpriteSheet::setOrigin(const Vector& pos)
{
    for (auto& sprite : m_sprites)
        sprite.setOrigin(pos);
}

sf::Sprite&  CSpriteSheet::operator[](int index)
{
    assert(index >= 0 && index < m_sprites.size());
    return m_sprites[index];
}

sf::Vector2f CSpriteSheet::getPosition() const
{
    return m_position;
}

void CSpriteSheet::scale(float fX, float fY)
{
    for (auto& sprite : m_sprites)
        sprite.scale(fX, fY);
}

void CSpriteSheet::update(int delta_time)
{
    if (isEnabled() && m_speed)
        m_index +=  m_speed*delta_time ;
}

void CSpriteSheet::setSpeed(float speed)
{
    m_speed = speed;
}

void CSpriteSheet::setColor(const sf::Color& color)
{
    for (auto& sprite : m_sprites)
        sprite.setColor(color);
}


void CSpriteSheet::setRotation(float angle)
{
    if (angle == m_sprites[0].getRotation())
        return;

    for (auto& sprite : m_sprites)
    {

        float w = sprite.getLocalBounds().width;
        float h = sprite.getLocalBounds().height;

        if (angle < 0) 
			angle += 360;
        if (angle > 360) 
			angle -= 360;

        if (angle == 0)
            sprite.setOrigin(0, 0);
        else if (angle == 90)
            sprite.setOrigin(0, h);
        else if (angle == 270)
            sprite.setOrigin(w, 0);
        else if (angle == 180)
            sprite.setOrigin(w, h);

        sprite.setRotation(angle);
    }
}

void CSpriteSheet::invert_h()
{
    //const auto&  rect = sprite.getTextureRect();
    //sf::IntRect new_rect(rect.width + rect.left, rect.top, -rect.width, rect.height);
    //return sf::Sprite(*texture, new_rect);
}

void CSpriteSheet::flipX(bool value)
{
    if (m_flipped != value)
    {
        m_flipped = value;
        for (auto& sprite : m_sprites)
        {
            auto rect = sprite.getTextureRect();
            rect.left += rect.width;
            rect.width = -rect.width;
            sprite.setTextureRect(rect);
        }
    }
}

void CSpriteSheet::setAnimOffset(float i)
{
    m_index += i;
}

AnimType CSpriteSheet::animType() const
{
    return m_anim_type;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------

Pallete::Pallete()
{
}

void Pallete::create(const std::initializer_list<sf::Color>& original_colors, const std::initializer_list<sf::Color>& swaped_colors)
{
    assert(original_colors.size() == swaped_colors.size());
    int arr_size = original_colors.size();
    const sf::String frag_shader =
			"#version 120\n"\
            "const int arr_size = " + toString(arr_size) + ";"\
                                                           "uniform vec3 color1[arr_size];"\
                                                           "uniform vec3 color2[arr_size];"\
                                                           "uniform sampler2D texture;"\
                                                           "void main()"\
                                                           "{"\
                                                           "vec4 myindex = texture2D(texture, gl_TexCoord[0].xy);"\
                                                           "vec3 index = vec3(myindex.r,myindex.g,myindex.b);"\
                                                           "gl_FragColor = myindex;"\
                                                           "for (int i=0; i < arr_size; ++i)"\
                                                           "if (index == color1[i]) {"\
                                                           "gl_FragColor = vec4(color2[i].r,color2[i].g,color2[i].b,myindex.a);"\
                                                           "break; }"\
                                                           "}";
    m_shader.loadFromMemory(frag_shader, sf::Shader::Fragment);
    auto colors1 = new sf::Glsl::Vec3[arr_size];
    auto colors2 = new sf::Glsl::Vec3[arr_size];
    int i = 0;
    for (auto original_color : original_colors)
        colors1[i++] = sf::Glsl::Vec3(original_color.r / 255.f, original_color.g / 255.f, original_color.b / 255.f);
    i = 0;
    for (auto swaped_color : swaped_colors)
        colors2[i++] = sf::Glsl::Vec3(swaped_color.r / 255.f, swaped_color.g / 255.f, swaped_color.b / 255.f);
    m_shader.setUniformArray("color1", (sf::Glsl::Vec3*)colors1, arr_size);
    m_shader.setUniformArray("color2", (sf::Glsl::Vec3*)colors2, arr_size);
    m_shader.setUniform("texture", sf::Shader::CurrentTexture);
    delete[] colors1;
    delete[] colors2;
}

void Pallete::apply()
{

    sf::Shader::bind(&m_shader);
}

void Pallete::cancel()
{
    sf::Shader::bind(NULL);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------

Animator::~Animator()
{
    for (auto anim : m_animations)
    {
        delete anim.second;
    }
}

void Animator::create(const std::string& name, const sf::Texture& texture, const Vector& off_set, const Vector& size, int cols, int rows, float speed, AnimType anim_type)
{
    CSpriteSheet* animation = new CSpriteSheet();
    animation->load(texture, off_set, size, cols, rows);
    animation->setAnimType(anim_type);
    animation->setSpeed(speed);
    assert(!m_animations[name]); // already exist
    m_animations[name] = animation;
    if (!m_current_animation) m_current_animation = animation;
}

void Animator::create(const std::string& name, const sf::Texture& texture, const Rect& rect)
{
    CSpriteSheet* animation = new CSpriteSheet();
    animation->load(texture, { { (int)rect.left(), (int)rect.top(), (int)rect.width(), (int)rect.height() } });
    assert(!m_animations[name]); // already exist
    m_animations[name] = animation;
    if (!m_current_animation) m_current_animation = animation;
}


void Animator::create(const std::string& name, const sf::Texture& texture, const std::vector<sf::IntRect>& rects, float _speed)
{
    CSpriteSheet* animation = new CSpriteSheet();

    animation->load(texture, rects);
    animation->setAnimType(AnimType::forward_cycle);
    animation->setSpeed(_speed);
    assert(!m_animations[name]); // already exist
    m_animations[name] = animation;
    if (!m_current_animation) m_current_animation = animation;
}


void Animator::play(const std::string& name)
{
    if (last_anim_name != name)
    {
        m_current_animation = m_animations[name];
        assert(m_current_animation); //not exist
        last_anim_name = name;

        //	if (m_current_animation->animType() == AnimType::forward_stop)
        m_current_animation->reset();
    }
}
void Animator::update(int delta_time)
{
    if (isEnabled())
        m_current_animation->update(delta_time);
}
void Animator::draw(sf::RenderWindow* wnd)
{
    if (isVisible())
    {
        if (m_pallete)
            m_pallete->apply();


        m_current_animation->setPosition(getPosition());
        m_current_animation->draw(wnd);

        if (m_pallete)
            m_pallete->cancel();
    }
}


void Animator::setPallete(Pallete* pallete)
{
    m_pallete = pallete;
}

void Animator::flipX(bool value)
{
    if (value != m_flipped)
    {
        m_flipped = value;
        for (auto& animation : m_animations)
            animation.second->flipX(value);
    }
}

void Animator::setColor(const sf::Color& color)
{
    for (auto& animation : m_animations)
        animation.second->setColor(color);
}

void Animator::setAnimOffset(float index)
{
    for (auto& animation : m_animations)
        animation.second->setAnimOffset(index);
}

void Animator::setSpeed(const std::string& anim, float speed)
{
    m_animations[anim]->setSpeed(speed);
}

void Animator::setSpriteOffset(const std::string& anim_name, int sprite_index, const Vector& value)
{
    CSpriteSheet* sheet = m_animations[anim_name];
    assert(sheet);
    sheet->operator[](sprite_index).setOrigin(-value);
}

void Animator::scale(float fX, float fY)
{
    for (auto& animation : m_animations)
        animation.second->scale(fX, fY);
}

CSpriteSheet* Animator::get(const std::string& str)
{
    return m_animations[str];
}

//---------------------------------------------------------------------------------------------------------

void CMusicManager::play(const std::string& name)
{
	if (name != "")
		m_current_music = name;
    assert(m_resources[m_current_music]); //unknown resource
	if (!m_current_music.empty())
		m_resources[m_current_music]->play();
}

void CMusicManager::stop()
{
	for (auto music : m_resources)
	{
		music.second->stop();
	}
	m_current_music.clear();
}

void CMusicManager::pause()
{
	for (auto music : m_resources)
		music.second->pause();
}

void CMusicManager::setPitch(float value)
{
    for (auto music : m_resources)
        music.second->setPitch(value);
}

//---------------------------------------------------------------------------------------------------------

CFlowText::CFlowText(const sf::Font& font, bool self_remove)
{
    m_text.setFont(font);
    m_text.setFillColor(sf::Color::Black);
    m_text.setCharacterSize(20);
    m_text.setStyle(sf::Text::Bold);
    m_flashing = false;
    m_self_remove = self_remove;
}

CFlowText* CFlowText::clone() const
{
    auto flow_text = new CFlowText(*m_text.getFont());
    flow_text->m_text =  m_text;
    flow_text->m_splash_vector = m_splash_vector;
    return flow_text;
}

bool CFlowText::isFlashing() const
{
    return m_flashing;
}
void CFlowText::splash(const Vector& pos, const std::string& text)
{
    m_flashing = true;
    setPosition(pos);
    m_text.setString(text);
    m_offset.x = m_offset.y = 0;
    m_time = 0;
}

void CFlowText::setSplashVector(const Vector& vector)
{
    m_splash_vector = vector;
}

void CFlowText::update(int delta_time)
{
    if (m_flashing)
    {
        m_time += delta_time;
        m_offset.x = m_time*0.03f*m_splash_vector.x;
        m_offset.y = m_time*0.03f*m_splash_vector.y;

        m_offset.x *= 2;
        m_color = (int)m_time*0.2f;
        if (m_color >= 255)
            m_flashing = false;

        const sf::Color& color = m_text.getFillColor();

        m_text.setFillColor(sf::Color(color.r, color.g, color.b, 255 - m_color));
    }
	else if (m_self_remove)
	{
		if (getParent())
			getParent()->removeObject(this);
		else
			delete this;
	}
}

void CFlowText::draw(sf::RenderWindow* window)
{
    if (m_flashing)
    {
        m_text.setPosition(getPosition() + m_offset);
        window->draw(m_text);
    }
}

void CFlowText::setTextColor(const sf::Color& color)
{
    m_text.setFillColor(color);
}

void CFlowText::setTextSize(int size)
{
    m_text.setCharacterSize(size);
}

//---------------------------------------------------------------------------------------------------------

CLabel::CLabel()
{
    init();
}

CLabel::CLabel(const std::string& str)
{
    init();
    setString(str);
}

CLabel::CLabel(const sf::Sprite& sprite)
{
    init();
    setSprite(sprite);
}

void CLabel::init()
{
    setName("Label");
    setFontColor(sf::Color::Black);
	m_sprite.setColor({ 255,255,255,50 });
}

void CLabel::setBounds(int x, int y, int w, int h)
{
    setPosition(x, y);
    m_rect = Rect(x, y, w, h);

    m_shape.setPosition(x, y);
    m_shape.setSize(sf::Vector2f(w, h));


    if (m_sprite.getTexture())
    {
        m_sprite.setPosition(sf::Vector2f(x, y));
        m_sprite.setScale((float)w / m_sprite.getTextureRect().width, (float)h / m_sprite.getTextureRect().height);
    }
}

void CLabel::setSprite(const sf::Sprite& sprite)
{
    m_sprite = sprite;
}

void CLabel::setFontColor(const sf::Color& color)
{
    m_text.setFillColor(color);
}

void CLabel::setFontSize(int size)
{
    m_text.setCharacterSize(size);
}

void CLabel::setFontName(const sf::Font& font)
{
    m_text.setFont(font);
}

void CLabel::setFontStyle(sf::Uint32 style)
{
    m_text.setStyle(style);
}

void CLabel::setTextAlign(int value)
{
    m_text_align = value;
}

void CLabel::setString(const std::string& str)
{
    m_text.setString(str);
}

void CLabel::setOutlineColor(const sf::Color& color)
{
    m_shape.setOutlineColor(color);
}

void CLabel::setFillColor(const sf::Color& color)
{
    m_shape.setFillColor(color);
}

void CLabel::setOutlineThickness(int value)
{
    m_shape.setOutlineThickness(value);
}

bool CLabel::contains(const Vector& point) const
{
    return m_rect.isContain(point);
}

void CLabel::draw(sf::RenderWindow* window)
{
    window->draw(m_shape);

    if (m_sprite.getTexture())
    {
        m_sprite.setPosition(getPosition());
        window->draw(m_sprite);
    }

    if (!m_text.getString().isEmpty())
    {
        if (m_text_align == center)
            m_text.setPosition(getPosition() + m_rect.size() / 2 - Vector(m_text.getGlobalBounds().width, m_text.getGlobalBounds().height) / 2);
        else if (m_text_align == left)
            m_text.setPosition(getPosition().x, getPosition().y);


        window->draw(m_text);
    }
}

void CLabel::onPropertySet(const std::string& name)
{
	CGameObject::onPropertySet(name);
	if (name == "text")
		setString(getProperty("text").asString());
}

void CLabel::onActivated()
{
	if (getProperty("x").isValid())
	{
		setBounds(
			getProperty("x").asFloat(),
			getProperty("y").asFloat(),
			getProperty("width").asFloat(),
			getProperty("height").asFloat());
			setString(getProperty("text").asString());
	}
	if (getProperty("hided").isValid() && getProperty("hided").asBool())
			hide();
}

Rect CLabel::getBounds() const
{
    return m_rect;
}

sf::Sprite& CLabel::getSprite()
{
    return m_sprite;
}

CLabel* CLabel::clone() const
{
    CLabel* new_label = new CLabel();

    new_label->m_text = m_text;
    new_label->m_sprite = m_sprite;
    new_label->m_rect = m_rect;
    new_label->m_shape = m_shape;
    new_label->m_text_align = m_text_align;
    Rect bounds = getBounds();
    new_label->setBounds(bounds.left(), bounds.top(), bounds.width(), bounds.height());

    return new_label;
}
//-------------------------------------------------------------------------------------------------------

Vector  collsionResponse(const Rect& own_rect, const Vector& own_speed, const Rect& other_rect, const Vector& other_speed, const float delta_time, ECollisionTag& collision_tag)
{
    Vector new_pos = own_rect.leftTop();

    Rect intersection = other_rect.getIntersection(own_rect);
    Vector delta_speed = own_speed - other_speed;

    enum Axis {none,vertical,horizontal} axis = none;

    if (!intersection.width() || !intersection.height())
        return new_pos;

    float dt = 0;
    if (delta_speed.x && delta_speed.y)
    {
        float dx_t = intersection.width() / std::abs(delta_speed.x);
        float dy_t = intersection.height() / std::abs(delta_speed.y);
        dt = std::min(dx_t,dy_t);
        axis = (dx_t > dy_t) ? vertical : horizontal;
    }
    else if (delta_speed.x)
    {
        dt = intersection.width() / std::abs(delta_speed.x);
        axis = horizontal;
    }
    else if (delta_speed.y)
    {
        dt = intersection.height() / std::abs(delta_speed.y);
        axis = vertical;
    }

    if (dt > delta_time)
        dt = delta_time;

    new_pos -=  dt*delta_speed;

    if (axis == vertical)
    {
        if (intersection.top() == other_rect.top())
            collision_tag |= ECollisionTag::floor;
        else  if (intersection.bottom() == other_rect.bottom())
            collision_tag |= ECollisionTag::cell;

    }
    else if (axis == horizontal)
    {
        if (intersection.left() == other_rect.left())
            collision_tag |= ECollisionTag::right;
        else if (intersection.right() == other_rect.right())
            collision_tag |= ECollisionTag::left;
    }

    return new_pos;
}

