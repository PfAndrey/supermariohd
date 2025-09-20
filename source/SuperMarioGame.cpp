#include <iomanip>
#include <cmath>
#include <cstdio>  // for sscanf

#include "tinyxml2.h"
#include <Format.hpp>

#include "Blocks.hpp"
#include "Enemies.hpp"

#include "Pickups.hpp"
#include "Items.hpp"
#include "SuperMarioGame.hpp"

#include "Logger.hpp"

using utils::toString;
using utils::toInt;
using utils::toFloat;

namespace {

struct Resource {
    std::string name;
    std::string filePath;
};

const std::string TEXTURES_DIR = MARIO_RES_PATH + "Textures/";

const std::vector<Resource> TEXTURE_RES = {
    { "Mario",      "Mario.png"},
    { "Tiles",      "Tiles.png"},
    { "AnimTiles",  "AnimTiles.png"},
    { "Enemies",    "Enemies.png"},
    { "Bowser",     "Bowser.png"},
    { "Items",      "Items.png"},
    { "Logo",       "Logo.png"},
    { "Sky",        "Backgrounds/Sky.png"},
    { "Night",      "Backgrounds/Night.png"},
    { "Underground","Backgrounds/Underground.png"},
    { "Castle",     "Backgrounds/Castle.png"},
    { "Water",      "Backgrounds/Water.png"}
};

};

MarioGame::MarioGame()
    : Game("SuperMario", { 1920, 1080 }) {
    LOG("MARIO_GAME", INFO, "Mario game created");

    //Load textures
    for (auto texture : TEXTURE_RES) {
        textureManager().loadFromFile(texture.name, TEXTURES_DIR + texture.filePath);

        if (texture.name.find("Backgrounds") != std::string::npos) {
            textureManager().get(texture.name)->setRepeated(true);
        }
    }

    //Load fonts
    const std::string fonts_dir = MARIO_RES_PATH + "Fonts/";
    for (auto font : { "arial", "menu_font", "main_font", "score_font", "some_font" }) {
        fontManager().loadFromFile(font, fonts_dir + font + ".ttf");
    }

    //Load sounds
    const std::string sounds_dir = MARIO_RES_PATH + "Sounds/";
    for (auto sound : { "breakblock", "bump", "coin", "fireball", "jump_super", "kick", "stomp","powerup_appears",
        "powerup", "pipe","flagpole", "bowser_falls", "bowser_fire", "mario_die" ,"stage_clear", "squish",
        "game_over","1-up","warning", "world_clear","pause","beep","fireworks" }) {
        soundManager().loadFromFile(sound, sounds_dir + sound + ".wav");
    }

    //Load music
    const std::string music_dir = MARIO_RES_PATH + "Music/";
    for (auto music : { "overworld", "underworld", "bowsercastle", "underwater", "invincibility" }) {
        musicManager().loadFromFile(music, music_dir + music + ".ogg");
    }

    //Configure input
    std::vector<std::pair<std::string, std::vector<std::string>>> inputs = {
        { "Fire", { "LShift", "[1]" } },
        { "Jump", { "Space",  "[2]" } },
        { "Pause", { "Enter", "[0]" } },
        { "Horizontal+", { "Right" } },
        { "Horizontal-", { "Left" } },
        { "Vertical-", { "Up" } },
        { "Vertical+", { "Down" } }
    };

    for (auto input : inputs) {
        inputManager().setupButton(input.first, input.second);
    }
}

MarioGame* MarioGame::instance() {
    static MarioGame s_instance;
    return &s_instance;
}

void MarioGame::syncMarioRank(GameObject* from_scene, GameObject* to_scene) {
    auto from_mario = from_scene->findChildObjectByType<Mario>();
    auto to_mario = to_scene->findChildObjectByType<Mario>();
    to_mario->syncState(from_mario);
}

void MarioGame::clearScenes() {
    if (!m_scene_stack.empty()) {
        for (auto m_scene : m_scene_stack) {
            m_scene->removeLater();
        }
        m_scene_stack.clear();
    }

    m_current_scene = nullptr;
}

