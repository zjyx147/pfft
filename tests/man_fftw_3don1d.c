#include <complex.h>
#include <fftw3-mpi.h>
#include <pfft.h>

// static fftw_complex my_function(ptrdiff_t i, ptrdiff_t k, ptrdiff_t j){return i +1.0/(j+1) + I*1.0/(k+1);}
static fftw_complex my_function(ptrdiff_t i, ptrdiff_t k, ptrdiff_t j){return 1.0;}

int main(int argc, char **argv)
{
    const ptrdiff_t n0 = 4, n1 = 4, n2 = 4;
    fftw_plan plan;
    fftw_complex *data;
    ptrdiff_t alloc_local, local_n0, local_0_start, i, j, k;

    MPI_Init(&argc, &argv);
    fftw_mpi_init();

    /* get local data size and allocate */
    alloc_local = fftw_mpi_local_size_3d(n0, n1, n2, MPI_COMM_WORLD,
					 &local_n0, &local_0_start);
    data = fftw_alloc_complex(alloc_local);

    /* create plan for in-place forward DFT */
    plan = fftw_mpi_plan_dft_3d(n0, n1, n2, data, data, MPI_COMM_WORLD,
				FFTW_FORWARD, FFTW_ESTIMATE);

    /* initialize data to some function my_function(x,y) */
    for (i = 0; i < local_n0; ++i) 
      for (j = 0; j < n1; ++j) 
        for (k = 0; k < n2; ++k)
          data[i*n1*n2 + j*n2 + k] = my_function(local_0_start + i, j, k);

    ptrdiff_t local_ni[3] = {local_n0, n1, n2}, local_i_start[3] = {local_0_start, 0, 0};
    pfft_apr_complex_3d(data, local_ni, local_i_start, "input:", MPI_COMM_WORLD);

    /* compute transforms, in-place, as many times as desired */
    fftw_execute(plan);

    ptrdiff_t local_no[3] = {local_n0, n1, n2}, local_o_start[3] = {local_0_start, 0, 0};
    pfft_apr_complex_3d(data, local_no, local_o_start, "output:", MPI_COMM_WORLD);

    fftw_destroy_plan(plan);

    MPI_Finalize();
}
