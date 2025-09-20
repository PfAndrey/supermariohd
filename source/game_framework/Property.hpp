#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <cstdint>
#include <string>

class Property {
public:
    Property() = default;
    Property(bool bool_value);
    Property(int int_value);
    Property(const std::string& string_value);
    Property(float float_value);
    Property(const Property& property);
    Property& operator=(const Property& property);
    Property(Property&& property);
    Property& operator=(Property&& property);
    ~Property();
    bool asBool() const;
    int asInt() const;
    float asFloat() const;
    const std::string& asString() const;
    bool isValid() const;

private:

    enum class Type : uint8_t {
        NONE    = 0,
        BOOL    = 1,
        INT     = 2,
        FLOAT   = 3,
        STRING  = 4
    };

    union {
        int int_data;
        float float_data;
        std::string* string_data;
        bool bool_data;
    };

    void setValue(const Property& property, bool steal = false);
    Type m_type = Type::NONE;
};

#endif // PROPERTY_HPP
