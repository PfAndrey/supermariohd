#ifndef RTIIX_HPP
#define RTIIX_HPP

/**
* @brief Macro for declaring type information.
*        Must be used in the class definition that derived from TypeIdentifiable.
* @param BaseType - base class type
*/
#define DECLARE_TYPE_INFO(BaseType)                               \
protected:                                                        \
    static size_t staticTypeId() {                                \
        static const size_t id = reinterpret_cast<size_t>(&id);   \
        return id;                                                \
    }                                                             \
    size_t typeId() const override {                              \
        return staticTypeId();                                    \
    }                                                             \
    bool isIdOfBaseType(size_t id) const override {               \
        return (id == BaseType::typeId() ||                       \
                BaseType::isIdOfBaseType(id));                    \
    }                                                             \
friend class TypeIdentifiable;                                    \
private:                                                          \
    static const char* typeName() { return #BaseType; }           \
    typedef BaseType Super;                                       \

/// @brief Base class for type identification.
class TypeIdentifiable
{
public:

    /**
    * @brief Checks if the object is an instance of the specified type.
    * @tparam T - type to check
    * @return true if the object is an instance of the specified type
    */
    template <typename T>
    bool isInstanceOf() const {
        return (typeId() == T::staticTypeId());
    }

    /**
    * @brief Checks if the object is a base class of the specified type.
    * @tparam T - type to check
    * @return true if the object is a base class of the specified type
    */
    template <typename T>
    bool isBaseClassOf() const {
        return isIdOfBaseType(T::staticTypeId());
    }

    /**
    * @brief Checks if the object is of the specified type (instance or derived from).
    * @tparam T - type to check
    * @return true if the object is of the specified type
    */
    template <typename T>
    bool isTypeOf() const {
        return (isInstanceOf<T>() || isBaseClassOf<T>());
    }

    /**
    * @brief Casts the object to the specified type.
    * @tparam T - type to cast
    * @param safeCast [in] - if true, checks if the object is of the specified type
    * @return pointer to the object of the specified type or nullptr if the object is not of the specified type (if check is true)
    */
    template <typename T>
    T* castTo(bool safeCast = false) {
        if (safeCast && !isTypeOf<T>()) {
            return nullptr;
        }
        return static_cast<T*>(this);
    }

protected:

    static size_t staticTypeId()  { return 0; }
    virtual size_t typeId() const { return 0; }
    virtual bool isIdOfBaseType(size_t) const { return false; }
};

#endif // !RTIIX_HPP
