#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

#include <functional>
#include <list>
#include <map>

#include "Property.hpp"
#include "Rect.hpp"
#include <RTIIX.hpp>
#include "Vector.hpp"

class GameObject : public TypeIdentifiable {

public:
    GameObject() = default;
    virtual ~GameObject();

    // properties
    void setName(const std::string& name);
    const std::string& getName() const;
    void setProperty(const std::string& name, const Property& property);
    Property getProperty(const std::string& name) const;

    // hierarchy
    void setParent(GameObject* game_object);
    GameObject* getParent() const;
    GameObject* addChild(GameObject* object);
    const std::list<GameObject*>& getChilds() const;
    void removeChildObject(GameObject* obj);
    void removeChildObjects();
    void removeLater();

    template <typename T>
    T* findChildObjectByType() {
        for (auto& obj : m_childObjects) {
            if (obj->isTypeOf<T>()) {
                return obj->castTo<T>();
            }
        }
        return nullptr;
    }

    template <typename T>
    std::vector<T*> findChildObjectsByType(bool findDeep = false) {
        std::vector<T*> objects;
        for (auto& obj : m_childObjects) {
            if (obj->isTypeOf<T>()) {
                objects.push_back(obj->castTo<T>());
            }

            if (findDeep) {
                auto objects_temp = obj->findChildObjectsByType<T>(findDeep);
                if (!objects_temp.empty()) {
                    objects.insert(objects.end(), objects_temp.begin(), objects_temp.end());
                }
            }
        }
        return objects;
    }

    template <typename T = GameObject>
    T* findChildObjectByName(const std::string& name) {
        auto it = std::find_if(m_childObjects.begin(), m_childObjects.end(),
            [this, &name](const GameObject* obj) {
                return obj->getName() == name;
            });

        if (it != m_childObjects.end()) {
            return (*it)->template castTo<T>(true);
        }

        return nullptr;
    }

    // order
    void moveToBack();
    void moveToFront();
    void moveUnderTo(GameObject* obj);

    static void invokePreupdateActions();

    void start();
    virtual void update(int delta_time);
    virtual void events(const sf::Event& event) {};

    void enable();
    void disable();
    bool isEnabled() const;

    void turnOn();
    void turnOff();

    // transform
    const Vector& getPosition() const;
    void setPosition(const Vector& point);
    void setPosition(float x, float y);
    void move(const Vector& point);

    // draw / appearance
    void hide();
    void show();
    bool isVisible() const;
    virtual void draw(sf::RenderWindow* window);

    // collisions
    virtual Rect getBounds() const;
    virtual void setBounds(const Rect& rect);
    void setSize(const Vector& size);

protected:
    virtual void onStarted() {};
    virtual void onParentSet() {};
    virtual void onPropertySet(const std::string& name);
    virtual void onPropertyGet(const std::string& name) const;
    virtual void onPositionChanged(const Vector& new_pos, const Vector& old_pos) {};

private:
    std::string m_name;
    std::map<std::string, Property> m_properties;
    GameObject* m_parentObject = nullptr;
    std::list<GameObject*> m_childObjects;
    bool m_enabled = true;
    bool m_visible = true;
    Vector m_pos;
    Vector m_size;
    static std::vector<std::function<void()>> m_preupdate_actions;
    bool m_started = false;
};

#endif // GAME_OBJECT_HPP
