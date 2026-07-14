#include <sys-sage/attribute.hpp>

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
