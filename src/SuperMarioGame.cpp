#include "SuperMarioGame.h"
#include <iomanip>
#include "tinyxml2.h"
#include "Pickups.h"
#include "Items.h"
#include "Blocks.h"
#include "Enemies.h"
#include <cmath>

void Timer::invoke(const std::function <void()>& func, int delay)
{
    m_invoke_list.push_back(std::make_pair<>(m_time + delay, func));
}

void Timer::update(int delta_time)
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

//--------------------------------------------------------------------------------
CMarioGame* CMarioGame::s_instance = NULL;

CMarioGame::CMarioGame() : CGame("SuperMario", {1280,720})
{
    //Load textures
    const std::string textures_dir = MARIO_RES_PATH + "Textures/";
    for (auto texture : { "Mario", "Tiles", "AnimTiles", "Enemies", "Bowser", "Items", "Logo" })
        textureManager().loadFromFile(texture, textures_dir + texture + ".png");

    for (auto texture : { "Sky", "Night", "Underground", "Castle", "Water" })
    {
        textureManager().loadFromFile(texture, textures_dir + "Backgrounds/" + texture + ".png");
        textureManager().get(texture)->setRepeated(true);
    }

    //Load fonts
    const std::string fonts_dir = MARIO_RES_PATH + "Fonts/";
    for (auto font : { "arial", "menu_font", "main_font", "score_font", "some_font"})
        fontManager().loadFromFile(font, fonts_dir + font + ".ttf");

    //Load sounds
    const std::string sounds_dir = MARIO_RES_PATH + "Sounds/";
    for (auto sound : { "breakblock", "bump", "coin", "fireball", "jump_super", "kick", "stomp","powerup_appears",
         "powerup", "pipe","flagpole", "bowser_falls", "bowser_fire", "mario_die","stage_clear", "game_over","1-up","warning", "world_clear"})
        soundManager().loadFromFile(sound, sounds_dir + sound + ".wav");

    //Load music
    const std::string music_dir = MARIO_RES_PATH + "Music/";
    for (auto music : { "overworld", "underworld", "bowsercastle", "underwater", "invincibility"})
        musicManager().loadFromFile(music, music_dir + music + ".ogg");

    //Configure input
    for (auto key : {sf::Keyboard::Left, sf::Keyboard::Right,sf::Keyboard::Up, sf::Keyboard::Down,
         sf::Keyboard::Space, sf::Keyboard::LShift })
        inputManager().registerKey(key);
}

CMarioGame::~CMarioGame()
{

}

CMarioGame* CMarioGame::instance()
{
    if (s_instance == NULL)
        s_instance = new CMarioGame();
    return s_instance;
}

void CMarioGame::syncMarioRank(CGameObject* from_scene, CGameObject* to_scene)
{
    auto mario_rank = from_scene->findObjectByName<CMario>("Mario")->getRank();
    to_scene->findObjectByName<CMario>("Mario")->setRank(mario_rank);
}

void CMarioGame::clearScenes()
{
    if (!m_scene_stack.empty())
    {
        for (auto m_scene : m_scene_stack)
            getRootObject()->removeObject(m_scene);
        m_scene_stack.clear();
    }
    m_current_scene = NULL;
}

void CMarioGame::loadLevel(const std::string& _level_name)
{
    std::string level_name = _level_name;
    if (_level_name == "[NEXT_LEVEL]")
        level_name = nextLevelName();
    setScene(new CMarioGameScene(MARIO_RES_PATH + "Levels/" + level_name  + ".tmx"));
    m_gui_object->moveToFront();
    m_current_stage_name = level_name;
}

void CMarioGame::loadSubLevel(const std::string& sublevel_name)
{
    pushScene(new CMarioGameScene(MARIO_RES_PATH + "Levels/" + sublevel_name + ".tmx"));
    m_gui_object->moveToFront();
}

void  CMarioGame::unloadSubLevel()
{
    popScene();
}

