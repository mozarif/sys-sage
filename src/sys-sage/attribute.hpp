#ifndef SYS_SAGE_SRC_ATTRIBUTE_HPP
#define SYS_SAGE_SRC_ATTRIBUTE_HPP

#include <nlohmann/json.hpp>
#include <sys-sage/comptime_str_manipulation.hpp>
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <ranges>
#include <string_view>
#include <tuple>
#include <unordered_map>

#define SYS_SAGE_STRINGIFY(...) #__VA_ARGS__
#define SYS_SAGE_EXTRACT_ARGS(...) __VA_ARGS__

// this macro handles type registration for atomic types
#define SYS_SAGE_REGISTER_TYPE_INTERNAL(type)                                       \
namespace sys_sage {                                                                \
    template <bool b>                                                               \
    struct TypeTrait<type, b> {                                                     \
        static constexpr bool serializable = HasToJson<type>;                       \
        static constexpr bool deserializable = HasFromJson<type>;                   \
                                                                                    \
        static constexpr decltype(auto) id = SYS_SAGE_STRINGIFY(type);              \
                                                                                    \
        template <typename U = type> requires (deserializable)                      \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)   \
        {                                                                           \
            return std::make_unique<Attribute<U>>(obj.get<U>());                    \
        }                                                                           \
                                                                                    \
        template <typename U = type> requires (b && serializable && deserializable) \
        inline static const auto registrar = []                                     \
        {                                                                           \
            TypeRegistry::Instance().Register(id, Deserialize);                     \
            return std::tuple<>{};                                                  \
        }();                                                                        \
                                                                                    \
        __attribute__((used, retain))                                               \
        static void UseRegistrar()                                                  \
        {                                                                           \
            if constexpr (b && serializable && deserializable)                      \
                (void) registrar<type>;                                             \
        }                                                                           \
    };                                                                              \
}

#define SYS_SAGE_REGISTER_TYPE(...) SYS_SAGE_REGISTER_TYPE_INTERNAL(SYS_SAGE_EXTRACT_ARGS(__VA_ARGS__))

namespace sys_sage {
    /**
     * @brief A concept for checking if a type supports JSON serialization.
     */
    template <typename T>
    concept HasToJson = requires (nlohmann::json &obj, const T &attr) { nlohmann::adl_serializer<T>::to_json(obj, attr); };

    /**
     * @brief A concept for checking if a type supports JSON deserialization.
     */
    template <typename T>
    concept HasFromJson = requires (const nlohmann::json &obj, T &attr) { nlohmann::adl_serializer<T>::from_json(obj, attr); }
                       || requires (const nlohmann::json &obj) { { nlohmann::adl_serializer<T>::from_json(obj) } -> std::same_as<T>; };

    /**
     * @class IAttribute
     *
     * @brief A polymorphic interface for type-erased attributes.
     */
    class IAttribute {
    public:
        /**
         * @brief Virtual destructor for correct cleanup of attributes.
         */
        virtual ~IAttribute() = default;

        /**
         * @brief Virtual JSON serialization for atttributes.
         *
         * @param obj The JSON object to be initialized to represent this attribute.
         */
        virtual void Serialize(nlohmann::json &obj) const = 0;
    };

    // forward declaration
    template <typename T, bool>
    class TypeTrait;
    
    /**
     * @class Attribute
     *
     * @brief A wrapper that stores an attribute of any type.
     */
    template <typename T>
    class Attribute : public IAttribute {
    public:
        /**
         * @brief Generic constructor using perfect forwarding.
         *
         * @param value Initializes this attribute using the given value.
         */
        template <typename U>
        Attribute(U &&_value);
    
        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        attribute.
         *
         * @return A reference to the value.
         */
        T &operator*();

        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        constant attribute.
         *
         * @return A reference to the const value.
         */
        const T &operator*() const;
    
        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        attribute.
         *
         * @return A pointer to the value.
         */
        T *operator->();

        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        constant attribute.
         *
         * @return A pointer to the constant value.
         */
        const T *operator->() const;

        /**
         * @brief JSON serialization for atttributes.
         *
         * @param obj The JSON object to be initialized to represent this attribute.
         */
        void Serialize(nlohmann::json &obj) const override;
    
    private:
        /**
         * @brief The underlying value.
         */
        T value;
    };

    /**
     * @class TypeRegistry
     *
     * @brief Manages type registration and stores deserialization callbacks functions.
     */
    class TypeRegistry {
    public:
        /**
         * @brief Returns the global static type registry.
         */
        static TypeRegistry &Instance();
    
        /**
         * @brief Registers a type.
         *
         * @param id A unique identifier for the `TypeDescriptor`.
         */
        void Register(std::string_view id, std::unique_ptr<IAttribute> (*callback)(const nlohmann::json&));
    
        /**
         * @brief Returns the deserialization callback associated to this type.
         *
         * @param id A unique identifier for the `TypeDescriptor`.
         */
        std::optional<std::unique_ptr<IAttribute>(*)(const nlohmann::json&)> GetCallBack(std::string_view id) const;
    
    private:
        /**
         * @brief Initializes the registry with some pre-defined types.
         */
        TypeRegistry();
        /**
         * @brief Maps the unique identifier of a `TypeDescriptor` to its deserialization callback routine.
         */
        std::unordered_map<std::string_view, std::unique_ptr<IAttribute>(*)(const nlohmann::json&)> callbacks;
    };

    /**
     * @class TypeTraits
     *
     * @brief Provides compile time meta data for a type.
     */
    template <typename T, bool b = true>
    struct TypeTrait {
        static constexpr bool serializable = HasToJson<T>;
        static constexpr bool deserializable = HasFromJson<T>;

