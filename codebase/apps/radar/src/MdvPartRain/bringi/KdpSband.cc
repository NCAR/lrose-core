// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:33:50 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// KdpSband.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////
//
// Kdp for SBand - based on Bringi code
//
////////////////////////////////////////////////////////////////

#include <iomanip>
#include <iostream>
#include <cmath>
#include "KdpSband.hh"
using namespace std;

const double KdpSband::fir3gain = 1.0;

const double KdpSband::fir3coef[FIR3ORDER+1] = {
  0.01040850049, 0.0136551033,  0.01701931136,
  0.0204494327,  0.0238905658,  0.02728575662,
  0.03057723021, 0.03370766631, 0.03662148602,
  0.03926611662, 0.04159320123, 0.04355972181,
  0.04512900539, 0.04627158699, 0.04696590613,
  0.04719881804, 0.04696590613, 0.04627158699,
  0.04512900539, 0.04355972181, 0.04159320123,
  0.03926611662, 0.03662148602, 0.03370766631,
  0.03057723021, 0.02728575662, 0.0238905658,
  0.0204494327,  0.01701931136, 0.0136551033,
  0.01040850049
};

// Constructor

KdpSband::KdpSband()
  
{

  setAltHv();
  
}

// Destructor

KdpSband::~KdpSband()
  
{

}

/////////////////////////////////////
// compute KDP

int KdpSband::compute(int nGates,
		      const double *range__,
		      const double *zh__,
		      const double *zdr__,
		      const double *phidp__,
		      const double *rhohv__,
		      const double *snr__,
		      double *kdp)
  