void CMarioGame::updateMusic()
{
    if (m_invincible_mode)
        return;
    std::string back_picture = m_current_scene->findObjectByType<CBackground>()->getProperty("Picture").asString();
    std::string music = "overworld";
    static std::map<std::string, std::string> backgroundToMusic =
    {
        {"Underground","underworld"},
        {"Water","underwater"},
        {"Castle","bowsercastle"}
    };
    if (backgroundToMusic.find(back_picture) != backgroundToMusic.end())
        music = backgroundToMusic[back_picture];
    stopMusic();
    playMusic(music);
}



void CMarioGame::setScene(CGameObject* new_scene)
{
    if (m_current_scene)
        syncMarioRank(m_current_scene, new_scene);
    clearScenes();
    m_current_scene = new_scene;
    new_scene->turnOff();
    getRootObject()->addObject(new_scene);
    m_scene_stack.push_back(new_scene);
    m_level_name = new_scene->castTo<CMarioGameScene>()->getLevelName();
}

void CMarioGame::pushScene(CGameObject* new_scene)
{
    if (m_current_scene)
    {
        syncMarioRank(m_current_scene, new_scene);
        m_current_scene->turnOff();
    }
    m_current_scene = new_scene;
    m_scene_stack.push_back(m_current_scene);
    getRootObject()->addObject(m_current_scene);

    updateMusic();
}

void CMarioGame::popScene()
{
    if (m_scene_stack.empty())
        return;
    m_scene_stack.pop_back();
    auto old_scene = m_current_scene;
    m_current_scene = m_scene_stack.back();
    syncMarioRank(m_current_scene, old_scene);
    getRootObject()->removeObject(old_scene);
    m_current_scene->turnOn();

    updateMusic();
}; 

CMarioGUI* CMarioGame::GUI()
{
    return m_gui_object;
}

void CMarioGame::init()
{
    getRootObject()->addObject(m_gui_object = new CMarioGUI());
    setState(GameState::main_menu);
}

void CMarioGame::updateGUI() 
{
    m_gui_object->setGameTime(m_game_time / 1000);
    m_gui_object->setLevelName(m_level_name);
    m_gui_object->setLives(m_lives);
    m_gui_object->setCoins(m_coins);
    m_gui_object->setScore(m_score);
    m_gui_object->setMarioRank(m_current_scene->findObjectByType<CMario>()->getRank());
}

void CMarioGame::reset()
{
    m_game_time =  300000;
    m_lives = 3;
    m_score = 0;
    m_coins = 0;
    m_delay_timer = 0;
    updateGUI();
}

void CMarioGame::setState(GameState state)
{
    m_game_state = state;

    switch (state)
    {
    case(GameState::main_menu):
    {
        loadLevel(m_first_stage_name);
        reset();
        m_gui_object->setState(GUIState::menu);
        m_current_scene->enable();
        m_current_scene->update(0);
        m_current_scene->disable();
        m_current_scene->show();
        break;
    }
    case(GameState::status):
    {
        m_current_scene->turnOff();
        m_gui_object->setState(GUIState::status);
        updateGUI();
        m_game_time = 300000;
        m_time_out_state = TimeOutState::none;
        musicManager().setPitch(1.f);
        m_delay_timer = 2500;
        stopMusic();
        break;
    }
    case(GameState::game_over):
    {
        CMarioGame::instance()->playSound("game_over");
        m_current_scene->turnOff();
        m_gui_object->setState(GUIState::gameover);
        m_delay_timer = 5000;
        break;
    }
    case(GameState::playing):
    {
        m_current_scene->update(500);
        m_current_scene->turnOn();
        m_gui_object->setState(GUIState::normal);
        updateMusic();
        break;
    }
    }
}

void CMarioGame::showStatus()
{
    setState(GameState::status);
}

std::string CMarioGame::nextLevelName() const
{
    int w = toInt(m_current_stage_name[6]+std::string());
    int l = toInt(m_current_stage_name[8]+std::string());
    if (l < 4)
    {
        l++;
    }
    else
    {
        assert(w < 8);
        w++;
        l = 1;
    }
    return "WORLD " + toString(w) + "-" + toString(l);
}


