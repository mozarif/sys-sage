#ifndef SYS_SAGE_SRC_ATTRIBUTE_HPP
#define SYS_SAGE_SRC_ATTRIBUTE_HPP

namespace sys_sage {
    // polymorphic interface for type-erased attributes
    class IAttribute {
    public:
        virtual ~IAttribute() = default;
    };
    
    // wrapper that stores an attribute of any type
    template <typename T>
    class Attribute : public IAttribute {
    public:
        template <typename U>
        Attribute(U &&_value);
    
        T &operator*();
        const T &operator*() const;
    
        T *operator->();
        const T *operator->() const;
    
    private:
        T value;
    };
}

#include "attribute.inl"

#endif
