#ifndef CAT_AWARE_CPP
#define CAT_AWARE_CPP

#include "defines.hpp"
#ifdef CAT_AWARE
//code written based on https://github.com/intel/intel-cmt-cat/blob/master/examples/c/CAT_MBA/allocation_app_l3cat.c

#include <pqos.h>
#include <cstring> //memset
#include <limits> //numeric_limits

#include "Topology.hpp"

using namespace std;


#define TIME_MEASUREMENT
#ifdef TIME_MEASUREMENT
#include <chrono>
    std::chrono::high_resolution_clock::time_point cat_third_party_t_start, cat_third_party_t_end;
    std::chrono::high_resolution_clock::time_point cat_sys_sage_t_start, cat_sys_sage_t_end;
    std::chrono::high_resolution_clock::time_point cat_total_t_start, cat_total_t_end;
    uint64_t cat_sys_sage_time;
    uint64_t cat_third_party_time;
    uint64_t cat_total_time;
#endif

//l3cat_ids, l3cat_id_count
uint64_t getCOSL3Bitmask(unsigned int socketId, unsigned int catCosId, unsigned * socketIdArray, unsigned socketIdArraySz)
{
    struct pqos_l3ca tab[PQOS_MAX_L3CA_COS]; //TODO fill this once per socket and reuse

    //find index of my socket in the socketIdArray
    int socketIndex = -1;
    for(unsigned i = 0; i< socketIdArraySz; i++){
        if (socketIdArray[i] == socketId){
            socketIndex = i;
            break;
        }
    }
    if(socketIndex == -1)
        return std::numeric_limits<uint64_t>::max();

    unsigned num = 0;
    #ifdef TIME_MEASUREMENT
        cat_third_party_t_start = std::chrono::high_resolution_clock::now();
    #endif
    int ret = pqos_l3ca_get(socketIndex, PQOS_MAX_L3CA_COS, &num, tab);
    #ifdef TIME_MEASUREMENT
        cat_third_party_t_end = std::chrono::high_resolution_clock::now();
        cat_third_party_time += cat_third_party_t_end.time_since_epoch().count()-cat_third_party_t_start.time_since_epoch().count();
    #endif
    if (ret == PQOS_RETVAL_OK)
    {
        for (unsigned n = 0; n < num; n++) {
            if(tab[n].class_id == catCosId){
                //cout << "-----returning socket " << socketId << " COS " << catCosId << " - found mask 0x" << hex << tab[n].u.ways_mask << dec << endl;
                return tab[n].u.ways_mask;
            }
        }
    }
    return std::numeric_limits<uint64_t>::max();
}

uint64_t getCoreCOS(unsigned int socketId, unsigned int coreId, unsigned * socketIdArray, unsigned socketIdArraySz, const struct pqos_cpuinfo *p_cpu)
{
    //find index of my socket in the socketIdArray
    int socketIndex = -1;
    for(unsigned i = 0; i< socketIdArraySz; i++){
        if (socketIdArray[i] == socketId){
            socketIndex = i;
            break;
        }
    }
    if(socketIndex == -1)
        return std::numeric_limits<uint64_t>::max();

    unsigned coreIdArraySz = 0;
    #ifdef TIME_MEASUREMENT
        cat_third_party_t_start = std::chrono::high_resolution_clock::now();
    #endif
    unsigned *coreIdArray = pqos_cpu_get_cores(p_cpu, socketIndex, &coreIdArraySz);
    #ifdef TIME_MEASUREMENT
        cat_third_party_t_end = std::chrono::high_resolution_clock::now();
        cat_third_party_time += cat_third_party_t_end.time_since_epoch().count()-cat_third_party_t_start.time_since_epoch().count();
    #endif
    if (coreIdArray == NULL || coreIdArray == 0) {
        printf("Error retrieving core information!\n");
        free(coreIdArray);
        free(socketIdArray);
        return std::numeric_limits<uint64_t>::max();
    }

    //find index of my core in coreIdArray
    int coreIndex = -1;
    for(unsigned i = 0; i< coreIdArraySz; i++){
        if (coreIdArray[i] == coreId){
            coreIndex = i;
            break;
        }
    }
    if(coreIndex == -1)
        return std::numeric_limits<uint64_t>::max();

    unsigned class_id = 0;
    #ifdef TIME_MEASUREMENT
        cat_third_party_t_start = std::chrono::high_resolution_clock::now();
    #endif
    int ret = pqos_alloc_assoc_get(coreIndex, &class_id);
    #ifdef TIME_MEASUREMENT
        cat_third_party_t_end = std::chrono::high_resolution_clock::now();
        cat_third_party_time += cat_third_party_t_end.time_since_epoch().count()-cat_third_party_t_start.time_since_epoch().count();
    #endif
    if (ret == PQOS_RETVAL_OK){
        //cout << "--returning socket " << socketId << " core " << coreId << "COS " << class_id << endl;
        return class_id;
    }
    return std::numeric_limits<uint64_t>::max();
}

