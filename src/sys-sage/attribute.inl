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
    void Attribute<T>::_ToJson(nlohmann::json &obj) const
    {
        // check at compile time whether the attribute supports JSON serialization
        if constexpr (requires { nlohmann::json(value); })
            obj = nlohmann::json(value);
        else
            obj = nlohmann::json();
    }
}
