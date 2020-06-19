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

#include <ctime>
#include <sys/time.h>
#include <string>
#include <iostream>
#include <rapmath/umath.h>
#include <rapmath/stats.h>
#include <rapmath/PolyFit.hh>
#include <rapmath/ForsytheFit.hh>
using namespace std;

static void _printRunTime(const string& str);

static void testExponentialFit(double a0, double a1, double a2,
                               double noiseMean, double noiseSdev);

static void testPolynomial(int order, int nObs,
                           vector<double> coeff,
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

  // testExponentialFit(0.1, 5.0, -5.0, 0.0, 0.1);

  // testExponentialFit(1.0, 0.3, -0.3, 0.0, 0.1);

  // testExponentialFit(0.0, 0.25, -0.25, 0.0, 0.02);

  // testLinearFit(0.33333, 150, 0.0, 5.0);

  // testLinearFit(4.25, 9.99, 0.0, 2.0);

  // testLinearFit(-2.50, -8.88, 0.0, 0.1);

  // testNewtRaph();

  // testPolynomialOrder3(13.0, 5.0, 0.25, -1.25, 0.0, 0.2);

  // testPolynomialOrder3(10.0, 2.0, -1.5, 1.20, 0.0, 0.5);

  vector<double> coeffs;
  coeffs.push_back(10.0);
  coeffs.push_back(2.0);
  coeffs.push_back(-1.5);
  coeffs.push_back(0.2);
  coeffs.push_back(0.55);
  coeffs.push_back(1.05);
  coeffs.push_back(0.75);
  testPolynomial(20, 500, coeffs, 0.0, 1.5);

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

  _printRunTime("start polynomial order 3");

  // create polynomial data

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
  cerr << "Polynomial details" << endl;
  cerr << "Input aaa[0]: " << aaa[0] << endl;
  cerr << "Input aaa[1]: " << aaa[1] << endl;
  cerr << "Input aaa[2]: " << aaa[2] << endl;
  cerr << "Input aaa[3]: " << aaa[3] << endl;
  cerr << "Noise mean: " << noiseMean << endl;
  cerr << "Noise sdev: " << noiseSdev << endl;
  cerr << "====================================" << endl;

  // try uPolyFit
  
  double stdErr, rSquared;
  uPolyFit(100, xx, yy, aaa, 4, &stdErr, &rSquared);
  cerr << "---------------------------------" << endl;
  cerr << "====>> output from uPolyFit() <<====" << endl;
  cerr << "Output aaa[0]: " << aaa[0] << endl;
  cerr << "Output aaa[1]: " << aaa[1] << endl;
  cerr << "Output aaa[2]: " << aaa[2] << endl;
  cerr << "Output aaa[3]: " << aaa[3] << endl;

  cerr << "---------------------------------" << endl;

  _printRunTime("end of uPolyFit");


  // try PolyFit

  PolyFit poly;
  for (int ii = 0; ii < 100; ii++) {
    poly.addValue(xx[ii], yy[ii]);
  }
  poly.setOrder(3);
  poly.performFit();
  vector<double> coeffs = poly.getCoeffs();

  cerr << "---------------------------------" << endl;
  cerr << "====>> output from PolyFit() <<====" << endl;
  cerr << "Output coeffs[0]: " << coeffs[0] << endl;
  cerr << "Output coeffs[1]: " << coeffs[1] << endl;
  cerr << "Output coeffs[2]: " << coeffs[2] << endl;
  cerr << "Output coeffs[3]: " << coeffs[3] << endl;

  _printRunTime("end of uPolyFit");

  // try Forsythe fit
  
  ForsytheFit forsythe;
  for (int ii = 0; ii < 100; ii++) {
    forsythe.addValue(xx[ii], yy[ii]);
  }
  forsythe.setOrder(3);
  forsythe.performFit();
  vector<double> fcoeffs = forsythe.getCoeffs();

  cerr << "---------------------------------" << endl;
  cerr << "====>> output from ForsytheFit() <<====" << endl;
  cerr << "Output fcoeffs[0]: " << fcoeffs[0] << endl;
  cerr << "Output fcoeffs[1]: " << fcoeffs[1] << endl;
  cerr << "Output fcoeffs[2]: " << fcoeffs[2] << endl;
  cerr << "Output fcoeffs[3]: " << fcoeffs[3] << endl;
  _printRunTime("end of ForsytheFit");

  cerr << "====================================" << endl;
  cerr << endl;

  // try ForsytheFit FORTRAN

  forsythe.performFitFortran();
  fcoeffs = forsythe.getCoeffs();
  
  cerr << "====>> output from ForsytheFitFortran() <<====" << endl;
  cerr << "Output fcoeffs[0]: " << fcoeffs[0] << endl;
  cerr << "Output fcoeffs[1]: " << fcoeffs[1] << endl;
  cerr << "Output fcoeffs[2]: " << fcoeffs[2] << endl;
  cerr << "Output fcoeffs[3]: " << fcoeffs[3] << endl;
  _printRunTime("end of ForsytheFitFortran");

  cerr << endl;

}


/////////////////////////////////////////////////////////
// test polynomial fit of specified order
// actual coeffs are derived from the vector passed in

