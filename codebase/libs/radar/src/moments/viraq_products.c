/* $Id: viraq_products.c,v 1.1 2008/03/05 05:00:25 dixon Exp $
 *
 */

#include        <math.h>

#include  <stdio.h>

#ifdef  DOS
#include        "..\include\globals.h"
#else
// #include "viraq/viraq.h"
#include "piraq/piraqx.h"
#include "rdow/products.h"
#include "products.h"
#endif

#define NO_Test_HV_Corr
#define NO_Test_VH_Corr

#define LOG2            0.69314718056
#define SMALL           0.0183156389
#define STARTGATE       5      /* start gate for power average of all gates */

#ifdef NOTDEF

/* compute 6 radar products aranged as:  */
/* velocity (m/s) */
/* power (dBm) */
/* NCP */
/* spectral width (m/s) */
/* reflectivity (dBZ) */
/* coherent reflectivity (dBZ) */

/* the products fill a floating point array with scientific parameters */
/* in regular units (i.e. m/s, dBm, dBZ, Hz ... ) */

/* note dwell->dataformat describes: 0 regular abp, 1 abpab polypulsepair, or 2 abpab dualprt */

void products(DWELL *dwell, RADAR *radar, float *prods)
   {
   switch(dwell->dataformat)
      {
      case DATA_SIMPLEPP: simplepp(dwell,radar,prods);    break;
      case DATA_POLYPP:   polypp(dwell,radar,prods);      break;
      case DATA_DUALPP:   dualprt(dwell,radar,prods);     break;
      case DATA_POL1:     dualpol1(dwell,radar,prods);     break;
      case DATA_POL2:     dualpol3(dwell,radar,prods);     break;
      case DATA_POL3:     dualpol3(dwell,radar,prods);     break;
      case DATA_SIMPLE16: newsimplepp(dwell,radar,prods);  break;
      case DATA_DOW:      newsimplepp(dwell,radar,prods);  break;
      case DATA_FULLPOL1: dualpolfull1(dwell,radar,prods);     break;
      case DATA_FULLPOLP: fullpolplus(dwell,radar,prods,0.0);     break;
      case DATA_MAXPOL:   fullpolplus(dwell,radar,prods,0.0);     break;
      case DATA_HVSIMUL:  hvsimul(dwell,radar,prods);     break;      
      case DATA_SHRTPUL:  fullpolplus(dwell,radar,prods,0.0); break;
      case DATA_SMHVSIM:  smallhvsimul(dwell,radar,prods);     break;
      default: printf("products: unrecognized data format\n"); exit(0); break;
      }
   }

/* create 16 scaled products from the simplepp moments */
void simplepp(DWELL *dwell, RADAR *radar, float *prods)      
   {
   int  i;   
   float        *aptr,*pptr;
   double       a,b,p,cp,pcorrect;
   double       logstuff,noise,velconst;
   double       dbm,widthconst,range,rconst,width,r12;
   double       sqrt();   

   rconst = radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / dwell->rcvr_pulsewidth);
   noise = (radar->noise_power > -10.0) ? 0.0 : 0.0;
   velconst = LIGHT_SPEED / (2.0 * radar->frequency * 2.0 * M_PI * dwell->prt[0]);
   pcorrect = radar->data_sys_sat
	    - 20.0 * log10(0x1000000 * dwell->rcvr_pulsewidth / 1.25E-7) 
	    - 10.0 * log10((double)dwell->hits)
	    - radar->receiver_gain;
   widthconst = (LIGHT_SPEED / radar->frequency) / dwell->prt[0] / (2.0 * sqrt(2.0) * M_PI);
   
   if(0)  velconst = -velconst;   /* fix later for velsign */

   aptr = (float *)dwell->abp;
   pptr = prods;
   range = 0.0;

   for(i=0; i<dwell->gates; i++,pptr += 16) /* 16 products */
      {
      a = *aptr++;
      b = *aptr++;
      p = *aptr++;
      
      if(i)     range = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * dwell->rcvr_pulsewidth);

      /* compute floating point, scaled, scientific products */
      *(pptr +  0) = velconst * atan2(b,a);                 /* velocity in m/s */
      *(pptr +  1) = dbm = 10.0 * log10(fabs(p)) + pcorrect;      /* power in dBm */
      *(pptr +  2) = (cp = sqrt(r12 = a*a+b*b))/p;                /* NCP no units */
      if((width = log(fabs((p-noise)/cp))) < 0.0) width = 0.0001;
      *(pptr +  3) = sqrt(width) * widthconst;  /* s.w. in m/s */
      *(pptr +  4) = dbm + rconst + range;                          /* in dBZ */
      *(pptr +  5) = 10.0 * log10(fabs(cp)) + pcorrect + rconst + range;  /* in dBZ */
      *(pptr +  6) = 0.0; 
      *(pptr +  7) = 0.0; 
      *(pptr +  8) = 0.0; 
      *(pptr +  9) = 0.0; 
      *(pptr + 10) = 0.0; 
      *(pptr + 11) = 0.0; 
      *(pptr + 12) = 0.0; 
      *(pptr + 13) = 0.0; 
      *(pptr + 14) = 0.0; 
      *(pptr + 15) = 0.0; 
      }
   }

void polypp(DWELL *dwell, RADAR *radar, float *prods)      
   {
   int  i;   
   float        *aptr,*pptr;
   double       a,b,p,a2,b2,cp,pcorrect,cp2;
   double       logstuff,noise,velconst;
   double       dbm,widthconst,range,rconst,width,r12,r22;
   double       sqrt();   

   rconst = radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / dwell->rcvr_pulsewidth);
   noise = (radar->noise_power > -10.0) ? 0.0 : 0.0;
   velconst = LIGHT_SPEED / (2.0 * radar->frequency * 2.0 * M_PI * dwell->prt);
   pcorrect = radar->data_sys_sat
	    - 20.0 * log10(0x1000000 * dwell->rcvr_pulsewidth / 1.25E-7) 
	    - 10.0 * log10((double)dwell->hits)
	    - radar->receiver_gain;
   widthconst = 0.33333333333333333 * (LIGHT_SPEED / radar->frequency) / dwell->prt / (2.0 * sqrt(2.0) * M_PI);
   
   aptr = (float *)dwell->abp;
   pptr = prods;
   range = 0.0;

   for(i=0; i<dwell->gates; i++)
      {
      a = *aptr++;
      b = *aptr++;
      p = *aptr++;
      a2 = *aptr++;
      b2 = *aptr++;
      
      r12 = a * a + b * b;
      r22 = a2 * a2 + b2 * b2;
      cp = sqrt(r12) * pow(r12/r22,0.1666666666666666);

      if(i)     range = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * dwell->rcvr_pulsewidth);

      /* compute floating point, scaled, scientific products */
      *pptr++ = velconst * atan2(-b,a);                 /* velocity in m/s */
      *pptr++ = dbm = 10.0 * log10(fabs(p)) + pcorrect;      /* power in dBm */
      *pptr++ = cp / p;                /* NCP no units */
      if((width = 0.5 * log(r12/r22)) < 0.0) width = 0.0001;
      *pptr++ = sqrt(width) * widthconst;  /* s.w. in m/s */
      *pptr++ = dbm + rconst + range;                          /* in dBZ */
      *pptr++ = 10.0 * log10(cp) + pcorrect + rconst + range;  /* in dBZ */
      pptr += 2;        /* 8 products total */
      }
   }

void dualprt(DWELL *dwell, RADAR *radar, float *prods)      
   {
   int  i;   
   float        *aptr,*pptr,*bptr;
   double       a,b,p,a2,b2,cp,pcorrect,biga,bigb;
   double       logstuff,noise,velconst;
   double       dbm,widthconst,range,rconst,width,r12;
   double       sqrt();   

   rconst = radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / dwell->rcvr_pulsewidth);
   noise = (radar->noise_power > -10.0) ? 0.0 : 0.0;
   velconst = LIGHT_SPEED / (2.0 * radar->frequency * 2.0 * M_PI * fabs(dwell->prt - dwell->prt2));
   pcorrect = radar->data_sys_sat
	    - 20.0 * log10(0x1000000 * dwell->rcvr_pulsewidth / 1.25E-7) 
	    - 10.0 * log10((double)dwell->hits)
	    - radar->receiver_gain;
   widthconst = (LIGHT_SPEED / radar->frequency) / dwell->prt / (2.0 * sqrt(2.0) * M_PI);
   
   aptr = (float *)dwell->abp;
   bptr = aptr + 3 * dwell->gates;
   pptr = prods;
   range = 0.0;

   for(i=0; i<dwell->gates; i++)
      {
      a = *aptr++;
      b = *aptr++;
      p = *aptr++;
      a2 = *bptr++;
      b2 = *bptr++;
      p += *bptr++;
      p /= 2.0;      

      if(i)     range = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * dwell->rcvr_pulsewidth);

      /* compute floating point, scaled, scientific products */
      biga = a * a2 + b * b2;
      bigb = a2 * b - a * b2;
      
      *pptr++ = velconst * atan2(bigb,biga);                 /* velocity in m/s */
      *pptr++ = dbm = 10.0 * log10(fabs(p)) + pcorrect;      /* power in dBm */
      *pptr++ = (cp = sqrt(r12 = a*a+b*b))/p;                /* NCP no units */
      if((width = log(fabs((p-noise)/cp))) < 0.0) width = 0.0001;
      *pptr++ = sqrt(width) * widthconst;  /* s.w. in m/s */
      *pptr++ = dbm + rconst + range;                          /* in dBZ */
      *pptr++ = 10.0 * log10(fabs(cp)) + pcorrect + rconst + range;  /* in dBZ */
      pptr += 2;        /* 8 products total */
      }
   }