        static constexpr decltype(auto) id = CompStrExtract<T>();

        template <typename U = T> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<U>>(obj.get<U>());
        }

        template <typename U = T> requires (b && serializable && deserializable)
        inline static const auto registrar = []
        {
            TypeRegistry::Instance().Register(id, Deserialize);
            return std::tuple<>{};
        }();

        __attribute__((used, retain))
        static void UseRegistrar()
        {
            if constexpr (b && serializable && deserializable)
                (void) registrar<T>;
        }
    };

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// PRE-DEFINE SOME TYPES /////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    template <bool b>
    struct TypeTrait<bool, b> {
        static constexpr bool serializable = HasToJson<bool>;
        static constexpr bool deserializable = HasFromJson<bool>;

        static constexpr decltype(auto) id = "bool";

        template <typename U = bool> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<bool>>(obj.get<bool>());
        }
    };

    template <bool b>
    struct TypeTrait<char, b> {
        static constexpr bool serializable = HasToJson<char>;
        static constexpr bool deserializable = HasFromJson<char>;

        static constexpr decltype(auto) id = "char";

        template <typename U = char> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<char>>(obj.get<char>());
        }
    };

    template <bool b>
    struct TypeTrait<signed char, b> {
        static constexpr bool serializable = HasToJson<signed char>;
        static constexpr bool deserializable = HasFromJson<signed char>;

        static constexpr decltype(auto) id = "signed char";

        template <typename U = signed char> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<signed char>>(obj.get<signed char>());
        }
    };

    template <bool b>
    struct TypeTrait<unsigned char, b> {
        static constexpr bool serializable = HasToJson<unsigned char>;
        static constexpr bool deserializable = HasFromJson<unsigned char>;

        static constexpr decltype(auto) id = "unsigned char";

        template <typename U = unsigned char> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<unsigned char>>(obj.get<unsigned char>());
        }
    };

    template <bool b>
    struct TypeTrait<short int, b> {
        static constexpr bool serializable = HasToJson<short int>;
        static constexpr bool deserializable = HasFromJson<short int>;

        static constexpr decltype(auto) id = "short int";

        template <typename U = short int> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<short int>>(obj.get<short int>());
        }
    };

    template <bool b>
    struct TypeTrait<unsigned short int, b> {
        static constexpr bool serializable = HasToJson<unsigned short int>;
        static constexpr bool deserializable = HasFromJson<unsigned short int>;

        static constexpr decltype(auto) id = "unsigned short int";

        template <typename U = unsigned short int> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<unsigned short int>>(obj.get<unsigned short int>());
        }
    };

    template <bool b>
    struct TypeTrait<int, b> {
        static constexpr bool serializable = HasToJson<int>;
        static constexpr bool deserializable = HasFromJson<int>;

        static constexpr decltype(auto) id = "int";

        template <typename U = int> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<int>>(obj.get<int>());
        }
    };

    template <bool b>
    struct TypeTrait<unsigned int, b> {
        static constexpr bool serializable = HasToJson<unsigned int>;
        static constexpr bool deserializable = HasFromJson<unsigned int>;

        static constexpr decltype(auto) id = "unsigned int";

        template <typename U = unsigned int> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<unsigned int>>(obj.get<unsigned int>());
        }
    };

    template <bool b>
    struct TypeTrait<long int, b> {
        static constexpr bool serializable = HasToJson<long int>;
        static constexpr bool deserializable = HasFromJson<long int>;

        static constexpr decltype(auto) id = "long int";

        template <typename U = long int> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<long int>>(obj.get<long int>());
        }
    };

    template <bool b>
    struct TypeTrait<unsigned long int, b> {
        static constexpr bool serializable = HasToJson<unsigned long int>;
        static constexpr bool deserializable = HasFromJson<unsigned long int>;

        static constexpr decltype(auto) id = "unsigned long int";

        template <typename U = unsigned long int> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<unsigned long int>>(obj.get<unsigned long int>());
        }
    };

    template <bool b>
    struct TypeTrait<long long int, b> {
        static constexpr bool serializable = HasToJson<long long int>;
        static constexpr bool deserializable = HasFromJson<long long int>;

        static constexpr decltype(auto) id = "long long int";

        template <typename U = long long int> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<long long int>>(obj.get<long long int>());
        }
    };

    template <bool b>
    struct TypeTrait<unsigned long long int, b> {
        static constexpr bool serializable = HasToJson<unsigned long long int>;
        static constexpr bool deserializable = HasFromJson<unsigned long long int>;

        static constexpr decltype(auto) id = "unsigned long long int";

        template <typename U = unsigned long long int> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<unsigned long long int>>(obj.get<unsigned long long int>());
        }
    };

    template <bool b>
    struct TypeTrait<float, b> {
        static constexpr bool serializable = HasToJson<float>;
        static constexpr bool deserializable = HasFromJson<float>;

        static constexpr decltype(auto) id = "float";

        template <typename U = float> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<float>>(obj.get<float>());
        }
    };

    template <bool b>
    struct TypeTrait<double, b> {
        static constexpr bool serializable = HasToJson<double>;
        static constexpr bool deserializable = HasFromJson<double>;

        static constexpr decltype(auto) id = "double";

        template <typename U = double> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<double>>(obj.get<double>());
        }
    };

    template <bool b>
    struct TypeTrait<long double, b> {
        static constexpr bool serializable = HasToJson<long double>;
        static constexpr bool deserializable = HasFromJson<long double>;

        static constexpr decltype(auto) id = "long double";

        template <typename U = long double> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<long double>>(obj.get<long double>());
        }
    };
}

#include <sys-sage/attribute.inl>

#endif
