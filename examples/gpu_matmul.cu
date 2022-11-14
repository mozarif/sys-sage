#include <cuda.h>
#include <iostream>


#define WARP_SIZE 32
#define FULL_MASK 0xffffffff

void dstMatrix(float *A, int n);
void zeroMatrix(float *A, int n);
void printMatrix(const char* name, float *A, int n);
void checkMatrix(float *to_check, float *reference, int n);
void checkCUDAError(const char *msg);
__global__ void matrixMultKernel_global(float* Ad, float* Bd, float* Cd, int n, int tile_size);
__global__ void matrixMultKernel_tiled(float* Ad, float* Bd, float* Cd, int n, int tile_size, int core_tile_size);

void usage(char* argv0)
{
    std::cerr << "usage: " << argv0 << " <gpu-topo path>" << std::endl;
    std::cerr << "       or" << std::endl;
    std::cerr << "       " << argv0 << " (uses predefined paths which may be incorrect.)" << std::endl;
    return;
}


__global__ void matrixMultKernel_cache2(float* Ad, float* Bd, float* Cd, int n, int tile_size_y, int tiles_per_warp)
{
    // __shared__ float Ad_shared[36][160];
    // __shared__ float Bd_shared[36][160];

    // extern __shared__ char array[];
    // float *Ad_shared = (float *)array;
    // float *Bd_shared = blockDim.x * blockDim.y * sizeof(float) + array;
    //
    // for(int tile_col_num = blockidx.x ; tile_col_num < num_tiles_col; tile_col_num += oversubscribe)
    // {
    //     int tileJ=tile_col_num;
    //     int tileI=blockIdx.y;
    //
    //     int elemJ=threadIdx.x;//cols
    //     int elemI=threadIdx.y;//rows
    //
    //     float resCell = 0;
    //
    //     for (int t=0; t<num_tiles_col; t++)
    //     {
    //       //j*n = row j + i=column i; A for each column block; B for each row block
    //       //load the blocks to the memory
    //       Ad_shared[elemI*tile_size_cols + elemJ] = Ad[(tileI*tile_size_rows+elemI)*n + elemJ + t*tile_size_cols];
    //       Bd_shared[elemI*tile_size_cols + elemJ] = Bd[tileJ*tile_size_cols + elemJ + elemI*n + t*n*tile_size_rows];
    //       __syncthreads();
    //
    //       for(int k=0; k<tile_size_cols; k++)
    //       {
    //          resCell += Ad_shared[elemI*tile_size_cols + k]*Bd_shared[k*tile_size_cols + elemJ];
    //       }
    //       __syncthreads();
    //     }
    //     Cd[(tileI*tile_size_rows+elemI)*n + tileJ*tile_size_cols+elemJ]=resCell;
    // }
}