{


  // copy input arrays, leaving extra space at the beginning for negative indices
  // and at the end for filtering as required

  double range_[MAX_GATES + EXTRA_GATES];
  double *range = range_ + EXTRA_HALF;
  memcpy(range, range__, nGates * sizeof(double));
  
  double zh_[MAX_GATES + EXTRA_GATES];
  double *zh = zh_ + EXTRA_HALF;
  memcpy(zh, zh__, nGates * sizeof(double));
  
  double zdr_[MAX_GATES + EXTRA_GATES];
  double *zdr = zdr_ + EXTRA_HALF;
  memcpy(zdr, zdr__, nGates * sizeof(double));
  
  double phidp_[MAX_GATES + EXTRA_GATES];
  double *phidp = phidp_ + EXTRA_HALF;
  memcpy(phidp, phidp__, nGates * sizeof(double));
  
  double rhohv_[MAX_GATES + EXTRA_GATES];
  double *rhohv = rhohv_ + EXTRA_HALF;
  memcpy(rhohv, rhohv__, nGates * sizeof(double));
  
  double snr_[MAX_GATES + EXTRA_GATES];
  double *snr = snr_ + EXTRA_HALF;
  memcpy(snr, snr__, nGates * sizeof(double));

  // allocate and initializa arrays

  int goodMask[MAX_GATES];
  
  for (int ii = 0; ii < nGates; ii++) {
    goodMask[ii] = 0; // 0 is good, 1 is bad
  }
  
  // initialize basic variables
  
  int madflt = 30;
  
  double thrs_phi=6;  // this value for CP2
  double thrcrr=0.9;  // set low since not used
  
  // save the raw phidp to checking unfold
  
  double phidpSave_[MAX_GATES + EXTRA_GATES];
  double *phidpSave = phidpSave_;

  for (int ii = 0; ii < nGates; ii++) {
    phidpSave[ii] = phidp[ii];
  }

  // unfold phidp
    
  unfoldPhidp(nGates, phidp, range, thrs_phi);

  int iend = nGates - 1;
  int ibegin = nGates - 1;

  // tmp array

  double zzz_[MAX_GATES + EXTRA_GATES];
  double *zzz = zzz_ + EXTRA_HALF;
  
  for (int ii = -EXTRA_HALF; ii < nGates + EXTRA_HALF; ii++) {
    zzz[ii] = 0.0;
  }

  // --------- FIND THE start AND stop BINs FOR FILTERING ---------------
  
  double sdPhidp_[MAX_GATES + EXTRA_GATES];
  double *sdPhidp = sdPhidp_ + EXTRA_HALF;
  
  snr[-1] = snr[0];
  bool inBad = true;
  int firsttime = 1;
  int j_arr = -1;
  int k_arr = -1;
  
  double init0 = 0.0;
  double init = 0.0;
  double delavg = 0.0;
  double avrg1 = 0.0;
  double avrg2 = 0.0;
  int mc = 0;
  int count = 0;
  
  for (int ii = 0; ii < nGates; ii++) {
    
    double yy[mgood];
    for (int jjj = 0; jjj < mgood; jjj++) {
      yy[jjj] = phidp[ii+jjj];
    }
    
    // compute sdev of phidp
    
    double phimn,sd_phidp;
    msr(phimn,sd_phidp,yy,mgood);
    sdPhidp[ii] = sd_phidp;

    if (inBad) {
      
      //  ********** Find begin of GOOD data loop ***********
      
      if ((rhohv[ii] >= thrcrr) && (sdPhidp[ii] < thrs_phi) && (range[ii] >= 1.5)) {
	
	count++;

	if (count == mgood) {
	  
	  for (int il = 0; il < mgood; il++) {
	    zzz[ii-il] = phidp[ii-il];
	    goodMask[ii-il] = 1;
	  }
	  
	  if (firsttime == 1) {
	    
	    ibegin = ii - mgood + 1; // begin of the 1st encountered cell
	    j_arr++;
	    init0=(zzz[ibegin]+zzz[ibegin+1]+zzz[ibegin+2]+zzz[ibegin+3])/4.0;
	    init = init0;    // Recorded for local trend
	    delavg = init0 - 999.0;

	    init = init0;
	    firsttime = 0;

	  } else {
	    
	    mc = ii - mgood + 1;  // begin of the successive encountered cells
	    j_arr++;
	    avrg1 = (zzz[mc+1]+zzz[mc+2]+zzz[mc+3]+zzz[mc+4])/4.0;
	    delavg = avrg1-avrg2;

	    if (avrg1 < init && avrg2 > init) {
	      avrg1 = avrg2;
	    } else if (avrg1 > init && avrg2 < init) {
	      avrg2 = init;
	    } else if (fabs(delavg) > 15. && avrg1 < avrg2) {
	      avrg1 = avrg2;
	    }

	    init = avrg1;     // Save local trend
	    double rmc = range[mc];
	    double rend = range[iend];
	    double d1 = rmc-rend;
	    double d2 = (avrg1-avrg2)/d1;
	    double d3 = (rend*avrg1-rmc*avrg2)/d1;
	    
	    for (int ij = iend+1; ij <= mc; ij++) {
	      double rij = range[ij];
	      zzz[ij] = rij*d2 - d3;
	    }

	  } // if (firsttime
	    
	  inBad = false;
	  count = 0;
	  iend  = nGates - 1;
	  
	} // if (count == mgood)
	  
      } else {
	
	count = 0;
	
      } // if ((rhohv[ii] >= thrcrr ...
      
    } else if (!inBad) { // if (loop == 1)
    
      // ************* Find END of GOOD DATA loop ****************  

      zzz[ii] = phidp[ii];

      // in this branch, the gate locates on good data segment
      // as we are looking for next bad data.
      
      goodMask[ii] = 1;
      
      if(ii == nGates - 1) {
	iend = nGates - 1;
	avrg2=(zzz[iend]+zzz[iend-1]+zzz[iend-2]+zzz[iend-3])/4.0;
	continue; // ii loop
      }

      if (sdPhidp[ii] > thrs_phi || rhohv[ii] < thrcrr) {

	count++;
	
	if (count == mbad) {  //Insert test to preserve hail/BB signal.
	  
	  double zhmn = 0.0;
	  double xx[mbad], yy[mbad];
	  for (int jj = 0; jj < mbad; jj++) {
	    zhmn += pow(10.0, (0.1*zh[ii-jj]));
	    yy[jj] = rhohv[ii-jj];
	    xx[jj] = phidp[ii-jj];
	  }
	  zhmn = zhmn/float(mbad);
	  double zhmn_log=10.0*log10(zhmn);

	  // Changing zh_mean value in BB or hail to 30 dBZ; 2/1/02 Bringi
	  // NOTE: BB with mean Zh<30 dBZ may be classified as "bad" data
	  
	  if (zhmn_log >= 25.0) {
	    
	    double ymn,sd;
	    msr(ymn,sd,yy,mbad);
	    double amean,test_sd_phidp;
	    msr(amean,test_sd_phidp,xx,mbad);

	    // rhohv in BB could go as low as 0.6
	    //  checking mean rhohv in BB/hail region
	    
	    if (ymn >= 0.6 && test_sd_phidp < (thrs_phi+7.5)) {
	      count = 0;
	      continue; // ii loop
	    }

	  } // if (zhmn_log >= 25.0)
	    
	  iend = ii - mbad;
	  k_arr++;
	  
	  for (int jj = 0; jj < mbad; jj++) {
	    //Inserted to clean the bad Zdr & Rhv.
	    goodMask[ii-jj] = 0; // bad value 
	  }
	  
	  avrg2=(zzz[iend]+zzz[iend-1]+zzz[iend-2]+zzz[iend-3])/4.0;
	  if (iend == nGates) {
	    continue; // ii loop
	  }
	  
	  zzz[ii] = avrg2;
	  zzz[ii-1] = avrg2;
	  inBad = true;
	  count = 0;

	} // if (count == mbad ...
	  
      } else {
	
	  count = 0;
	  
      } // if (sdPhidp[ii] > thrs_phi

    } // if (inBad)

  } // ii (nGates)

  //--------------- END of FINDING start AND stop BINs -------------------

  if(ibegin == nGates - 1) {  // NO good data in whole ray. RETURN.
    return -1;
  }

  int kbegin = ibegin;
  int kend = iend;
  double ext = avrg2;

  // working arrays

  double xxx_[MAX_GATES + EXTRA_GATES];
  double *xxx = xxx_ + EXTRA_HALF;

  double yyy_[MAX_GATES + EXTRA_GATES];
  double *yyy = yyy_ + EXTRA_HALF;

  for (int ii = -EXTRA_HALF; ii < nGates + EXTRA_HALF; ii++) {
    xxx[ii] = 0.0;
    yyy[ii] = 0.0;
  }

  for (int ii = -90; ii < kbegin; ii++) {   // Set the initial conditions
    zzz[ii] = init0;
  }

  for (int ii = kend + 1; ii < nGates + 30; ii++) { // Extend data record
    zzz[ii] = ext;
  }

  for (int ii = -91; ii < nGates + 30; ii++) { // Copy raw data array
    xxx[ii] = zzz[ii];
  }

  //------------- MAIN LOOP of Phidp Adaptive Filtering --------------------

  double thres = 4.0;	// this value depends on the expected backscatter diff phase

  for (int mm = 0; mm < madflt; mm++) {

    // TIE DOWN THE INITIAL and EXTENDING DATA RECORD

    for (int ii = -90; ii < kbegin; ii++) {   // Set the initial conditions
      zzz[ii] = init0;
    }
    
    for (int ii = kend + 1; ii < nGates + 30; ii++) { // Extend data record
      zzz[ii] = ext;
    }
    
    // FIR FILTER SECTION

    for (int ii = -6; ii < nGates + 5; ii++) {
      
      double acc = 0.0;

      for (int j = 0; j <= FIR3ORDER; j++) {
	int jjj = ii-FIR3ORDER/2+j;
	acc = acc + fir3coef[j]*zzz[jjj];
      }

      yyy[ii] = acc * fir3gain;

    } // END of FIR FILTERING

    for (int ii = 1; ii <= nGates; ii++) {
      
      double delt = fabs(xxx[ii]-yyy[ii]);
      if (delt >= thres) {
	zzz[ii]=yyy[ii];
      } else {
	zzz[ii]=xxx[ii];
      }

    }

  } // mm - END LOOP for Phidp Adaptive Filtering
  
  double ad1_[MAX_GATES + EXTRA_GATES];
  double *ad1 = ad1_ + EXTRA_HALF;
  
  double fl_[MAX_GATES + EXTRA_GATES];
  double *fl = fl_ + EXTRA_HALF;
  
  double df_[MAX_GATES + EXTRA_GATES];
  double *df = df_ + EXTRA_HALF;

  //  PUT KDP,DELTA,PDPCORRECTED,PDPCORRECTED-FILTERED into PLFILE

  for (int ii = -6; ii < nGates + 3; ii++) {
    ad1[ii] = zzz[ii];
    fl[ii] = yyy[ii];
    df[ii] = xxx[ii] - yyy[ii];
  }
  
  // CALCULATE KDP

  for (int ii = 0; ii < nGates; ii++) {
    
    // Check Zh range
    // default value for nadp is 10
    
    int nadp = 10;
    
    if (ii >= 10 && ii < nGates - 11) {

      if (zh[ii] < 20.0) {
	nadp = 15;
      } else if (zh[ii] < 35.0) {
	nadp = 8;
      } else {
	nadp = 2;
      }
      
    }
    
    double xx[maxNadp],yy[maxNadp];
    for (int jj = 0; jj <= nadp; jj++) {
      xx[jj] = range[ii - nadp/2 + jj];
      yy[jj] = fl[ii - nadp/2 + jj];
    }
      
    // compute Kdp based on lse fit to Adap flt Phidp

    double aa, bb;
    lse(aa,bb,xx,yy,nadp+1);

    kdp[ii] = aa / 2.0;
    
  } // ii

  return 0;

}
  
