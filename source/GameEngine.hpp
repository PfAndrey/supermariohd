#ifndef GAME_ENGINE_HPP
#define GAME_ENGINE_HPP

#include <assert.h>
#include <memory>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <Collisions.hpp>
#include <InputManager.hpp>
#include <GameObject.hpp>
#include <Rect.hpp>
#include <ResourceManager.hpp>
#include <RTIIX.hpp>
#include <TimerManager.hpp>
#include <Vector.hpp>
#include "TileMap.hpp"

#include <iostream>

namespace math {
    int sign(float value);

    template <typename T>
    T clamp(const T& value, const T& min, const T& max) {
        if (value < min)
            return min;
        if (value > max)
            return max;
        return value;
    }
}

class EventManager {
public:
    void pushEvent(const sf::Event& event);
    void subscribe(GameObject* object);
    void unsubcribe(GameObject* object);

private:
    std::list<GameObject*> m_subcribes;
};

using TextureManager = ResourceManager<sf::Texture>;
using FontManager = ResourceManager<sf::Font>;
using SoundManager = ResourceManager<sf::SoundBuffer>;


template <>
inline bool ResourceManager<sf::Music>::loadFromFile(const std::string& name, const std::string& file_path) {
    assert(m_resources[name] == nullptr); // allready exist
    m_resources[name] = new sf::Music();
    if (!m_resources[name]->openFromFile(file_path)) {
        std::cerr << "can't load resource from file: " << file_path << std::endl;
        return false;
    }
    m_resources[name]->setLooping(true);
    return true;
}

template <>
inline bool ResourceManager<sf::Font>::loadFromFile(const std::string& name, const std::string& file_path) {
    auto it = m_resources.find(name);
    if (it != m_resources.end())
    {
        LOG("RES_MNG", ERROR, "Resource with name %s already exists", name.c_str());
        return false;
    }

    auto resource = new sf::Font();
    if (!resource->openFromFile(file_path))
    {
        LOG("RES_MNG", ERROR, "Failed to load %s", name.c_str());
        return false;
    }

    LOG("RES_MNG", DEBUG, "Loaded resource '%s' from file %s", name.c_str(), file_path.c_str());

    m_resources[name] = resource;
    return true;
}

class MusicManager : public ResourceManager<sf::Music> {
public:
    void play(const std::string& name = "");
    void stop();
    void pause();
    void setPitch(float value);

private:
    std::string m_current_music;
};

class Game {
public:
    Game(const std::string& name, const Vector& screen_size);
    virtual ~Game() = default;
    void run();
    GameObject* getRootObject();
    TextureManager& textureManager();
    FontManager& fontManager();
    SoundManager& soundManager();
    EventManager& eventManager();
    InputManager& inputManager();
    MusicManager& musicManager();
    void playSound(const std::string& name);
    void playMusic(const std::string& name);
    void stopMusic();
    Vector screenSize() const;

protected:
    virtual void init();
    virtual void update(int delta_time);
    void setClearColor(const sf::Color& color);

private:
    GameObject* m_root_object = nullptr;
    TextureManager m_texture_manager;
    FontManager m_font_manager;
    SoundManager m_sound_manager;
    MusicManager m_music_manager;
    EventManager m_event_manager;
    InputManager m_input_manager;
    std::vector<std::unique_ptr<sf::Sound>> m_activeSounds;
    int m_activeSoundSlot = 0;

    std::unique_ptr<sf::RenderWindow> m_window;

    Vector m_screen_size;
    sf::Color m_clear_color = sf::Color::Black;
    void draw(sf::RenderWindow* render_window);
    void updateStats(const sf::Time time);
    sf::Time m_min_time = sf::seconds(3600);
    sf::Time m_max_time = sf::Time::Zero;
};

enum class AnimType : uint8_t {
    MANUAL                 = 0,
    FORWARD                = 1,
    FORWARD_STOP           = 2,
    FORWARD_CYCLE          = 3,
    FORWARD_BACKWARD_CYCLE = 4
};

enum class RepeatMode : uint8_t {
    CLAMP  = 0,
    REPEAT = 1,
};

