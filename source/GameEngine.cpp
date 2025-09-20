#include <assert.h>
#include <iostream>
#include <future>

#include <Format.hpp>
#include "GameEngine.hpp"
#include <Logger.hpp>

namespace math {

int sign(float value) {
    return ((value > 0) - (value < 0));
}

} // namespace math



void EventManager::pushEvent(const sf::Event& event) {
    for (auto& sub : m_subcribes) {
        sub->events(event);
    }
}

void EventManager::subscribe(GameObject* object) {
    m_subcribes.push_back(object);
}

void EventManager::unsubcribe(GameObject* object) {
    auto it = std::find(m_subcribes.begin(), m_subcribes.end(), object);
    if (it != m_subcribes.end()) {
        m_subcribes.erase(it);
    }
}
//---------------------------------------------------------------------------
//! Game
//---------------------------------------------------------------------------
void Game::init() {
}

Game::Game(const std::string& name, const Vector& screen_size) {
    m_root_object = new GameObject();
    m_root_object->setName(name);
    m_screen_size = screen_size;
}

void Game::updateStats(const sf::Time time) {
    if (time > m_max_time) {
        m_max_time = time;
    }

    if (time < m_min_time) {
        m_min_time = time;
    }

    static int k = 0;

    if (!(++k % 60)) {
        std::cout << "min: " << m_min_time.asMilliseconds()
                  << " max: " << m_max_time.asMilliseconds() << std::endl;
        m_min_time = sf::seconds(3600);
        m_max_time = sf::Time::Zero;
    }
}

void Game::setClearColor(const sf::Color& color) {
    m_clear_color = color;
}

void Game::run() {
    m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode({(unsigned)m_screen_size.x, (unsigned)m_screen_size.y}), "title");
    init();

    sf::Clock clock;
    sf::Time acumulator = sf::Time::Zero;
    const sf::Time ups = sf::seconds(1.f / 60.f);
    //m_window->setFramerateLimit(60);
    //m_window->setVerticalSyncEnabled(true);

    m_root_object->start();

    while (m_window->isOpen()) {
        // game loop
        // pull mouse, keyboard events
        while (const std::optional event = m_window->pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                m_window->close();
                exit(0);
            }
            if (event.has_value())
                eventManager().pushEvent(event.value());
        }

        // update tick
        sf::Time elapsedTime = clock.restart();
        acumulator += elapsedTime;
        //updateStats(elapsedTime);

        while (acumulator > ups) {
            acumulator -= ups;
            inputManager().update(ups.asMilliseconds());
            update(ups.asMilliseconds());
            sf::sleep(sf::milliseconds(10));
        }

        // draw
        m_window->clear(m_clear_color);
        draw(m_window.get());
        m_window->display();
    }
}

GameObject* Game::getRootObject() {
    return m_root_object;
}

void Game::draw(sf::RenderWindow* render_window) {
    m_root_object->draw(render_window);
}

void Game::update(int delta_time) {
    GameObject::invokePreupdateActions(); // remove obj, change z-oreder, etc
    m_root_object->update(delta_time);
}

TextureManager& Game::textureManager() {
    return m_texture_manager;
}

FontManager& Game::fontManager() {
    return m_font_manager;
}

SoundManager& Game::soundManager() {
    return m_sound_manager;
}

EventManager& Game::eventManager() {
    return m_event_manager;
}

InputManager& Game::inputManager() {
    return m_input_manager;
}

MusicManager& Game::musicManager() {
    return m_music_manager;
}

void Game::playMusic(const std::string& name) {
    m_music_manager.play(name);
}

void Game::stopMusic() {
    m_music_manager.stop();
}

void Game::playSound(const std::string& name) {
    auto& soundBuff = *soundManager().get(name);

    // remove older sounds
    m_activeSounds.erase(
        std::remove_if(m_activeSounds.begin(), m_activeSounds.end(),
            [](const sf::Sound& s) { return s.getStatus() == sf::Sound::Status::Stopped; }),
        m_activeSounds.end()
    );

    m_activeSounds.emplace_back(soundBuff);
    m_activeSounds.back().play();
}