__global__ void matrixMultKernel_cache(float* Ad, float* Bd, float* Cd, int n, int tile_size_rows, int tile_size_cols, int split_size_rows, int splits_per_tile_cols, int num_tiles_col )
{
    extern __shared__ float array[];
    float *Ad_shared = array;
    float *Bd_shared = &array[tile_size_rows * tile_size_cols];

    int thread_in_warp = threadIdx.x;
    int warp_id = threadIdx.y;

    float* resArray = new float(split_size_rows * splits_per_tile_cols);

    // if(warp_id == 0 && thread_in_warp == 0)
    // {
    //     printf("starting block %d\n", blockIdx.x);
    // }
    if(blockIdx.x != 0)
    {
        clock_t start_clock = clock();
        clock_t clock_offset = 0;
        while (clock_offset < 100000000)
        {
            clock_offset = clock() - start_clock;
        }
    }
// else
// {
    if(blockIdx.x * tile_size_rows >= n)
    {
        // if(warp_id == 0 && thread_in_warp == 0)
        //     printf("block %d skipping -- indexes %d and on\n", blockIdx.x, blockIdx.x * tile_size_rows);
        return;
    }

    if(blockIdx.x == 0 && warp_id == 0 && thread_in_warp == 0)
    {
        for(int row = 0; row < n; row++ )
        {
            for(int col = 0; col < n ; col++)
            {
                printf("%d-%d  ", Ad[row*n + col], Bd[row*n + col]);
            }
            printf("\n");
        }
    }

    for(int tile_col_num = 0 ; tile_col_num < num_tiles_col; tile_col_num++)
    {//extra loop to iterate over tiles in a col

        int tile_col=tile_col_num;//J
        int tile_row=blockIdx.x;//I

        if(warp_id == 0 && thread_in_warp == 0)
            printf("block %d --(%d) processing tile col indexes %d to %d\n", blockIdx.x, tile_size_rows * tile_size_cols, tile_col_num * tile_size_cols, (tile_col_num+1) * tile_size_cols -1);

        for(int i = 0; i < split_size_rows * splits_per_tile_cols; i++ )
        {
            resArray[i] = 0;
        }

        if(warp_id == 0 && thread_in_warp == 0)
            printf("block %d --(%d) processing tile col indexes %d to %d\n", blockIdx.x, tile_size_rows * tile_size_cols, tile_col_num * tile_size_cols, (tile_col_num+1) * tile_size_cols -1);

        for (int t=0; t<num_tiles_col; t++)
        {
            for(int row = 0; row < split_size_rows; row++ )
            {
                int elem_row = warp_id * split_size_rows + row;
                if(tile_row*tile_size_rows+elem_row >= n) break;
                for(int split_col = 0; split_col < splits_per_tile_cols ; split_col++)
                {
                    int elem_col = split_col*WARP_SIZE + thread_in_warp;
                    if(tile_col*tile_size_cols+elem_col >= n) break;
                    Ad_shared[elem_row*tile_size_cols + elem_col] = Ad[(tile_row * tile_size_rows + elem_row)*n + elem_col + t * tile_size_cols];
                    Bd_shared[elem_row*tile_size_cols + elem_col] = Bd[ tile_col*tile_size_cols + elem_col  + elem_row * n + t * n * tile_size_rows];
                }
            }
            __syncthreads();



            if(warp_id == 0 && thread_in_warp == 0)
                printf("block %d tile %d -- filled shared array t %d\n", blockIdx.x, tile_col_num, t);

            if(warp_id == 0 && thread_in_warp == 0)
            {
                for(int row = 0; row < tile_size_rows; row++ )
                {
                    for(int col = 0; col < 12 ; col++)
                    {
                        printf("%d ", Ad_shared[row*tile_size_cols + col]);
                    }
                    printf("\n");
                }
            }

            // if(warp_id == 0 && thread_in_warp == 0)
            // {
            //     for(int row = 0; row < split_size_rows*4; row++ )
            //     {
            //         for(int split_col = 0; split_col < splits_per_tile_cols*tile_size_cols; split_col++)
            //         {
            //             printf("%d-%d  ", Ad_shared[row*splits_per_tile_cols*tile_size_cols + split_col], Bd_shared[row*splits_per_tile_cols*tile_size_cols + split_col]);
            //         }
            //         printf("\n");
            //     }
            // }
            for(int k=0; k<tile_size_cols; k++)
            {
                // if(warp_id == 0 && thread_in_warp == 0)
                //     printf("block %d tile %d --k=%d \n", blockIdx.x, tile_col_num, k);
                for(int row = 0; row < split_size_rows; row++ )
                {
                    int elem_row = warp_id * split_size_rows + row;
                    // if(thread_in_warp == 0)
                    //     printf("block %d tile %d --k=%d -- elem_row=%d \n", blockIdx.x, tile_col_num, k, elem_row);
                    if(tile_row*tile_size_rows+elem_row >= n) break;
                    // for(int col = 0; col < tile_size_cols; col++)
                    // {
                    //     if(warp_id == 0 && thread_in_warp == 0)
                    //     {
                    //         if(tile_col*tile_size_cols+col >= n) break;
                    //         resArray[row*splits_per_tile_cols + split_col] += Ad_shared[elem_row * tile_size_cols + k] * Bd_shared[k * tile_size_rows + elem_row];
                    //     }
                    // }

                    // for(int split_col = 0; split_col < splits_per_tile_cols; split_col++)
                    // {
                    //     int elem_col = split_col*WARP_SIZE + thread_in_warp;
                    //     // if(warp_id == 0 && thread_in_warp == 31)
                    //     //     printf("block %d tile %d --k=%d -- elem_row=%d  -- elem_col=%d \n", blockIdx.x, tile_col_num, k, elem_row, elem_col);
                    //     if(tile_col*tile_size_cols+elem_col >= n) break;
                    //     if(elem_row * tile_size_cols + k >= tile_size_rows * tile_size_cols)
                    //         printf("!!!!! block %d tile %d (%d)--elem_row %d * tile_size_cols %d + k %d \n", blockIdx.x, tile_col_num, tile_size_rows * tile_size_cols, elem_row, tile_size_cols, k);
                    //     if(k * tile_size_rows + elem_row >= tile_size_rows * tile_size_cols)
                    //         printf("!!!!2 block %d tile %d (%d)--k %d * tile_size_rows %d + elem_row %d \n", blockIdx.x, tile_col_num, tile_size_rows * tile_size_cols, k, tile_size_rows, elem_row);
                    //
                    //     resArray[row*splits_per_tile_cols + split_col] += Ad_shared[elem_row * tile_size_cols + k] * Bd_shared[k * tile_size_rows + elem_row];
                    // }
                }
            }
            if(warp_id == 0 && thread_in_warp == 0)
                printf("block %d tile %d -- finished computation number t %d\n", blockIdx.x, tile_col_num, t);
            __syncthreads();

        }
        //fan-in from all threads in one warp that have co-computed the same indexes (one row)
        for(int i = 0; i< split_size_rows * splits_per_tile_cols; i++)
        {
            for (int offset = 16; offset > 0; offset /= 2)
            {
                resArray[i] += __shfl_down_sync(FULL_MASK, resArray[i], offset);
            }
        }

        if(thread_in_warp == 0)
        {
            for(int row = 0; row < split_size_rows; row++ )
            {
                int elem_row = warp_id * split_size_rows + row;
                if(tile_row*tile_size_rows+elem_row >= n) break;
                for(int split_col = 0; split_col < splits_per_tile_cols; split_col++)
                {
                    int elem_col = split_col*WARP_SIZE + thread_in_warp;
                    if(tile_col*tile_size_cols+elem_col >= n) break;
                    Cd[(tile_row * tile_size_rows +elem_row) * n + tile_col * tile_size_cols + elem_col] = resArray[row * splits_per_tile_cols + split_col ];
                }
            }
            if(warp_id == 0 && thread_in_warp == 0)
                printf("block %d finishing tile %d \n", blockIdx.x,tile_col_num);
        }
    }
// }
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

    float *A,*B,*C,*D; /* arrays for matrices */
    //int n; /* n=matrix size */
    int n = atoi(argv[1]);
    printf("Matrix mult. of size %d : \n", n);

    int size = n*n*sizeof(float);
    A = (float *) malloc(size);
    dstMatrix(A,n);
    B = (float *) malloc(size);
    dstMatrix(B,n);
    C = (float *) malloc(size);
    zeroMatrix(C,n);
    D = (float *) malloc(size);
    zeroMatrix(D,n);

    float *Ad, *Bd, *Cd, *Dd;

    cudaMalloc((void**)&Ad, size); checkCUDAError("allocate memory for A");
    cudaMalloc((void**)&Bd, size); checkCUDAError("allocate memory for B");
    cudaMalloc((void**)&Cd, size); checkCUDAError("allocate memory for C");
    cudaMalloc((void**)&Dd, size); checkCUDAError("allocate memory for D");

    cudaMemcpy(Ad,A, size, cudaMemcpyHostToDevice); checkCUDAError("memory of A not transferred");
    cudaMemcpy(Bd,B, size, cudaMemcpyHostToDevice); checkCUDAError("memory of B not transferred");
    cudaMemcpy(Cd,C, size, cudaMemcpyHostToDevice); checkCUDAError("memory of C not transferred");
    cudaMemcpy(Dd,D, size, cudaMemcpyHostToDevice); checkCUDAError("memory of D not transferred");

    //n=4000
    int gpu_num_cores = 3840;
    int gpu_num_sm = 30;
    int oversubscribe = 2;
    int shared_mem_size = 96 * 1024 * 0.9;

    int num_cores_per_sm = gpu_num_cores / gpu_num_sm;
    int num_warps_per_sm = num_cores_per_sm / WARP_SIZE;
    printf("matrix size %d * %d , num warps %d\n", n, n, num_warps_per_sm*gpu_num_sm);

    int num_row_splits = num_warps_per_sm * gpu_num_sm * oversubscribe;
    int split_size_rows = (n +num_row_splits-1)/num_row_splits;
    int split_size_cols = WARP_SIZE;
    printf("split dims %d rows x %d cols \n", split_size_rows, split_size_cols);

    int splits_per_tile_rows = num_warps_per_sm;
    int tile_size_rows = split_size_rows * num_warps_per_sm;

    int splits_per_tile_cols = shared_mem_size / (2 * oversubscribe * tile_size_rows * WARP_SIZE * sizeof(float));
    int tile_size_cols = splits_per_tile_cols * WARP_SIZE;

    printf("num splits per tile = %d row x %d col \n", splits_per_tile_rows, splits_per_tile_cols);
    printf("\ntile dim %d row x %d col = %d bytes \n",tile_size_rows, tile_size_cols, tile_size_cols * tile_size_rows * 4);
    printf("grid: %d x %d-row split-sets \n", gpu_num_sm * oversubscribe, split_size_rows*num_warps_per_sm);
    int num_tiles_col = (n+tile_size_cols-1) / tile_size_cols;
    int num_tiles_row = (n+tile_size_rows-1) / tile_size_rows;
    dim3 block_dim(WARP_SIZE, num_warps_per_sm);
    dim3 grid_dim(oversubscribe * gpu_num_sm);//gpu_num_sm * oversubscribe
    //dim3 grid_dim(num_tiles_col, num_tiles_row);//gpu_num_sm * oversubscribe
    unsigned block_shared_mem_size = tile_size_rows * tile_size_cols * sizeof(float) * 2;
    matrixMultKernel_cache<<<grid_dim,block_dim, block_shared_mem_size>>>(Ad,Bd,Cd,n,tile_size_rows, tile_size_cols, split_size_rows, splits_per_tile_cols, num_tiles_col);
    checkCUDAError("matrixMultKernel_cache failed");
    cudaMemcpy(C,Cd, size, cudaMemcpyDeviceToHost); checkCUDAError("memory of D not transferred back");

    printMatrix("C", C, n);
    //checkMatrix(D,C,n);
//////////
    //
    // int tile_size_rows = WARP_SIZE;//TODO threads_per_sm?
    // int tile_size_cols = shared_mem_size / (2 * oversubscribe * tile_size_rows * sizeof(float));
    // printf("\ntile dim %d x %d = %d bytes \n",tile_size_rows, tile_size_cols, tile_size_cols * tile_size_rows * 4);
    //
    // dim3 block_dim(tile_size_cols, tile_size_rows);
    // dim3 grid_dim(oversubscribe, gpu_num_sm * num_warps_per_sm);//gpu_num_sm * oversubscribe
    // unsigned shared_mem_size = tile_size_rows * tile_size_cols * sizeof(float) * 2;
    // matrixMultKernel_cache2<<<dimGrid,dimBlock, shared_mem_size >>>(Ad,Bd,Cd,n);
    //

    return 0;


    //
    //
    // int num_cores = 2000;
    // int tile_size_bytes = 24588/64;// = //24588/64 B per core
    //
    // float *A,*B,*C,*D; /* arrays for matrices */
    // //int n; /* n=matrix size */
    // n = atoi(argv[1]);
    // printf("Matrix mult. of size %d : \n", n);
    //
    // int size = n*n*sizeof(float);
    // A = (float *) malloc(size);
    // dstMatrix(A,n);
    // B = (float *) malloc(size);
    // dstMatrix(B,n);
    // C = (float *) malloc(size);
    // zeroMatrix(C,n);
    // D = (float *) malloc(size);
    // zeroMatrix(D,n);
    //
    // float *Ad, *Bd, *Cd, *Dd;
    //
    // cudaMalloc((void**)&Ad, size); checkCUDAError("allocate memory for A");
    // cudaMalloc((void**)&Bd, size); checkCUDAError("allocate memory for B");
    // cudaMalloc((void**)&Cd, size); checkCUDAError("allocate memory for C");
    // cudaMalloc((void**)&Dd, size); checkCUDAError("allocate memory for D");
    //
    // cudaMemcpy(Ad,A, size, cudaMemcpyHostToDevice); checkCUDAError("memory of A not transferred");
    // cudaMemcpy(Bd,B, size, cudaMemcpyHostToDevice); checkCUDAError("memory of B not transferred");
    // cudaMemcpy(Cd,C, size, cudaMemcpyHostToDevice); checkCUDAError("memory of C not transferred");
    // cudaMemcpy(Dd,D, size, cudaMemcpyHostToDevice); checkCUDAError("memory of D not transferred");
    //
    // int num_cores_1dim = sqrt(num_cores);
    // if(num_cores_1dim * num_cores_1dim < num_cores)
    //     num_cores_1dim+=1;
    //
    // int tile_size = n/num_cores_1dim;
    // if(tile_size * num_cores_1dim < n)
    //     tile_size+=1;
    //
    //
    // dim3 dimBlock((num_cores_1dim>32?32:num_cores_1dim), (num_cores_1dim>32?32:num_cores_1dim));
    // dim3 dimGrid( (num_cores_1dim%32==0?num_cores_1dim/32:num_cores_1dim/32+1), (num_cores_1dim%32==0?num_cores_1dim/32:num_cores_1dim/32+1) );
    // printf("launching kernel: matrix %d x %d, each thread computes tile=%d x %d, there is %d cores in each dimension (blockDim %d, gridDim %d) \n", n, n, tile_size, tile_size, num_cores_1dim, (num_cores_1dim>32?32:num_cores_1dim), (num_cores_1dim%32==0?num_cores_1dim/32:num_cores_1dim/32+1) );
    //
    // matrixMultKernel_global<<<dimGrid,dimBlock>>>(Ad,Bd,Cd,n, tile_size);
    // cudaMemcpy(C,Cd, size, cudaMemcpyDeviceToHost); checkCUDAError("memory of C not transferred back");
    //
    // printf("matrixMultKernel_global done \n");
    //
    // matrixMultKernel_tiled<<<dimGrid,dimBlock>>>(Ad,Bd,Dd,n, tile_size, 7);
    // cudaMemcpy(D,Dd, size, cudaMemcpyDeviceToHost); checkCUDAError("memory of D not transferred back");
    //
    // //printMatrix("C", C, n);
    // checkMatrix(D,C,n);
    //
    // cudaFree(Ad);
    // cudaFree(Bd);
    // cudaFree(Cd);
    //
    // return(0);
    //


}

