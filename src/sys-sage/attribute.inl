namespace sys_sage {
    template <typename T>
    template <typename U>
    Attribute<T>::Attribute(U &&_value) : value(std::forward<U>(_value)) {}
    
    template <typename T>
    T &Attribute<T>::operator*()
    {
        return value;
    }
    
    template <typename T>
    const T &Attribute<T>::operator*() const
    {
        return value;
    }
    
    template <typename T>
    T *Attribute<T>::operator->()
    {
        return &value;
    }
    
    template <typename T>
    const T *Attribute<T>::operator->() const
    {
        return &value;
    }

    template <typename T>
    void Attribute<T>::Serialize(nlohmann::json &obj) const
    {
        // check at compile time whether the attribute supports JSON serialization
        if constexpr (requires { nlohmann::json(value); }) {
            // check at compile time whether the type is registered
            if constexpr (TypeTrait<T>::registered) {
                obj = {
                    { "_sys_sage_type", TypeTrait<T>::descriptor.id },
                    { "_sys_sage_value", value }
                };
            } else {
                obj = value;
            }
        } else {
            obj = nlohmann::json();
        }
    }

    template <typename T>
    std::unique_ptr<IAttribute> TypeCallBack<T>::Deserialize(const nlohmann::json &obj)
    {
        if constexpr (valid)
            return std::make_unique<Attribute<T>>(obj.get<T>());
        else
            return nullptr;
    }

    template <typename T>
    void TypeRegistry::Register(std::string_view id)
    {
        if constexpr (TypeCallBack<T>::valid)
            callbacks.emplace(id, TypeCallBack<T>::Deserialize);
    }

    template <typename T>
    TypeRegistrar<T>::TypeRegistrar(std::string_view id)
    {
        TypeRegistry::Instance().Register<T>(id);
    }
}