Vector Game::screenSize() const {
    return Vector((int)m_window->getSize().x, (int)m_window->getSize().y);
}

SpriteSheet::SpriteSheet() {
    m_speed = 0.03f;
    m_current_sprite = nullptr;
    setAnimType(AnimType::MANUAL);
    m_index = 0;
}

SpriteSheet::SpriteSheet(const sf::Texture& texture, const std::vector<sf::IntRect>& rects) {
    load(texture, rects);
}

void SpriteSheet::load(const sf::Texture& texture, const std::vector<sf::IntRect>& rects) {
    m_sprites.clear();
    for (auto& rect : rects) {
        m_sprites.emplace_back(texture, rect);
    }

    setSpriteIndex(0);
}

void SpriteSheet::load(const sf::Texture& texture, const Vector& off_set, const Vector& size, int cols, int rows, float speed) {
    m_sprites.clear();
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x){
            m_sprites.emplace_back(texture, sf::IntRect({(int)(x * std::abs(size.x) + off_set.x),
                                                         (int)(y * std::abs(size.y) + off_set.y)},
                                                        {(int)size.x, (int)size.y}));
        }
    }

    setSpriteIndex(0);

    if (speed) {
        setSpeed(speed);
        setAnimType(AnimType::FORWARD_CYCLE);
    } else {
        setAnimType(AnimType::MANUAL);
    }
}

void SpriteSheet::reset() {
    m_index = 0;
}

bool SpriteSheet::empty() const {
    return m_sprites.empty();
}

void SpriteSheet::setSpriteIndex(int index) {
    assert(index >= 0 && index < m_sprites.size());
    m_current_sprite = &m_sprites[index];
}

sf::Sprite* SpriteSheet::currentSprite() {
    return m_current_sprite;
}

void SpriteSheet::setAnimType(AnimType type) {
    m_anim_type = type;
    m_index = 0;
}

void SpriteSheet::draw(sf::RenderWindow* wnd, int spriteIndex) {
    auto& sprite = m_sprites[spriteIndex];
    sprite.setPosition(m_position);
    wnd->draw(sprite);
}

void SpriteSheet::draw(sf::RenderWindow* wnd, int spriteIndex, const Rect& _draw_area, RepeatMode mode) {
    auto sprite = m_sprites[spriteIndex];
    sprite.setPosition(m_position + m_torigin);

    const sf::Rect<int> draw_area({ (int)_draw_area.left(), (int)_draw_area.top()},
                                  { (int)_draw_area.width(), (int)_draw_area.height()});

    if (!draw_area.size.x || !draw_area.size.y) {
        return;
    }

    Vector size = { (float)abs(sprite.getTextureRect().size.x), (float)abs(sprite.getTextureRect().size.y) };

    int c_w = draw_area.size.x / size.x;
    int c_h = draw_area.size.y / size.y;

    int rem_w = draw_area.size.x % (int)size.x;
    int rem_h = draw_area.size.y % (int)size.y;

    for (int y = 0; y < c_h; ++y) {
        for (int x = 0; x < c_w; ++x) {
            sprite.setPosition({x * size.x + draw_area.position.x, y * size.y + draw_area.position.y});
            wnd->draw(sprite);
        }
    }

    if (rem_h) {
        auto new_rect = sprite.getTextureRect();
        auto old_height = new_rect.size.y;
        new_rect.size.y = rem_h;
        sprite.setTextureRect(new_rect);

        for (int x = 0; x < c_w; ++x) {
            sprite.setPosition({x * size.x + draw_area.position.x, c_h * size.y + draw_area.position.y});
            wnd->draw(sprite);
        }

        new_rect.size.y = old_height;
        sprite.setTextureRect(new_rect);
    }

    if (rem_w) {
        auto new_rect = sprite.getTextureRect();
        new_rect.size.x = rem_w;
        sprite.setTextureRect(new_rect);

        for (int y = 0; y < c_h; ++y) {
            sprite.setPosition({c_w * size.x + draw_area.position.x, y * size.y + draw_area.position.y});
            wnd->draw(sprite);
        }
    }
}

