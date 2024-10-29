#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP

#include <string>
#include "Vector.hpp"


class InputManager {
public:
    InputManager();
    Vector getXYAxis() const;
    bool isButtonPressed(const std::string& button) const;
    bool isButtonDown(const std::string& button) const;
    bool isButtonUp(const std::string& button) const;
    void setupButton(const std::string& button, const std::vector<std::string>& keys);
    void update(int delta_time);

private:

    template <typename T>
    T applyDeadzone(const T& value, const T& limit) const;

    bool isKeyJustPressed(const sf::Keyboard::Key& key) const;
    bool isKeyJustReleased(const sf::Keyboard::Key& key) const;
    bool isKeyPressed(const sf::Keyboard::Key& key) const;
    bool isJoystickButtonPressed(int index) const;
    bool isJoystickButtonJustPressed(int index) const;
    bool isJoystickButtonJustReleased(int index) const;
    void registerKey(const sf::Keyboard::Key& key);
    void unregisterKey(const sf::Keyboard::Key& key);
    void registerJoysticButton(int index);
    sf::Keyboard::Key toKey(const std::string& str);

    std::unordered_map<sf::Keyboard::Key, bool> m_keys_prev, *m_keys_prev_ptr, m_keys_now, *m_keys_now_ptr;
    std::unordered_map<int, bool> m_jsk_btns_prev, *m_jsk_btns_prev_ptr, m_jsk_btns_now, *m_jsk_btns_now_ptr;
    sf::Keyboard::Key m_axis_keys[4];
    std::unordered_map<std::string, sf::Keyboard::Key> m_btn_to_key;
    std::unordered_map<std::string, int> m_jsk_btn_to_key;
};

#endif // !INPUT_MANAGER_HPP
