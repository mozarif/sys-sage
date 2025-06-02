#ifndef CORE_HPP
#define CORE_HPP

#include "Component.hpp"

namespace sys_sage {

    /**
    Class Core - represents a CPU core, or a GPU streaming multiprocessor.
    \n This class is a child of Component class, therefore inherits its attributes and methods.
    */
    class Core : public Component {
    public:
        /**
        Core constructor (no automatic insertion in the Component Tree). Sets:
        @param _id = id, default 0
        @param _name = name, default "Core"
        @param componentType=>SYS_SAGE_COMPONENT_CORE
        */
        Core(int _id = 0, std::string _name = "Core");
        /**
        Core constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "Core"
        @param componentType=>SYS_SAGE_COMPONENT_CORE
        */
        Core(Component * parent, int _id = 0, std::string _name = "Core");
        /**
        * @private
        * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
        */
        ~Core() override = default;
    private:

    #ifdef PROC_CPUINFO
    public:
        /**
        * Refreshes the frequency of the core.
        */
        int RefreshFreq(bool keep_history = false);

        /**
        * Sets the frequency of the core.
        */
        void SetFreq(double _freq);

        /**
        * Gets the frequency of the core.
        */
        double GetFreq() const;
    private:
        double freq;
    #endif
    };
}

#endif //CORE_HPP