/* finish this later */
void dualpol1(DWELL *dwell, RADAR *radar, float *prods)      
   {
   int  i;   
   float        *aptr,*pptr,*bptr;
   double       a,b,p,a2,b2,p2,cp1,cp2,v,dp,pcorrect,biga,bigb;
   double       v1a,v1b,v2a,v2b,theta,psi_d1,cp;
   double       logstuff,noise,velconst;
   double       dbm,widthconst,range,rconst,width,r12;
   double       sqrt();   

   rconst = radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / dwell->rcvr_pulsewidth);
   noise = (radar->noise_power > -10.0) ? 0.0 : 0.0;
   velconst = LIGHT_SPEED / (2.0 * radar->frequency * 2.0 * M_PI * fabs(dwell->prt));
   pcorrect = radar->data_sys_sat
	    - 20.0 * log10(0x1000000 * dwell->rcvr_pulsewidth / 1.25E-7) 
	    - 10.0 * log10((double)dwell->hits)
	    - radar->receiver_gain;
   widthconst = (LIGHT_SPEED / radar->frequency) / dwell->prt / (2.0 * sqrt(2.0) * M_PI);
   
   aptr = (float *)dwell->abp;
   bptr = aptr + 3 * dwell->gates;
   pptr = prods;
   range = 0.0;

   for(i=0; i<dwell->gates; i++)
      {
      a = *aptr++;
      b = *aptr++;
      p = *aptr++;
      a2 = *bptr++;
      b2 = *bptr++;
      p2 = *bptr++;

      if(i)     range = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * dwell->rcvr_pulsewidth);

/* compute floating point, scaled, scientific products */
      
      v1a = atan2(b,a);
      v2a = atan2(b2,a2);
      theta = v1a - v2a;
      if (theta > M_PI)
	theta -= 2.0 * M_PI;
      if (theta < -M_PI)
	theta += 2.0 * M_PI;      
      dp = theta * 0.5;
      if (dp > radar->phaseoffset + M_PI)
	dp -= M_PI;
      if (dp < radar->phaseoffset)
	dp += M_PI;
      psi_d1 = v1a - dp;
      if (psi_d1 < -M_PI)
	psi_d1 += (2.0 * M_PI);
      if (psi_d1 > M_PI)
	psi_d1 -= (2.0 * M_PI);
      v = psi_d1;
      cp1 = sqrt(a * a + b * b); 
      cp2 = sqrt(a2 * a2 + b2 * b2);
      cp = cp1 + cp2;
/*      if(dp>1.50e-02 || dp<-1.50e-02)
	dp = M_PI/2.0;
*/
      *pptr++ = velconst * v;                 /* velocity in m/s */
      *pptr++ = dbm = 10.0 * log10(fabs(p + p2)) + pcorrect;      /* power in dBm */
      *pptr++ = cp / (p + p2);                /* NCP no units */
      if((width = log(fabs((p + p2 - 2.0*noise)/cp))) < 0.0) width = 0.0001;
      *pptr++ = sqrt(width) * widthconst;  /* s.w. in m/s */
      *pptr++ = dbm + rconst + range;                          /* in dBZ */
      *pptr++ = 10.0 * log10(fabs(cp)) + pcorrect + rconst + range;  /* in dBZ */
      *pptr++ = 10.0 * log10(p2/ p);  /* Zdr */
      *pptr++ = dp * 180.0 / M_PI;                    /* PHI dp */
      }
   }


