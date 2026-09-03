#ifndef ATOMSITE_HPP
#define ATOMSITE_HPP

#include <sys-sage/QuantumBackend.hpp>

namespace sys_sage {

    //SVTODO in general check the quantum-related classes
    //SVTODO does it make sense to inherit from QuantumBackend?
    /**
     * @class AtomSite
     * @brief Represents a physical site of an atom in a quantum backend.
     *
     * Used as part of the quantum hardware abstraction in sys-sage.
     * Typically associated with a QuantumBackend and may be used to model
     * the physical location or properties of an atom or qubit site.
     *
     * @note The full implementation is in the corresponding .cpp file.
     * @see QuantumBackend
     */
    class AtomSite : public QuantumBackend{
    public:

        /**
        AtomSite constructor (no automatic insertion in the Component Tree). Sets:
        @param _id = id, default 0
        @param _name = name, default "QuantumBackend"
        @param componentType=>SYS_SAGE_COMPONENT_QUANTUM_BACKEND
        */
        AtomSite(int _id = 0, const std::string &_name = "AtomSite");

        /**
        AtomSite constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "QuantumBackend"
        @param componentType=>SYS_SAGE_COMPONENT_ATOM_SITE
        */
        AtomSite(Component *parent, int _id = 0, const std::string &_name = "AtomSite");

        /**
         * @private
         * @brief (Private, internal) Helper function for XML dump generation.
         * 
         * Should normally not be used directly. Used internally for exporting the topology to XML.
         * @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL)
         * @return Pointer to the created XML subtree node.
         */
        xmlNodePtr _CreateXmlSubtree() override;

        /**
         * @private
         *
         * @brief Initializes a JSON object that represents this component.
         *        Intended for internal use.
         *
         * @param obj The JSON object to be initialized.
         */
        void _ToJson(nlohmann::json &obj) const override;

        /**
         * @private
         *
         * @brief Initializes this component through JSON. Intended for
         *        internal use.
         *
         * @param obj The JSON object containing the data.
         *
         * @return 0 on success, 1 otherwise.
         */
        int _FromJson(const nlohmann::json &obj) override;

        //SVTODO move to private?
        /**
         * @struct SiteProperties
         * @brief Properties describing the physical layout and parameters of the atom site.
         *
         * Includes geometry, AOD (Acousto-Optic Deflector) configuration, and physical interaction parameters.
         */
        struct SiteProperties {
            int nRows;                   ///< Number of rows in the site grid
            int nColumns;                ///< Number of columns in the site grid
            int nAods;                   ///< Number of AODs
            int nAodIntermediateLevels;  ///< Number of intermediate AOD levels
            int nAodCoordinates;         ///< Number of AOD coordinates
            double interQubitDistance;   ///< Distance between qubits
            double interactionRadius;    ///< Radius of interaction
            double blockingFactor;       ///< Blocking factor for site operations
        } properties;

        /**
         * @brief Shuttling times for various operations, indexed by operation name.
         */
        std::map <std::string, double> shuttlingTimes;

        /**
        * @brief Average fidelities for shuttling operations, indexed by operation name.
        */
        std::map <std::string, double> shuttlingAverageFidelities;

    };
} //namespace sys_sage 
#endif //ATOMSITE_HPP