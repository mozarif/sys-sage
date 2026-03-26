namespace sys_sage {
    template <typename T>
    void Component::SetAttribute(const std::string &key, T &&value)
    {
        attributes[key] = std::make_unique<Attribute<std::decay_t<T>>>(std::forward<T>(value));
    }

    template <typename T>
    T *Component::GetAttribute(const std::string &key)
    {
        auto it = attributes.find(key);
        if (it == attributes.end())
          return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
          return nullptr;

        return &(*(*attribute));
    }

    template <typename T>
    const T *Component::GetAttribute(const std::string &key) const
    {
        auto it = attributes.find(key);
        if (it == attributes.end())
          return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
          return nullptr;

        return &(*(*attribute));
    }


    template <typename T>
    T *Component::GetAttribute(Component::attribIterator it)
    {
        if (it == attributes.end())
          return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
          return nullptr;

        return &(*(*attribute));
    }

    template <typename T>
    const T *Component::GetAttribute(Component::constAttribIterator it) const
    {
        if (it == attributes.end())
          return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
          return nullptr;

        return &(*(*attribute));
    }
}
