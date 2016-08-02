#define MAX_GATES 5000
#define EXTRA_GATES 6
#define FIR3ORDER 30
  
using namespace std;
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <cmath>

/////////////////////////////////////////////////////////////////////////

// This is a Linear Least Square Estimate subroutine to fit a linear
// equation for (xi,yi) (i=1,...,n), so that
//                         yi = a * xi + b
// INPUTs: x(i), y(i), n, (i=1,...,n ).
// OUTPUTs: a ( slope ), b ( intercept ).
//                                                Li Liu   Sep. 23, 92

void LSE(double &a, double &b, const double *x, const double *y, int n)
  
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
//   cerr << "6666 total,xsum,ysum,xxsum,xysum: " << ", " << total << ", " << xsum << ", " << ysum << ", " << xxsum << ", " << xysum << endl;
//   cerr << "det,a,b: " << det << ", " << a << ", " << b << endl;

}

//////////////////////////////////////////////////////////////////////////

//  To calculate the mean (ymn) and standard deviation (sd, or,
//  mean square root, msr) of the array y(i) (i=1,...,n).
//                                               Li Liu  Sep. 19, 95

void msr(double &ymn, double &sd, const double *y, int n)

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

void compute_kdp(int nGates,
		 const double *range__,
		 const double *zh__,
		 const double *zdr__,
		 const double *phidp__,
		 const double *rhohv__,
		 const double *snr__,
		 double *kdp)

