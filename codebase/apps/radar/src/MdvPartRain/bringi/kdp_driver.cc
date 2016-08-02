/***********************************************************************
 * c_driver.c
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "KdpSband.hh"
using namespace std;

#define MAX_GATES 5000

extern void compute_kdp(int nGates,
			const double *range,
			const double *zh,
			const double *zdr,
			const double *phidp,
			const double *rhohv,
			const double *snr_s,
			double *kdp);

int main(int argc, char **argv)

{
  
  char line[8192];
  int ii;
  int nGates = 0;
  double zh[MAX_GATES], zdr[MAX_GATES], phidp[MAX_GATES], rhohv[MAX_GATES];
  double range[MAX_GATES], snr_s[MAX_GATES];
  double kdp[MAX_GATES];
  
  /* open testcase file */
  
  FILE *in;
  const char *testcaseFilename = "test_input.dat";

  if ((in = fopen(testcaseFilename, "r")) == NULL) {
    fprintf(stderr, "ERROR - c_driver\n");
    fprintf(stderr, "  Cannot open test case file: %s\n", testcaseFilename);
    return -1;
  }

  /* read in data */
  
  while (!feof(in)) {
    
    if (fgets(line, 8192, in) != NULL) {
      
      double _range, _zh, _zdr, _phidp, _rhohv;

      if (sscanf(line, "%lg%lg%lg%lg%lg",
		 &_range, &_zh, &_zdr, &_phidp, &_rhohv) == 5) {
	range[nGates] = _range;
	zh[nGates] = _zh;
	zdr[nGates] = _zdr;
	phidp[nGates] = _phidp;
	rhohv[nGates] = _rhohv;
	nGates++;
      }
      
    }
    
  }

  /* compute kdp */

  KdpSband kdpS;
  kdpS.compute(nGates, range, zh, zdr, phidp, rhohv, snr_s, kdp);

  /* print results */

  for (ii = 0; ii < nGates; ii++) {
    cerr << ii << " "
	 << range[ii] << " "
	 << zh[ii] << " "
	 << zdr[ii] << " "
	 << phidp[ii] << " "
	 << rhohv[ii] << " "
	 << kdp[ii] << endl;
  }

  return 0;

}