/////////////////////////////////////////////////////////////////////////
// This is a Linear Least Square Estimate subroutine to fit a linear
// equation for (xi,yi) (i=1,...,n), so that
//                         yi = a * xi + b
// INPUTs: x(i), y(i), n, (i=1,...,n ).
// OUTPUTs: a ( slope ), b ( intercept ).
//                                                Li Liu   Sep. 23, 92

void KdpSband::lse(double &a, double &b, const double *x, const double *y, int n)
  
{

  double xsum = 0.0;
  double ysum = 0.0;
  double xxsum = 0.0;
  double xysum = 0.0;
  double total = n;
  for (int i = 0; i < n; i++) {
    if (x[i] > 1.e35 || y[i] > 1.e35) {
      total--;
    } else {
      xsum += x[i];
      ysum += y[i];
      xxsum += x[i]*x[i];
      xysum += x[i]*y[i];
    }
  }
  double det = total * xxsum - xsum * xsum;
  a = ( total*xysum - xsum*ysum ) / det;
  b = ( ysum*xxsum - xsum*xysum ) / det;

}

//////////////////////////////////////////////////////////////////////////
//  To calculate the mean (ymn) and standard deviation (sd, or,
//  mean square root, msr) of the array y(i) (i=1,...,n).
//                                               Li Liu  Sep. 19, 95

