//----------------------------------------------------------------------
// Module: test_kd_interp.cc
//
// Author: Gerry Wiener
//
// Date:   8/27/02
//
// Description:
//     
//----------------------------------------------------------------------

// Include files 
#include <kd/kd_interp.hh>

// Constant, macro and type definitions 
#define INDXY(nx, ny, x, y) ((nx) * ((int)y) + ((int)x))

// Global variables 

// Functions and objects

void test1()
{
  // Setup a grid of 16 points
  const int XSIZE = 4;
  const int YSIZE = 4;
  const int SIZE = XSIZE * YSIZE;

  KD_real xdata[SIZE];			// I - x coordinates of data points
  KD_real ydata[SIZE];			// I - y coordinates of data points
  int ndata = SIZE;			// I - number of data points
  KD_real data_val[SIZE];			// I - values at data points
  KD_real xgrid[SIZE];			// I - x coordinates of interp grid
  KD_real ygrid[SIZE];			// I - y coordinates of interp grid
  const int xdim = XSIZE;		// I - size of xinterp
  const int ydim = YSIZE; 		// I - size of yinterp
  KD_real grid_val[xdim * ydim];	// O - interp grid values (should have dimension xdim * ydim)

  int ind;

  printf("\nTesting nearest neighbor point-based interpolation\n");

  for (int i=0; i<SIZE; i++)
    data_val[i] = i;

  // Set up grid of data points
  for (int j=0; j<ydim; j++)
    for (int i=0; i<xdim; i++)
      {
	ind = INDXY(xdim, ydim, i, j);
	xdata[ind] = i;
	ydata[ind] = j;
      }

  for (int i=0; i<xdim; i++)
    {
      xgrid[i] = i + 0.25;
    }

  for (int j=0; j<ydim; j++)
    {
      ygrid[j] = j + 0.25;
    }

  kd_interp(xdata, ydata, ndata, data_val, xgrid, ygrid, xdim, ydim, grid_val);

  ind = 0;
  for (int j=0; j<ydim; j++)
    for (int i=0; i<xdim; i++)
      {
        printf("grid point: %g, %g\n", xgrid[i], ygrid[j]);
	printf("grid_val[%d,%d] = %g\n", j, i, grid_val[ind]);
	ind++;
      }

}

void test2()
{
  // Setup a 2x2 grid 
  const int XSIZE = 2;
  const int YSIZE = 2;
  const int SIZE = XSIZE * YSIZE;

  KD_real xdata[SIZE];			// I - x coordinates of data points
  KD_real ydata[SIZE];			// I - y coordinates of data points
  int ndata = SIZE;			// I - number of data points
  KD_real data_val[SIZE];		// I - values at data points
  KD_real xgrid[SIZE];			// I - x coordinates of interp grid
  KD_real ygrid[SIZE];			// I - y coordinates of interp grid
  const int xdim = XSIZE;		// I - size of xinterp
  const int ydim = YSIZE; 		// I - size of yinterp
  KD_real grid_val[xdim * ydim];	// O - interp grid values (should have dimension xdim * ydim)

  int ind;

  printf("\nTesting nearest neighbor rectangle interpolation\n");

  for (int i=0; i<SIZE; i++)
    data_val[i] = i;

  // Set up grid of data points
  for (int j=0; j<ydim; j++)
    for (int i=0; i<xdim; i++)
      {
	ind = INDXY(xdim, ydim, i, j);
	xdata[ind] = i - 0.5;
	ydata[ind] = j - 0.5;
      }

  for (int i=0; i<xdim; i++)
    {
      xgrid[i] = i;
    }

  for (int j=0; j<ydim; j++)
    {
      ygrid[j] = j;
    }

  //  KD_real dist_crit = 0.6 * sqrt(2);

  KD_real dist_crit[] = {0, 0.5, 1.0, 1.5, 2.0, 2.5};
  for (int k=0; k<6; k++)
    {
      int sel = KD::MIN;
      printf("dist_crit: %g\n", dist_crit[k]);
      kd_rect_interp(xdata, ydata, ndata, data_val, xgrid, ygrid, xdim, ydim, dist_crit[k], sel, grid_val);

      ind = 0;
      for (int j=0; j<ydim; j++)
        for (int i=0; i<xdim; i++)
          {
            printf("grid_val[%d,%d] = %g\n", j, i, grid_val[ind]);
            ind++;
          }
      printf("\n");
    }
}

int main()
{
  test1();
  test2();
}