void MarioGame::loadLevel(const std::string& level_name) {
    if (level_name == "[NEXT_LEVEL]") {
        m_current_stage_name = nextLevelName();
    } else {
        m_current_stage_name = level_name;
    }

    setScene(new MarioGameScene(MARIO_RES_PATH + "Levels/" + m_current_stage_name + ".tmx"));
    m_gui_object->moveToFront();
}

void MarioGame::loadSubLevel(const std::string& sublevel_name) {
    pushScene(new MarioGameScene(MARIO_RES_PATH + "Levels/" + sublevel_name + ".tmx"));
    m_gui_object->moveToFront();
}

void MarioGame::unloadSubLevel() {
    popScene();
}

void MarioGame::updateMusic() {
    if (m_invincible_mode) {
        return;
    }

    static const std::map<std::string, std::string> backgroundToMusic = {
        {"Underground", "underworld"},
        {"Water",       "underwater"},
        {"Castle",      "bowsercastle"}
    };

    std::string back_picture = m_current_scene->findChildObjectByType<Background>()->getProperty("Picture").asString();
    auto it = backgroundToMusic.find(back_picture);

    std::string music = "overworld";
    if (it != backgroundToMusic.end()) {
        music = it->second;
    }

    stopMusic();
    playMusic(music);
}

void MarioGame::setScene(GameObject* new_scene) {
    if (m_current_scene) {
        syncMarioRank(m_current_scene, new_scene);
    }

    clearScenes();
    m_current_scene = new_scene;
    new_scene->turnOff();
    getRootObject()->addChild(new_scene);
    m_scene_stack.push_back(new_scene);
    m_level_name = new_scene->castTo<MarioGameScene>()->getLevelName();
}

void MarioGame::pushScene(GameObject* new_scene) {
    if (m_current_scene) {
        syncMarioRank(m_current_scene, new_scene);
        m_current_scene->turnOff();
    }

    m_current_scene = new_scene;
    m_scene_stack.push_back(m_current_scene);
    getRootObject()->addChild(m_current_scene);
    updateMusic();
}

void MarioGame::popScene() {
    if (m_scene_stack.empty()) {
        return;
    }

    m_scene_stack.pop_back();
    auto old_scene = m_current_scene;
    m_current_scene = m_scene_stack.back();
    syncMarioRank(m_current_scene, old_scene);
    old_scene->removeLater();
    m_current_scene->turnOn();

    updateMusic();
};

MarioGUI* MarioGame::GUI() {
    return m_gui_object;
}

void MarioGame::init() {
    getRootObject()->addChild(m_gui_object = new MarioGUI());
    setState(GameState::MAIN_MENU);
}

void MarioGame::updateGUI() {
    m_gui_object->setGameTime(m_game_time / 1000);
    m_gui_object->setLevelName(m_level_name);
    m_gui_object->setLives(m_lives);
    m_gui_object->setCoins(m_coins);
    m_gui_object->setScore(m_score);
    m_gui_object->setMarioRank(m_current_scene->findChildObjectByType<Mario>()->getRank());
}

void MarioGame::reset() {
    m_game_time =  300000;
    m_lives = 3;
    m_score = 0;
    m_coins = 0;
    m_delay_timer = 0;
    updateGUI();
}

void MarioGame::setState(GameState state) {
    m_game_state = state;

    switch (state) {
    case GameState::MAIN_MENU:
        loadLevel(FIRST_STAGE_NAME);
        reset();
        m_gui_object->setState(GUIState::MENU);
        m_current_scene->start();
        m_current_scene->update(0); // to refresh camera pos
        m_current_scene->show();
        break;
    case GameState::STATUS:
        m_current_scene->turnOff();
        m_gui_object->setState(GUIState::STATUS);
        updateGUI();
        m_game_time = 300000;
        m_time_out_state = TimeOutState::NONE;
        musicManager().setPitch(1.f);
        m_delay_timer = 2500;
        stopMusic();
        break;
    case GameState::GAME_OVER:
        MARIO_GAME.playSound("game_over");
        m_current_scene->turnOff();
        m_gui_object->setState(GUIState::GAME_OVER);
        m_delay_timer = 5000;
        break;
    case GameState::PLAYING:
        m_current_scene->update(500);
        m_current_scene->turnOn();
        m_gui_object->setState(GUIState::NORMAL);
        updateMusic();
        break;
    }
}

