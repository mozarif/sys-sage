#pragma once

#include <type_traits>
#include <stdexcept>

/**
 * A container which implements RAII (Resource acquisition is initialization) for
 * automatic destructor dispatch.
 */
template <class T>
class RAII
{
    using Dtor = void (*)(T *);

    T *inner;  /** The acquired value. Is never null. */
    Dtor dtor; /** The destructor to run during de-initialization. */

public:
    /**
     * Constructs a new RAII object owning an inner value and an associated destructor.
     */
    RAII(T *inner, Dtor dtor)
        : inner{inner}, dtor{dtor}
    {
        if (inner == nullptr)
        {
            throw std::runtime_error{"Failed to acquire test resource"};
        }
    }

    /**
     * Sometimes, the destructor accepts a generic `void *` instead of the concrete value type.
     */
    RAII(T *inner, void (*dtor)(void *))
    requires(!std::is_same_v<T, void>)
        : inner{inner}, dtor{reinterpret_cast<Dtor>(dtor)}
    {
        if (inner == nullptr)
        {
            throw std::runtime_error{"Failed to acquire test resource"};
        }
    }

    RAII(const RAII &) = delete;

    /**
     * Transfer ownership.
     */
    RAII(RAII &&rhs) : inner{rhs.inner}, dtor{rhs.dtor}
    {
        rhs.dtor = nullptr;
    }

    RAII &operator=(const RAII &) = delete;

    RAII &operator=(RAII &&rhs)
    {
        inner = rhs.inner;
        dtor = rhs.dtor;
        rhs.dtor = nullptr;
    };

    ~RAII()
    {
        if (dtor != nullptr)
        {
            dtor(inner);
        }
    }

    T *operator->()
    {
        return inner;
    }

    operator T *()
    {
        return inner;
    }

    T *operator*()
    {
        return inner;
    }
};

template <class T, class D>
RAII(T *, D) -> RAII<T>;

template <class D>
RAII(void *, D) -> RAII<void>;