{

  double fir3gain = 1.0;
  
  double fir3coef[FIR3ORDER+1] = {
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

  // copy input arrays, leaving extra space at the beginning for negative indices

  double range_[MAX_GATES + EXTRA_GATES];
  double *range = range_ + EXTRA_GATES;
  memcpy(range, range__, nGates * sizeof(double));
  
  double zh_[MAX_GATES + EXTRA_GATES];
  double *zh = zh_ + EXTRA_GATES;
  memcpy(zh, zh__, nGates * sizeof(double));
  
  double zdr_[MAX_GATES + EXTRA_GATES];
  double *zdr = zdr_ + EXTRA_GATES;
  memcpy(zdr, zdr__, nGates * sizeof(double));
  
  double phidp_[MAX_GATES + EXTRA_GATES];
  double *phidp = phidp_ + EXTRA_GATES;
  memcpy(phidp, phidp__, nGates * sizeof(double));
  
  double rhohv_[MAX_GATES + EXTRA_GATES];
  double *rhohv = rhohv_ + EXTRA_GATES;
  memcpy(rhohv, rhohv__, nGates * sizeof(double));
  
  double snr_[MAX_GATES + EXTRA_GATES];
  double *snr = snr_ + EXTRA_GATES;
  memcpy(snr, snr__, nGates * sizeof(double));

  // compute zv
  
  double zv_[MAX_GATES + EXTRA_GATES];
  double *zv = zv_ + EXTRA_GATES;
  for (int ii = 0; ii < nGates; ii++) {
    zv[ii] = zh[ii] - zdr[ii];
  }

  // allocate and initializa arrays

  double phidpAdap[MAX_GATES + EXTRA_GATES];
  double phidpAdfl[MAX_GATES + EXTRA_GATES];
  double kdpAdap[MAX_GATES + EXTRA_GATES];
  double delta[MAX_GATES + EXTRA_GATES];
  double goodMask[MAX_GATES + EXTRA_GATES];

  for (int ii = 0; ii < nGates + EXTRA_GATES; ii++) {
    phidpAdap[ii] = 0.0;
    phidpAdfl[ii] = 0.0;
    kdpAdap[ii] = 0.0;
    delta[ii] = 0;
    goodMask[ii] = 0.0; // 0.0 is good, 1.0 is bad
  }
  
  //   double tempcorr = 0.0;
  //   double tempfix = 0.0;
  //   double anitial = 0.0;
  //   double avrg2 = 0.0;
  //   int k_arr = 0;

  // initialize basic variables
  
  double r_min = 5; // the minimum range in km depends on radar system, e.g. beyond far field.

  double thres = 4.0;	// this value depends on the expected backscatter diff phase

  int madflt = 30;
  
  double thrs_phi=6; // this value for CP2
  double thrcrr=0.9;   // set low since not used
  
  // $$$$$$$ UNWRAP DIFF PHASE $$$$$$$$
  
  //   int jj_test = 0;
  int inwrap = 0;
  double extra = 0;
  int jcount = 0;

  int ibegin = 0;
  double avgbg = 0;
  
  double phinit0 = -999; // automatically decide initial phase
  if (phinit0 != -999) {
    ibegin = 1;
    avgbg = phinit0;
  } //  to adjust the reference phase by observation (ytwang)

  double thrs_unfold = 90.0; // must be checked for CP2

//   for (int ii = 0; ii < nGates; ii++) {
//     cerr << "ii,range,phidp: "
//  	 << setw(10) << ii << ", "
//  	 << setw(10) << range[ii] << ", "
//  	 << setw(10) << phidp[ii] << endl;
//   }


  // This is Yanting's modified unfold code 
  
  double phidpSave_[MAX_GATES + EXTRA_GATES];
  double *phidpSave = phidpSave_;
  double xx[31], yy[31];

  int count = 0;
  int mgood = 10;
  int mbad = 5;
  int iend = nGates - 1;

  for (int ii = 0; ii < nGates; ii++) {

    // cerr << "111111111 ii: " << ii << endl;
    
    phidpSave[ii] = phidp[ii]; // save the raw phidp to checking unfold
    
    if(inwrap == 1) {
      
      if (phidp[ii] < (avgbg - thrs_unfold + extra)) {
	
	// Depending on the pulsing mode, if alternate H/V, the folding value should be 
	// changed to 180 degrees; if simultaneous transmission mode (slant 45 degree), 
	// then the folding value should be 360 degree.
	
	phidp[ii] += 360.0;
	
      }
      
      for (int jjj = 0; jjj < 5; jjj++) {
	xx[jjj] = range[ii-jjj-1];
	yy[jjj] = phidp[ii-jjj-1];
      }

      double phimn_slp, bb;
      LSE(phimn_slp, bb, xx, yy, 5);
      
      // cerr << "5555 phimn_slp, bb: " << phimn_slp << ", " << bb << endl;

      if(phimn_slp > -5.0 && phimn_slp < 20.0) {
	extra += (range[ii] - range[ii-1]) * phimn_slp;
      }

    } else { // if(inwrap == 1) 

      for (int jjj = 0; jjj < mgood; jjj++) {
	yy[jjj] = phidp[ii+jjj];
	// cerr << "3333 yy: " << yy[jjj] << endl;
      }

      double phimn,sd_phidp;
      msr(phimn,sd_phidp,yy,mgood);
      // cerr << "4444 phimn,sd_phidp: " << phimn << ", " << sd_phidp << endl;

      if(ibegin == 1) {

	if(sd_phidp < thrs_phi) {
	  double avgpdp=(phidp[ii]+phidp[ii+1])/2.0;
	  if((avgpdp-avgbg) > 40.0) {
	    inwrap=1;
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
	    // cerr << "BEGIN ran/pbg/::" << range[ii] << ", " << avgbg << endl;
	  }

	} else {

	  jcount=0;

	}

      } // if(ibegin == 1)

    } // if(inwrap == 1) 

  } // ii

    
  // c$$$$$$$$$$$$$$$$$$$$$$$$$$$$

  //    x[ii] -- input unfolded raw phidp data;
  //    z[ii] -- updated profile;
  //    y[ii] -- filtered profile.
  ibegin = nGates - 1;

  double x_[MAX_GATES + 100];
  double *x = x_ + 100;

  double y_[MAX_GATES + 100];
  double *y = y_ + 100;

  double z_[MAX_GATES + 250];
  double *z = z_ + 250;

  for (int ii = -EXTRA_GATES; ii < nGates; ii++) {
    z[ii] = 0.0;
    x[ii] = 0.0;
  }

  // --------- FIND THE start AND stop BINs FOR FILTERING ---------------
  
  double sdPhidp_[MAX_GATES + EXTRA_GATES];
  double *sdPhidp = sdPhidp_ + EXTRA_GATES;
  
  snr[-1] = snr[0];
  int loop = 1;
  int firsttime = 1;
  int j_arr = -1;
  int k_arr = -1;
  
  int ibegin_arr[FIR3ORDER+1], iend_arr[FIR3ORDER+1];
  double r_begin_arr[FIR3ORDER+1], r_end_arr[FIR3ORDER+1];

  double init0 = 0.0;
  double init = 0.0;
  double delavg = 0.0;
  double avrg1 = 0.0;
  double avrg2 = 0.0;
  int mc = 0;
  
  for (int ii = 0; ii < nGates; ii++) {
    
    for (int jjj = 0; jjj < mgood; jjj++) {
      yy[jjj] = phidp[ii+jjj];
      //      cerr << "2222 ii,jjj,yy,phidp: " << ii << ", " << ", " << jjj << ", "
      // << yy[jjj] << endl;
    }

    
    // compute sdev of phidp
    
    double phimn,sd_phidp;
    msr(phimn,sd_phidp,yy,mgood);
    sdPhidp[ii] = sd_phidp;

    // double SNR = snr[ii];
    // double SNRslevel = 3.0;
    
    if (loop == 1) {
      
      //  ********** Find begin of GOOD data loop ***********
      
      if ((rhohv[ii] >= thrcrr) && (sdPhidp[ii] < thrs_phi) && (range[ii] >= 1.5)) {
	
	count++;

// 	cerr << "count, rhohv[ii], sdPhidp[ii], range[ii]: " <<
// 	  count << ", " << rhohv[ii] << ", " << sdPhidp[ii] << ", " << range[ii] << endl;
	
	if (count == mgood) {
	  
	  for (int il = 0; il < mgood; il++) {
	    z[ii-il] = phidp[ii-il];
// 	    cerr << "AAAA ii,il,ii-il,z: " << ii << ", " << il << ", " << ii - il << ", " << z[ii-il] << endl;
	    goodMask[ii-il] = 1;
	  }
	  
	  if (firsttime == 1) {
	    
	    ibegin = ii - mgood + 1; // begin of the 1st encountered cell
	    j_arr++;
	    ibegin_arr[j_arr] = ibegin;
	    r_begin_arr[j_arr] = range[ibegin];
	    init0=(z[ibegin]+z[ibegin+1]+z[ibegin+2]+z[ibegin+3])/4.0;
	    init = init0;    // Recorded for local trend
	    delavg = init0 - phinit0;
// 	    cerr << "7777 init0, delavg: " << init0 << ", " << delavg << endl;
// 	    cerr << "7777 j_arr, ibegin: " << j_arr << ", " << ibegin << endl;
	  } else {
	    
	    mc = ii - mgood + 1;  // begin of the successive encountered cells
	    j_arr++;
	    ibegin_arr[j_arr]=mc;
	    r_begin_arr[j_arr]=range[mc];
	    avrg1 = (z[mc+1]+z[mc+2]+z[mc+3]+z[mc+4])/4.0;
	    delavg = avrg1-avrg2;

// 	    cerr << "8888 j_arr, mc: " << j_arr << ", " << mc << endl;
// 	    cerr << "8888 avrg1, avrg2, delavg: " << avrg1 << ", " << avrg2 << ", " << delavg << endl;

	  } // if (firsttime == 1

	  if (firsttime == 1) {

	    firsttime = 0;
	    init = init0;

	  } else {

// 	    cerr << "BBBB init, delavg, avrg1, avrg2: " << init << ", " << delavg << ", " << avrg1 << ", " << avrg2 << endl;
	    if (avrg1 < init && avrg2 > init) {
	      avrg1 = avrg2;
	    } else if (avrg1 > init && avrg2 < init) {
	      avrg2 = init;
	    } else if (abs(delavg) > 15. && avrg1 < avrg2) {
	      avrg1 = avrg2;
	    }

// 	    cerr << "CCCC init, avrg1, avrg2: " << init << ", " << avrg1 << ", " << avrg2 << endl;

	    init = avrg1;     // Save local trend
	    double rmc = range[mc];
	    double rend = range[iend];
	    double d1 = rmc-rend;
	    double d2 = (avrg1-avrg2)/d1;
	    double d3 = (rend*avrg1-rmc*avrg2)/d1;
	    
	    for (int ij = iend+1; ij <= mc; ij++) {
	      double rij = range[ij];
	      z[ij] = rij*d2 - d3;
	    }

	  } // if (firsttime
	    
	  loop  = 2;
	  count = 0;
	  mgood = 10;
	  iend  = nGates - 1;
	  
	} // if (count == mgood)
	  
      } else {
	
	count = 0;
	
      } // if ((rhohv[ii] >= thrcrr ...
      
    } else if (loop == 2) { // if (loop == 1)
    
      // ************* Find END of GOOD DATA loop ****************  

      z[ii] = phidp[ii];

      // in this branch, the gate locates on good data segment
      // as we are looking for next bad data.
      
      goodMask[ii] = 1;
      
      if(ii == nGates - 1) {
	iend = nGates - 1;
	avrg2=(z[iend]+z[iend-1]+z[iend-2]+z[iend-3])/4.0;
// 	cerr << "DDDD avrg2: " << avrg2 << endl;
	continue; // go to 1000
      }

//       cerr << "GGGG ii,z,sdphidp: " << ii << ", " << z[ii] << ", " << sdPhidp[ii] << endl;
//       cerr << "thrs_phi: " << thrs_phi << endl;

      if (sdPhidp[ii] > thrs_phi || rhohv[ii] < thrcrr) {

// 	cerr << "HHHH, count, mbad: " << count << ", " << mbad << endl;

	count++;
	
	if (count == mbad) {  //Insert test to preserve hail/BB signal.
	  
	  double zhmn = 0.0;
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
	      continue; // goto 1000
	    }

	  } // if (zhmn_log >= 25.0)
	    
	  iend = ii - mbad;
// 	  cerr << "FFFF mbad,iend: " << mbad << ", " << iend << endl;
	  k_arr++;
	  
	  iend_arr[k_arr] = iend;
	  r_end_arr[k_arr] = range[iend];
	  
	  // print *,'k_arr',k_arr,iend_arr(k_arr)
	  
	  for (int jj = 0; jj < mbad; jj++) {
	    //Inserted to clean the bad Zdr & Rhv.
	    goodMask[ii-jj] = 0.0; // bad value 
	  }
	  
	  avrg2=(z[iend]+z[iend-1]+z[iend-2]+z[iend-3])/4.0;
// 	  cerr << "EEEE iend, avrg2: " << iend << ", " << avrg2 << endl;
	  if (iend == nGates) {
	    continue; // goto 1000
	  }
	  
	  z[ii] = avrg2;
	  z[ii-1] = avrg2;
	  loop = 1;
	  count = 0;

	} // if (count == mbad ...
	  
      } else {
	
	  count = 0;
	  
      } // if (sdPhidp[ii] > thrs_phi

    } // if (loop == 1)

  } // ii : 1000

  //--------------- END of FINDING start AND stop BINs -------------------

  if(ibegin == nGates - 1) {  // NO good data in whole ray. RETURN.
//     cerr << "No good data in entire beam!" << endl;
    return;
  }

  int kbegin = ibegin;
  int kend = iend;
  double ext = avrg2;