void CMarioGame::update(int delta_time)
{
    CGame::update(delta_time);
    m_timer.update(delta_time);

    switch (m_game_state)
    {
    case (GameState::main_menu):
    {
        if (inputManager().isKeyPressed(sf::Keyboard::Space))
            setState(GameState::status);
        break;
    }
    case (GameState::status):
    {
        m_delay_timer -= delta_time;
        if (m_delay_timer < 0)
            setState(GameState::playing);
        break;
    }
    case (GameState::playing):
    {
        m_game_time -= delta_time;

        switch (m_time_out_state)
        {
        case(TimeOutState::none):
        {
            if (m_game_time < 100000)
            {
                m_time_out_state = TimeOutState::start_warning;
                stopMusic();
                playSound("warning");
            }
            break;
        }
        case(TimeOutState::start_warning):
        {
            if (m_game_time < 97000)
            {
                musicManager().setPitch(1.35f);
                updateMusic();
                m_time_out_state = TimeOutState::warning;
            }
            break;
        }
        case(TimeOutState::warning):
        {
            if (m_game_time < 0)
            {
                m_current_scene->findObjectByType<CMario>()->setState(MarioState::died);
                setState(GameState::time_out);
            }
            break;
        }
        }

        m_gui_object->setGameTime(m_game_time / 1000);
        break;
    }
    case (GameState::level_over):
    {
        if (m_game_time > 0)
        {
            m_delay_timer -= delta_time;
            if (m_delay_timer < 0)
            {
                m_delay_timer = 20;
                m_game_time -= 1000;
                addScore(50);
                if (m_game_time < 0)
                    m_game_time = 0;

                m_gui_object->setGameTime(m_game_time / 1000);
            }
        }
        break;
    }
    case (GameState::game_over):
    {
        m_delay_timer -= delta_time;
        if (m_delay_timer < 0)
            setState(GameState::main_menu);
        break;
    }
    default:
    {
        break;
    }
    };
}

Timer& CMarioGame::timer()
{
    return m_timer;
}

void CMarioGame::addScore(int value, const Vector& vector)
{
    m_score += value;
    GUI()->setScore(m_score);

    if (vector != Vector::zero)
    {
        auto flow_text = m_gui_object->createFlowText();
        flow_text->splash(vector, toString(value));
        m_current_scene->addObject(flow_text);
    }
}

void CMarioGame::addCoin()
{
    ++m_coins;
    if (m_coins >= 100)
    {
        addLive();
        m_coins = 0;
        playSound("1-up");
    }

    GUI()->setCoins(m_coins);
}

void CMarioGame::addLive()
{
    GUI()->setLives(++m_lives);

    auto vector = m_current_scene->findObjectByType<CMario>()->getBounds().center();
    if (vector != Vector::zero)
    {
        auto flow_text = m_gui_object->createFlowText();
        flow_text->splash(vector, "1 up");
        m_current_scene->addObject(flow_text);
    }
}

void CMarioGame::invincibleMode(bool value)
{
    m_invincible_mode = value;
    if (!value)
    {
        updateMusic();
    }
    else
    {
        auto mario = m_current_scene->findObjectByType<CMario>();
        mario->setInvincibleMode(true);
        stopMusic();
        playMusic("invincibility");
    }
}

void CMarioGame::setScore(int score)
{
    m_score = score;
    GUI()->setScore(m_score);
}

int CMarioGame::getScore() const
{
    return m_score;
}

int CMarioGame::getGameTime() const
{
    return m_game_time;
}

CMarioGame& MarioGame()
{
    return *CMarioGame::instance();
}

CLabel* CMarioGame::createText(const std::string& text, const Vector& pos)
{
    CLabel* label = GUI()->createLabel();
    label->setString(text);
    label->setPosition(pos);
    return label;
}

void CMarioGame::setEndLevelStatus()
{
    m_game_state = GameState::level_over;
}

void CMarioGame::marioDied()
{
    loadLevel(m_current_stage_name);
    --m_lives;
    if (m_lives > 0)
        setState(GameState::status);
    else
        setState(GameState::game_over);
}

//--------------------------------------------------------------------------------

