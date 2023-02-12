#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <link.h>
#include <cassert>

#include "input.h"
#include "heat.h"
#include "timing.h"
#include "omp.h"
#include "mmintrin.h"
using namespace std;

////////////////
#include <papi.h>
#include "sys-sage.hpp"
#include <pthread.h>
#include <unistd.h>
#include <map>
#include <sstream>
#include <cstring>
#include <sys/wait.h>
#include <memory>
#include <chrono>
#include <unistd.h>
#include <sys/syscall.h>

#define NUM_MY_PAPI_EVENTS 4
#define MEASURE_CHILD_THREADS 1

class GreenEntry{
public:
    GreenEntry(uint64_t _time, int _threadId, double _freq, long long* _papi_counters)
    {
        time = _time;
        threadId = _threadId;
        frequency = _freq;
        std::memcpy(papi_counters, _papi_counters, NUM_MY_PAPI_EVENTS*sizeof(long long));
    };
    void PrintHeader()
    {
        cout << "time, thread, frequency, PAPI_TOT_INS, PAPI_L2_TCA, PAPI_L3_LDM, PAPI_L3_TCM" << endl;
    }
    void Print()
    {
		printf("%lld, %d, %f, %lld, %lld, %lld, %lld \n", time, threadId, frequency, papi_counters[0], papi_counters[1], papi_counters[2], papi_counters[3]);
		//cout << time << ", " << threadId << ", " << frequency;
        //cout << ", " << papi_counters[0] << ", " << papi_counters[1] << ", " << papi_counters[2] << ", " << papi_counters[3] << endl;
    };
    uint64_t time;
    int threadId;
    double frequency;
    long long papi_counters[4];
};

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void get_child_threads(int pid, std::map<int, int>* tcm)
{
	//std::map<int, int> tid_core_map = *tcm;
    string cmd_get_threadId = "ps -o tid,psr -p " + std::to_string(pid) + " -T | tail -n +2";
    string str_coreId = exec(cmd_get_threadId.c_str());

	(*tcm).clear();

	std::stringstream ss(str_coreId);
    std::string line;
	int tid, core;
	pid_t this_tid = syscall(__NR_gettid);
	//pthread_id_np_t this_tid = pthread_getthreadid_np();
    while (std::getline(ss, line)) {
        std::stringstream ss_line(line);
        ss_line >> tid;
        ss_line >> core;
		if(tid != this_tid)
			(*tcm)[tid] = core;
        //std::cout << "tid " << tid <<" on core " << (*tcm)[tid] << std::endl;
    }
    return;
}

int attach_new_papi_tid(int tid, int* eventset_array, int* eventset_occupied)
{
	int retval;
	for(int i = 0; i< MEASURE_CHILD_THREADS; i++)
	{
		if(eventset_occupied[i] == -1)//free
		{
			eventset_array[i] = PAPI_NULL;
			retval=PAPI_create_eventset( &(eventset_array[i]) );
			if (retval!=PAPI_OK) fprintf(stderr,"Error creating eventset! %s\n",PAPI_strerror(retval));
			retval+=PAPI_add_event(eventset_array[i],PAPI_TOT_INS); // PAPI_TOT_INS 0x80000032  Yes   No   Instructions completed
			retval+=PAPI_add_event(eventset_array[i],PAPI_L2_TCA); // PAPI_L2_DCA  0x80000041  Yes   No   Level 2 data cache accesses
			retval+=PAPI_add_event(eventset_array[i],PAPI_L3_LDM); // PAPI_L3_LDM  0x8000000e  Yes   No   Level 3 load misses
			retval+=PAPI_add_event(eventset_array[i],PAPI_L3_TCM); // PAPI_L3_TCM  0x80000008  Yes   No   Level 3 cache misses
			if (retval!=PAPI_OK) fprintf(stderr,"Error eventset! %s\n",PAPI_strerror(retval));

			retval=PAPI_attach(eventset_array[i], tid);
			if (retval!=PAPI_OK) fprintf(stderr,"Error attaching papi: %s\n", PAPI_strerror(retval));

            retval=PAPI_start(eventset_array[i]);
            if (retval!=PAPI_OK) fprintf(stderr,"Error starting papi: %s\n", PAPI_strerror(retval));

			eventset_occupied[i]=tid;
			cout << "attached papi eventset " << i << " to tid " << tid << endl;
			return 1;
		}
	}
	cout << "!!! no free eventset to attach tid " << tid << endl;
	return 0;
}
int detach_papi_tid(int tid, int* eventset_array, int* eventset_occupied)
{
	int retval;
    long long pca[MEASURE_CHILD_THREADS];
	for(int i = 0; i< MEASURE_CHILD_THREADS; i++)
	{
		if(eventset_occupied[i] == tid)
		{
            retval=PAPI_stop(eventset_array[i], pca);
            if (retval!=PAPI_OK) fprintf(stderr,"Error starting papi: %s\n", PAPI_strerror(retval));

			retval=PAPI_detach(eventset_array[i]);
			if (retval!=PAPI_OK) fprintf(stderr,"Error detaching:  %s\n", PAPI_strerror(retval));
 			retval = PAPI_destroy_eventset( &(eventset_array[i]));
			if (retval!=PAPI_OK) fprintf(stderr,"Error PAPI_destroy_eventset:  %s\n", PAPI_strerror(retval));

			eventset_occupied[i] = -1;
			cout << "detached papi eventset " << i << " from tid " << tid << endl;

			return 1;
		}
	}
	cout << "!!! no tid to detach found - tid: " << tid << endl;
	return 0;
}