void SpriteSheet::draw(sf::RenderWindow* wnd) {
    m_current_sprite->setPosition(m_position);
    wnd->draw(*m_current_sprite);
}

void SpriteSheet::setPosition(const Vector& pos) {
    m_position = pos;
}

void SpriteSheet::setOrigin(const Vector& pos) {
    m_torigin = pos;
    for (auto& sprite : m_sprites) {
        sprite.setOrigin(pos);
    }
}

void SpriteSheet::setOrigin(const Vector& pos, int spriteId) {
    if (spriteId < 0 || spriteId >= m_sprites.size()) {
        return;
    }

    m_sprites[spriteId].setOrigin(pos);
}
//---------------------------------------------------------------------------
Vector SpriteSheet::getPosition() const
{
    return m_position;
}

void SpriteSheet::scale(float fX, float fY) {
    for (auto& sprite : m_sprites) {
        sprite.scale({fX, fY});
    }
}

void SpriteSheet::update(int delta_time) {
    switch (m_anim_type)
    {
    case AnimType::MANUAL:
        break;
    case AnimType::FORWARD_BACKWARD_CYCLE:
    {
        int size = m_sprites.size();
        int current_slide = int(m_index) % (size * 2);
        if (current_slide > size - 1) {
            current_slide = 2 * size - 1 - current_slide;
        }
        setSpriteIndex(current_slide);
        break;
    }
    case AnimType::FORWARD_CYCLE:
    {
        int current_slide = int(m_index) % m_sprites.size();
        setSpriteIndex(current_slide);
        break;
    }
    case AnimType::FORWARD_STOP:
    {
        int current_slide = int(m_index);
        if (current_slide < m_sprites.size()) {
            setSpriteIndex(current_slide);
        }
        break;
    }
    case AnimType::FORWARD:
    {
        int current_slide = int(m_index);
        if (current_slide < m_sprites.size()) {
            setSpriteIndex(current_slide);
        } else {
            return;
        }
        break;
    }
    }

    if (m_speed) {
        m_index += m_speed * delta_time;
    }
}

void SpriteSheet::setSpeed(float speed) {
    m_speed = speed;
}

void SpriteSheet::setColor(const sf::Color& color) {
    for (auto& sprite : m_sprites) {
        sprite.setColor(color);
    }
}

void SpriteSheet::flipX(bool value) {
    if (m_flipped != value) {
        m_flipped = value;
        for (auto& sprite : m_sprites) {
            auto rect = sprite.getTextureRect();
            rect.position.x += rect.size.x;
            rect.size.x = -rect.size.x;
            sprite.setTextureRect(rect);
        }
    }
}
//---------------------------------------------------------------------------
//! Pallete
//---------------------------------------------------------------------------
Pallete::Pallete() {
}