//   cerr << "JJJJ ibegin, kbegin, ext: " << ibegin << ", " << kbegin << ", " <<  ext << endl;
//   cerr << "init0: " << init0 << endl;

  for (int ii = -90; ii < kbegin; ii++) {   // Set the initial conditions
    z[ii] = init0;
  }

  for (int ii = kend + 1; ii < nGates + 30; ii++) { // Extend data record
    z[ii] = ext;
  }

  for (int ii = -91; ii < nGates + 30; ii++) { // Copy raw data array
    x[ii] = z[ii];
  }

  //------------- MAIN LOOP of Phidp Adaptive Filtering --------------------

//   cerr << "kend: " << kend << endl;

//   for (int ii = -91; ii < nGates + 30; ii++) {
//     cerr << "ii+1, z: " << ii+1 << ", " << z[ii] << endl;
//   }

  for (int mloop = 0; mloop < madflt; mloop++) {

    // TIE DOWN THE INITIAL and EXTENDING DATA RECORD

    for (int ii = -90; ii < kbegin; ii++) {   // Set the initial conditions
      z[ii] = init0;
    }
    
    for (int ii = kend + 1; ii < nGates + 30; ii++) { // Extend data record
      z[ii] = ext;
    }
    
//    for (int ii = -91; ii < nGates + 30; ii++) {
//      cerr << "222 ii+1, z: " << ii+1 << ", " << z[ii] << endl;
//    }

    // FIR FILTER SECTION

    for (int ii = -6; ii < nGates + 5; ii++) {
      
      double acc = 0.0;

      for (int j = 0; j <= FIR3ORDER; j++) {
	int jjj = ii-FIR3ORDER/2+j;
	acc = acc + fir3coef[j]*z[jjj];
// 	cerr << "PPPP j,jjj+1,acc,coeff,z: "
// 	     << j << ", "
// 	     << jjj+1 << ", "
// 	     << acc << ", "
// 	     << fir3coef[j] << ", "
// 	     << z[jjj] << endl;
      }

      y[ii]=acc*fir3gain;

//       cerr << "QQQQ mloop+1,ii+1,y[ii]: " << mloop+1 << ", " << ii+1 << ", " << y[ii] << endl;
      
    } // END of FIR FILTERING

    for (int ii = 1; ii <= nGates; ii++) {
      
      double delt = fabs(x[ii]-y[ii]);
      if (delt >= thres) {
	z[ii]=y[ii];
      } else {
	z[ii]=x[ii];
      }

    }

  } // mloop - END LOOP for Phidp Adaptive Filtering
  