void reattach_papi_eventsets(std::map<int, int>* tid_core_map_old, std::map<int, int>* tid_core_map, int* eventset_array, int* eventset_occupied)
{
	//check if all tids from _old are also in new
	for (auto const& x : *tid_core_map_old){
		if (tid_core_map->find(x.first) == tid_core_map->end()) {
			detach_papi_tid(x.first,eventset_array,eventset_occupied);
		}
	}
	//check if there are new tids not present in _old
	for (auto const& x : *tid_core_map){
		if (tid_core_map_old->find(x.first) == tid_core_map_old->end()) {
			attach_new_papi_tid(x.first,eventset_array,eventset_occupied);
		}
	}
	return;
}
void* green_score_thread(void* cm)
{
	volatile int* continue_measurement = (volatile int*)cm;
	//create root Topology and one node
	Topology* topo = new Topology();
	Node* n = new Node(topo, 1);
	if(parseHwlocOutput(n, "hwloc.xml") != 0) { //adds topo to a next node
		//usage(argv[0]);
		return NULL;
	}

	vector<GreenEntry>* greenEntries = new vector<GreenEntry>();

	PAPI_library_init(PAPI_VER_CURRENT);
	volatile long long ** papi_counters_array = new volatile long long*[MEASURE_CHILD_THREADS]();
	for(int i = 0; i<MEASURE_CHILD_THREADS; i++){ papi_counters_array[i] = new volatile long long[NUM_MY_PAPI_EVENTS]();}
	int* eventset = new int [MEASURE_CHILD_THREADS]();
	int* eventset_occupied = new int [MEASURE_CHILD_THREADS]();
	for(int i = 0; i< MEASURE_CHILD_THREADS; i++) {eventset_occupied[i] = -1;}


	// int* measured_cores = new int [MEASURE_CHILD_THREADS]();
	// int* measured_tids = new int [MEASURE_CHILD_THREADS]();
	int retval;

	int pid = ::getpid();
	cout << "pid of this process: " << pid << endl;

			//tid, core#
	std::map<int, int>* tid_core_map_old = new std::map<int, int>();
	std::map<int, int>* tid_core_map = new std::map<int, int>();
	std::map<int, int>* map_tmp;

	get_child_threads(pid, tid_core_map_old);
	for (auto const& x : *tid_core_map_old){
		int success = attach_new_papi_tid(x.first, eventset, eventset_occupied );
	}

	std::chrono::high_resolution_clock::time_point ts, ts_start = std::chrono::high_resolution_clock::now();
	// Wait until process exits
	do {
		n->RefreshCpuCoreFrequency();
		get_child_threads(pid, tid_core_map);
		reattach_papi_eventsets(tid_core_map_old, tid_core_map, eventset, eventset_occupied);

		//start all counters
		for(int i = 0; i< MEASURE_CHILD_THREADS; i++)
		{
			if(eventset_occupied[i] != -1)
			{
				// cout << "starting papi on tid " << eventset_occupied[i] << endl;
                retval=PAPI_reset(eventset[i]);
				//retval=PAPI_start(eventset[i]);
				if (retval!=PAPI_OK) fprintf(stderr,"Error starting papi: %s\n", PAPI_strerror(retval));
			}
		}

		usleep(100000);

		//stop all counters
		for(int i = 0; i< MEASURE_CHILD_THREADS; i++)
		{
			if(eventset_occupied[i] != -1)
			{
				//retval=PAPI_stop(eventset[i], papi_counters_array[i]);
                retval=PAPI_read(eventset[i], (long long*)papi_counters_array[i]);
				if (retval!=PAPI_OK) fprintf(stderr,"Error starting papi: %s\n", PAPI_strerror(retval));
			}
		}

		ts = std::chrono::high_resolution_clock::now();
		long long time = (ts.time_since_epoch().count()-ts_start.time_since_epoch().count())/1000000;

        usleep(100000);

		//read PAPI measurements
		for(int i = 0; i< MEASURE_CHILD_THREADS; i++)
		{
			if(eventset_occupied[i] != -1)
			{
				int tid = eventset_occupied[i];
				Thread * t = (Thread*)n->FindSubcomponentById((*tid_core_map)[tid], SYS_SAGE_COMPONENT_THREAD);
				if(t==NULL)
					return NULL;
				cout << "LIVE: sys-sage Thread " << (*tid_core_map)[tid] << ", tid " << tid << " Time : " << time << ", Frequency: " << t->GetFreq() << " ---- "<< papi_counters_array[i][0] << " " << papi_counters_array[i][1] << " " << papi_counters_array[i][2] << " " << papi_counters_array[i][3]<< endl;
				greenEntries->push_back(GreenEntry(time, t->GetId(), t->GetFreq(), (long long*)papi_counters_array[i]));
			}
		}

		map_tmp = tid_core_map_old;
		tid_core_map_old = tid_core_map;
		tid_core_map = map_tmp;
	} while(*continue_measurement);

	cout << "=========PRINT===============" << endl;
	(*greenEntries)[0].PrintHeader();
	for(GreenEntry g : *greenEntries)
	{
		g.Print();
	}
	return NULL;
}

