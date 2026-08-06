#include <sys-sage/attribute.hpp>

sys_sage::TypeRegistry::TypeRegistry()
{
    if constexpr (TypeTrait<bool>::serializable && TypeTrait<bool>::deserializable)
        callbacks.emplace( TypeTrait<bool>::id, TypeTrait<bool>::Deserialize<> );

    if constexpr (TypeTrait<char>::serializable && TypeTrait<char>::deserializable)
        callbacks.emplace( TypeTrait<char>::id, TypeTrait<char>::Deserialize<> );

    if constexpr (TypeTrait<signed char>::serializable && TypeTrait<signed char>::deserializable)
        callbacks.emplace( TypeTrait<signed char>::id, TypeTrait<signed char>::Deserialize<> );

    if constexpr (TypeTrait<unsigned char>::serializable && TypeTrait<unsigned char>::deserializable)
        callbacks.emplace( TypeTrait<unsigned char>::id, TypeTrait<unsigned char>::Deserialize<> );

    if constexpr (TypeTrait<short>::serializable && TypeTrait<short>::deserializable)
        callbacks.emplace( TypeTrait<short>::id, TypeTrait<short>::Deserialize<> );

    if constexpr (TypeTrait<unsigned short>::serializable && TypeTrait<unsigned short>::deserializable)
        callbacks.emplace( TypeTrait<unsigned short>::id, TypeTrait<unsigned short>::Deserialize<> );

    if constexpr (TypeTrait<int>::serializable && TypeTrait<int>::deserializable)
        callbacks.emplace( TypeTrait<int>::id, TypeTrait<int>::Deserialize<> );

    if constexpr (TypeTrait<unsigned int>::serializable && TypeTrait<unsigned int>::deserializable)
        callbacks.emplace( TypeTrait<unsigned int>::id, TypeTrait<unsigned int>::Deserialize<> );

    if constexpr (TypeTrait<long>::serializable && TypeTrait<long>::deserializable)
        callbacks.emplace( TypeTrait<long>::id, TypeTrait<long>::Deserialize<> );

    if constexpr (TypeTrait<unsigned long>::serializable && TypeTrait<unsigned long>::deserializable)
        callbacks.emplace( TypeTrait<unsigned long>::id, TypeTrait<unsigned long>::Deserialize<> );

    if constexpr (TypeTrait<long long>::serializable && TypeTrait<long long>::deserializable)
        callbacks.emplace( TypeTrait<long long>::id, TypeTrait<long long>::Deserialize<> );

    if constexpr (TypeTrait<unsigned long long>::serializable && TypeTrait<unsigned long long>::deserializable)
        callbacks.emplace( TypeTrait<unsigned long long>::id, TypeTrait<unsigned long long>::Deserialize<> );

    if constexpr (TypeTrait<float>::serializable && TypeTrait<float>::deserializable)
        callbacks.emplace( TypeTrait<float>::id, TypeTrait<float>::Deserialize<> );

    if constexpr (TypeTrait<double>::serializable && TypeTrait<double>::deserializable)
        callbacks.emplace( TypeTrait<double>::id, TypeTrait<double>::Deserialize<> );

    if constexpr (TypeTrait<long double>::serializable && TypeTrait<long double>::deserializable)
        callbacks.emplace( TypeTrait<long double>::id, TypeTrait<long double>::Deserialize<> );

    if constexpr (TypeTrait<std::string>::serializable && TypeTrait<std::string>::deserializable)
        callbacks.emplace( TypeTrait<std::string>::id, TypeTrait<std::string>::Deserialize<> );
}

sys_sage::TypeRegistry &sys_sage::TypeRegistry::Instance()
{
    static TypeRegistry registry;
    return registry;
}

void sys_sage::TypeRegistry::Register(std::string_view id, std::unique_ptr<IAttribute> (*callback)(const nlohmann::json&))
{
    callbacks.emplace(id, callback);
}

std::optional<std::unique_ptr<sys_sage::IAttribute>(*)(const nlohmann::json&)> sys_sage::TypeRegistry::GetCallBack(std::string_view id) const
{
    if (auto it = callbacks.find(id); it != callbacks.end())
        return it->second;

    return std::nullopt;
}