void KdpSband::msr(double &ymn, double &sd, const double *y, int n)

{
  
  double ysum  = 0.0;
  double yysum = 0.0;
  double total = n;

  for (int i = 0; i < n; i++) {
    if (fabs(y[i]) > 1.e35) {
      total--;
    } else {
      ysum += y[i];
    }
  }

  if (total > 0) {
    ymn = ysum / total;
  } else {
    ymn = 0;
  }

  for (int i = 0; i < n; i++) {
    if (fabs(y[i]) < 1.e35) {
      double diff = y[i] - ymn;
      yysum += diff * diff;
    }
  }
  if (total > 0) {
    sd = sqrt(yysum/total);
  } else {
    sd = 0.0;
  }

}

////////////////////
// unfold phidp
// This is Yanting's modified unfold code 

void KdpSband::unfoldPhidp(int nGates,
			   double *phidp,
			   double *range,
			   double thrs_phi)

{
  
  bool inwrap = false;
  double extra = 0;
  int jcount = 0;
  int ibegin = 0;

  double avgbg = 0;
  double thrs_unfold = 90.0; // must be checked for CP2
  double xx[unfoldLen], yy[unfoldLen];

  double r_min = 5; // the minimum range in km depends on radar system, e.g. beyond far field.

  for (int ii = 0; ii < nGates; ii++) {

    if(inwrap) {
      
      if (phidp[ii] < (avgbg - thrs_unfold + extra)) {
	
	// Depending on the pulsing mode, if alternate H/V, the folding value should be 
	// 180 degrees; if simultaneous transmission mode (slant 45 degree), 
	// then the folding value should be 360 degrees.
	
	phidp[ii] += foldRange;
	
      }
      
      for (int jjj = 0; jjj < unfoldLen; jjj++) {
	xx[jjj] = range[ii-jjj-1];
	yy[jjj] = phidp[ii-jjj-1];
      }

      double phimn_slp, bb;
      lse(phimn_slp, bb, xx, yy, unfoldLen);
      
      if(phimn_slp > -5.0 && phimn_slp < 20.0) {
	extra += (range[ii] - range[ii-1]) * phimn_slp;
      }

    } else { // if(inwrap) 

      for (int jjj = 0; jjj < mgood; jjj++) {
	yy[jjj] = phidp[ii+jjj];
      }

      double phimn,sd_phidp;
      msr(phimn,sd_phidp,yy,mgood);

      if(ibegin == 1) {

	if(sd_phidp < thrs_phi) {
	  double avgpdp=(phidp[ii]+phidp[ii+1])/2.0;
	  if((avgpdp-avgbg) > 40.0) {
	    inwrap = true;
	  }
	}

      } else {

	if(sd_phidp < thrs_phi && range[ii] >= r_min) {

	  jcount++;
	  if(jcount == 5) {
	    double sum=0.0;
	    for (int ln = 0; ln < 5; ln++) {
	      sum = sum + phidp[ii-ln];
	    }
	    avgbg=sum/5.0;
	    ibegin=1;
	  }

	} else {

	  jcount=0;
	  
	}
	
      } // if(ibegin)

    } // if(inwrap) 

  } // ii

}
    