void Pallete::create(const std::initializer_list<sf::Color>& original_colors, const std::initializer_list<sf::Color>& swaped_colors) {
    assert(original_colors.size() == swaped_colors.size());
    int arr_size = original_colors.size();
    const std::string frag_shader =
            "#version 120\n"\
            "const int arr_size = " + utils::toString(arr_size) + ";"\
            "uniform vec3 color1[arr_size];"\
            "uniform vec3 color2[arr_size];"\
            "uniform sampler2D texture;"\
            "void main()"\
            "{"\
            "   vec4 myindex = texture2D(texture, gl_TexCoord[0].xy);"\
            "   vec3 index = vec3(myindex.r,myindex.g,myindex.b);"\
            "   gl_FragColor = myindex;"\
            "   for (int i=0; i < arr_size; ++i)"\
            "       if (index == color1[i]) {"\
            "           gl_FragColor = vec4(color2[i].r,color2[i].g,color2[i].b,myindex.a);"\
            "           break; }"\
            "}";

    m_shader.loadFromMemory(frag_shader, sf::Shader::Type::Fragment);

    std::vector<sf::Glsl::Vec3> colors1(arr_size);
    std::vector<sf::Glsl::Vec3> colors2(arr_size);

    for (int i = 0; i < arr_size; ++i) {
        auto& original_color = original_colors.begin()[i];
        auto& swaped_color = swaped_colors.begin()[i];

        colors1[i] = sf::Glsl::Vec3(original_color.r , original_color.g, original_color.b) / 255.f;
        colors2[i] = sf::Glsl::Vec3(swaped_color.r, swaped_color.g, swaped_color.b) / 255.f;
    }

    m_shader.setUniformArray("color1", colors1.data(), arr_size);
    m_shader.setUniformArray("color2", colors2.data(), arr_size);
    m_shader.setUniform("texture", sf::Shader::CurrentTexture);
}

void Pallete::apply() {
    sf::Shader::bind(&m_shader);
}

void Pallete::cancel() {
    sf::Shader::bind(nullptr);
}
//---------------------------------------------------------------------------
//! Animator
//---------------------------------------------------------------------------
Animator::~Animator() {
    for (auto& anim : m_animations) {
        delete anim.second;
    }
}

void Animator::create(const std::string& name, const sf::Texture& texture, const Vector& off_set,
                      const Vector& size, int cols, int rows, float speed, AnimType anim_type) {
    SpriteSheet* animation = new SpriteSheet();
    animation->load(texture, off_set, size, cols, rows);
    animation->setAnimType(anim_type);
    animation->setSpeed(speed);
    assert(!m_animations[name]); // already exist
    m_animations[name] = animation;

    if (!m_current_animation) {
        m_current_animation = animation;
    }
}

void Animator::create(const std::string& name, const sf::Texture& texture, const Rect& rect) {
    if (m_animations.find(name) != m_animations.end()) {
        LOG("Animator", ERROR, "animation already exist: " + name);
        return;
    }

    SpriteSheet* animation = new SpriteSheet();
    animation->load(texture, { { {(int)rect.left(), (int)rect.top()}, {(int)rect.width(), (int)rect.height()} } });
    m_animations[name] = animation;

    if (!m_current_animation) {
        m_current_animation = animation;
    }
}

void Animator::create(const std::string& name, const sf::Texture& texture, const std::vector<Rect>& rects, float speed) {
    if (m_animations.find(name) != m_animations.end()) {
        LOG("Animator", ERROR, "animation already exist: " + name);
        return;
    }

    std::vector<sf::IntRect> intRects(rects.size());
    for (size_t i = 0; i < intRects.size(); ++i) {
        auto& intRect = intRects[i];
        intRect.position.x = rects[i].left();
        intRect.position.y = rects[i].top();
        intRect.size.x = rects[i].width();
        intRect.size.y = rects[i].height();
    }

    SpriteSheet* animation = new SpriteSheet();
    animation->load(texture, intRects);
    animation->setAnimType(AnimType::FORWARD_CYCLE);
    animation->setSpeed(speed);

    m_animations[name] = animation;
 
    if (!m_current_animation) {
        m_current_animation = animation;
    }
}

void Animator::play(const std::string& name) {
    if (last_anim_name != name) {
        m_current_animation = m_animations[name];
        assert(m_current_animation); //not exist
        last_anim_name = name;
        m_current_animation->reset();
    }
}

void Animator::update(int delta_time) {
    if (isEnabled()) {
        m_current_animation->update(delta_time);
    }
}

void Animator::draw(sf::RenderWindow* wnd) {
    if (isVisible()) {
        if (m_pallete) {
            m_pallete->apply();
        }

        m_current_animation->setPosition(getPosition());
        m_current_animation->draw(wnd);

        if (m_pallete) {
            m_pallete->cancel();
        }
    }
}