__global__ void matrixMultKernel_global(float* Ad, float* Bd, float* Cd, int n, int tile_size)
{
    int first_row_index = threadIdx.x * tile_size * n;
    int first_col_index = threadIdx.y * tile_size;

//    if(threadIdx.x==19)
//        printf("index first_row 7, accessing elems row %d to %d col %d to %d \n", first_row_index/n, first_row_index/n +tile_size, first_col_index, first_col_index + tile_size);

    for(int row=0; row < tile_size && row + threadIdx.x * tile_size < n ; row++)
    {
        for(int col=0; col < tile_size && col + threadIdx.y * tile_size < n; col++)
        {
            for(int j=0; j < n ; j++)
            {
                Cd[first_row_index + row*n + first_col_index + col] += Ad[first_row_index + row*n + j]* Bd[j*n + first_col_index + col];
            }
        }
    }
}


__global__ void matrixMultKernel_optimized(float* Ad, float* Bd, float* Cd, int n, int tile_size_y, int tiles_per_warp)
{
    // cuda_warps = 8*GPU_cores / 32
    // cuda_warps/SM = cores-per-SM/32*8
    // 1 tile - x=to fit into shared memory on SM, y=32
    // num-tiles = n/x * n/y
    // num-tiles-per-warp = num-tiles/cuda_warps

    // int row_in_tile = threadIdx.x;
    //
    // __shared__ float Ads[32][tile_size_y];
    // __shared__ float Bds[32][tile_size_y];
    //
    // float c_part = 0;
    // for(int col = 0; col < tile_size_y; col ++)
    // {
    //     c_part += Ad[ row_in_tile * n + col ] * B[ row_in_tile * n + col ]
    // }
}