void MarioGame::showStatus() {
    setState(GameState::STATUS);
}

std::string MarioGame::nextLevelName() const {
    int world = 1;
    int level = 1;

    // parse current stage
    if (sscanf(m_current_stage_name.c_str(), "WORLD %d-%d", &world, &level) != 2) {
        // failed to parse
    }

    // increment level
    if (level < 4) {
        ++level;
    } else {
        ++world;
        level = 1;
    }

    // format new stage name
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "WORLD %d-%d", world, level);

    return std::string(buffer);
}

void MarioGame::update(int delta_time) {
    Game::update(delta_time);
    m_timer.update(delta_time);

    switch (m_game_state) {
    case GameState::MAIN_MENU:
        if (inputManager().isButtonDown("Pause")) {
            setState(GameState::STATUS);
        }
        break;
    case GameState::STATUS:
        m_delay_timer -= delta_time;
        if (m_delay_timer < 0) {
            setState(GameState::PLAYING);
        }
        break;
    case GameState::PLAYING:
        if (inputManager().isButtonDown("Pause")) {
            playSound("pause");
            if (m_current_scene->isEnabled()) {
                m_current_scene->disable();
                musicManager().pause();
                m_gui_object->pause(true);
                globalTimer().setPause(true);
                break;
            } else {
                m_current_scene->enable();
                musicManager().play();
                m_gui_object->pause(false);
                globalTimer().setPause(false);
            }
        }

        if (!m_current_scene->isEnabled()) {
            break;
        }

        m_game_time -= delta_time;

        switch (m_time_out_state) {
        case TimeOutState::NONE:
            if (m_game_time < 100000) {
                m_time_out_state = TimeOutState::START_WARNING;
                stopMusic();
                playSound("warning");
            }
            break;
        case TimeOutState::START_WARNING:
            if (m_game_time < 97000) {
                musicManager().setPitch(1.35f);
                updateMusic();
                m_time_out_state = TimeOutState::WARNING;
            }
            break;
        case TimeOutState::WARNING:
            if (m_game_time < 0) {
                m_current_scene->findChildObjectByType<Mario>()->kill();
                setState(GameState::TIME_OUT);
            }
            break;
        }
        m_gui_object->setGameTime(m_game_time / 1000);
        break;
    case GameState::LEVEL_OVER:
        if (m_game_time > 0) {
            m_delay_timer -= delta_time;
            if (m_delay_timer < 0) {
                m_delay_timer = 12;
                m_game_time -= 1000;
                addScore(50);
                static int i = 0; ++i;
                if (!(i%4)) {
                    playSound("beep");
                }
                if (m_game_time < 0) {
                    m_game_time = 0;
                }
                m_gui_object->setGameTime(m_game_time / 1000);
            }
        }
        break;
    case GameState::GAME_OVER:
        m_delay_timer -= delta_time;
        if (m_delay_timer < 0) {
            setState(GameState::MAIN_MENU);
        }
        break;
    default:
        break;
    };
}

void MarioGame::addScore(int value, const Vector& vector) {
    m_score += value;
    GUI()->setScore(m_score);

    if (vector != Vector::ZERO) {
        auto flow_text = m_gui_object->createFlowText();
        flow_text->splash(vector, toString(value));
        m_current_scene->addChild(flow_text);
    }
}

void MarioGame::addCoin() {
    ++m_coins;
    if (m_coins >= 100) {
        addLive();
        m_coins = 0;
        playSound("1-up");
    }

    GUI()->setCoins(m_coins);
}

void MarioGame::addLive() {
    GUI()->setLives(++m_lives);

    auto vector = m_current_scene->findChildObjectByType<Mario>()->getBounds().center();
    if (vector != Vector::ZERO) {
        auto flow_text = m_gui_object->createFlowText();
        flow_text->splash(vector, "1 up");
        m_current_scene->addChild(flow_text);
    }
}