/***************************************************************************

special comments:
	1) the H and V gains must be matched with hardware because
	   the DSP's average HH and VV lag2 together.
	2) that means the two gains to adjust in software are copol
	   and cross pol, however they are labeled receiver_gain and
	   vreceiver_gain respectively.

calibration steps:
	1) Set H and V attenuators for 10 - 14 dB noise above floor
	2) Set receiver_gain and vreceiver_gain to the same value
	3) Inject a horizontal and vertical test pulse of known power.
	4) Adjust H and V IF gains for equal power
	5) Adjust the software receiver_gain to get the correct Phv power.
	6) Adjust the hardware copol attenuator to get the correct H power.
	7) Adjust the software vreceiver_gain to get the correct V Power.

****************************************************************************/
#define LOG2            0.69314718056
#define SMALL           0.0183156389
void dualpol3(DWELL *dwell, RADAR *radar, float *prods)      
   {
   int          i,avecount;   
   short        *aptr;
   float        *gate0;
   double       scale2ln,scale2db;
   double       h_channel_radar_constant,v_channel_radar_constant;
   double       angle_to_velocity_scale_factor;
   double       horiz_offset_to_dBm,verti_offset_to_dBm;
   double       cp1,v1a,lnpv,cp2,v2a,lnph,lnhv,lag2;
   double       ph_off,temp,theta,dp,lncp,v;
   double       horiz_dBm_at_coupler,verti_dBm_at_coupler;
   double       horiz_coherent_dBm_at_coupler;
   double       cross_dBm_at_coupler,laghh_dBm_at_coupler;
   double       hchan_noise_power,vchan_noise_power,lncoherent;
   double       widthconst,range_correction,coher_noise_power;
   double       linear_h_power,linear_v_power,average_h_power,average_v_power;
   
   /* initialize the things used for h and v power average */
   average_h_power = average_v_power = 0.0;
   gate0 = prods;
   avecount = 0;

   scale2ln = 0.004 * log(10.0) / 10.0;  /* from counts to natural log */
   scale2db = 0.004 / scale2ln;         /* from natural log to 10log10() */

   h_channel_radar_constant  
	= radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / dwell->rcvr_pulsewidth)
	  + 10.0 * log10(radar->peak_power / dwell->hxmit_power);

   v_channel_radar_constant 
	= radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / dwell->rcvr_pulsewidth)
	  + 10.0 * log10(radar->peak_power / dwell->vxmit_power)
	  + 2.0 * (radar->antenna_gain - radar->vantenna_gain);

   angle_to_velocity_scale_factor
	= LIGHT_SPEED / (2.0 * radar->frequency * 2.0 * M_PI * dwell->prt);

   horiz_offset_to_dBm = radar->data_sys_sat - 20.0 * log10(0x10000) 
			- radar->receiver_gain + 10.0 * log10(2.0);  /* correct for half power measurement */

   verti_offset_to_dBm = radar->data_sys_sat - 20.0 * log10(0x10000) 
			- radar->vreceiver_gain + 10.0 * log10(2.0); /* correct for half power measurement */

   widthconst = (LIGHT_SPEED / radar->frequency) / dwell->prt / (4.0 * sqrt(2.0) * M_PI);

   ph_off = 20.0 * M_PI / 180.0; /* set phase offset to be 20 deg */

   aptr = dwell->abp;
   range_correction = 0.0;

   /* these powers reflect the LNA and waveguide performance */
   /* they cannot be broken down into co an cross powers */
   hchan_noise_power = (radar-> noise_power > -10.0) ? 0.0 : exp((radar-> noise_power - horiz_offset_to_dBm) / scale2db);
   vchan_noise_power = (radar->vnoise_power > -10.0) ? 0.0 : exp((radar->vnoise_power - verti_offset_to_dBm) / scale2db);
   coher_noise_power = exp((-129.0 - verti_offset_to_dBm) / scale2db);
   
   for(i=0; i<dwell->gates; i++,prods+=16)
      {
      /* read in the raw data from structure */
      cp1  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from H to V pulse pair */
      v1a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from H to V pulse pair */
      lnpv = *aptr++ * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
      cp2  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from V to H pulse pair */
      v2a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from V to H pulse pair */
      lnph = *aptr++ * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
      lag2 = *aptr++ * scale2ln;        /* natural log  of |R(2)| from H to H + |R(2)| from V to V */
      lnhv = *aptr++ * scale2ln;        /* natural log  of V power on H xmit pulse */
      lncoherent = (cp1 + log(1 + exp(cp2 - cp1)) - LOG2);  /* natural log  of coherent power */

      /* NCP */
      /* it is best if this parameter is computed before noise correction */
      lncp = (lag2 - lnpv - log(1 + exp(lnph -  lnpv))); 
      prods[2] = exp(lncp); 
	      
      /* h/v cross correlation RHOhv (normalized dB) */
      /* it is best if this parameter is computed before noise correction */

      prods[9] = exp(cp1 + log(1 + exp(cp2 - cp1)) - (LOG2 + 0.5 * (lnph + lnpv) + 0.25 * lncp));  


      /* subtract raw noise power from the raw log powers */
      linear_h_power = exp(lnph) - hchan_noise_power;   if(linear_h_power <= 0.0)       linear_h_power = SMALL;       
      lnph = log(linear_h_power);   /* corrected h chan power */
      linear_v_power = exp(lnpv) - vchan_noise_power;   if(linear_v_power <= 0.0)       linear_v_power = SMALL;       
      lnpv = log(linear_v_power);   /* corrected v chan power */
      temp = exp(lnhv) - vchan_noise_power;       lnhv = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */
      temp = exp(lncoherent) - coher_noise_power;       lncoherent = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */

      /* convert the raw log powers to dBm at the test pulse waveguide coupler */
      horiz_dBm_at_coupler          = lnph * scale2db + horiz_offset_to_dBm;    /* HH  power in dBm */
      horiz_coherent_dBm_at_coupler = lncoherent * scale2db + horiz_offset_to_dBm;    /* HH  coherent power in dBm */
      verti_dBm_at_coupler          = lnpv * scale2db + verti_offset_to_dBm;    /* VV  power in dBm */
      cross_dBm_at_coupler          = lnhv * scale2db + horiz_offset_to_dBm;    /* HV  power in dBm (V rcvd on H xmit) */
      laghh_dBm_at_coupler          = lag2 * scale2db + horiz_offset_to_dBm;    /* HH lag2 power in dBm */

      /* average h and v power for solar cal purposes */
      if(i >= STARTGATE)
	 {
	 avecount++;
	 average_h_power += linear_h_power;
	 average_v_power += linear_v_power;
	 }

      /* compute range correction in dB. Skip the first gate */
      if(i)     range_correction = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * dwell->rcvr_pulsewidth);

      /* subtract out the system phase from v1a */
      v1a -= radar->phaseoffset;
      if(v1a < -M_PI)   v1a += 2.0 * M_PI;
      else if(v1a > M_PI)   v1a -= 2.0 * M_PI;
      
      /* add in the system phase to v2a */
      v2a += radar->phaseoffset;
      if(v2a < -M_PI)       v2a += 2.0 * M_PI;
      else if(v2a > M_PI)   v2a -= 2.0 * M_PI;
      
      /* compute the total difference */
      theta = v2a - v1a;
      if (theta > M_PI)         theta -= 2.0 * M_PI;
      else if (theta < -M_PI)   theta += 2.0 * M_PI;      
      
      /* figure the differential phase (from - 20 to +160) */
      dp = theta * 0.5;
      if (dp < -ph_off)   dp += M_PI;
      /* note: dp cannot be greater than +160, range is +/- 90 */        

      /* compute the velocity */
      v = v1a + dp;
      if (v < -M_PI)       v += 2.0 * M_PI;
      else if (v > M_PI)   v -= 2.0 * M_PI;
      
      /* velocity in m/s */
      prods[0] = v * angle_to_velocity_scale_factor;

      /* horizontal power in dBm at test pulse coupler */
      prods[1] = horiz_dBm_at_coupler;

      /* this space intentionaly left blank */
      /* NCP is computed above, before noise correction */

      /* spectral width in m/s */
      if( lncp > 0.0)   lncp = 0.0;

      prods[3] = widthconst * sqrt(-lncp);  

      /* horizontal reflectivity in dBZ */
      prods[4] = horiz_dBm_at_coupler + h_channel_radar_constant + range_correction;

      /* horizontal coherent reflectivity in dBZ */
      prods[5] = horiz_coherent_dBm_at_coupler + h_channel_radar_constant + range_correction;  

      /* differential reflectivity Zdr in dB */
      prods[6] = horiz_dBm_at_coupler + h_channel_radar_constant - verti_dBm_at_coupler - v_channel_radar_constant + radar->zdr_fudge_factor;

      /* differential phase PHI dp in degrees */
      prods[7] = dp * 180.0 / M_PI;

      /* linear depolarization LDR in dB */ 
      prods[8] = (cross_dBm_at_coupler - radar->vantenna_gain) - (horiz_dBm_at_coupler - radar->antenna_gain); /* from H pulse LDRH */

      /* this space intentionaly left blank */
      /* RHOhv is computed above, before noise correction */

      /* v power in dBm */ 
      prods[10] = verti_dBm_at_coupler;

      /* V reflectivity in dBZ */
      prods[11] = verti_dBm_at_coupler + v_channel_radar_constant + range_correction;

      /* crosspolar power in dBm */
      prods[12] = cross_dBm_at_coupler;
      }

   /* now insert the average h and v power into the gate 0 data */
   /* this is only necessary for the realtime system */
   if(avecount == 0) return;
   gate0[ 1] = log(average_h_power / (double)avecount) * scale2db + horiz_offset_to_dBm;    /* average HH  power in dBm */
   gate0[10] = log(average_v_power / (double)avecount) * scale2db + verti_offset_to_dBm;    /* average VV  power in dBm */
   gate0[ 6] = (log(average_h_power / (double)avecount) - log(average_v_power / (double)avecount)) * scale2db;    /* average ZDR in dB */
   }

void newsimplepp(DWELL *dwell, RADAR *radar, float *pptr)      
   {
   int  i;   
   short        *aptr;
   double       cp,v,p,pcorrect;
   double       velconst,dbm,widthconst,range,rconst,scale2db,scale2ln;
   double       sqrt();   

   scale2ln = 0.004 * log(10.0) / 10.0;  /* from counts to natural log */
   scale2db = 0.004 / scale2ln;         /* from natural log to 10log10() */
   velconst = LIGHT_SPEED / (2.0 * radar->frequency * 2.0 * fabs(dwell->prt) * 32768.0);
   rconst = radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / dwell->rcvr_pulsewidth);
   pcorrect = radar->data_sys_sat
	    - 20.0 * log10(0x10000) 
	    - radar->receiver_gain;
   /* NOTE: 0x10000 is just the standard offset for all systems */
   widthconst = (LIGHT_SPEED / radar->frequency) / dwell->prt / (2.0 * sqrt(2.0) * M_PI);
   
   aptr = dwell->abp;
   range = 0.0;
   
   for(i=0; i<dwell->gates; i++)
      {
      cp = *aptr++;    /* 0.004 dB / bit */
      v  = *aptr++;    /* nyquist = 65536 = +/- 32768 */
      p  = *aptr++;    /* 0.004 dB / bit */

      if(i)     range = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * dwell->rcvr_pulsewidth);

      /* compute floating point, scaled, scientific products */
      *pptr++ = velconst * v;                 /* velocity in m/s */
      *pptr++ = dbm = 0.004 * p + pcorrect;      /* power in dBm */
      *pptr++ = exp(scale2ln*(cp - p));                /* NCP no units */
      *pptr++ = sqrt(scale2ln * fabs(p-cp)) * widthconst;  /* s.w. in m/s */
      *pptr++ = dbm + rconst + range;                          /* in dBZ */
      *pptr++ = 0.004 * cp + pcorrect + rconst + range;  /* in dBZ */
      pptr += 10;
      }
   }


