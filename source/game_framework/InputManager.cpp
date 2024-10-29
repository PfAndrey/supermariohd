#include "InputManager.hpp"
#include <Format.hpp>

InputManager::InputManager() {
    m_keys_prev_ptr = &m_keys_prev;
    m_keys_now_ptr = &m_keys_now;
    m_jsk_btns_prev_ptr = &m_jsk_btns_prev;
    m_jsk_btns_now_ptr = &m_jsk_btns_now;

    for (auto& ax : m_axis_keys) {
        ax = sf::Keyboard::Key::Unknown;
    }
}

void InputManager::registerKey(const sf::Keyboard::Key& key) {
    m_keys_prev.insert(std::make_pair(key, false));
    m_keys_now.insert(std::make_pair(key, false));
}

void InputManager::unregisterKey(const sf::Keyboard::Key& key) {
    m_keys_prev.erase(m_keys_prev.find(key));
    m_keys_now.erase(m_keys_now.find(key));
}

void InputManager::registerJoysticButton(int index) {
    m_jsk_btns_prev.insert(std::make_pair(index, false));
    m_jsk_btns_now.insert(std::make_pair(index, false));
}

bool InputManager::isKeyJustPressed(const sf::Keyboard::Key& key) const {
    return m_keys_prev_ptr->count(key) &&
        !(*m_keys_prev_ptr)[key] &&
        (*m_keys_now_ptr)[key];
}

bool InputManager::isKeyJustReleased(const sf::Keyboard::Key& key) const {
    return (m_keys_prev_ptr->count(key)) &&
        (*m_keys_prev_ptr)[key] &&
        !(*m_keys_now_ptr)[key];
}

bool InputManager::isKeyPressed(const sf::Keyboard::Key& key) const {
    return (m_keys_now_ptr->count(key) &&
        (*m_keys_now_ptr)[key]);
}

Vector InputManager::getXYAxis() const
{
    Vector value;
    if (sf::Joystick::isConnected(0)) {
        value.x = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) / 100.f;
        value.y = -sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) / 100.f;
        value.x = applyDeadzone(value.x, 0.5f);
        value.y = applyDeadzone(value.y, 0.5f);
    }

    if ((m_axis_keys[0] != -1) && sf::Keyboard::isKeyPressed(m_axis_keys[0])) value.y = -1;
    if ((m_axis_keys[1] != -1) && sf::Keyboard::isKeyPressed(m_axis_keys[1])) value.x = 1;
    if ((m_axis_keys[2] != -1) && sf::Keyboard::isKeyPressed(m_axis_keys[2])) value.y = 1;
    if ((m_axis_keys[3] != -1) && sf::Keyboard::isKeyPressed(m_axis_keys[3])) value.x = -1;

    return value;
}

bool InputManager::isButtonPressed(const std::string& button) const {
    return (m_btn_to_key.count(button) && isKeyPressed(m_btn_to_key.at(button))) ||               // keyboard
        (m_jsk_btn_to_key.count(button) && isJoystickButtonPressed(m_jsk_btn_to_key.at(button))); // joystick
}

bool InputManager::isButtonDown(const std::string& button) const {
    return (m_btn_to_key.count(button) && isKeyJustPressed(m_btn_to_key.at(button))) ||
        (m_jsk_btn_to_key.count(button) && isJoystickButtonJustPressed(m_jsk_btn_to_key.at(button)));
}

bool InputManager::isButtonUp(const std::string& button) const {
    return (m_btn_to_key.count(button) && isKeyJustReleased(m_btn_to_key.at(button))) ||
        (m_jsk_btn_to_key.count(button) && isJoystickButtonJustPressed(m_jsk_btn_to_key.at(button)));
}

sf::Keyboard::Key InputManager::toKey(const std::string& str) {
    if ((str.length() == 1) && (str[0] >= 'A') && (str[0] <= 'Z')) {
        return static_cast<sf::Keyboard::Key>(str[0] - 65);
    }

    using utils::strHash;

    switch (strHash(str.c_str())) {
    case strHash("Left"):
        return sf::Keyboard::Left;
    case strHash("Right"):
        return sf::Keyboard::Right;
    case strHash("Up"):
        return sf::Keyboard::Up;
    case strHash("Down"):
        return sf::Keyboard::Down;
    case strHash("Space"):
        return sf::Keyboard::Space;
    case strHash("LShift"):
        return sf::Keyboard::LShift;
    case strHash("Enter"):
        return sf::Keyboard::Enter;
    case strHash("Return"):
        return sf::Keyboard::Return;
    default:
        return sf::Keyboard::Unknown;
        break;
    }
}

void InputManager::setupButton(const std::string& button, const std::vector<std::string>& keys) {
    static const std::unordered_map<std::string, int> special_keys = {
        { "Vertical-",   0 },
        { "Horizontal+", 1 },
        { "Vertical+",   2 },
        { "Horizontal-", 3 }
    };

    if (special_keys.count(button)) {
        m_axis_keys[special_keys.at(button)] = toKey(keys[0]);
        return;
    }

    for (auto key : keys) {
        if ((key.front() == '[') && (key.back() == ']'))  { //joystick btn
            int index = utils::toInt(key.substr(1, key.length() - 2));
            registerJoysticButton(index);
            m_jsk_btn_to_key[button] = index;
        } else {
            sf::Keyboard::Key pkey = toKey(key);
            registerKey(pkey);
            m_btn_to_key[button] = pkey;
        }
    }
}

void InputManager::update(int delta_time) {
    std::swap(m_keys_now_ptr, m_keys_prev_ptr);
    for (auto& key : *m_keys_now_ptr) {
        key.second = sf::Keyboard::isKeyPressed(key.first);
    }

    std::swap(m_jsk_btns_now, m_jsk_btns_prev);
    for (auto& btn : *m_jsk_btns_now_ptr) {
        btn.second = sf::Joystick::isButtonPressed(0, btn.first);
    }
}

bool InputManager::isJoystickButtonPressed(int index) const {
    return m_jsk_btns_now_ptr->count(index) &&
        (*m_jsk_btns_now_ptr)[index];
}

bool InputManager::isJoystickButtonJustPressed(int index) const {
    return (m_jsk_btns_prev_ptr->count(index) &&
        !(*m_jsk_btns_prev_ptr)[index] &&
        (*m_jsk_btns_now_ptr)[index]);
}

bool InputManager::isJoystickButtonJustReleased(int index) const {
    return (m_jsk_btns_prev_ptr->count(index) &&
        (*m_jsk_btns_prev_ptr)[index] &&
        !(*m_jsk_btns_now_ptr)[index]);
}

template <typename T>
T InputManager::applyDeadzone(const T& value, const T& limit) const {
    if (abs(value) <= abs(limit)) {
        return 0;
    }

    return value;
}
