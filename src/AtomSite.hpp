#ifndef ATOMSITE_HPP
#define ATOMSITE_HPP

#include "QuantumBackend.hpp"

namespace sys_sage {

    //SVTODO in general check the quantum-related classes
    //SVTODO does it make sense to inherit from QuantumBackend?
    class AtomSite : public QuantumBackend{
    public:

        /**
        @private 
        !!Should normally not be used!! Helper function of XML dump generation.
        @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
        */
        xmlNodePtr _CreateXmlSubtree() override;

    //SVTODO move to private?
        struct SiteProperties {

            int nRows;
            int nColumns;
            int nAods;
            int nAodIntermediateLevels;
            int nAodCoordinates;
            double   interQubitDistance;
            double   interactionRadius;
            double   blockingFactor;   
        } properties;

        std::map <std::string, double> shuttlingTimes;
        std::map <std::string, double> shuttlingAverageFidelities;

    };
} //namespace sys_sage 
#endif //ATOMSITE_HPP