/***************************************************************************

	   FULL DUAL POLARIZATION MATRIX FROM ALTERNATING HV

		EXPECTS 12 PARAMETERS IN THE ABP ARRAY

special comments:
	1) the H and V gains must be matched with hardware because
	   the DSP's average HH and VV lag2 together.
	2) that means the two gains to adjust in software are copol
	   and cross pol, however they are labeled receiver_gain and
	   vreceiver_gain respectively.

calibration steps:
	1) Set H and V attenuators for 10 - 14 dB noise above floor
	2) Set receiver_gain and vreceiver_gain to the same value
	3) Inject a horizontal and vertical test pulse of known power.
	4) Adjust H and V IF gains for equal power
	5) Adjust the software receiver_gain to get the correct Phv power.
	6) Adjust the hardware copol attenuator to get the correct H power.
	7) Adjust the software vreceiver_gain to get the correct V Power.

****************************************************************************/
void dualpolfull1(DWELL *dwell, RADAR *radar, float *prods)      
   {
   int          i,avecount;   
   short        *aptr;
   float        *gate0;
   double       scale2ln,scale2db;
   double       h_channel_radar_constant,v_channel_radar_constant;
   double       angle_to_velocity_scale_factor;
   double       horiz_offset_to_dBm,verti_offset_to_dBm;
   double       cp1,v1a,lnpv,cp2,v2a,lnph,lnhv,lag2;
   double       lncrhv,vcrhv,lncrvh,vcrvh;
   double       ph_off,temp,theta,dp,lncp,v;
   double       horiz_dBm_at_coupler,verti_dBm_at_coupler;
   double       horiz_coherent_dBm_at_coupler;
   double       cross_dBm_at_coupler,laghh_dBm_at_coupler;
   double       hchan_noise_power,vchan_noise_power,lncoherent;
   double       widthconst,range_correction,coher_noise_power;
   double       linear_h_power,linear_v_power,average_h_power,average_v_power;
 
   /* initialize the things used for h and v power average */
   average_h_power = average_v_power = 0.0;
   gate0 = prods;
   avecount = 0;

   scale2ln = 0.004 * log(10.0) / 10.0;  /* from counts to natural log */
   scale2db = 0.004 / scale2ln;         /* from natural log to 10log10() */

   h_channel_radar_constant 
	= radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / dwell->rcvr_pulsewidth)
	  + 10.0 * log10(radar->peak_power / dwell->hxmit_power);

   v_channel_radar_constant 
	= radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / dwell->rcvr_pulsewidth)
	  + 10.0 * log10(radar->peak_power / dwell->vxmit_power)
	  + 2.0 * (radar->antenna_gain - radar->vantenna_gain);

   angle_to_velocity_scale_factor
	= LIGHT_SPEED / (2.0 * radar->frequency * 2.0 * M_PI * dwell->prt);

   horiz_offset_to_dBm = radar->data_sys_sat - 20.0 * log10(0x10000) 
			- radar->receiver_gain + 10.0 * log10(2.0);  /* correct for half power measurement */

   verti_offset_to_dBm = radar->data_sys_sat - 20.0 * log10(0x10000) 
			- radar->vreceiver_gain + 10.0 * log10(2.0); /* correct for half power measurement */

   widthconst = (LIGHT_SPEED / radar->frequency) / dwell->prt / (4.0 * sqrt(2.0) * M_PI);

   ph_off = 20.0 * M_PI / 180.0; /* set phase offset to be 20 deg */

   aptr = dwell->abp;
   range_correction = 0.0;

   /* these powers reflect the LNA and waveguide performance */
   /* they cannot be broken down into co an cross powers */
   hchan_noise_power = (radar-> noise_power > -10.0) ? 0.0 : exp((radar-> noise_power - horiz_offset_to_dBm) / scale2db);
   vchan_noise_power = (radar->vnoise_power > -10.0) ? 0.0 : exp((radar->vnoise_power - verti_offset_to_dBm) / scale2db);
   coher_noise_power = exp((-129.0 - verti_offset_to_dBm) / scale2db);
   
   for(i=0; i<dwell->gates; i++,prods+=16)
      {
      /* read in the raw data from structure */
      cp1  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from H to V pulse pair */
      v1a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from H to V pulse pair */
      lnpv = *aptr++ * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
      cp2  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from V to H pulse pair */
      v2a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from V to H pulse pair */
      lnph = *aptr++ * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
      lag2 = *aptr++ * scale2ln;        /* natural log  of |R(2)| from H to H + |R(2)| from V to V */
      lnhv = *aptr++ * scale2ln;        /* natural log  of V power on H xmit pulse */


      lncrhv = *aptr++ * scale2ln;        /* natural log  of cross correlation on H xmit pulse */
      vcrhv  = *aptr++ * M_PI / 32768.0;  /* radian angle of cross correlation on H xmit pulse */

      lncrvh = *aptr++ * scale2ln;        /* natural log  of cross correlation on H xmit pulse */
      vcrvh  = *aptr++ * M_PI / 32768.0;  /* radian angle of cross correlation on H xmit pulse */

      lncoherent = (cp1 + log(1 + exp(cp2 - cp1)) - LOG2);  /* natural log  of coherent power */

      /* NCP */
      /* it is best if this parameter is computed before noise correction */
      lncp = (lag2 - lnpv - log(1 + exp(lnph -  lnpv)));
      prods[2] = exp(lncp);
      
      /* h/v cross correlation RHOhv (normalized dB) */
      /* it is best if this parameter is computed before noise correction */
      prods[9] = exp(cp1 + log(1 + exp(cp2 - cp1)) - (LOG2 + 0.5 * (lnph + lnpv) + 0.25 * lncp));
      
      /* subtract raw noise power from the raw log powers */
      linear_h_power = exp(lnph) - hchan_noise_power;   if(linear_h_power <= 0.0)       linear_h_power = SMALL;       
      lnph = log(linear_h_power);   /* corrected h chan power */
      linear_v_power = exp(lnpv) - vchan_noise_power;   if(linear_v_power <= 0.0)       linear_v_power = SMALL;       
      lnpv = log(linear_v_power);   /* corrected v chan power */
      temp = exp(lnhv) - vchan_noise_power;       lnhv = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */
      temp = exp(lncoherent) - coher_noise_power;       lncoherent = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */

      /* convert the raw log powers to dBm at the test pulse waveguide coupler */
      horiz_dBm_at_coupler          = lnph * scale2db + horiz_offset_to_dBm;    /* HH  power in dBm */
      horiz_coherent_dBm_at_coupler = lncoherent * scale2db + horiz_offset_to_dBm;    /* HH  coherent power in dBm */
      verti_dBm_at_coupler          = lnpv * scale2db + verti_offset_to_dBm;    /* VV  power in dBm */
      cross_dBm_at_coupler          = lnhv * scale2db + horiz_offset_to_dBm;    /* HV  power in dBm (V rcvd on H xmit) */
      laghh_dBm_at_coupler          = lag2 * scale2db + horiz_offset_to_dBm;    /* HH lag2 power in dBm */

      /* average h and v power for solar cal purposes */
      if(i >= STARTGATE)
	 {
	 avecount++;
	 average_h_power += linear_h_power;
	 average_v_power += linear_v_power;
	 }

      /* compute range correction in dB. Skip the first gate */
      if(i)     range_correction = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * dwell->rcvr_pulsewidth);

      /* subtract out the system phase from v1a */
      v1a -= radar->phaseoffset;
      if(v1a < -M_PI)   v1a += 2.0 * M_PI;
      else if(v1a > M_PI)   v1a -= 2.0 * M_PI;
      
      /* add in the system phase to v2a */
      v2a += radar->phaseoffset;
      if(v2a < -M_PI)       v2a += 2.0 * M_PI;
      else if(v2a > M_PI)   v2a -= 2.0 * M_PI;
      
      /* compute the total difference */
      theta = v2a - v1a;
      if (theta > M_PI)         theta -= 2.0 * M_PI;
      else if (theta < -M_PI)   theta += 2.0 * M_PI;      
      
      /* figure the differential phase (from - 20 to +160) */
      dp = theta * 0.5;
      if (dp < -ph_off)   dp += M_PI;
      /* note: dp cannot be greater than +160, range is +/- 90 */        

      /* compute the velocity */
      v = v1a + dp;
      if (v < -M_PI)       v += 2.0 * M_PI;
      else if (v > M_PI)   v -= 2.0 * M_PI;
      
      /* velocity in m/s */
      prods[0] = v * angle_to_velocity_scale_factor;
#ifdef Test_HV_Corr
      prods[0] = vcrhv * angle_to_velocity_scale_factor;
      horiz_dBm_at_coupler = lncrhv * scale2db + horiz_offset_to_dBm;    /* HH  power in dBm */
#endif
#ifdef Test_VH_Corr
      prods[0] = vcrvh * angle_to_velocity_scale_factor;
      horiz_dBm_at_coupler = lncrvh * scale2db + horiz_offset_to_dBm;    /* HH  power in dBm */
#endif

      /* horizontal power in dBm at test pulse coupler */
      prods[1] = horiz_dBm_at_coupler;