class SpriteSheet {
public:
    SpriteSheet();
    SpriteSheet(const sf::Texture& texture, const std::vector<sf::IntRect>& rects);
    void load(const sf::Texture& texture, const std::vector<sf::IntRect>& rects);
    void load(const sf::Texture& texture, const Vector& off_set, const Vector& size, int cols, int rows, float speed = 0.f);
    void setAnimType(AnimType type);
    void reset();
    void setPosition(const Vector& pos);
    Vector getPosition() const;
    void setSpeed(float speed);
    void setColor(const sf::Color& color);
    void scale(float fX, float fY);
    void flipX(bool isFliped);
    bool empty() const;
    void setOrigin(const Vector& vector);
    void setOrigin(const Vector& pos, int spriteId);

    void draw(sf::RenderWindow* wnd);
    void draw(sf::RenderWindow* wnd, int spriteIndex);
    void draw(sf::RenderWindow* wnd, int spriteIndex, const Rect& draw_area, RepeatMode mode = RepeatMode::CLAMP);
    void update(int delta_time);
    void setSpriteIndex(int index);

private:
    sf::Sprite* currentSprite();

    AnimType m_anim_type;
    bool m_flipped = false;
    std::vector<sf::Sprite> m_sprites;
    sf::Sprite* m_current_sprite = nullptr;
    float m_speed = 0.f;
    sf::Vector2f m_position;
    sf::Vector2f m_torigin;
    float m_index = 0;
};

class Pallete {
public:
    Pallete();
    void create(const std::initializer_list<sf::Color>& original_colors,
                const std::initializer_list<sf::Color>& swaped_colors);
    void apply();
    void cancel();

private:
    sf::Shader m_shader;
    int m_old_shader = 0;
};

class Animator : public GameObject {
public:
    ~Animator();
    void create(const std::string& name, const sf::Texture& texture, const Vector& off_set, const Vector& size,
                int cols, int rows, float speed, AnimType anim_type = AnimType::FORWARD_CYCLE);
    void create(const std::string& name, const sf::Texture& texture, const Rect& rect);
    void create(const std::string& name, const sf::Texture& texture, const std::vector<Rect>& rects, float speed);
    void play(const std::string& name);
    void update(int delta_time) override;
    void draw(sf::RenderWindow* wnd) override;
    void flipX(bool value);
    void setColor(const sf::Color& color);
    void setSpeed(const std::string& animation, float speed);
    void setSpriteOffset(const std::string& anim_name, int sprite_index, const Vector& value);
    void setPallete(Pallete* pallete);
    void scale(float fX, float fY);
    void setOrigin(const std::string& animName, const Vector& diff);

private:
    Pallete* m_pallete = nullptr;
    std::unordered_map<std::string, SpriteSheet*> m_animations;
    SpriteSheet* m_current_animation = nullptr;
    std::string last_anim_name;
    bool m_flipped = false;
};

class FlowText : public GameObject {
public:
    FlowText(const sf::Font& font, bool self_remove = false);
    void setTextColor(const sf::Color& color);
    void setTextSize(int size);
    void splash(const Vector& pos, const std::string& text);
    void update(int delta_time) override;
    void draw(sf::RenderWindow* window) override;
    bool isFlashing() const;
    void setSplashVector(const Vector& vector);
    FlowText* clone() const;

private:
    bool m_self_remove = false;
    sf::Text m_text;
    Vector m_offset;
    Vector m_splash_vector = {1,-1};
    float m_time;
    int m_color;
    bool m_flashing;
};

class Label : public GameObject {
public:
    enum { left, center };
    Label();
    Label(const sf::Sprite& sprite);
    void setSprite(const sf::Sprite& sprite);
    void setString(const std::string& str);
    void setTextAlign(int value);
    void setFontColor(const sf::Color& color);
    void setFontSize(int size);
    void setOutlineColor(const sf::Color& color);
    void setFillColor(const sf::Color& color);
    void setOutlineThickness(int value);
    void setBounds(int x, int y, int w, int h);
    Rect getBounds() const override;
    void setFontName(const sf::Font& font);
    void setFontStyle(uint32_t style);
    bool contains(const Vector& point) const;
    Label* clone() const;
    sf::Sprite& getSprite();
    void draw(sf::RenderWindow* window) override;

protected:
    void onPropertySet(const std::string& name) override;
    void onStarted() override;
    sf::RectangleShape m_shape;

private:
    void init();
    int m_text_align = center;
    std::unique_ptr<sf::Sprite> m_sprite;
    std::unique_ptr<sf::Text> m_text;
    Rect m_rect;
};

#endif // GAME_ENGINE_HPP