int Node::UpdateL3Partitioning(){

    #ifdef TIME_MEASUREMENT
        cat_total_t_start = std::chrono::high_resolution_clock::now();
    #endif
    struct pqos_config cfg;
    const struct pqos_cpuinfo *p_cpu = NULL;
    const struct pqos_cap *p_cap = NULL;
    unsigned l3cat_id_count, *p_l3cat_ids = NULL;
    int ret;

    #ifdef TIME_MEASUREMENT
        cat_third_party_t_start = std::chrono::high_resolution_clock::now();
    #endif
    memset(&cfg, 0, sizeof(cfg));

    /* PQoS Initialization - Check and initialize CAT and CMT capability */
    ret = pqos_init(&cfg);
    if (ret != PQOS_RETVAL_OK) {
            std::cerr << "Error initializing PQoS library!" << std::endl;
            return 0;
    }
    /* Get CMT capability and CPU info pointer */
    ret = pqos_cap_get(&p_cap, &p_cpu);
    if (ret != PQOS_RETVAL_OK) {
            std::cerr << "Error retrieving PQoS capabilities!" << std::endl;
            return 0;
    }
    /* Get CPU l3cat id information to set COS */
    p_l3cat_ids = pqos_cpu_get_l3cat_ids(p_cpu, &l3cat_id_count);
    if (p_l3cat_ids == NULL) {
            std::cerr << "Error retrieving CPU socket information!" << std::endl;
            return 0;
    }
    #ifdef TIME_MEASUREMENT
        cat_third_party_t_end = std::chrono::high_resolution_clock::now();
        cat_third_party_time = cat_third_party_t_end.time_since_epoch().count()-cat_third_party_t_start.time_since_epoch().count();
    #endif

    vector<Chip*> sockets;
    GetSubcomponentsByType((vector<Component*>*)&sockets, SYS_SAGE_COMPONENT_CHIP);
    for(Chip * socket : sockets)
    {
        //std::cout << "socket " << socket->GetComponentTypeStr() << " id " << socket->GetId() << std::endl;
        vector<Thread*> threads;
        socket->GetSubcomponentsByType((vector<Component*>*)&threads, SYS_SAGE_COMPONENT_THREAD);
        for(Thread* thread: threads)
        {
            //std::cout << "  thread " << thread->GetComponentTypeStr() << " id " << thread->GetId() << std::endl;
            uint64_t* cos = new uint64_t();
            uint64_t* mask = new uint64_t();
            *cos = getCoreCOS(socket->GetId(), thread->GetId(), p_l3cat_ids, l3cat_id_count, p_cpu);
            if(*cos == std::numeric_limits<uint64_t>::max()){
                cerr << "getCoreCOS failed" << endl;
                continue;
            }
            *mask = getCOSL3Bitmask(socket->GetId(), *cos, p_l3cat_ids, l3cat_id_count);
            if(*mask == std::numeric_limits<uint64_t>::max()){
                cerr << "getCOSL3Bitmask failed" << endl;
                continue;
            }

            //find L3 cache above the threads
            Component* c = (Component*)thread;
            while(c->GetParent() != NULL){
                //go up until L3 found
                c = c->GetParent();
                if(c->GetComponentType() == SYS_SAGE_COMPONENT_CACHE && ((Cache*)c)->GetCacheLevel() == 3)
                    break;
            };
            if(c==NULL || c->GetComponentType() != SYS_SAGE_COMPONENT_CACHE){
                cerr << "L3 cache not found" << endl; continue;
            }

            DataPath* d = thread->GetDpByType(SYS_SAGE_DATAPATH_TYPE_L3CAT, SYS_SAGE_DATAPATH_BIDIRECTIONAL);
            if(d == NULL)
            {   //no DP exists between this thread and L3; create a new one
                d = new DataPath(thread, c, SYS_SAGE_DATAPATH_BIDIRECTIONAL, SYS_SAGE_DATAPATH_TYPE_L3CAT);
            }
            d->attrib.insert({"CATcos", (void*)cos});
            d->attrib.insert({"CATL3mask", (void*)mask});
        }
    }
    #ifdef TIME_MEASUREMENT
        cat_total_t_end = std::chrono::high_resolution_clock::now();
        cat_total_time = cat_total_t_end.time_since_epoch().count()-cat_total_t_start.time_since_epoch().count();
        cout << "cat_total_time, " << cat_total_time << ", ";
        cout << "cat_third_party_time, " << cat_third_party_time << ", ";
    #endif
    return 1;
}

long long Thread::GetDynamicL3Size()
{
    //look for dp_outgoing where attrib contains "CATL3mask"
    for(auto it = std::begin(dp_outgoing); it != std::end(dp_outgoing); ++it)
    {
        DataPath* dp = *it;
        auto search = dp->attrib.find("CATL3mask");
        if (search == dp->attrib.end()) {
            continue;
        }
        uint64_t* mask = (uint64_t*)search->second;

        Cache* c = (Cache*)dp->GetTarget();
        int available_cache_associativity_ways = 0;
        for(int bit = 0; bit<c->GetCacheAssociativityWays(); bit++){
            if((*mask & (1<<bit)) == (uint64_t)(1<<bit)){
                available_cache_associativity_ways++;
            }
        }
        //cout << "GetDynamicL3Size: size " << c->GetCacheSize() << " tot_ways " << c->GetCacheAssociativityWays() << ", available ways " << available_cache_associativity_ways << endl;
        return c->GetCacheSize() / c->GetCacheAssociativityWays() * available_cache_associativity_ways ;
    }

    Component* c = (Component*)this;
    while(c->GetParent() != NULL){
        //go up until L3 found
        c = c->GetParent();
        if(c->GetComponentType() == SYS_SAGE_COMPONENT_CACHE && ((Cache*)c)->GetCacheLevel() == 3)
            return ((Cache*)c)->GetCacheSize();
    };
    return -1;
}

#endif //CAT_AWARE
#endif //CAT_AWARE_CPP