std::map<std::string, Property> parseProperties(tinyxml2::XMLElement* object)
{
    std::map<std::string, Property> parsed;

    //common properties
    parsed["x"] = Property(toFloat(object->Attribute("x")));
    parsed["y"] = Property(toFloat(object->Attribute("y")));
    parsed["width"] = Property(toFloat(object->Attribute("width")));
    parsed["height"] = Property(toFloat(object->Attribute("height")));
    parsed["name"] = Property(toString(object->Attribute("name")));

    //specific properties
    tinyxml2::XMLElement* properties = object->FirstChildElement("properties");
    if (properties)
        for (auto property = properties->FirstChildElement("property"); property != NULL; property = property->NextSiblingElement())
        {

            std::string type("string");
            if (property->Attribute("type")) type = property->Attribute("type");
            std::string name = property->Attribute("name");
            std::string value = property->Attribute("value");

            if (type == "int")
                parsed[name] = toInt(value);
            else if (type == "float")
                parsed[name] = toFloat(value);
            else if (type == "bool")
                parsed[name] = toBool(value);
            else
                parsed[name] = toString(value);
        }

    return parsed;
}

template <typename T>
CGameObject* goFabric()
{
    return new T;
}

CGameObject* parseGameObject(tinyxml2::XMLElement* element)
{
    std::string obj_type = element->Attribute("type");
    CGameObject* object = NULL;

    static std::unordered_map<std::string, CGameObject* (*)()> fabrics =
    {
    { "Mario",             goFabric<CMario> },
    { "Goomba",            goFabric<CGoomba> },
    { "Koopa",             goFabric<CKoopa> },
    { "HammerBro",         goFabric<CHammerBro> },
    { "Bowser",            goFabric<CBowser> },
    { "BuzzyBeetle",       goFabric<CBuzzyBeetle> },
    { "LakitySpawner",     goFabric<CLakitySpawner> },
    { "CheepCheep",        goFabric<CCheepCheep> },
    { "Blooper",           goFabric<CBlooper> },
    { "CheepCheepSpawner", goFabric<CCheepCheepSpawner>},
    { "BulletBillSpawner", goFabric<CBulletBillSpawner> },
    { "PiranhaPlant",      goFabric<CPiranhaPlant> },
    { "Podoboo",           goFabric<CPodoboo> },
    { "Coin",              goFabric<CCoin> },
    { "Jumper",            goFabric<CJumper> },
    { "FireBar",           goFabric<CFireBar> },
    { "MoveablePlatform",  goFabric<CMoveablePlatform> },
    { "FallingPlatform",   goFabric<CFallingPlatform> },
    { "PlatformSystem",    goFabric<CPlatformSystem> },
    { "Background",        goFabric<CBackground> },
    { "LevelPortal",       goFabric<CLevelPortal> },
    { "EndLevelFlag",      goFabric<CEndLevelFlag> },
    { "EndLevelKey",       goFabric<CEndLevelKey> },
    { "CastleFlag",        goFabric<CCastleFlag> },
    { "Princess",          goFabric<CPrincess> }
};

    auto object_fabric = fabrics[obj_type];
    assert(object_fabric); //there is no fabric for such object
    object = object_fabric();

    auto properties = parseProperties(element);
    for (auto& property : properties)
        object->setProperty(property.first, property.second);

    return object;
}