double gettime() {
	return wtime();
}

double* ts;

void usage(char *s) {
	fprintf(stderr, "Usage: %s <input file> [result file]\n\n", s);
}

int main(int argc, char *argv[]) {

	volatile int continue_measurement = 1;
	pthread_t green_score;
	pthread_create(&green_score, NULL, green_score_thread, (void *)&continue_measurement);


	//omp_set_dynamic(0);     // Explicitly disable dynamic teams
	//omp_set_num_threads(1);


	int i, j, k;
	FILE *infile, *resfile;
	char *resfilename;
	int np, iter, chkflag;
	double rnorm0, rnorm1, t0, t1;
	double tmp[8000000];

	// algorithmic parameters
	algoparam_t param;

	// timing

	double residual;

	// set the visualization resolution
	param.visres = 100;

	// check arguments
	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	// check input file
	if (!(infile = fopen(argv[1], "r"))) {
		fprintf(stderr, "\nError: Cannot open \"%s\" for reading.\n\n", argv[1]);

		usage(argv[0]);
		return 1;
	}

	// check result file
	resfilename = (argc >= 3) ? argv[2] : (char*)"heat.ppm";

	if (!(resfile = fopen(resfilename, "w"))) {
		fprintf(stderr, "\nError: Cannot open \"%s\" for writing.\n\n", resfilename);

		usage(argv[0]);
		return 1;
	}

	// check input
	if (!read_input(infile, &param)) {
		fprintf(stderr, "\nError: Error parsing input file.\n\n");

		usage(argv[0]);
		return 1;
	}

	print_params(&param);
	ts = (double*)calloc((int)(param.max_res - param.initial_res + param.res_step_size) / param.res_step_size, sizeof(double));

	int exp_number = 0;

	for (param.act_res = param.initial_res; param.act_res <= param.max_res; param.act_res = param.act_res + param.res_step_size) {
		if (!initialize(&param)) {
			fprintf(stderr, "Error in Jacobi initialization.\n\n");

			usage(argv[0]);
		}

		for (i = 0; i < param.act_res + 2; i++) {
			for (j = 0; j < param.act_res + 2; j++) {
				param.uhelp[i * (param.act_res + 2) + j] = param.u[i * (param.act_res + 2) + j];
			}
		}

		// starting time
		ts[exp_number] = wtime();
		residual = 999999999;
		np = param.act_res + 2;

		t0 = gettime();

		for (iter = 0; iter < param.maxiter; iter++) {
			residual = relax_jacobi_seq(&(param.u), &(param.uhelp), np, np);
		}

		t1 = gettime();
		ts[exp_number] = wtime() - ts[exp_number];

		printf("\n\nResolution: %u\n", param.act_res);
		printf("===================\n");
		printf("Execution time: %f\n", ts[exp_number]);
		printf("Residual: %f\n\n", residual);

		printf("megaflops:  %.1lf\n", (double) param.maxiter * (np - 2) * (np - 2) * 7 / ts[exp_number] / 1000000);
		printf("  flop instructions (M):  %.3lf\n", (double) param.maxiter * (np - 2) * (np - 2) * 7 / 1000000);

		exp_number++;
	}

	param.act_res = param.act_res - param.res_step_size;

	coarsen(param.u, param.act_res + 2, param.act_res + 2, param.uvis, param.visres + 2, param.visres + 2);

	write_image(resfile, param.uvis, param.visres + 2, param.visres + 2);

	finalize(&param);


	continue_measurement = 0;
	pthread_join(green_score, NULL);
	return 0;
}