void Animator::setPallete(Pallete* pallete) {
    m_pallete = pallete;
}

void Animator::flipX(bool value) {
    if (value != m_flipped) {
        m_flipped = value;
        for (auto& animation : m_animations) {
            animation.second->flipX(value);
        }
    }
}

void Animator::setColor(const sf::Color& color) {
    for (auto& animation : m_animations) {
        animation.second->setColor(color);
    }
}

void Animator::setSpeed(const std::string& anim, float speed) {
    m_animations[anim]->setSpeed(speed);
}

void Animator::setSpriteOffset(const std::string& anim_name, int sprite_index, const Vector& value) {
    SpriteSheet* sheet = m_animations[anim_name];
    assert(sheet);
    sheet->setOrigin(-value, sprite_index);
}

void Animator::scale(float fX, float fY) {
    for (auto& animation : m_animations) {
        animation.second->scale(fX, fY);
    }
}

void Animator::setOrigin(const std::string& animName, const Vector& diff) {
    auto it = m_animations.find(animName);
    if (it != m_animations.end()) {
        it->second->setOrigin(diff);
    }
}

//SpriteSheet* Animator::get(const std::string& str) {
//    return m_animations[str];
//}
//---------------------------------------------------------------------------
//! MusicManager
//---------------------------------------------------------------------------
void MusicManager::play(const std::string& name) {
    if (!name.empty()) {
        m_current_music = name;
    }

    auto it = m_resources.find(m_current_music);

    if (it != m_resources.end()) {
        it->second->play();
    } else {
        LOG("MusicManager", ERROR, "music not found: " + m_current_music);
    }
}

void MusicManager::stop() {
    for (auto& music : m_resources) {
        music.second->stop();
    }
    m_current_music.clear();
}

void MusicManager::pause() {
    for (auto& music : m_resources) {
        music.second->pause();
    }
}

void MusicManager::setPitch(float value) {
    for (auto music : m_resources) {
        music.second->setPitch(value);
    }
}
//---------------------------------------------------------------------------
//! FlowText
//---------------------------------------------------------------------------
FlowText::FlowText(const sf::Font& font, bool self_remove) 
    : m_text(font, "")
{
    m_text.setFillColor(sf::Color::Black);
    m_text.setCharacterSize(20);
    m_text.setStyle(sf::Text::Bold);
    m_flashing = false;
    m_self_remove = self_remove;
}

FlowText* FlowText::clone() const {
    const sf::Font& font = m_text.getFont();
    auto flow_text = new FlowText(font);
    flow_text->m_text =  m_text;
    flow_text->m_splash_vector = m_splash_vector;
    return flow_text;
}

bool FlowText::isFlashing() const {
    return m_flashing;
}

void FlowText::splash(const Vector& pos, const std::string& text) {
    m_flashing = true;
    setPosition(pos);
    m_text.setString(text);
    m_offset.x = m_offset.y = 0;
    m_time = 0;
}

void FlowText::setSplashVector(const Vector& vector) {
    m_splash_vector = vector;
}

void FlowText::update(int delta_time) {
    if (m_flashing) {
        m_time += delta_time;
        m_offset.x = m_time*0.03f*m_splash_vector.x;
        m_offset.y = m_time*0.03f*m_splash_vector.y;

        m_offset.x *= 2;
        m_color = (int)m_time*0.2f;
        if (m_color >= 255)
            m_flashing = false;

        const sf::Color& color = m_text.getFillColor();

        m_text.setFillColor(sf::Color(color.r, color.g, color.b, 255 - m_color));
    } else if (m_self_remove) {
        removeLater();
    }
}

void FlowText::draw(sf::RenderWindow* window) {
    if (m_flashing) {
        m_text.setPosition(getPosition() + m_offset);
        window->draw(m_text);
    }
}

void FlowText::setTextColor(const sf::Color& color) {
    m_text.setFillColor(color);
}