prods[1] = lnhv * scale2db + horiz_offset_to_dBm; /* magnitude of ave I Q */

      /* this space intentionaly left blank */
      /* NCP is computed above, before noise correction */

      /* spectral width in m/s */
      if( lncp > 0.0)   lncp = 0.0;
      prods[3] = widthconst * sqrt(-lncp);

      /* horizontal reflectivity in dBZ */
      prods[4] = horiz_dBm_at_coupler + h_channel_radar_constant + range_correction;

      /* horizontal coherent reflectivity in dBZ */
      prods[5] = horiz_coherent_dBm_at_coupler + h_channel_radar_constant + range_correction;

      /* differential reflectivity Zdr in dB */
      prods[6] = horiz_dBm_at_coupler + h_channel_radar_constant - verti_dBm_at_coupler - v_channel_radar_constant + radar->zdr_fudge_factor;

      /* differential phase PHI dp in degrees */
      prods[7] = dp * 180.0 / M_PI;

      /* linear depolarization LDRH in dB */ 
      prods[8] = (cross_dBm_at_coupler - radar->vantenna_gain) - (horiz_dBm_at_coupler - radar->antenna_gain); /* from H pulse LDRH */
      
      /* this space intentionaly left blank */
      /* RHOhv is computed above, before noise correction */

      /* v power in dBm */ 
      prods[10] = verti_dBm_at_coupler;

      /* V reflectivity in dBZ */
      prods[11] = verti_dBm_at_coupler + v_channel_radar_constant + range_correction;

      /* crosspolar power in dBm */
      prods[12] = cross_dBm_at_coupler;
      }

   /* now insert the average h and v power into the gate 0 data */
   /* this is only necessary for the realtime system */
   if(avecount == 0) return;
   gate0[ 1] = log(average_h_power / (double)avecount) * scale2db + horiz_offset_to_dBm;    /* average HH  power in dBm */
   gate0[10] = log(average_v_power / (double)avecount) * scale2db + verti_offset_to_dBm;    /* average VV  power in dBm */
   gate0[ 6] = (log(average_h_power / (double)avecount) - log(average_v_power / (double)avecount)) * scale2db;    /* average ZDR in dB */
   }

/***************************************************************************

	   FULL DUAL POLARIZATION MATRIX FROM ALTERNATING HV

		EXPECTS 15 PARAMETERS IN THE ABP ARRAY

special comments:
	1) the H and V gains must be matched with hardware because
	   the DSP's average HH and VV lag2 together.
	2) that means the two gains to adjust in software are copol
	   and cross pol, however they are labeled receiver_gain and
	   vreceiver_gain respectively.

calibration steps:
	1) Set H and V attenuators for 10 - 14 dB noise above floor
	2) Set receiver_gain and vreceiver_gain to the same value
	3) Inject a horizontal and vertical test pulse of known power.
	4) Adjust H and V IF gains for equal power
	5) Adjust the software receiver_gain to get the correct Phv power.
	6) Adjust the hardware copol attenuator to get the correct H power.
	7) Adjust the software vreceiver_gain to get the correct V Power.
****************************************************************************/

//#define Test_LDRv
//#define Test_rhohv
//#define Test_rhovh

void hvsimul(DWELL *dwell, RADAR *radar, float *prods)      
   {
   int          i,avecount;   
   short        *aptr;
   float        *gate0;
   double       scale2ln,scale2db;
   double       h_channel_radar_constant,v_channel_radar_constant;
   double       angle_to_velocity_scale_factor;
   double       horiz_offset_to_dBm,verti_offset_to_dBm;
   double       cp1,v1a,lnpv,cp2,v2a,lnph,lnhv,lag2;
   double       lncrhv,vcrhv,lncrvh,vcrvh;
   double       ph_off,temp,theta,dp,lncp,v;
   double       horiz_dBm_at_coupler,verti_dBm_at_coupler;
   double       horiz_coherent_dBm_at_coupler;
   double       crosshv_dBm_at_coupler,crossvh_dBm_at_coupler,laghh_dBm_at_coupler;
   double       hchan_noise_power,vchan_noise_power,lncoherent;
   double       widthconst,range_correction,coher_noise_power;
   double       linear_h_power,linear_v_power,average_h_power,average_v_power;
   double       lnvh,lniq,phiq;
   double       linear_hvcross_mag,average_real_hvcross,average_imag_hvcross;
   double       linear_vhcross_mag,average_real_vhcross,average_imag_vhcross;

   /* initialize the things used for h and v power average */
   average_h_power = average_v_power = 0.0;
   /* initialize the things used for hv antenna isolation average */
   average_real_hvcross = average_imag_hvcross = 0.0;
   average_real_vhcross = average_imag_vhcross = 0.0;
   gate0 = prods;
   avecount = 0;

   scale2ln = 0.004 * log(10.0) / 10.0;  /* from counts to natural log */
   scale2db = 0.004 / scale2ln;         /* from natural log to 10log10() */

   h_channel_radar_constant 
	= radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / dwell->rcvr_pulsewidth)
	  + 10.0 * log10(radar->peak_power / dwell->hxmit_power);

   v_channel_radar_constant 
	= radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / dwell->rcvr_pulsewidth)
	  + 10.0 * log10(radar->peak_power / dwell->vxmit_power)
	  + 2.0 * (radar->antenna_gain - radar->vantenna_gain);

   angle_to_velocity_scale_factor
	= LIGHT_SPEED / (2.0 * radar->frequency * 2.0 * M_PI * dwell->prt);

   horiz_offset_to_dBm = radar->data_sys_sat - 20.0 * log10(0x10000) 
			- radar->receiver_gain + 10.0 * log10(2.0);  /* correct for half power measurement */

   verti_offset_to_dBm = radar->data_sys_sat - 20.0 * log10(0x10000) 
			- radar->vreceiver_gain + 10.0 * log10(2.0); /* correct for half power measurement */

   widthconst = (LIGHT_SPEED / radar->frequency) / dwell->prt / (4.0 * sqrt(2.0) * M_PI);

   ph_off = 20.0 * M_PI / 180.0; /* set phase offset to be 20 deg */

   aptr = dwell->abp;
   range_correction = 0.0;

   /* these powers reflect the LNA and waveguide performance */
   /* they cannot be broken down into co an cross powers */
   hchan_noise_power = (radar-> noise_power > -10.0) ? 0.0 : exp((radar-> noise_power - horiz_offset_to_dBm) / scale2db);
   vchan_noise_power = (radar->vnoise_power > -10.0) ? 0.0 : exp((radar->vnoise_power - verti_offset_to_dBm) / scale2db);
   coher_noise_power = exp((-129.0 - verti_offset_to_dBm) / scale2db);
   
   for(i=0; i<dwell->gates; i++,prods+=16)
      {
      /* read in the raw data from structure */
      cp1  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from V pulse pair */
      v1a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from V pulse pair */
      lnpv = *aptr++ * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
      cp2  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from H pulse pair */
      v2a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from H pulse pair */
      lnph = *aptr++ * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
      lnvh = *aptr++ * scale2ln;        /* natural log  of |HV*| -- used for Phidp */
      theta = *aptr++ * M_PI / 32768.0; /* radian angle of VH* ; i.e. differential phase */
      lniq = *aptr++ * scale2ln;        /* natural log  of average I and average Q from H pulses */
      phiq = *aptr++ * M_PI / 32768.0;  /* radian angle of average I and Q */

      lncoherent = (cp1 + log(1 + exp(cp2 - cp1)) - LOG2);  /* natural log  of coherent power */

      /* NCP */
      /* it is best if this parameter is computed before noise correction */
      
      lncp =  log(exp(cp1 - lnpv) + exp(cp2 - lnph)) - log(2.0);
//      lncp = cp1 - lnpv;
      prods[2] = exp(lncp);        /* average of V and H NCP's */
      
      /* h/v cross correlation RHOhv (normalized dB) */
      /* it is best if this parameter is computed before noise correction */
      prods[9] = exp(lnvh - (log(0.5) + (lnpv + log(1 + exp(lnph - lnpv)))));
      
      /* subtract raw noise power from the raw log powers */
      linear_h_power = exp(lnph) - hchan_noise_power;   if(linear_h_power <= 0.0)       linear_h_power = SMALL;       
      lnph = log(linear_h_power);   /* corrected h chan power */
      linear_v_power = exp(lnpv) - vchan_noise_power;   if(linear_v_power <= 0.0)       linear_v_power = SMALL;       
      lnpv = log(linear_v_power);   /* corrected v chan power */

      /* convert the raw log powers to dBm at the test pulse waveguide coupler */
      horiz_dBm_at_coupler          = lnph * scale2db + horiz_offset_to_dBm;    /* HH  power in dBm */
      horiz_coherent_dBm_at_coupler = lncoherent * scale2db + horiz_offset_to_dBm;    /* HH  coherent power in dBm */
      verti_dBm_at_coupler          = lnpv * scale2db + verti_offset_to_dBm;    /* VV  power in dBm */

      if(i >= STARTGATE)        /* only average past startgate */
	 {
	 avecount++;
	 
	 /* average h and v power for solar cal purposes */
	 average_h_power += linear_h_power;
	 average_v_power += linear_v_power;
	 
	 }

      /* compute range correction in dB. Skip the first gate */
      if(i)     range_correction = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * dwell->rcvr_pulsewidth);


      /* compute velocity by averaging v1a and v2a */

      v = v1a;          /* default v to v1a so compiler is happy */

      if((v1a < 0.0 && v2a < 0.0 ) || (v1a >= 0.0 && v2a >= 0.0))        
	 v = (v1a + v2a) / 2.0;
      else if(abs(v1a) < 0.5*M_PI && abs(v2a) < 0.5*M_PI)
	      v = (v1a + v2a) / 2.0;
      else
	 {
	    if(abs(v1a) >= 0.5*M_PI)
	       {
		if(v1a < 0)
		   v = (v1a + 2.0 * M_PI + v2a) / 2.0;
		if(v1a > 0)
		   v = (v1a - 2.0 * M_PI + v2a) / 2.0;
	       }
	    else if(abs(v2a) >= 0.5*M_PI)
	       {
		 if(v2a < 0)
		   v = (v2a + 2.0 * M_PI + v1a) / 2.0;
		 if(v2a > 0)
		   v = (v2a - 2.0 * M_PI + v1a) / 2.0;
	       }
	    else
	       {
		v = M_PI;
		printf("Dang\n");
	       }
	 }
      if(v > M_PI)           
	 v -= 2.0 * M_PI;
      if(v < -M_PI)
	 v += 2.0 * M_PI;


      /* figure the differential phase (from - 20 to +160) */
      
      theta -= radar->phaseoffset;
      dp = theta;
      if (dp < -ph_off)   dp += M_PI;
      /* note: dp cannot be greater than +160, range is +/- 90 */        

      /* velocity in m/s */
      prods[0] = v * angle_to_velocity_scale_factor;

      /* horizontal power in dBm at test pulse coupler */
      prods[1] = horiz_dBm_at_coupler;

      /* this space intentionaly left blank */
      /* NCP is computed above, before noise correction */

      /* spectral width in m/s */
      if( lncp > 0.0)   lncp = 0.0;
      prods[3] = widthconst * sqrt(-lncp);

      /* horizontal reflectivity in dBZ */
      prods[4] = horiz_dBm_at_coupler + h_channel_radar_constant + range_correction;

      /* horizontal coherent reflectivity in dBZ */
      prods[5] = horiz_coherent_dBm_at_coupler + h_channel_radar_constant + range_correction;

      /* differential reflectivity Zdr in dB */
      prods[6] = horiz_dBm_at_coupler + h_channel_radar_constant - verti_dBm_at_coupler - v_channel_radar_constant + radar->zdr_fudge_factor;

      /* differential phase PHI dp in degrees */
      prods[7] = dp * 180.0 / M_PI;

      /* v power in dBm */ 
      prods[8] = verti_dBm_at_coupler;

      /* this space intentionaly left blank */
      /* RHOhv is computed above, before noise correction */

      /* v power in dBm */ 
      prods[10] = verti_dBm_at_coupler;
      
      /* V reflectivity in dBZ */
      prods[11] = verti_dBm_at_coupler + v_channel_radar_constant + range_correction;
      
      /* v power in dBm */ 
      prods[12] = verti_dBm_at_coupler;
      
      }

   /* now insert the average h and v power into the gate 0 data */
   /* this is only necessary for the realtime system */
   if(avecount == 0) return;
   gate0[ 1] = log(average_h_power / (double)avecount) * scale2db + horiz_offset_to_dBm;    /* average HH  power in dBm */
   gate0[10] = log(average_v_power / (double)avecount) * scale2db + verti_offset_to_dBm;    /* average VV  power in dBm */
   }
