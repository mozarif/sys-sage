
#ifndef CACHE_HPP
#define CACHE_HPP

#include "Component.hpp"

namespace sys_sage {

    /**
    Class Cache - represents a data cache memories in the system (of different levels/purposes).
    \n This class is a child of Component class, therefore inherits its attributes and methods.
    */
    class Cache : public Component {
    public:
        /**
        Cache constructor (no automatic insertion in the Component Tree). Sets:
        @param _id = id, default 0
        @param _cache_level - (int) cache level (1=L1, 2=L2, ...), default 0. This value is represented as a cahce_type, i.e. either one of int cache_level or string cache_type can be used.
        @param _cache_size - size of the cache (Bytes), default 0
        @param _associativity - number of cache associativity ways, default -1
        @param _cache_line_size - size of a cache line (Bytes), default -1
        @param componentType=>SYS_SAGE_COMPONENT_CACHE
        */
        Cache(int _id = 0, int  _cache_level = 0, long long _cache_size = -1, int _associativity = -1, int _cache_line_size = -1);
        /**
        Cache constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _cache_type - (string) name/type of the cache (constant, textture, L1, ...). Only one of (int) cache_level or (string) cache_type can be used.
        @param _cache_size - size of the cache (Bytes), default 0
        @param _associativity - number of cache associativity ways, default -1
        @param _cache_line_size - size of a cache line (Bytes), default -1
        @param componentType=>SYS_SAGE_COMPONENT_CACHE
        */
        Cache(Component * parent, int _id, std::string _cache_type, long long _cache_size = 0, int _associativity = -1, int _cache_line_size = -1);
        /**
        Cache constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _cache_level - (int) cache level (1=L1, 2=L2, ...), default 0. This value is represented as a cahce_type, i.e. either one of int cache_level or string cache_type can be used.
        @param _cache_size - size of the cache (Bytes), default 0
        @param _associativity - number of cache associativity ways, default -1
        @param _cache_line_size - size of a cache line (Bytes), default -1
        @param componentType=>SYS_SAGE_COMPONENT_CACHE
        */
        Cache(Component * parent, int _id = 0, int _cache_level = 0, long long _cache_size = -1, int _associativity = -1, int _cache_line_size = -1);
        /**
        * @private
        * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
        */
        ~Cache() override = default;

        /**
        @returns cache level of this cache, assuming there's only 1 or no digit in the "cache_type" (e.g. "L1", "texture")
        */
        int GetCacheLevel() const;
        
        /**
        Sets cache level of this cache using integer value (For e.g. "1" for "L1", etc.)
        @param _cache_level - value for cache_type
        */
        void SetCacheLevel(int _cache_level);
        
        /**
        Retrieves cache name of this cache (e.g. "L1", "texture")
        @returns cache name 
        @see cache_name
        */
        const std::string& GetCacheName() const;
        
        /**
        Sets cache name of this cache (e.g. "L1", "texture")
        @param _cache_name - value for cache_type
        */
        void SetCacheName(std::string _name);

        /**
         * Retrieves size/capacity of the cache
         * @return size
         * @see size
        */
        long long GetCacheSize() const;
        /**
         * Sets size/capacity of the cache
         * @param _size = size
        */
        void SetCacheSize(long long _cache_size);
        /**
        @returns the number of the cache associativity ways of this cache
        */
        int GetCacheAssociativityWays() const;

        /**
        Sets cache associativity ways of this cache
        @param _associativity - value for cache_associativity_ways
        */
        void SetCacheAssociativityWays(int _associativity);
        /**
        @returns the size of a cache line of this cache
        */
        int GetCacheLineSize() const;
        /**
         * Sets the size of a cache line of this cache
        @param _cache_line_size = cache_line_size
        */
        void SetCacheLineSize(int _cache_line_size);
        /**
        @private
        !!Should normally not be caller from the outside!! Helper function of XML dump generation.
        @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
        */
        xmlNodePtr _CreateXmlSubtree() override;
    private:
        std::string cache_type; /**< cache level or cache type */
        long long cache_size;  /**< size/capacity of the cache */
        int cache_associativity_ways; /**< number of cache associativity ways */
        int cache_line_size; /**< size of a cache line */

    #ifdef NVIDIA_MIG
    public:
        /**
         * Gets the MIG size of the cache element.
         */
        long long GetMIGSize(std::string uuid = "") const;
    #endif
    };
}
#endif //CACHE_HPP