void FlowText::setTextSize(int size) {
    m_text.setCharacterSize(size);
}
//---------------------------------------------------------------------------
//! Label
//---------------------------------------------------------------------------
Label::Label() {
    init();
}

Label::Label(const sf::Sprite& sprite) {
    init();
    m_sprite = std::make_unique<sf::Sprite>(sprite);
    m_sprite->setColor({ 255,255,255,50 });
    setSprite(sprite);
}

void Label::init() {
    setName("Label");
}

void Label::setBounds(int x, int y, int w, int h) {
    setPosition(x, y);
    m_rect = Rect(x, y, w, h);

    m_shape.setPosition(sf::Vector2f(x, y));
    m_shape.setSize(sf::Vector2f(w, h));

    if (m_sprite) {
        m_sprite->setPosition(sf::Vector2f(x, y));
        m_sprite->setScale({(float)w / m_sprite->getTextureRect().size.x,
                            (float)h / m_sprite->getTextureRect().size.y});
    }
}

void Label::setSprite(const sf::Sprite& sprite) {
    *m_sprite = sprite;
}

void Label::setFontColor(const sf::Color& color) {
    if (m_text) {
        m_text->setFillColor(color);
    }
}

void Label::setFontSize(int size) {
    m_text->setCharacterSize(size);
}

void Label::setFontName(const sf::Font& font) {
    if (!m_text) {
        m_text = std::make_unique<sf::Text>(font, "");
        return;
    }

    m_text->setFont(font);
}

void Label::setFontStyle(uint32_t style) {
    m_text->setStyle(style);
}

void Label::setTextAlign(int value) {
    m_text_align = value;
}

void Label::setString(const std::string& str) {
    m_text->setString(str);
}

void Label::setOutlineColor(const sf::Color& color) {
    m_shape.setOutlineColor(color);
}

void Label::setFillColor(const sf::Color& color) {
    m_shape.setFillColor(color);
}

void Label::setOutlineThickness(int value) {
    m_shape.setOutlineThickness(value);
}

bool Label::contains(const Vector& point) const {
    return m_rect.isContain(point);
}

void Label::draw(sf::RenderWindow* window) {
    window->draw(m_shape);

    if (m_sprite) {
        m_sprite->setPosition(getPosition());
        window->draw(*m_sprite);
    }

    if (m_text && !m_text->getString().isEmpty()) {
        if (m_text_align == center) {
            Vector textBounds(m_text->getGlobalBounds().size.x, m_text->getGlobalBounds().size.y);
            m_text->setPosition(getPosition() + m_rect.size() / 2 - textBounds / 2);
        } else if (m_text_align == left) {
            m_text->setPosition({getPosition().x, getPosition().y});
        }

        window->draw(*m_text);
    }
}

void Label::onPropertySet(const std::string& name) {
    GameObject::onPropertySet(name);
    if (name == "text") {
        setString(getProperty("text").asString());
    }
}

void Label::onStarted() {
    if (getProperty("x").isValid()) {
        setBounds(
            getProperty("x").asFloat(),
            getProperty("y").asFloat(),
            getProperty("width").asFloat(),
            getProperty("height").asFloat());
            setString(getProperty("text").asString());
    }
    if (getProperty("hided").isValid() && getProperty("hided").asBool()) {
        hide();
    }
}

Rect Label::getBounds() const {
    return m_rect;
}

sf::Sprite& Label::getSprite() {
    return *m_sprite;
}

Label* Label::clone() const {
    Label* new_label = new Label();

    if (m_text)
        new_label->m_text = std::make_unique<sf::Text>(*m_text);
 
    if (m_sprite)
        new_label->m_sprite = std::make_unique<sf::Sprite>(*m_sprite);

    new_label->m_rect = m_rect;
    new_label->m_shape = m_shape;
    new_label->m_text_align = m_text_align;
    Rect bounds = getBounds();
    new_label->setBounds(bounds.left(), bounds.top(), bounds.width(), bounds.height());

    return new_label;
}