#endif

// Added from oye's fpp.c
# ifndef DEGREES
# define DEGREES(x) ((x)*57.29577951)
# define RADIANS(x) ((x)*0.017453292)
# define UEMPTY_FLAG 2147483647
# define EMPTY_FLAG -32768
# endif

void fullpolplus(PIRAQX *dwell, short *aptr, FULL_POL_PLUS_PRODS *prodPtrs,
		 double sband_zdr_bias, double sband_h_rconst,double sband_v_rconst)
   {
   int          i,avecount;   
   float        *gate0;
   double       scale2ln,scale2db;
   double       h_channel_radar_constant,v_channel_radar_constant;
   double       angle_to_velocity_scale_factor;
   double       horiz_offset_to_dBm,verti_offset_to_dBm;
   double       cp1,v1a,lnpv,cp2,v2a,lnph,lnhv,lag2;
   double       lncrhv,vcrhv,lncrvh,vcrvh;
   double       ph_off,temp,theta,dp,lncp,v;
   double       horiz_dBm_at_coupler,verti_dBm_at_coupler;
   double       horiz_coherent_dBm_at_coupler;
   double       crosshv_dBm_at_coupler,crossvh_dBm_at_coupler,laghh_dBm_at_coupler;
   double       hchan_noise_power,vchan_noise_power,lncoherent;
   double       widthconst,range_correction,coher_noise_power;
   double       linear_h_power,linear_v_power,average_h_power,average_v_power;
   double       lnvh,lniq,phiq;
   double       linear_hvcross_mag,average_real_hvcross,average_imag_hvcross;
   double       linear_vhcross_mag,average_real_vhcross,average_imag_vhcross;

   /* initialize the things used for h and v power average */
   average_h_power = average_v_power = 0.0;
   /* initialize the things used for hv antenna isolation average */
   average_real_hvcross = average_imag_hvcross = 0.0;
   average_real_vhcross = average_imag_vhcross = 0.0;
   avecount = 0;

   scale2ln = 0.004 * log(10.0) / 10.0;  /* from counts to natural log */
   scale2db = 0.004 / scale2ln;         /* from natural log to 10log10() */


   if(sband_h_rconst > 0.0){
       dwell->h_rconst = h_channel_radar_constant = sband_h_rconst;
   }else{
       dwell->h_rconst = 
	   h_channel_radar_constant 
	   = dwell->rconst - 20.0 * log10(dwell->xmit_pulsewidth / dwell->rcvr_pulsewidth)
	   + 10.0 * log10(dwell->peak_power / dwell->xmit_power);
   }

   if(sband_v_rconst > 0.0){
       dwell->v_rconst = v_channel_radar_constant = sband_v_rconst;
   }else{
       dwell->v_rconst = 
	   v_channel_radar_constant 
	   = dwell->rconst - 20.0 * log10(dwell->xmit_pulsewidth / dwell->rcvr_pulsewidth)
	   + 10.0 * log10(dwell->peak_power / dwell->vxmit_power)
	   + 2.0 * (dwell->antenna_gain - dwell->vantenna_gain);
   }

   angle_to_velocity_scale_factor
	= LIGHT_SPEED / (2.0 * dwell->frequency * 2.0 * M_PI * dwell->prt[0]);

   horiz_offset_to_dBm = dwell->data_sys_sat - 20.0 * log10(0x10000) 
			- dwell->receiver_gain + 10.0 * log10(2.0);  /* correct for half power measurement */

   verti_offset_to_dBm = dwell->data_sys_sat - 20.0 * log10(0x10000) 
			- dwell->vreceiver_gain + 10.0 * log10(2.0); /* correct for half power measurement */

   widthconst = (LIGHT_SPEED / dwell->frequency) / dwell->prt[0] / (4.0 * sqrt(2.0) * M_PI);

   ph_off = 20.0 * M_PI / 180.0; /* set phase offset to be 20 deg */

   range_correction = 0.0;

   /* these powers reflect the LNA and waveguide performance */
   /* they cannot be broken down into co an cross powers */
   hchan_noise_power = (dwell-> noise_power > -10.0) ? 0.0 : exp((dwell-> noise_power - horiz_offset_to_dBm) / scale2db);
   vchan_noise_power = (dwell->vnoise_power > -10.0) ? 0.0 : exp((dwell->vnoise_power - verti_offset_to_dBm) / scale2db);
   coher_noise_power = exp((-129.0 - verti_offset_to_dBm) / scale2db);
   
   for(i=0; i<dwell->gates; i++)
      {
      /* read in the raw data from structure */
      cp1  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from H to V pulse pair */
      v1a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from H to V pulse pair */
      lnpv = *aptr++ * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
      cp2  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from V to H pulse pair */
      v2a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from V to H pulse pair */
      lnph = *aptr++ * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
      lag2 = *aptr++ * scale2ln;        /* natural log  of |R(2)| from H to H + |R(2)| from V to V */
      lnhv = *aptr++ * scale2ln;        /* natural log  of V power on H xmit pulse */

      lncrhv = *aptr++ * scale2ln;        /* natural log  of cross correlation on H xmit pulse */
      vcrhv  = *aptr++ * M_PI / 32768.0;  /* radian angle of cross correlation on H xmit pulse */

      lncrvh = *aptr++ * scale2ln;        /* natural log  of cross correlation on H xmit pulse */
      vcrvh  = *aptr++ * M_PI / 32768.0;  /* radian angle of cross correlation on H xmit pulse */
      
      /* below follows the "plus" parameters */

      lnvh = *aptr++ * scale2ln;        /* natural log  of H power on V xmit pulse */
      lniq = *aptr++ * scale2ln;        /* natural log  of average I and average Q from H pulses */
      phiq = *aptr++ * M_PI / 32768.0;  /* radian angle of average I and Q */

      lncoherent = (cp1 + log(1 + exp(cp2 - cp1)) - LOG2);  /* natural log  of coherent power */

      /* NCP */
      /* it is best if this parameter is computed before noise correction */
      lncp = (lag2 - lnpv - log(1 + exp(lnph -  lnpv)));

      *(prodPtrs->ncp)++ =                /* prods[2]*/
	exp(lncp);
      
      /* h/v cross correlation RHOhv (normalized dB) */
      /* it is best if this parameter is computed before noise correction */

      *(prodPtrs->rho_hv)++ =                /* prods[9]*/
	exp(cp1 + log(1 + exp(cp2 - cp1)) - (LOG2 + 0.5 * (lnph + lnpv) + 0.25 * lncp));
      
      /* subtract raw noise power from the raw log powers */
      linear_h_power = exp(lnph) - hchan_noise_power;   if(linear_h_power <= 0.0)       linear_h_power = SMALL;       
      lnph = log(linear_h_power);   /* corrected h chan power */
      linear_v_power = exp(lnpv) - vchan_noise_power;   if(linear_v_power <= 0.0)       linear_v_power = SMALL;       
      lnpv = log(linear_v_power);   /* corrected v chan power */
      temp = exp(lnhv) - vchan_noise_power;       lnhv = temp < 0.0 ? log(SMALL) : log(temp);   /* corrected cross power from V channel */
      temp = exp(lnvh) - hchan_noise_power;       lnvh = temp < 0.0 ? log(SMALL): log(temp);   /* corrected cross power H channel */
      temp = exp(lncoherent) - coher_noise_power;       lncoherent = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */

      /* convert the raw log powers to dBm at the test pulse waveguide coupler */
      horiz_dBm_at_coupler          = lnph * scale2db + horiz_offset_to_dBm;    /* HH  power in dBm */
      horiz_coherent_dBm_at_coupler = lncoherent * scale2db + horiz_offset_to_dBm;    /* HH  coherent power in dBm */
      verti_dBm_at_coupler          = lnpv * scale2db + verti_offset_to_dBm;    /* VV  power in dBm */
      crosshv_dBm_at_coupler        = lnhv * scale2db + verti_offset_to_dBm;    /* HV  power in dBm (V rcvd on H xmit) (5/18/00 MAR) */
      crossvh_dBm_at_coupler        = lnvh * scale2db + horiz_offset_to_dBm;    /* VH  power in dBm (H rcvd on V xmit) (5/18/00 MAR) */
      laghh_dBm_at_coupler          = lag2 * scale2db + horiz_offset_to_dBm;    /* HH lag2 power in dBm */


      /* compute range correction in dB. Skip the first gate */
      if(i)     range_correction = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * dwell->rcvr_pulsewidth);

      /* subtract out the system phase from v1a */
      v1a -= dwell->phaseoffset;
      if(v1a < -M_PI)   v1a += 2.0 * M_PI;
      else if(v1a > M_PI)   v1a -= 2.0 * M_PI;
      
      /* add in the system phase to v2a */
      v2a += dwell->phaseoffset;
      if(v2a < -M_PI)       v2a += 2.0 * M_PI;
      else if(v2a > M_PI)   v2a -= 2.0 * M_PI;
      
      /* compute the total difference */
      theta = v2a - v1a;
      if (theta > M_PI)         theta -= 2.0 * M_PI;
      else if (theta < -M_PI)   theta += 2.0 * M_PI;      
      
      /* figure the differential phase (from - 20 to +160) */
      dp = theta * 0.5;
      if (dp < -ph_off)   dp += M_PI;
      /* note: dp cannot be greater than +160, range is +/- 90 */        

      /* compute the velocity */
      v = v1a + dp;
      if (v < -M_PI)       v += 2.0 * M_PI;
      else if (v > M_PI)   v -= 2.0 * M_PI;
      
      /* velocity in m/s */
      *(prodPtrs->horizVelocity)++ = v * angle_to_velocity_scale_factor;

      /* horizontal power in dBm at test pulse coupler */
      *(prodPtrs->horizPower)++ = horiz_dBm_at_coupler;

      /* this space intentionaly left blank */
      /* NCP is computed above, before noise correction */

      /* spectral width in m/s */
      if( lncp > 0.0)   lncp = 0.0;
      *(prodPtrs->width)++ = widthconst * sqrt(-lncp);

      /* horizontal reflectivity in dBZ */
      *(prodPtrs->horizReflect)++ =
	horiz_dBm_at_coupler + h_channel_radar_constant + range_correction;

      /* horizontal coherent reflectivity in dBZ */
      *(prodPtrs->horizCoherentReflect)++ =
	horiz_coherent_dBm_at_coupler + h_channel_radar_constant + range_correction;

      /* differential reflectivity Zdr in dB */
      *(prodPtrs->zdr)++ =
	horiz_dBm_at_coupler + h_channel_radar_constant - verti_dBm_at_coupler - v_channel_radar_constant + dwell->zdr_fudge_factor + sband_zdr_bias;

      /* differential phase PHI dp in degrees */
      *(prodPtrs->phi_dp)++ =
	dp * 180.0 / M_PI;

      /* this space intentionaly left blank */
      /* RHOhv is computed above, before noise correction */

      /* v power in dBm */ 
      *(prodPtrs->vertPower)++ =
        verti_dBm_at_coupler;

      /* V reflectivity in dBZ */
      *(prodPtrs->vertReflect)++ =
	verti_dBm_at_coupler + v_channel_radar_constant + range_correction;

      /* linear depolarization LDR in dB */ 
      *(prodPtrs->ldr)++ =
	(crosshv_dBm_at_coupler - dwell->vantenna_gain) - (horiz_dBm_at_coupler - dwell->antenna_gain); /* from H pulse LDRH */

      /* Ldr Vertical */
      //*(prodPtrs->vertLdr)++ = 
      // (crossvh_dBm_at_coupler - rhdr->antenna_gain) - (verti_dBm_at_coupler - rhdr->vantenna_gain); /* from V pulse LDRV "plus" parameter */  
      *(prodPtrs->vertLdr)++ =   (crossvh_dBm_at_coupler - dwell->antenna_gain) - (verti_dBm_at_coupler - dwell->vantenna_gain); /* from V pulse LDRV "plus" parameter */ 

      /* crosspolar power in dBm ("DX") */
       *(prodPtrs->hvPwr)++ = crosshv_dBm_at_coupler;

      /* crosspolar power in dBm ("DW") */
       *(prodPtrs->vhPwr)++ = crossvh_dBm_at_coupler;

       /* cross correlation rho horizontal */
       /* magnitude */
       *(prodPtrs->horizRho)++ = exp( lncrhv -0.5 * ( lnhv + lnph ));
       
       /* angle */
       *(prodPtrs->horizRhoAng)++ = DEGREES( vcrhv );
       
       /* cross correlation rho vertical */
       /* magnitude */
       *(prodPtrs->vertRho)++ =
	 exp( lncrvh -0.5 * ( lnpv + lnvh ));

       /* angle */
       *(prodPtrs->vertRhoAng)++ = DEGREES( vcrvh );

       /* IQ magnitude */
       *(prodPtrs->iqNorm)++ = lniq * scale2db + horiz_offset_to_dBm;

       /* IQ angle */
       *(prodPtrs->iqAng)++ = DEGREES( phiq );

      }
   }