void MarioGame::invincibleMode(bool value) {
    m_invincible_mode = value;
    if (!value) {
        updateMusic();
    } else {
        auto mario = m_current_scene->findChildObjectByType<Mario>();
        mario->setInvincibleMode(true);
        stopMusic();
        playMusic("invincibility");
    }
}

void MarioGame::setScore(int score) {
    m_score = score;
    GUI()->setScore(m_score);
}

int MarioGame::getScore() const {
    return m_score;
}

int MarioGame::getGameTime() const {
    return m_game_time;
}

MarioGame& marioGame() {
    return *MarioGame::instance();
}

Label* MarioGame::createText(const std::string& text, const Vector& pos) {
    Label* label = GUI()->createLabel();
    label->setString(text);
    label->setPosition(pos);
    return label;
}

void MarioGame::setEndLevelStatus() {
    m_game_state = GameState::LEVEL_OVER;
}

void MarioGame::marioDied() {
    loadLevel(m_current_stage_name);

    --m_lives;

    if (m_lives > 0) {
        setState(GameState::STATUS);
    } else {
        setState(GameState::GAME_OVER);
    }
}

GameObject* MarioGame::currentScene() const {
    return m_current_scene;
}

Mario* MarioGame::getPlayer(int index) const {
    (void)index; // unused

    if (!m_current_scene) {
        return nullptr;
    }

    return m_current_scene->findChildObjectByType<Mario>();
}

void MarioGame::playSound(const std::string& name, const Vector& pos) {
    if (!m_current_scene) {
        return;
    }

    m_current_scene->castTo<MarioGameScene>()->playSoundAtPoint(name, pos);
}

void MarioGame::playSound(const std::string& name) {
    Game::playSound(name);
}

TimerManager& MarioGame::globalTimer() {
    return m_timer;
}

std::map<std::string, Property> parseProperties(tinyxml2::XMLElement* object) {
    std::map<std::string, Property> parsed;

    //common properties
    parsed["x"] = toFloat(object->Attribute("x"));
    parsed["y"] = toFloat(object->Attribute("y"));
    parsed["width"]  = toFloat(object->Attribute("width"));
    parsed["height"] = toFloat(object->Attribute("height"));
    parsed["name"]   = toString(object->Attribute("name"));
 
    //specific properties
    tinyxml2::XMLElement* properties = object->FirstChildElement("properties");
    if (properties) {
        for (auto property = properties->FirstChildElement("property"); property; property = property->NextSiblingElement()) {

            std::string type = "string";
            if (property->Attribute("type")) {
                type = property->Attribute("type");
            }

            const std::string& name = property->Attribute("name");
            const std::string& value = property->Attribute("value");

            using utils::strHash;

            switch (strHash(type.c_str())) {
            case strHash("int"):
                parsed[name] = utils::toInt(value);
                break;
            case strHash("float"):
                parsed[name] = utils::toFloat(value);
                break;
            case strHash("bool"):
                parsed[name] = utils::toBool(value);
                break;
            default:
                parsed[name] = toString(value);
                break;
            }
        }
    }

    //parse text element
    tinyxml2::XMLElement* text_properties = object->FirstChildElement("text");
    if (text_properties) {
        parsed["text"] = std::string(text_properties->FirstChild()->Value());
    }

    return parsed;
}

template <typename T>
GameObject* goFabric() {
    return new T;
}

GameObject* textFabric() {
    Label* lab = new Label();
    lab->setFontName(*MARIO_GAME.fontManager().get("some_font"));
    lab->setFontStyle(sf::Text::Bold);
    lab->setFontColor({ 255,255,220 });
    lab->setFontSize(36);
    lab->setTextAlign(Label::center);
    lab->setFillColor(sf::Color::Transparent);
    return lab;
}