CBlocks* parseBlocks(tinyxml2::XMLElement* element)
{
    tinyxml2::XMLElement* data = element->FirstChildElement("layer")->FirstChildElement("data");

    //convert cvs format to string, where each char definite type of block
    std::string dirty_string = data->GetText();
    std::string new_string, buf;
    for (size_t i = 0; i < dirty_string.size(); ++i)
    {
        if (isdigit(dirty_string[i]))
            buf += dirty_string[i];
        else if (!buf.empty())
        {
            new_string += (char)toInt(buf);
            buf.clear();
        }
    }

    static auto blocks_fabric = [](char c) -> AbstractBlock*
    {
        switch (c)
        {
        case(0): return NULL;
        case(AbstractBlock::BRICK_TILE_CODE): return new CBricksBlock();
        case(AbstractBlock::MONEY_TILE_CODE): return new CMoneyBox();
        case(AbstractBlock::COIN_TILE_CODE): return new CQuestionBlock<CTwistedCoin>(AbstractBlock::COIN_TILE_CODE);
        case(AbstractBlock::MUSHROOM_TILE_CODE): return new CQuestionBlock<CMushroom>(AbstractBlock::MUSHROOM_TILE_CODE);
        case(AbstractBlock::BRICK_MUSHROOM_TILE_CODE): return new CQuestionBlock<CMushroom>(AbstractBlock::BRICK_MUSHROOM_TILE_CODE,false,true);
        case(AbstractBlock::LADDER_TILE_CODE): return new CQuestionBlock<CLadder>(AbstractBlock::LADDER_TILE_CODE);
        case(AbstractBlock::INVIZ_LADDER_TILE_CODE): return new CQuestionBlock<CLadder>(AbstractBlock::INVIZ_LADDER_TILE_CODE, true);
        case(AbstractBlock::BRICK_LADDER_TILE_CODE): return new CQuestionBlock<CLadder>(AbstractBlock::BRICK_LADDER_TILE_CODE, false,true);
        case(AbstractBlock::INIZ_UP_TILE_CODE): return new CQuestionBlock<COneUpMushroom>(AbstractBlock::INIZ_UP_TILE_CODE, true);
        case(AbstractBlock::INIZ_COIN_CODE): return new CQuestionBlock<CTwistedCoin>(AbstractBlock::INIZ_COIN_CODE, true);
        case(AbstractBlock::LIVE_UP_CODE): return new CQuestionBlock<COneUpMushroom>(AbstractBlock::LIVE_UP_CODE, false, true);
        case(AbstractBlock::STAR_CODE): return new CQuestionBlock<CStar>(AbstractBlock::STAR_CODE, false, true);
        default: return new CStaticBlock(c);
        }
    };

    auto blocks = new CBlocks(toInt(element->Attribute("width")),
                              toInt(element->Attribute("height")),
                              toInt(element->Attribute("tilewidth")));
    blocks->loadFromString(new_string, blocks_fabric);
    return blocks;
}

void CMarioGameScene::loadFromFile(const std::string& filepath)
{
    setName("MarioGameScene");
    m_level_name = filepath;
    auto it1 = --m_level_name.end();

    while (*it1 != '.')
        it1--;
    auto it2 = it1;
    while (*it2 != '/' || it2 == m_level_name.begin())
        it2--;
    m_level_name = std::string(++it2, it1);

    clear();
    tinyxml2::XMLDocument documet;
    bool status = documet.LoadFile(filepath.c_str());
    assert(status == tinyxml2::XML_SUCCESS); // cant load file
    tinyxml2::XMLElement* root_element = documet.FirstChildElement();

    //Load tilemap
    m_blocks = parseBlocks(root_element);
    addObject(m_blocks);

    //Load objects
    tinyxml2::XMLElement* objects = root_element->FirstChildElement("objectgroup");
    for (auto obj = objects->FirstChildElement("object"); obj != NULL; obj = obj->NextSiblingElement())
    {
        auto object = parseGameObject(obj);
        if (object)
            addObject(object);
    }

    m_mario = findObjectByType<CMario>();
    m_mario->moveToFront();
    assert(m_mario); // no mario object in scene
    setCameraOnTarget();

}

const std::string& CMarioGameScene::getLevelName() const
{
    return m_level_name;
}

void CMarioGameScene::init()
{
    setName("MarioGameScene");
    m_view.setSize(screen_size);
    CMarioGame::instance()->eventManager().subscribe(this);
}

CMarioGameScene::CMarioGameScene()
{
    init();
}

CMarioGameScene::CMarioGameScene(const std::string& filepath)
{
    loadFromFile(filepath);
    init();
}

void CMarioGameScene::update(int delta_time)
{
    CGameObject::update(delta_time);

    Vector camera_pos = m_view.getCenter();
    const Vector delta = (m_mario->getBounds().center() - camera_pos)*delta_time;
    camera_pos.x += delta.x *0.0075f;
    camera_pos.y += delta.y *0.0005f;
    camera_pos = m_mario->getBounds().center();
    camera_pos.x = math::clamp(camera_pos.x, screen_size.x / 2.f, m_blocks->width() - screen_size.x / 2.f);
    camera_pos.y = math::clamp(camera_pos.x, screen_size.y / 2.f, m_blocks->height() - screen_size.y / 2.f);
    m_view.setCenter(camera_pos);
}

