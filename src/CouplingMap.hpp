#ifndef COUPLINGMAP_HPP
#define COUPLINGMAP_HPP

#include "Relation.hpp"

namespace sys_sage {

   class CouplingMap : public Relation {
    public:
        //SVDOCTODO
        CouplingMap(Qubit* q1, Qubit*q2);
        CouplingMap(const std::vector<Component*>& components, int _id = 0, bool _ordered = true);
        // CouplingMap();

        // void Print();
        // void DeleteRelation();
        //SVDOCTODO
        void SetFidelity(double _fidelity);
        //SVDOCTODO
        double GetFidelity() const;
        //SVDOCTODO
        void Delete() override;
        //SVDOCTODO private
        xmlNodePtr _CreateXmlEntry() override;
    private:
        double fidelity;
    };
}
#endif //COUPLINGMAP_HPP