GameObject* parseGameObject(tinyxml2::XMLElement* element) {
    std::string obj_type = element->Attribute("type");

    static std::unordered_map<std::string, GameObject* (*)()> fabrics =
    {
        { "Mario",             goFabric<Mario> },
        { "Goomba",            goFabric<Goomba> },
        { "Koopa",             goFabric<Koopa> },
        { "HammerBro",         goFabric<HammerBro> },
        { "Bowser",            goFabric<Bowser> },
        { "BuzzyBeetle",       goFabric<BuzzyBeetle> },
        { "LakitySpawner",     goFabric<LakitySpawner> },
        { "CheepCheep",        goFabric<CheepCheep> },
        { "Blooper",           goFabric<Blooper> },
        { "CheepCheepSpawner", goFabric<CheepCheepSpawner>},
        { "BulletBillSpawner", goFabric<BulletBillSpawner> },
        { "PiranhaPlant",      goFabric<PiranhaPlant> },
        { "Podoboo",           goFabric<Podoboo> },
        { "Coin",              goFabric<Coin> },
        { "Jumper",            goFabric<Jumper> },
        { "FireBar",           goFabric<FireBar> },
        { "MoveablePlatform",  goFabric<MoveablePlatform> },
        { "FallingPlatform",   goFabric<FallingPlatform> },
        { "PlatformSystem",    goFabric<PlatformSystem> },
        { "Background",        goFabric<Background> },
        { "LevelPortal",       goFabric<LevelPortal> },
        { "EndLevelFlag",      goFabric<EndLevelFlag> },
        { "EndLevelKey",       goFabric<EndLevelKey> },
        { "CastleFlag",        goFabric<CastleFlag> },
        { "Princess",          goFabric<Princess> },
        { "Trigger",           goFabric<Trigger> },
        { "Text",              textFabric }
    }; 

    auto obj_fabric = fabrics.find(obj_type);
    if (obj_fabric == fabrics.end()) {
        // no fabric for this object
        return nullptr;
    }

    GameObject* object = obj_fabric->second();
    auto properties = parseProperties(element);
    for (const auto& property : properties) {
        object->setProperty(property.first, property.second);
    }

    return object;
}

Blocks* parseBlocks(tinyxml2::XMLElement* element) {
    tinyxml2::XMLElement* data = element->FirstChildElement("layer")->FirstChildElement("data");

    // parse blocks indexes from cvs format (1,20,30,40,3)
    std::string blocks_indexes_string = data->GetText();
    std::vector<char> blockData;
    std::string buf;

    for (auto chr : blocks_indexes_string) {
        if (isdigit(chr)) {
            buf += chr;
        } else if (!buf.empty()) {
            blockData.push_back((char)utils::toInt(buf));
            buf.clear();
        }
    }

    static auto blocks_fabric = [](char tileCode) -> AbstractBlock* {
        static const bool INVIZ_STYLE = true, NOT_INVIZ_STYLE = false;
        auto tileId = static_cast<TileCode>(tileCode);

        switch (tileId) {
        case TileCode::EMPTY:
            return nullptr;
        case TileCode::BRICK:
            return new BrickBlock();
        case TileCode::COIN_BOX:
            return new CoinBoxBlock(tileId);
        case TileCode::QUESTION_ONE_COIN:
            return new QuestionBlock(tileId, prize<TwistedCoin>());
        case TileCode::QUESTION_MUSHROOM:
            return new QuestionBlock(tileId, prize<Mushroom>());
        case TileCode::BRICK_MUSHROOM:
            return new QuestionBlock(tileId, prize<Mushroom>());
        case TileCode::LADDER:
            return new QuestionBlock(tileId, prize<Ladder>());
        case TileCode::INVIZ_LADDER:
            return new QuestionBlock(tileId, prize<Ladder>());
        case TileCode::BRICK_LADDER:
            return new QuestionBlock(tileId, prize<Ladder>());
        case TileCode::INVIZ_UP:
            return new QuestionBlock(tileId, prize<OneUpMushroom>());
        case TileCode::INVIZ_COIN:
            return new QuestionBlock(tileId, prize<TwistedCoin>());
        case TileCode::BRICK_LIVE_UP:
            return new QuestionBlock(tileId, prize<OneUpMushroom>());
        case TileCode::BRICK_STAR:
            return new QuestionBlock(tileId, prize<Star>());
        default:
            return new StaticBlock(tileId);
        }
    };

    auto blocks = new Blocks(toInt(element->Attribute("width")),
                             toInt(element->Attribute("height")),
                             toInt(element->Attribute("tilewidth")),
                             toInt(element->Attribute("tileheight")));

    blocks->loadFromArray(blockData, blocks_fabric);
    return blocks;
}

