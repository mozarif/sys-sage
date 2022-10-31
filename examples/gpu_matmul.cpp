#include <cuda.h>

void usage(char* argv0)
{
    std::cerr << "usage: " << argv0 << " <gpu-topo path>" << std::endl;
    std::cerr << "       or" << std::endl;
    std::cerr << "       " << argv0 << " (uses predefined paths which may be incorrect.)" << std::endl;
    return;
}

int main(int argc, const char *argv[]) {

    // //create root Topology and one node
    // Topology* topo = new Topology();
    // Node* n = new Node(topo,1);
    //
    // cout << "-- Parsing gpu-topo benchmark from file " << gpuTopoPath << endl;
    // if(parseGpuTopo((Component*)n, gpuTopoPath, 0, ";") != 0) { //adds topo to a next node
    //     return 1;
    // }
    // cout << "-- End parseGpuTopo" << endl;

    int num_cores = 3800;
    int tile_size_bytes = 24588/64;// = //24588/64 B per core
    int

    float *A,*B,*C; /* arrays for matrices */
    int n; /* n=matrix size */
    n = atoi(argv[1]);
    printf("Matrix mult. of size %d : \n", n);

    int size = n*n*sizeof(float);
    A = (float *) malloc(size);
    dstMatrix(A,n);
    B = (float *) malloc(size);
    dstMatrix(B,n);
    C = (float *) malloc(size);
    zeroMatrix(C,n);

    float *Ad, *Bd, *Cd;

    cudaMalloc((void**)&Ad, size); checkCUDAError("allocate memory for A");
    cudaMalloc((void**)&Bd, size); checkCUDAError("allocate memory for B");
    cudaMalloc((void**)&Cd, size); checkCUDAError("allocate memory for C");

    cudaMemcpy(Ad,A, size, cudaMemcpyHostToDevice); checkCUDAError("memory of A not transferred");
    cudaMemcpy(Bd,B, size, cudaMemcpyHostToDevice); checkCUDAError("memory of B not transferred");
    cudaMemcpy(Cd,C, size, cudaMemcpyHostToDevice); checkCUDAError("memory of C not transferred");

    int num_cores_1dim = sqrt(num_cores);
    if(num_cores_1dim * num_cores_1dim < num_cores)
        num_cores_1dim+=1;

    int tile_size = n/num_cores_1dim;
    if(tile_size * num_cores_1dim < n)
        tile_size+=1;

    dim3 dimBlock(num_cores_1dim, num_cores_1dim);
    dim3 dimGrid(1, 1);
    matrixMultKernel_global<<<dimGrid,dimBlock>>>(Ad,Bd,Cd,n, tile_size);

    cudaMemcpy(C,Cd, size, cudaMemcpyDeviceToHost); checkCUDAError("memory of C not transferred back");

    cudaFree(Ad);
    cudaFree(Bd);
    cudaFree(Cd);

    return(0);
}

__global__ void matrixMultKernel_global(float* Ad, float* Bd, float* Cd, int n, int tile_size)
{
    int first_row = threadIdx.x * tile_size;
    int first_col = threadIdx.y * tile_size;

    float cPart[tile_size][tile_size];
    for(int row=0; row < tile_size ; row++)
    {
        for(int col=0; col < tile_size ; col++)
        {
            cPart[row][col] = 0;
            for(int j=0; row < n ; j++)
            {
                cPart += Ad[row*n + j]* Bd[j*n + col];
            }
        }
    }
    for(int row=0; row < tile_size ; row++)
    {
        for(int col=0; col < tile_size ; col++)
        {
            Cd[first_row*n + row*n + first_col + col] += cPart[row][col];
        }
    }
}

// __global__ void matrixMultKernel_tiled(float* Ad, float* Bd, float* Cd, int n)
// {
//       __shared__ float Ad_shared[TILE_SIZE][TILE_SIZE];
//       __shared__ float Bd_shared[TILE_SIZE][TILE_SIZE];
//
//       //SLOWER..why?
//       // int tileI=blockIdx.x;
//       // int tileJ=blockIdx.y;
//       //
//       // int elemI=threadIdx.x;
//       // int elemJ=threadIdx.y;
//
//       int tileJ=blockIdx.x;
//       int tileI=blockIdx.y;
//
//       int elemJ=threadIdx.x;
//       int elemI=threadIdx.y;
//
//       float resCell = 0;
//       for (int t=0; t<n/TILE_SIZE; t++)
//       {
//         //j*n = row j + i=column i; A for each column block; B for each row block
//         //load the blocks to the memory
//         Ad_shared[elemI][elemJ] = Ad[(tileI*TILE_SIZE+elemI)*n + elemJ + t*TILE_SIZE];
//         Bd_shared[elemI][elemJ] = Bd[tileJ*TILE_SIZE + elemJ + elemI*n + t*n*TILE_SIZE];
//         __syncthreads();
//
//         for(int k=0; k<TILE_SIZE; k++)
//         {
//            resCell += Ad_shared[elemI][k]*Bd_shared[k][elemJ];
//         }
//         __syncthreads();
//         Cd[(tileI*TILE_SIZE+elemI)*n + tileJ*TILE_SIZE+elemJ]=resCell;
//       }
//
// }


/*
 * helper function to check for errors in CUDA calls
 * source: NVIDIA
 */
void checkCUDAError(const char *msg) {
    cudaError_t err = cudaGetLastError();

    if( cudaSuccess != err) {
        fprintf(stderr, "\nCuda error (%s): %s.\n", msg, cudaGetErrorString( err) );
        exit(-1);
    }
}

/* initialise Matrix: discrete Sine Transform */
void dstMatrix(float *A, int n)
{
   int i,k;

   for (i=0; i<n; i++)
     for (k=0; k<n; k++)
	    A[i*n+k] = sin( ((i+1)*(k+1)*M_PI)/(n+1));
}
