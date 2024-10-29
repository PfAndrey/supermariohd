#include <assert.h>
#include "Property.hpp"

Property::~Property() {
    if (m_type == Type::STRING) {
        delete string_data;
    }
}

Property::Property(const Property& property) {
    setValue(property);
}

Property& Property::operator=(const Property& property) {
    setValue(property);
    return *this;
}

Property::Property(Property&& property) {
    setValue(property, true);
}

Property& Property::operator=(Property&& property) {
    setValue(property, true);
    return *this;
}

void Property::setValue(const Property& property, bool steal) {
    if (m_type == Type::STRING) {
        if (string_data != nullptr) {
            delete string_data;
            string_data = nullptr;
        }
    }

    m_type = property.m_type;

    switch (m_type) {
    case Type::FLOAT:
        float_data = property.float_data;
        break;
    case Type::INT:
        int_data = property.int_data;
        break;
    case Type::BOOL:
        bool_data = property.bool_data;
        break;
    case Type::STRING:
        if (steal) {
            string_data = property.string_data;
            const_cast<Property&>(property).string_data = nullptr;
        } else {
            string_data = new std::string(*property.string_data);
        }
        break;
    default:
        break;
    }
}

Property::Property(bool bool_value) {
    m_type = Type::BOOL;
    bool_data = bool_value;
}

Property::Property(int int_value) {
    m_type = Type::INT;
    int_data = int_value;
}

Property::Property(const std::string& string_value) {
    m_type = Type::STRING;
    string_data = new std::string(string_value);
}

Property::Property(float float_value) {
    m_type = Type::FLOAT;
    float_data = float_value;
}

bool Property::asBool() const {
    assert(m_type == Type::BOOL);
    return bool_data;
}

int Property::asInt() const {
    assert(m_type == Type::INT);
    return int_data;
}

float Property::asFloat() const {
    assert(m_type == Type::FLOAT);
    return float_data;
}

const std::string& Property::asString() const {
    assert(m_type == Type::STRING);
    return *string_data;
}

bool Property::isValid() const {
    return (m_type != Type::NONE);
}