void MarioGameScene::loadFromFile(const std::string& filepath) {
    setName("MarioGameScene");
    m_level_name = filepath;
    auto it1 = --m_level_name.end();
    while (*it1 != '.') it1--;
    auto it2 = it1;
    while (*it2 != '/' || it2 == m_level_name.begin()) it2--;
    m_level_name = std::string(++it2, it1);

    //@TODO: use filesystem to extract file name

    removeChildObjects();
    tinyxml2::XMLDocument documet;
    bool status = documet.LoadFile(filepath.c_str());
    assert(status == tinyxml2::XML_SUCCESS); // cant load file
    tinyxml2::XMLElement* root_element = documet.FirstChildElement();

    //Load tilemap
    m_blocks = parseBlocks(root_element);
    addChild(m_blocks);

    //Load objects
    tinyxml2::XMLElement* objects = root_element->FirstChildElement("objectgroup");
    for (auto obj = objects->FirstChildElement("object"); obj ; obj = obj->NextSiblingElement()) {
        auto object = parseGameObject(obj);
        if (object) {
            addChild(object);
        }
    }

    m_mario = findChildObjectByType<Mario>();
    m_mario->moveToFront();
    assert(m_mario); // no mario object in scene
    setCameraOnTarget();
}

const std::string& MarioGameScene::getLevelName() const {
    return m_level_name;
}

void MarioGameScene::init() {
    setName("MarioGameScene");
    m_view.setSize(screen_size);
    MARIO_GAME.eventManager().subscribe(this);
}

MarioGameScene::MarioGameScene() {
    init();
}

MarioGameScene::MarioGameScene(const std::string& filepath) {
    loadFromFile(filepath);
    init();
}

void MarioGameScene::update(int delta_time) {
    GameObject::update(delta_time);

    Vector camera_pos = m_view.getCenter();
    const Vector delta = (m_mario->getBounds().center() - camera_pos) * delta_time;
    camera_pos.x += delta.x * 0.0075f;
    camera_pos.y += delta.y * 0.0005f;
	//camera_pos = m_mario->getBounds().center();
    auto blocks_rect = m_blocks->getRenderBounds();
    camera_pos.x = math::clamp(camera_pos.x, screen_size.x / 2.f, blocks_rect.width() - screen_size.x / 2.f);
    camera_pos.y = math::clamp(camera_pos.x, screen_size.y / 2.f, blocks_rect.height() - screen_size.y / 2.f);
    m_view.setCenter(camera_pos);
    m_camera_rect = Rect(Vector(m_view.getCenter()) - Vector(m_view.getSize()) * 0.5, m_view.getSize());
}

void MarioGameScene::draw(sf::RenderWindow* render_window) {
    if (!isVisible()) {
        return;
    }

    render_window->setView(m_view);
    const auto& camera_rect = cameraRect();

    for (auto& obj : getChilds()) {
        if (obj->isVisible() && camera_rect.isIntersect(obj->getBounds())) {
            obj->draw(render_window);
        }
    }

    render_window->setView(render_window->getDefaultView());
}

void MarioGameScene::events(const sf::Event& event) {
    //if (event.is<sf::Event::Resized>()) {
       // screen_size = Vector(event.size.width / scale_factor, event.size.height / scale_factor);
       // m_view.setSize(screen_size);
   // }
}

Vector MarioGameScene::pointToScreen(const Vector& vector) {
    return (vector - (Vector(m_view.getCenter()) - Vector(m_view.getSize()) / 2)) * SCALE_FACTOR;
}

Vector MarioGameScene::screenToPoint(const Vector& vector) {
    return vector / SCALE_FACTOR + (Vector(m_view.getCenter()) - Vector(m_view.getSize()) / 2);
}

