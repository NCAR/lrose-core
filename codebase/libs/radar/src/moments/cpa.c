/* typedef struct Complex { */
/*   double re, im; */
/* } Complex_t; */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef RadarComplex_t Complex_t;

/********************************************************
 * Simple CPA method - C version
 */

double compute_cpa(const Complex_t *iq, int nSamples)
  
{

  int i;
  double avI, avQ, avMag, phasorLen, cpa;
  double sumMag = 0.0;
  double sumI = 0.0, sumQ = 0.0;

  for (i = 0; i < nSamples; i++) {
    double ii = iq[i].re;
    double qq = iq[i].im;
    sumI += ii;
    sumQ += qq;
    sumMag += sqrt(ii * ii + qq * qq);
  }

  /* compute mean I and Q */
  
  avI = sumI / nSamples;
  avQ = sumQ / nSamples;
  avMag = sumMag / nSamples;
  phasorLen = sqrt(avI * avI + avQ * avQ);
  cpa = phasorLen / avMag;
  return cpa;

}
    
/********************************************************
 * Modified CPA method - C version.
 * The modified formulation searches for for the
 * minimum 5-pt running CPA and then computes the CPA
 * values on each side of the minimum. The mean of
 * these two values is returned.
 *
 * This formulation works well for time series in
 * which the CPA value is high, then becomes low for
 * a short period, and then returns to high values
 * for the rest of the series.
 */

double compute_cpa_mod(const Complex_t *iq, int nSamples)
  
{

  Complex_t *iqPhasor;

  int i;
  int nrun = 5;
  int nhalf = nrun/2;
  int minIndex;

  double *mag, *runningCpa;
  double sumI, sumQ;
  double sumMag;
  double minRunningCpa;
  double dI0, dQ0, dist0;
  double dI1, dQ1, dist1;
  double cpa;

  /* check we have enough points for the running CPA */

  if (nSamples < 8) {
    return compute_cpa(iq, nSamples);
  }
  
  /* compute the phasor vector and magnitude array */

  iqPhasor =
    (Complex_t *) malloc(nSamples * sizeof(Complex_t));
  mag = (double *) malloc(nSamples * sizeof(double));

  sumI = 0.0;
  sumQ = 0.0;
  for (i = 0; i < nSamples; i++) {
    double ii = iq[i].re;
    double qq = iq[i].im;
    sumI += ii;
    sumQ += qq;
    iqPhasor[i].re = sumI;
    iqPhasor[i].im = sumQ;
    mag[i] = sqrt(ii * ii + qq * qq);
  }

  /* compute the 5-point running CPA */

  runningCpa = (double *) malloc(nSamples * sizeof(double));
  memset(runningCpa, 0, nSamples * sizeof(double));

  sumMag = 0.0;
  for (i = 0; i < nrun - 1; i++) {
    sumMag += mag[i];
  }

  for (i = nhalf; i < nSamples - nhalf; i++) {
    double dI = iqPhasor[i-nhalf].re - iqPhasor[i+nhalf].re;
    double dQ = iqPhasor[i-nhalf].im - iqPhasor[i+nhalf].im;
    double dist = sqrt(dI * dI + dQ * dQ);
    sumMag += mag[i+nhalf];
    runningCpa[i] = dist / sumMag;
    sumMag -= mag[i-nhalf];
  }

  for (i = 0; i < nhalf; i++) {
    runningCpa[i] = runningCpa[nhalf];
    runningCpa[nSamples-i-1] = runningCpa[nSamples-nhalf-1];
  }

  /* find location of the minimum 5-pt running cpa */
  /* this point has the max curvature of the phasor line */
  
  minRunningCpa = 99.0;
  minIndex = 0;

  for (i = nhalf; i < nSamples - nhalf; i++) {
    if (runningCpa[i] < minRunningCpa) {
      minRunningCpa = runningCpa[i];
      minIndex = i;
    }
  }

  /* compute CPA distance on either side of minimum */
  
  dI0 = iqPhasor[minIndex].re;
  dQ0 = iqPhasor[minIndex].im;
  dist0 = sqrt(dI0 * dI0 + dQ0 * dQ0);

  dI1 = iqPhasor[nSamples-1].re - iqPhasor[minIndex].re;
  dQ1 = iqPhasor[nSamples-1].im - iqPhasor[minIndex].im;
  dist1 = sqrt(dI1 * dI1 + dQ1 * dQ1);

  /* compute sum of the magnitudes */

  sumMag = 0;
  for (i = 0; i < nSamples; i++) {
    sumMag += mag[i];
  }
  
  /* cpa is the algebraic distance sum
   * divided by the sum of the magnitudes */

  cpa = (dist0 + dist1) / sumMag;
  
  free(iqPhasor);
  free(mag);
  free(runningCpa);

  return cpa;

}

