#include <assert.h>
#include <iostream>

#include <Format.hpp>
#include "GameObject.hpp"

const Vector& GameObject::getPosition() const {
    return m_pos;
}

void GameObject::setPosition(const Vector& point) {
    onPositionChanged(point, m_pos);
    m_pos = point;
}

void GameObject::setPosition(float x, float y) {
    setPosition(Vector(x, y));
}

void GameObject::move(const Vector& point) {
    m_pos += point;
}

void GameObject::setSize(const Vector& size) {
    m_size = size;
}

Rect GameObject::getBounds() const {
    return Rect(m_pos,m_size);
}

void GameObject::setBounds(const Rect& rect) {
    m_pos = rect.leftTop();
    m_size = rect.size();
}

void GameObject::setParent(GameObject* game_object) {
    m_parentObject = game_object;
}

GameObject* GameObject::getParent() const
{
    return m_parentObject;
}
//---------------------------------------------------------------------------
void GameObject::update(int delta_time)
{
    if (isEnabled())
    {
        for (auto& obj : m_childObjects)
        {
            if (obj->isEnabled())
            {
                obj->update(delta_time);
            }
        }
    }
}
//---------------------------------------------------------------------------
void GameObject::start()
{
    for (auto& obj : m_childObjects)
    {
        obj->start();
    }

    if (!m_started)
    {
        m_started = true;
        onStarted();
    }
}
//---------------------------------------------------------------------------
void GameObject::setName(const std::string& name)
{
    m_name = name;
}
//---------------------------------------------------------------------------
const std::string& GameObject::getName() const
{
    return m_name;
}
//---------------------------------------------------------------------------
void GameObject::setProperty(const std::string& name, const Property& property)
{
    m_properties[name] = property;
    onPropertySet(name);
};
//---------------------------------------------------------------------------
Property GameObject::getProperty(const std::string& name) const
{
    onPropertyGet(name);
    return const_cast<GameObject*>(this)->m_properties[name];
};
//---------------------------------------------------------------------------
void GameObject::disable()
{
    m_enabled = false;
}
//---------------------------------------------------------------------------
void GameObject::enable()
{
    m_enabled = true;
}
//---------------------------------------------------------------------------
bool GameObject::isEnabled() const
{
    return m_enabled;
}
//---------------------------------------------------------------------------
void GameObject::hide()
{
    m_visible = false;
}
//---------------------------------------------------------------------------
void GameObject::show()
{
    m_visible = true;
}
//---------------------------------------------------------------------------
bool GameObject::isVisible() const
{
    return m_visible;
}
//---------------------------------------------------------------------------
void GameObject::turnOn()
{
    show();
    enable();
}
//---------------------------------------------------------------------------
void GameObject::turnOff()
{
    hide();
    disable();
}
//---------------------------------------------------------------------------
GameObject* GameObject::addChild(GameObject* object)
{
    m_childObjects.push_back(object);
    object->setParent(this);
    object->onParentSet();

    if (m_started)
    {
        object->start();
    }

    return object;
}
//---------------------------------------------------------------------------
const std::list<GameObject*>& GameObject::getChilds() const
{
    return m_childObjects;
}
//---------------------------------------------------------------------------
GameObject::~GameObject()
{
    for (auto& obj : m_childObjects)
    {
        delete obj;
    }
}
//---------------------------------------------------------------------------
void GameObject::draw(sf::RenderWindow* window)
{
    if (!isVisible())
    {
        return;
    }

    for (auto& obj : m_childObjects)
    {
        if (obj->isVisible())
        {
            obj->draw(window);
        }
    }
}
//---------------------------------------------------------------------------
void GameObject::removeChildObject(GameObject* object)
{
    auto it = std::find(m_childObjects.begin(), m_childObjects.end(), object);

    if (it != m_childObjects.end())
    {
        m_childObjects.erase(it);
        delete object;
    }
}
//---------------------------------------------------------------------------
void GameObject::onPropertySet(const std::string& name)
{
    using utils::strHash;

    switch (strHash(name.c_str()))
    {
    case strHash("x"):
        setPosition(m_properties["x"].asFloat(), getPosition().y);
        break;
    case strHash("y"):
        setPosition(getPosition().x, m_properties["y"].asFloat());
        break;
    case strHash("name"):
        setName(m_properties["name"].asString());
        break;
    default:
        break;
    }
}
//---------------------------------------------------------------------------
void GameObject::onPropertyGet(const std::string& name) const
{
}
//---------------------------------------------------------------------------
void GameObject::moveToBack()
{
    if (!getParent())
    {
        // add warning
        return;
    }

    m_preupdate_actions.push_back([this]()
    {
        auto list = &(getParent()->m_childObjects);
        auto it = std::find(list->begin(), list->end(), this);
        assert(*it == this);
        auto tmp = *it;
        it = list->erase(it);
        list->push_front(tmp);
    });
}

void GameObject::moveToFront()
{
    if (!getParent()) {
       // add warning
       return;
    }

    m_preupdate_actions.push_back([this]()
    {
        auto list = &(getParent()->m_childObjects);
        auto it = std::find(list->begin(), list->end(), this);
        assert(*it == this);
        auto tmp = *it;
        it = list->erase(it);
        list->push_back(tmp);
    });
}

void GameObject::moveUnderTo(GameObject* obj) {
    if (!getParent()) {
        // add warning
        return;
    }

    m_preupdate_actions.push_back([this, obj]()
    {
        auto list = &(getParent()->m_childObjects);
        auto this_obj = std::find(list->begin(), list->end(), this);
        auto other_obj = std::find(list->begin(), list->end(), obj);
        assert(this_obj != list->end() && other_obj != list->end());
        list->erase(this_obj);
        list->insert(other_obj, this);
    });
}
//---------------------------------------------------------------------------
void GameObject::removeChildObjects()
{
    for (auto object : m_childObjects)
    {
        delete object;
    }
    m_childObjects.clear();
}
//---------------------------------------------------------------------------
void GameObject::removeLater()
{
    m_preupdate_actions.push_back([this]()
    {
        if (getParent())
        {
            getParent()->removeChildObject(this);
        }
        else
        {
            delete this;
        }
    });
}
//---------------------------------------------------------------------------
std::vector<std::function<void()>> GameObject::m_preupdate_actions = std::vector<std::function<void()>>();
//---------------------------------------------------------------------------
void GameObject::invokePreupdateActions()
{
    for (auto& action : m_preupdate_actions)
    {
        if (action)
        {
            action();
        }
    }

    m_preupdate_actions.clear();
}

//---------------------------------------------------------------------------
// End of File
//---------------------------------------------------------------------------