void MarioGameScene::playSoundAtPoint(const std::string& name, const Vector& pos) {
    if (cameraRect().isContain(pos)) {
        MARIO_GAME.playSound(name);
    }
}

Rect MarioGameScene::cameraRect() const {
    return m_camera_rect;
}

void MarioGameScene::setCameraOnTarget() {
    m_view.setCenter(m_mario->getBounds().center());
}

MarioGameScene::~MarioGameScene() {
    MARIO_GAME.eventManager().unsubcribe(this);
}
//---------------------------------------------------------------------------
//! MarioGUI
//---------------------------------------------------------------------------
MarioGUI::MarioGUI() {
    const int y_gui_pos = 5;
    m_score_lab = new Label();
    m_score_lab->setFontName(*MARIO_GAME.fontManager().get("some_font"));
    m_score_lab->setPosition({ 70,y_gui_pos });
    m_score_lab->setFontStyle(sf::Text::Bold);
    m_score_lab->setFontColor({255,255,220});
    m_score_lab->setFontSize(40);
    m_score_lab->setTextAlign(Label::left);
    addChild(m_score_lab);

    m_coin_counter_lab = createLabel();
    m_coin_counter_lab->setPosition({ 490, y_gui_pos });
    addChild(m_coin_counter_lab);

    m_world_lab = createLabel();
    m_world_lab->setPosition({ 720, y_gui_pos });
    addChild(m_world_lab);

    m_timer = createLabel();
    m_timer->setPosition({ 1080, y_gui_pos });
    addChild(m_timer);

    m_level_name = createLabel();
    m_level_name->setPosition(MARIO_GAME.screenSize() / 2.f + Vector::UP*100.f);
    m_level_name->setTextAlign(Label::center);
    addChild(m_level_name);

    m_lives = createLabel();
    m_lives->setPosition(MARIO_GAME.screenSize() / 2.f + Vector(-15,-18));
    addChild(m_lives);

    auto texture = MARIO_GAME.textureManager().get("Mario");
    m_mario_pix = new Animator();
    m_mario_pix->create("small", *texture, { 0,96,32,32 });
    m_mario_pix->setSpriteOffset("small", 0, Vector::DOWN * 22.f);
    m_mario_pix->create("big", *texture, { 0,32,32,64 });
    m_mario_pix->setPosition(MARIO_GAME.screenSize() / 2.f + Vector(-64,-44));
    m_mario_pix->play("big");
    m_mario_pix->scale(1.3f, 1.3f);
    m_mario_pix->hide();
    addChild(m_mario_pix);
    m_fire_pallete.create({ sf::Color(202,77,62), sf::Color(132,133,30) }, { sf::Color(255,255,255), sf::Color(202,77,62) });

    m_flow_text = new FlowText(*MARIO_GAME.fontManager().get("main_font"));
    m_flow_text->setTextColor(sf::Color::Red);
    m_flow_text->setSplashVector({ 0,-3 });
    m_flow_text->setTextSize(14);
    addChild(m_flow_text);

    const auto scr_size = MARIO_GAME.screenSize();

    m_game_logo = new Label(sf::Sprite(*MARIO_GAME.textureManager().get("Logo"), sf::IntRect( {0,0}, {750, 300})));
    m_game_logo->setPosition(scr_size.x * 0.3f, scr_size.y * 0.25f);
    addChild(m_game_logo);

    m_menu_selector = new Label(sf::Sprite(*MARIO_GAME.textureManager().get("Items"), sf::IntRect( {128, 150}, {32, 32})));
   // m_menu_selector->setPosition(scr_size.x * 0.8, scr_size.y * 2);
    addChild(m_menu_selector);

    m_one_player_lab = createLabel();
    m_one_player_lab->setPosition(MARIO_GAME.screenSize() / 2.f + Vector::DOWN * 80.f);
    m_one_player_lab->setTextAlign(Label::center);
    m_one_player_lab->setString("1 PLAYER GAME");
    m_one_player_lab->setFontColor(sf::Color(22, 85,122));
    addChild(m_one_player_lab);

    m_two_player_lab = m_one_player_lab->clone();
    m_two_player_lab->setString("2 PLAYER GAME");
    m_two_player_lab->setPosition(MARIO_GAME.screenSize() / 2.f + Vector::DOWN * 135.f);
    addChild(m_two_player_lab);

    m_one_player_lab->setFontColor({ 0,0,0 });

    m_coin = new Animator();
    m_coin->create("twist", *MARIO_GAME.textureManager().get("Items"), Vector(0, 84), Vector(32, 32), 4, 1, 0.01f);
    m_coin->play("twist");
    m_coin->setPosition(458, 15);
    addChild(m_coin);

    m_game_over_lab = createLabel();
    m_game_over_lab->setPosition(MARIO_GAME.screenSize() / 2.f);
    m_game_over_lab->setTextAlign(Label::center);
    m_game_over_lab->setString("GAME OVER");
    addChild(m_game_over_lab);

    m_paused_label = createLabel();
    m_paused_label->setPosition(MARIO_GAME.screenSize() / 2.f);
    m_paused_label->setTextAlign(Label::center);
    m_paused_label->setString("PAUSED");
    addChild(m_paused_label);
    m_paused_label->hide();
}