void CMarioGameScene::draw(sf::RenderWindow* render_window)
{
    render_window->setView(m_view);
    CGameObject::draw(render_window);
    render_window->setView(render_window->getDefaultView());
}

void CMarioGameScene::events(const sf::Event& event)
{
    if (event.type == sf::Event::Resized)
    {
        //screen_size = Vector(event.size.width / scale_factor, event.size.height / scale_factor);
        //m_view.setSize(screen_size);
    }
}

Vector CMarioGameScene::pointToScreen(const Vector& vector)
{
    return (vector - (Vector(m_view.getCenter()) - Vector(m_view.getSize()) / 2))*scale_factor;
}

Vector CMarioGameScene::screenToPoint(const Vector& vector)
{
    return vector / scale_factor + (Vector(m_view.getCenter()) - Vector(m_view.getSize()) / 2);
}

Rect CMarioGameScene::cameraRect() const
{
    return Rect( Vector(m_view.getCenter()) - Vector(m_view.getSize())*0.5,m_view.getSize() );
}

void CMarioGameScene::setCameraOnTarget()
{
    m_view.setCenter(m_mario->getBounds().center());
}

CMarioGameScene::~CMarioGameScene()
{
    CMarioGame::instance()->eventManager().unsubcribe(this);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------

CMarioGUI::CMarioGUI()
{
    const int y_gui_pos = 5;
    m_score_lab = new CLabel();
    m_score_lab->setFontName(*MarioGame().fontManager().get("some_font"));
    m_score_lab->setPosition({ 70,y_gui_pos });
    m_score_lab->setFontStyle(sf::Text::Bold);
    m_score_lab->setFontColor({255,255,220});
    m_score_lab->setFontSize(40);
    m_score_lab->setTextAlign(CLabel::left);
    addObject(m_score_lab);

    m_coin_counter_lab = createLabel();
    m_coin_counter_lab->setPosition({ 490,y_gui_pos });
    addObject(m_coin_counter_lab);

    m_world_lab = createLabel();
    m_world_lab->setPosition({ 720,y_gui_pos });
    addObject(m_world_lab);

    m_timer = createLabel();
    m_timer->setPosition({ 1080,y_gui_pos });
    addObject(m_timer);

    m_level_name = createLabel();
    m_level_name->setPosition(MarioGame().screenSize() / 2.f + Vector::up*100.f);
    m_level_name->setTextAlign(CLabel::center);
    addObject(m_level_name);

    m_lives = createLabel();
    m_lives->setPosition(MarioGame().screenSize() / 2.f + Vector(-15,-18));
    addObject(m_lives);

    auto texture = MarioGame().textureManager().get("Mario");
    m_mario_pix = new Animator();
    m_mario_pix->create("small", *texture, { 0,96,32,32 });
    m_mario_pix->setSpriteOffset("small", 0, Vector::down*22.f);
    m_mario_pix->create("big", *texture, { 0,32,32,64 });
    m_mario_pix->setPosition(MarioGame().screenSize() / 2.f + Vector(-64,-44));
    m_mario_pix->play("big");
    m_mario_pix->scale(1.3f, 1.3f);
    m_mario_pix->hide();
    addObject(m_mario_pix);
    m_fire_pallete.create({ sf::Color(202,77,62), sf::Color(132,133,30) }, { sf::Color(255,255,255),sf::Color(202,77,62) });

    m_flow_text = new CFlowText(*MarioGame().fontManager().get("main_font"));
    m_flow_text->setTextColor(sf::Color::Red);
    m_flow_text->setSplashVector({ 0,-3 });
    m_flow_text->setTextSize(14);
    addObject(m_flow_text);

    m_game_logo = new CLabel(sf::Sprite(*CMarioGame::instance()->textureManager().get("Logo"), { 0,0, 750,300 }));
    m_game_logo->setPosition(280, 70);
    addObject(m_game_logo);

    m_menu_selector = new CLabel(sf::Sprite(*CMarioGame::instance()->textureManager().get("Items"), { 128,150,32,32 }));
    m_menu_selector->setPosition(450, 435);
    addObject(m_menu_selector);

    m_one_player_lab = createLabel();
    m_one_player_lab->setPosition(MarioGame().screenSize() / 2.f + Vector::down*80.f);
    m_one_player_lab->setTextAlign(CLabel::center);
    m_one_player_lab->setString("1 PLAYER GAME");
    m_one_player_lab->setFontColor(sf::Color(22, 85,122));
    addObject(m_one_player_lab);

    m_two_player_lab = m_one_player_lab->clone();
    m_two_player_lab->setString("2 PLAYER GAME");
    m_two_player_lab->setPosition(MarioGame().screenSize() / 2.f + Vector::down*135.f);
    addObject(m_two_player_lab);

    m_one_player_lab->setFontColor({ 0,0,0 });

    m_coin = new Animator();
    m_coin->create("twist", *MarioGame().textureManager().get("Items"), Vector(0, 84), Vector(32, 32), 4, 1, 0.01);
    m_coin->play("twist");
    m_coin->setPosition(458, 15);
    addObject(m_coin);

    m_game_over_lab = createLabel();
    m_game_over_lab->setPosition(MarioGame().screenSize() / 2.f);
    m_game_over_lab->setTextAlign(CLabel::center);
    m_game_over_lab->setString("GAME OVER");
    addObject(m_game_over_lab);
}

void CMarioGUI::setMarioRank(MarioRank rank)
{
    if (rank == MarioRank::small)
    {
        m_mario_pix->play("small");
    }
    else
        m_mario_pix->play("big");

    if (rank == MarioRank::fire)
        m_mario_pix->setPallete(&m_fire_pallete);
    else
        m_mario_pix->setPallete(NULL);
}

void CMarioGUI::update(int delta_time)
{
    CGameObject::update(delta_time);

    if (m_state == GUIState::menu)
    {
        m_tmr += delta_time;
        float x = 0.85 + 0.15*cos(m_tmr*0.01);
        m_menu_selector->setPosition(450 + 16*(1-x), 435 + 16 *(1-x));
        m_menu_selector->getSprite().setScale(x,x);
    }

}

void CMarioGUI::postDraw(sf::RenderWindow* render_window)
{

}

void CMarioGUI::draw(sf::RenderWindow* render_window)
{ 
    CGameObject::draw(render_window);
}

void CMarioGUI::setScore(int value)
{
    std::stringstream str_stream;
    str_stream << "MARIO: " << std::setw(6) << std::setfill('0') << value;
    m_score_lab->setString(str_stream.str());
}

void CMarioGUI::setCoins(int value)
{
    std::stringstream str_stream;
    str_stream << "x" << std::setw(2) << std::setfill('0') << value;
    m_coin_counter_lab->setString(str_stream.str());
}

void CMarioGUI::setGameTime(int time)
{
    std::stringstream str_stream;
    str_stream << "TIME: " << std::setw(3) << std::setfill('0') << time;
    m_timer->setString(str_stream.str());
}

CFlowText* CMarioGUI::createFlowText()
{
    return m_flow_text->clone();
}

CLabel* CMarioGUI::createLabel()
{
    return m_score_lab->clone();
}

void CMarioGUI::setState(GUIState state)
{
    m_state = state;
    CGameObject* states_labs[3][5] =
    {
        { m_level_name, m_lives, m_mario_pix, NULL, NULL}, //status
        { m_game_logo,m_one_player_lab,m_menu_selector,m_two_player_lab, NULL }, //menu
        { m_game_over_lab,NULL,NULL,NULL,NULL } //game over
    };

    for (auto state_labs : states_labs)
        for (int i = 0; state_labs[i]; ++i)
            state_labs[i]->turnOff();

    if (state == GUIState::normal)
        return;

    auto  state_labs = states_labs[(int)state - 1];
    for (int i = 0; state_labs[i]; ++i)
        state_labs[i]->turnOn();
}

void CMarioGUI::setLevelName(const std::string& string)
{
    m_level_name->setString(string);
    m_world_lab->setString(string);
}

void CMarioGUI::setLives(int value)
{
    m_lives->setString("  x  " + toString(value));
}