//   for (int ii = -90; ii < nGates + 30; ii++) {
//     cerr << "ii+1, z: " << ii+1 << ", " << z[ii] << endl;
//   }

//   cerr << "KKKK" << endl;

  double ad1_[MAX_GATES + EXTRA_GATES];
  double *ad1 = ad1_ + EXTRA_GATES;
  
  double fl_[MAX_GATES + EXTRA_GATES];
  double *fl = fl_ + EXTRA_GATES;
  
  double df_[MAX_GATES + EXTRA_GATES];
  double *df = df_ + EXTRA_GATES;

  //  PUT KDP,DELTA,PDPCORRECTED,PDPCORRECTED-FILTERED into PLFILE

  for (int ii = -6; ii < nGates + 3; ii++) {
    ad1[ii] = z[ii];
    fl[ii] = y[ii];
    df[ii] = x[ii] - y[ii];
  }
  
//   for (int ii = -6; ii < nGates + 3; ii++) {
//     cerr << "YYYY ii+1, y: " << ii+1 << ", " << y[ii] << endl;
//   }

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
    
    //    cerr << "ii+1, zh, nadp: " << ii+1 << ", " << zh[ii] << ", " << nadp << endl;

    for (int jj = 0; jj <= nadp; jj++) {
      xx[jj] = range[ii - nadp/2 + jj];
      yy[jj] = fl[ii - nadp/2 + jj];
      // cerr << "jj, xx, yy: " << jj << ", " << xx[jj] << ", " << yy[jj] << endl;
    }
      
    // Improved Kdp base on LSE fit to Adap flt Phidp
    double aa, bb;
    LSE(aa,bb,xx,yy,nadp+1);

//     cerr << "MMMM nadp, aa, bb: " << nadp << ", " << aa << ", " << bb << endl;

    
    kdp[ii] = aa / 2.0;
    
  } // ii

}