// end from oye's

extern int DumpGate;

#ifdef OBSOLETE  // old version of 2nd frequency code

void smallhvsimul(PIRAQX *dwell,  short *aptr, SMALLHV_SIMUL_PRODS *prodPtrs)      
   {
   int          i,avecount;   
//   short        *aptr;
//   float        *gate1;
   double       scale2ln,scale2db;
   double       h_channel_radar_constant,v_channel_radar_constant;
   double       angle_to_velocity_scale_factor;
   double       horiz_offset_to_dBm,verti_offset_to_dBm;
   double       cp1,v1a,lnpv,cp2,v2a,lnph,lnhv,lag2;
   double       lncrhv,vcrhv,lncrvh,vcrvh;
   double       ph_off,temp,theta,dp,lncp,lncpv,lncpvh,v;
   double       horiz_dBm_at_coupler,verti_dBm_at_coupler;
   double       horiz_coherent_dBm_at_coupler;
   double       crosshv_dBm_at_coupler,crossvh_dBm_at_coupler,laghh_dBm_at_coupler;
   double       hchan_noise_power,vchan_noise_power,lncoherent;
   double       widthconst,range_correction,coher_noise_power;
   double       linear_h_power,linear_v_power,average_h_power,average_v_power;
   double       lnvh,lniq,phiq;
   double       linear_hvcross_mag,average_real_hvcross,average_imag_hvcross;
   double       linear_vhcross_mag,average_real_vhcross,average_imag_vhcross;
   short *dataBuf = aptr;  // for diagnostics

   /* diagnostic variables */
   double	horizDbz, horizPower;

   /* initialize the things used for h and v power average */
   average_h_power = average_v_power = 0.0;
   /* initialize the things used for hv antenna isolation average */
   average_real_hvcross = average_imag_hvcross = 0.0;
   average_real_vhcross = average_imag_vhcross = 0.0;
   /* gate1 = prods + 16; */
   avecount = 0;

   scale2ln = 0.004 * log(10.0) / 10.0;  /* from counts to natural log */
   scale2db = 0.004 / scale2ln;         /* from natural log to 10log10() */

   dwell->h_rconst = 
     h_channel_radar_constant = dwell->h_rconst
       = dwell->rconst - 20.0 * log10(dwell->xmit_pulsewidth / dwell->rcvr_pulsewidth)
	  + 10.0 * log10(dwell->peak_power / dwell->xmit_power);

   dwell->v_rconst = 
     v_channel_radar_constant  = dwell->v_rconst
       = dwell->rconst - 20.0 * log10(dwell->xmit_pulsewidth / dwell->rcvr_pulsewidth)
	 + 10.0 * log10(dwell->peak_power / dwell->vxmit_power)
	   + 2.0 * (dwell->antenna_gain - dwell->vantenna_gain);

   angle_to_velocity_scale_factor
	= LIGHT_SPEED /
       (2.0 * dwell->frequency * 2.0 * M_PI * dwell->prt[0]);

   horiz_offset_to_dBm = dwell->data_sys_sat - 20.0 * log10(0x10000) 
			- dwell->receiver_gain;  /* DO NOT correct for half power measurement */

   verti_offset_to_dBm = dwell->data_sys_sat - 20.0 * log10(0x10000) 
			- dwell->vreceiver_gain; /* DO NOT correct for half power measurement */

   widthconst = (LIGHT_SPEED / dwell->frequency) / dwell->prt[0] / (4.0 * sqrt(2.0) * M_PI);

//!!!   ph_off = 20.0 * M_PI / 180.0; /* set phase offset to be 20 deg */

   // aptr = dwell->abp;
   range_correction = 0.0;

   /* these powers reflect the LNA and waveguide performance */
   /* they cannot be broken down into co an cross powers */
   hchan_noise_power = (dwell-> noise_power > -10.0) ? 0.0 : exp((dwell-> noise_power - horiz_offset_to_dBm) / scale2db);
   vchan_noise_power = (dwell->vnoise_power > -10.0) ? 0.0 : exp((dwell->vnoise_power - verti_offset_to_dBm) / scale2db);
   coher_noise_power = exp((-129.0 - verti_offset_to_dBm) / scale2db);
   
   for(i=0; i<dwell->gates; i++)
      {
      /* read in the raw data from structure */
      cp1  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from V pulse pair */
      v1a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from V pulse pair */
      lnpv = *aptr++ * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
      cp2  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from H pulse pair */
      v2a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from H pulse pair */
      lnph = *aptr++ * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
      lnvh = *aptr++ * scale2ln;        /* natural log  of |HV*| -- used for Phidp */
      theta = *aptr++ * M_PI / 32768.0; /* radian angle of VH* ; i.e. differential phase */

      lncoherent = (cp1 + log(1 + exp(cp2 - cp1)) - LOG2);  /* natural log  of coherent power */

      /* NCP  */
      
      lncp = cp2 - lnph;
      *(prodPtrs->ncp)++ =                /* prods[2]*/ exp(lncp);        /* copolar NCP */
      
      /* this product isn't meaningful until we transmit simultaneously,
         but we'll leave it in for now
      */
      /* h/v cross correlation RHOhv (normalized dB) */
      /* it is best if this parameter is computed before noise correction */
      *(prodPtrs->rho_hv)++ =                /* prods[9]*/
          exp(lnvh - (log(0.5) + (lnpv + log(1 + exp(lnph - lnpv)))));
      
      /* subtract raw noise power from the raw log powers */
      linear_h_power = exp(lnph) - hchan_noise_power;   if(linear_h_power <= 0.0)       linear_h_power = SMALL;       
      lnph = log(linear_h_power);   /* corrected h chan power */
      linear_v_power = exp(lnpv) - vchan_noise_power;   if(linear_v_power <= 0.0)       linear_v_power = SMALL;       
      lnpv = log(linear_v_power);   /* corrected v chan power */

      /* convert the raw log powers to dBm at the test pulse waveguide coupler */
      horiz_dBm_at_coupler          = lnph * scale2db + horiz_offset_to_dBm;    /* HH  power in dBm */
      horiz_coherent_dBm_at_coupler = lncoherent * scale2db + horiz_offset_to_dBm;    /* HH  coherent power in dBm */
      verti_dBm_at_coupler          = lnpv * scale2db + verti_offset_to_dBm;    /* VV  power in dBm */

      if(i >= STARTGATE)        /* only average past startgate */
	 {
	 avecount++;
	 
	 /* average h and v power for solar cal purposes */
	 average_h_power += linear_h_power;
	 average_v_power += linear_v_power;
	 
	 }

      /* compute range correction in dB. Skip the first gate */
      if(i)     range_correction = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * dwell->rcvr_pulsewidth);

      /* compute velocity by averaging v1a and v2a */
      v = atan2(exp(cp1) * sin(v1a) + exp(cp2) * sin(v2a),exp(cp1) * cos(v1a) + exp(cp2) * cos(v2a));

      /* figure the differential phase (from - 20 to +160) */
      theta -= dwell->phaseoffset;
      dp = theta;
      if (dp < -ph_off)   dp += M_PI;
      /* note: dp cannot be greater than +160, range is +/- 90 */        
      /* velocity in m/s */
      *(prodPtrs->horizVelocity)++ = v * angle_to_velocity_scale_factor;

      /* horizontal power in dBm at test pulse coupler */
      *(prodPtrs->horizPower)++ = horizPower = horiz_dBm_at_coupler;

      /* this space intentionaly left blank */
      /* NCP is computed above, before noise correction */

      /* spectral width in m/s */
      if( lncp > 0.0)   lncp = 0.0;
      *(prodPtrs->width)++ = widthconst * sqrt(-lncp);

      /* horizontal reflectivity in dBZ */
      *(prodPtrs->horizReflect)++ = horizDbz  =
          horiz_dBm_at_coupler + h_channel_radar_constant + range_correction;

      /* horizontal coherent reflectivity in dBZ */
      *(prodPtrs->horizCoherentReflect)++ =
          horiz_coherent_dBm_at_coupler + h_channel_radar_constant + range_correction;

      /* differential reflectivity Zdr in dB */
      *(prodPtrs->zdr)++ =
          horiz_dBm_at_coupler + h_channel_radar_constant - verti_dBm_at_coupler - v_channel_radar_constant + dwell->zdr_fudge_factor;

      /* differential phase PHI dp in degrees */
      *(prodPtrs->phi_dp)++ =
          -dp * 180.0 / M_PI;

      /* v power in dBm */ 
      *(prodPtrs->vertPower)++ =
        verti_dBm_at_coupler;

      /* this space intentionaly left blank */
      /* RHOhv is computed above, before noise correction */
      
      /* V reflectivity in dBZ */
      *(prodPtrs->vertReflect)++ =
           verti_dBm_at_coupler + v_channel_radar_constant + range_correction;
      
      if (i == DumpGate) {
      	int junk = i;  /* good place to pause*/
      }
      }

   /* now insert the average h and v power into the gate 1 data */
   /* this is only necessary for the realtime system */
#ifdef REALTIME
   if(avecount == 0) return;
   gate1[ 1] = log(average_h_power / (double)avecount) * scale2db + horiz_offset_to_dBm;    /* average HH  power in dBm */
   gate1[10] = log(average_v_power / (double)avecount) * scale2db + verti_offset_to_dBm;    /* average VV  power in dBm */
#endif
}
#endif