static void testPolynomial(int order, int nObs,
                           vector<double> coeff,
                           double noiseMean, double noiseSdev)
  
{
  
  _printRunTime("start testPolynomial");

  // if too few coeffs are passed in, derived the remainder
  // from the original list

  int nCoeffIn = coeff.size();
  int nExtra = order + 1 - nCoeffIn;
  if (nCoeffIn > 0) {
    for (int ii = 0; ii < nExtra; ii++) {
      int jj = ii % nCoeffIn;
      double cc = coeff[jj] / (ii + 3.0);
      coeff.push_back(cc);
    } // ii
  }

  // create polynomial data

  double deltax = 5.0 / nObs;
  double startx = 0.0 - (nObs / 2) * deltax;
  vector<double> xx, yy;
  for (int ii = 0; ii < nObs; ii++) {
    double noise = STATS_normal_gen(noiseMean, noiseSdev);
    double xval = startx + ii * deltax;
    xx.push_back(xval);
    double yval = coeff[0];
    for (int mm = 1; mm <= order; mm++) {
      yval += coeff[mm] * pow(xval, (double) mm);
    }
    yval += noise;
    yy.push_back(yval);
  }

  int nPasses = 1000;
  cerr << "====================================" << endl;
  cerr << "Polynomial details" << endl;
  for (int mm = 0; mm <= order; mm++) {
    cerr << "  coeff[" << mm << "]: " << coeff[mm] << endl;
  }
  cerr << "  Noise mean: " << noiseMean << endl;
  cerr << "  Noise sdev: " << noiseSdev << endl;
  cerr << "  nPasses: " << nPasses << endl;
  cerr << "====================================" << endl;

  // try uPolyFit

  double stdErr, rSquared;
  vector<double> aaa;
  aaa.resize(order + 1);
  for (int jj = 0; jj < nPasses; jj++) {
    uPolyFit(nObs, xx.data(), yy.data(), aaa.data(), order, &stdErr, &rSquared);
  }
  cerr << "---------------------------------" << endl;
  cerr << "====>> output from uPolyFit() <<====" << endl;
  for (int mm = 0; mm <= order; mm++) {
    cerr << "  aaa[" << mm << "]: " << aaa[mm] << " (" << coeff[mm] << ")" << endl;
  }
  cerr << "stdErr: " << stdErr << endl;
  cerr << "rSquared: " << rSquared << endl;
  _printRunTime("end of uPolyFit");
  cerr << "---------------------------------" << endl;

  // try PolyFit

  PolyFit poly;
  poly.setValues(xx, yy);
  poly.setOrder(order);
  for (int jj = 0; jj < nPasses; jj++) {
    poly.performFit();
  }
  aaa = poly.getCoeffs();

  cerr << "---------------------------------" << endl;
  cerr << "====>> output from PolyFit() <<====" << endl;
  for (int mm = 0; mm <= order; mm++) {
    cerr << "  aaa[" << mm << "]: " << aaa[mm] << " (" << coeff[mm] << ")" << endl;
  }
  _printRunTime("end of PolyFit");

  // try Forsythe fit
  
  ForsytheFit forsythe;
  forsythe.setValues(xx, yy);
  forsythe.setOrder(order);
  for (int jj = 0; jj < nPasses; jj++) {
    forsythe.performFit();
  }
  aaa = forsythe.getCoeffs();
  stdErr = forsythe.computeStdErrEst(rSquared);
  cerr << "---------------------------------" << endl;
  cerr << "====>> output from ForsytheFit() <<====" << endl;
  for (int mm = 0; mm <= order; mm++) {
    cerr << "  aaa[" << mm << "]: " << aaa[mm] << " (" << coeff[mm] << ")" << endl;
  }
  cerr << "stdErr: " << stdErr << endl;
  cerr << "rSquared: " << rSquared << endl;
  _printRunTime("end of ForsytheFit");

  cerr << "====================================" << endl;
  cerr << endl;

  // try ForsytheFit FORTRAN

  for (int jj = 0; jj < nPasses; jj++) {
    forsythe.performFitFortran();
  }
  aaa = forsythe.getCoeffs();
  cerr << "====>> output from ForsytheFitFortran() <<====" << endl;
  for (int mm = 0; mm <= order; mm++) {
    cerr << "  aaa[" << mm << "]: " << aaa[mm] << " (" << coeff[mm] << ")" << endl;
  }
  _printRunTime("end of ForsytheFitFortran");

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

// Print the elapsed run time since the previous call, in seconds.

static struct timeval _timeA;
static bool _firstTime = true;

static void _printRunTime(const string& str)
{
  if (_firstTime) {
    gettimeofday(&_timeA, NULL);
    _firstTime = false;
  }
  struct timeval tvb;
  gettimeofday(&tvb, NULL);
  double deltaSec = tvb.tv_sec - _timeA.tv_sec
    + 1.e-6 * (tvb.tv_usec - _timeA.tv_usec);
  cerr << "TIMING, task: " << str << ", secs used: " << deltaSec << endl;
  _timeA.tv_sec = tvb.tv_sec;
  _timeA.tv_usec = tvb.tv_usec;
}