__global__ void matrixMultKernel_tiled(float* Ad, float* Bd, float* Cd, int n, int tile_size, int core_tile_size)
{
    int first_row_index = threadIdx.x * tile_size * n;
    int first_col_index = threadIdx.y * tile_size;

    for(int row=0; row < tile_size && row + threadIdx.x * tile_size < n ; row+=core_tile_size)
    {
        for(int col=0; col < tile_size && col + threadIdx.y * tile_size < n; col+=core_tile_size)
        {
            for(int j=0; j < n ; j+=core_tile_size)
            {
                for(int row_tile=row; row_tile < row+core_tile_size && row_tile + threadIdx.x * tile_size < n ; row_tile++)
                {
                    for(int col_tile=col; col_tile < col+core_tile_size && col_tile + threadIdx.y * tile_size < n; col_tile++)
                    {
                        for(int j_tile=j; j_tile < j+core_tile_size && j_tile < n ; j_tile++)
                        {
                            Cd[first_row_index + row_tile*n + first_col_index + col_tile] += Ad[first_row_index + row_tile*n + j_tile]* Bd[j_tile*n + first_col_index + col_tile];
                        }
                    }
                }
            }
        }
    }
}


/*
 * helper function to check for errors in CUDA calls
 * source: NVIDIA
 */
