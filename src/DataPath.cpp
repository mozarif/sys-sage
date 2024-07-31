#include "DataPath.hpp"

#include <cstdint>
#include <algorithm>

DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, int _type){
    return NewDataPath(_source,_target,_oriented,_type,(double)-1,(double)-1);
}
DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, double _bw, double _latency){
    return NewDataPath(_source,_target,_oriented,SYS_SAGE_DATAPATH_TYPE_NONE,_bw,_latency);
}
DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, int _type, double _bw, double _latency)
{
    DataPath* p = new DataPath(_source, _target, _oriented, _type, _bw, _latency);
    return p;
}

Component * DataPath::GetSource() {return source;}
Component * DataPath::GetTarget() {return target;}
double DataPath::GetBw() {return bw;}
double DataPath::GetLatency() {return latency;}
int DataPath::GetDpType() {return dp_type;}
int DataPath::GetOriented() {return oriented;}

DataPath::DataPath(Component* _source, Component* _target, int _oriented, int _type): DataPath(_source, _target, _oriented, _type, -1, -1) {}
DataPath::DataPath(Component* _source, Component* _target, int _oriented, double _bw, double _latency): DataPath(_source, _target, _oriented, SYS_SAGE_DATAPATH_TYPE_NONE, _bw, _latency) {}
DataPath::DataPath(Component* _source, Component* _target, int _oriented, int _type, double _bw, double _latency): source(_source), target(_target), oriented(_oriented), dp_type(_type), bw(_bw), latency(_latency)
{
    if(_oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL)
    {
        _source->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
        _target->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
        _source->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);
        _target->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);
    }
    else if(_oriented == SYS_SAGE_DATAPATH_ORIENTED)
    {
        _source->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
        _target->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);
    }
    else
    {
        delete this;
        return;//error
    }
}

void DataPath::DeleteDataPath()
{
    if(oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL)
    {
        std::vector<DataPath*>* source_dp_outgoing = source->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        std::vector<DataPath*>* source_dp_incoming = source->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);
        std::vector<DataPath*>* target_dp_outgoing = target->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        std::vector<DataPath*>* target_dp_incoming = target->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);

        source_dp_outgoing->erase(std::remove(source_dp_outgoing->begin(), source_dp_outgoing->end(), this), source_dp_outgoing->end());
        target_dp_outgoing->erase(std::remove(target_dp_outgoing->begin(), target_dp_outgoing->end(), this), target_dp_outgoing->end());
        source_dp_incoming->erase(std::remove(source_dp_incoming->begin(), source_dp_incoming->end(), this), source_dp_incoming->end());
        target_dp_incoming->erase(std::remove(target_dp_incoming->begin(), target_dp_incoming->end(), this), target_dp_incoming->end());
    }
    else if(oriented == SYS_SAGE_DATAPATH_ORIENTED)
    {
        std::vector<DataPath*>* source_dp_outgoing = source->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        std::vector<DataPath*>* target_dp_incoming = target->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);
        source_dp_outgoing->erase(std::remove(source_dp_outgoing->begin(), source_dp_outgoing->end(), this), source_dp_outgoing->end());
        target_dp_incoming->erase(std::remove(target_dp_incoming->begin(), target_dp_incoming->end(), this), target_dp_incoming->end());
    }
    delete this;
}


void DataPath::Print()
{
    cout << "DataPath src: (" << source->GetComponentTypeStr() << ") id " << source->GetId() << ", target: (" << target->GetComponentTypeStr() << ") id " << target->GetId() << " - bw: " << bw << ", latency: " << latency;
    if(!attrib.empty())
    {
        cout << " - attrib: ";
        for (const auto& n : attrib) {
            uint64_t* val = (uint64_t*)n.second;
            std::cout << n.first << " = " << *val << "; ";
        }
    }
    cout << endl;
}

void DataPath::DeleteRelation()
{
    DeleteDataPath();
}

QuantumGate::QuantumGate()
{
    gate_size = 1;
    type = SYS_SAGE_1Q_QUANTUM_GATE;
}

QuantumGate::QuantumGate(size_t _gate_size) : gate_size(_gate_size){}

QuantumGate::QuantumGate(size_t _gate_size, std::string _name, double _fidelity, std::string _unitary)
: gate_size(_gate_size), name(_name), fidelity(_fidelity), unitary(_unitary){}

QuantumGate::QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits)
: gate_size(_gate_size), qubits(_qubits) {}

QuantumGate::QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits, std::string _name, double _fidelity, std::string _unitary)
: gate_size(_gate_size), qubits(_qubits), name(_name), fidelity(_fidelity), unitary(_unitary) {}

void QuantumGate::SetGateProperties(std::string _name, double _fidelity, std::string _unitary)
{
    name = _name;
    fidelity = _fidelity;
    unitary = _unitary;
    SetGateType();
}


void QuantumGate::SetGateType()
{
    if(gate_size == 1)
    {
        if(name == "id") type = SYS_SAGE_QUANTUMGATE_TYPE_ID;
        else if(name == "rz") type = SYS_SAGE_QUANTUMGATE_TYPE_RZ;
        else if(name == "sx") type = SYS_SAGE_QUANTUMGATE_TYPE_SX;
        else if(name == "x") type = SYS_SAGE_QUANTUMGATE_TYPE_X;
        else type = SYS_SAGE_QUANTUMGATE_TYPE_UNKNOWN;
    }

    else if(gate_size == 2)
    {
        if(name == "cx") type = SYS_SAGE_QUANTUMGATE_TYPE_CNOT;
        else type = SYS_SAGE_QUANTUMGATE_TYPE_UNKNOWN;
    }

    else if(gate_size > 2)
    {
        if(name == "toffoli") type = SYS_SAGE_QUANTUMGATE_TYPE_TOFFOLI;
        else type = SYS_SAGE_QUANTUMGATE_TYPE_UNKNOWN;
    }

    else
    {
        type = SYS_SAGE_QUANTUMGATE_TYPE_UNKNOWN;
    }

}

int QuantumGate::GetGateType() const
{
    return type;
}

double QuantumGate::GetFidelity() const
{
    return fidelity;
}

size_t QuantumGate::GetGateSize() const
{
    return gate_size;
}

std::string QuantumGate::GetUnitary() const
{
    return unitary;
}

std::string QuantumGate::GetName() const
{
    return name;
}

void QuantumGate::Print()
{
}

void QuantumGate::DeleteRelation()
{
}
