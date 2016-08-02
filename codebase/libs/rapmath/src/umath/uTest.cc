// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1992 - 2014 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** See LICENCE.TXT if applicable for licence details 
// ** 2014/05/26 09:53:32 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// uTest.cc
//
// TTest object for umath
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2014
//
///////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include <rapmath/umath.h>
#include <rapmath/stats.h>
using namespace std;

static void testExponentialFit(double a0, double a1, double a2,
                               double noiseMean, double noiseSdev);

static void testPolynomialOrder3(double a0, double a1, double a2, double a3,
                                 double noiseMean, double noiseSdev);

static void testLinearFit(double mm, double cc,
                          double noiseMean, double noiseSdev);

static void testNewtRaph();

static void cubic(double x, double *val, double *deriv);

// main

int main(int argc, char **argv)
  
{

  //////////////////////
  // Checking uCurveFit

  testExponentialFit(0.1, 5.0, -5.0, 0.0, 0.1);

  testExponentialFit(1.0, 0.3, -0.3, 0.0, 0.1);

  testExponentialFit(0.0, 0.25, -0.25, 0.0, 0.02);

  testPolynomialOrder3(13.0, 5.0, 0.25, -1.25, 0.0, 0.2);

  testPolynomialOrder3(10.0, 2.0, -1.5, 1.20, 0.0, 0.2);

  testLinearFit(0.33333, 150, 0.0, 5.0);

  testLinearFit(4.25, 9.99, 0.0, 2.0);

  testLinearFit(-2.50, -8.88, 0.0, 0.1);

  testNewtRaph();

  return 0;

}

/////////////////////////////////////////////////////////
// test exponential fit

static void testExponentialFit(double a0, double a1, double a2,
                               double noiseMean, double noiseSdev)

{

  double aa[3];
  aa[0] = a0;
  aa[1] = a1;
  aa[2] = a2;
  
  double xx[100];
  double yy[100];

  for (int ii = 0; ii < 100; ii++) {
    
    double noise = STATS_normal_gen(noiseMean, noiseSdev);

    double val = ii;
    xx[ii] = val;
    yy[ii] = aa[0] + aa[1] * exp(val * aa[2]) + noise;

  }

  cerr << "====================================" << endl;
  cerr << "Exponential fit" << endl;

  cerr << "Input aa[0]: " << aa[0] << endl;
  cerr << "Input aa[1]: " << aa[1] << endl;
  cerr << "Input aa[2]: " << aa[2] << endl;
  cerr << "Noise mean: " << noiseMean << endl;
  cerr << "Noise sdev: " << noiseSdev << endl;

  double stdErr, rSquared;
  uExponentialFit(100, xx, yy, aa, &stdErr, &rSquared);

  cerr << "---------------------------------" << endl;

  cerr << "Output aa[0]: " << aa[0] << endl;
  cerr << "Output aa[1]: " << aa[1] << endl;
  cerr << "Output aa[2]: " << aa[2] << endl;

  cerr << "====================================" << endl;
  cerr << endl;

}

/////////////////////////////////////////////////////////
// test polynomial fit order 3

static void testPolynomialOrder3(double a0, double a1, double a2, double a3,
                                 double noiseMean, double noiseSdev)

{
  
  double aaa[4];
  aaa[0] = a0;
  aaa[1] = a1;
  aaa[2] = a2;
  aaa[3] = a3;

  double xx[100];
  double yy[100];
  
  for (int ii = 0; ii < 100; ii++) {
    
    double noise = STATS_normal_gen(noiseMean, noiseSdev);
    
    double val = ii;
    xx[ii] = val;
    yy[ii] = aaa[0] + aaa[1] * val + aaa[2] * val * val +
      aaa[3] * val * val * val + noise;

  }

  cerr << "====================================" << endl;
  cerr << "Poly fit" << endl;

  cerr << "Input aaa[0]: " << aaa[0] << endl;
  cerr << "Input aaa[1]: " << aaa[1] << endl;
  cerr << "Input aaa[2]: " << aaa[2] << endl;
  cerr << "Input aaa[3]: " << aaa[3] << endl;
  cerr << "Noise mean: " << noiseMean << endl;
  cerr << "Noise sdev: " << noiseSdev << endl;
  
  double stdErr, rSquared;
  uPolyFit(100, xx, yy, aaa, 4, &stdErr, &rSquared);

  cerr << "---------------------------------" << endl;
  
  cerr << "Output aaa[0]: " << aaa[0] << endl;
  cerr << "Output aaa[1]: " << aaa[1] << endl;
  cerr << "Output aaa[2]: " << aaa[2] << endl;
  cerr << "Output aaa[3]: " << aaa[3] << endl;

  cerr << "====================================" << endl;
  cerr << endl;

}


/////////////////////////////////////////////////////////
// test linear fit

static void testLinearFit(double mm, double cc,
                          double noiseMean, double noiseSdev)

{

  double xx[100];
  double yy[100];
  
  for (int ii = 0; ii < 100; ii++) {
    
    double noise = STATS_normal_gen(noiseMean, noiseSdev);
    
    double val = ii;
    xx[ii] = val;
    yy[ii] = mm * xx[ii] + cc + noise;

  }

  cerr << "====================================" << endl;
  cerr << "Linear fit" << endl;
  cerr << "Input mm: " << mm << endl;
  cerr << "Input cc: " << cc << endl;
  cerr << "Noise mean: " << noiseMean << endl;
  cerr << "Noise sdev: " << noiseSdev << endl;

  double aa[2];
  double xmean, ymean, xsdev, ysdev;
  double corr;
  double stdErr, rSquared;
  
  uLinearFit(100, xx, yy, aa, &xmean, &ymean, &xsdev, &ysdev,
             &corr, &stdErr, &rSquared);

  cerr << "---------------------------------" << endl;
  cerr << "Fit mm: " << aa[1] << endl;
  cerr << "Fit cc: " << aa[0] << endl;
  cerr << "    xmean: " << xmean << endl;
  cerr << "    ymean: " << ymean << endl;
  cerr << "    xsdev: " << xsdev << endl;
  cerr << "    ysdev: " << ysdev << endl;
  cerr << "    corr: " << corr << endl;
  cerr << "    stdErr: " << stdErr << endl;
  cerr << "    rSquared: " << rSquared << endl;
  
  uLinearFitPC(100, xx, yy, &mm, &cc);

  cerr << "---------------------------------" << endl;
  cerr << "PC Fit mm: " << mm << endl;
  cerr << "PC Fit cc: " << cc << endl;
  
  cerr << "====================================" << endl;
  cerr << endl;

}


/////////////////////////////////////////////////////////
// testing uNewtRaph
// get roots of cubic

static void testNewtRaph()
  
{
  
  cerr << "Testing uNewtRaph" << endl;
  cerr << "Roots should be: -4, -1, 2" << endl;

  double root;

  uNewtRaph(cubic, -6, -3, 0.0001, &root);
  cerr << "========>> root: " << root << endl;

  uNewtRaph(cubic, -3, 0.0, 0.0001, &root);
  cerr << "========>> root: " << root << endl;

  uNewtRaph(cubic, 1.0, 300.0, 0.0001, &root);
  cerr << "========>> root: " << root << endl;

  
}

static void cubic(double x, double *val, double *deriv)

{

  *val = (pow(x, 3.0) + 3 * pow(x, 2.0) - 6.0 * x - 8.0) / 4.0;
  *deriv = (3.0 * x * x + 6.0 * x - 6.0) / 4.0;

}