void MarioGUI::setMarioRank(MarioRank rank) {
     m_mario_pix->play((rank == MarioRank::SMALL) ? "small"
                                                  : "big");
     m_mario_pix->setPallete((rank == MarioRank::FIRE) ? &m_fire_pallete
                                                       : nullptr);
}

void MarioGUI::update(int delta_time) {
    GameObject::update(delta_time);

    if (m_state == GUIState::MENU) {
        m_tmr += delta_time;
        const auto scr_size = MARIO_GAME.screenSize();
        float x = 0.85f + 0.15f*cos(m_tmr * 0.01f);
        m_menu_selector->setPosition(scr_size.x * 0.4 + 16*(1-x), scr_size.y * 0.57 + 16 *(1-x));
        m_menu_selector->getSprite().setScale({x,x});
    }
}

void MarioGUI::draw(sf::RenderWindow* render_window) {
    GameObject::draw(render_window);
}

void MarioGUI::setScore(int value) {
    std::stringstream str_stream;
    str_stream << "MARIO: " << std::setw(6) << std::setfill('0') << value;
    m_score_lab->setString(str_stream.str());
}

void MarioGUI::setCoins(int value) {
    std::stringstream str_stream;
    str_stream << "x" << std::setw(2) << std::setfill('0') << value;
    m_coin_counter_lab->setString(str_stream.str());
}

void MarioGUI::setGameTime(int time) {
    std::stringstream str_stream;
    str_stream << "TIME: " << std::setw(3) << std::setfill('0') << time;
    m_timer->setString(str_stream.str());
}

FlowText* MarioGUI::createFlowText() {
    return m_flow_text->clone();
}

Label* MarioGUI::createLabel() {
    return m_score_lab->clone();
}

void MarioGUI::setState(GUIState state) {
    m_state = state;

    GameObject* states_labs[3][5] = {
        // status
        { m_level_name, m_lives, m_mario_pix, nullptr, nullptr},
        // menu
        { m_game_logo, m_one_player_lab, m_menu_selector, m_two_player_lab, nullptr },
        // game over
        { m_game_over_lab, nullptr, nullptr, nullptr, nullptr }
    };

    for (auto state_labs : states_labs) {
        for (int i = 0; state_labs[i]; ++i) {
            state_labs[i]->turnOff();
        }
    }

    if (state == GUIState::NORMAL) {
        return;
    }

    auto state_labs = states_labs[(int)state - 1];

    for (int i = 0; state_labs[i]; ++i) {
        state_labs[i]->turnOn();
    }
}

void MarioGUI::setLevelName(const std::string& string) {
    m_level_name->setString(string);
    m_world_lab->setString(string);
}

void MarioGUI::setLives(int value) {
    m_lives->setString("  x  " + toString(value));
}

void MarioGUI::pause(bool ispaused) {
    if (ispaused) {
        m_paused_label->show();
    } else {
        m_paused_label->hide();
    }
}