void checkCUDAError(const char *msg) {
    cudaError_t err = cudaGetLastError();

    if( cudaSuccess != err) {
        fprintf(stderr, "\nCuda error (%s): %s.\n", msg, cudaGetErrorString( err) );
        //exit(-1);
    }
}

/* print Matrix */
void printMatrix(const char* name, float *A, int n)
{
   int i,k;

   printf("Matrix %s (size %d)\n",name,n);

   for (i=0; i<n; i++) {
     for (k=0; k<n; k++) {
       printf("%f ", A[i*n+k]);
     }

     printf("\n");
   }
}

void checkMatrix(float *to_check, float *reference, int n)
{
    int i,k;
    for (i=0; i<n; i++) {
      for (k=0; k<n; k++) {
        if(to_check[i*n+k] != reference[i*n+k]){
            printf("[%d,%d] %f vs ref %f \n", i,k,to_check[i*n+k],  reference[i*n+k] );
            return;
        }
      }
  }
}

/* set Matrix values to zero */
void zeroMatrix(float *A, int n)
{
   int i,k;

   for (i=0; i<n; i++)
     for (k=0; k<n; k++)
	    A[i*n+k] = 0;
}

/* initialise Matrix: discrete Sine Transform */
void dstMatrix(float *A, int n)
{
   int i,k;

   for (i=0; i<n; i++)
     for (k=0; k<n; k++)
        A[i*n+k] = i+k+1;
	    //A[i*n+k] = sin( ((i+1)*(k+1)*M_PI)/(n+1));
}
