// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/***************************************************************************
 * process_data.c
 *
 * Reads a beam in ds format from a radar queue, reformats it into rdata format,
 * does clipping and other processing, and writes beam to an mmq.
 *
 * Jaimi Yee
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1998
 *
 ****************************************************************************/
#include <rapformats/rData.hh>
#include "dsr2mmq.h"
using namespace std;

#ifndef PI
#define PI 3.141592653589
#endif

#define ERR_COUNT 1000
#define TOL 0.00001

/*
 * function prototypes
 */
static int  clip_beam(void);
static int  load_ray(void);
static int  check_minmax(void);
static void send_ray(void);
static void print_ray(void);

/*
 * static variables
 */
static DsRadarParams            *radarParams;
static DsRadarBeam              *radarBeam;
static vector< DsFieldParams* > *fieldParams;
static rData                    *ray;

void process_data()
{
   int contents;
   
   //
   // Get ahold of the Ds radar message components
   //
   radarParams = &(Glob->radarMsg->getRadarParams());
   radarBeam   = &(Glob->radarMsg->getRadarBeam());
   fieldParams = &(Glob->radarMsg->getFieldParams());
   ray         = new rData();

   while (TRUE) 
   { 

     PMU_auto_register("Reading radar queue");
    
     /*
      * get a message from the radar queue
      */
     if (Glob->radarQueue->getDsBeam( *(Glob->radarMsg), &contents )) {
        fprintf(stderr, "radarQueue:getDsBeam() failed.\n");
        sleep (1);
     } else { 

        /* 
         * if the parameters have not been set,
         * don't process this beam
         */
        if (!Glob->radarMsg->allParamsSet()) 
           continue;

        /*
         * update the ray with the new data
         */
        if (load_ray() == 0) {
            if (Glob->params.print_summary)
              print_ray();
            send_ray();
	}
     }
   }

   delete ray;

}

/*****************************************
 * load_ray()
 *
 * loads ray with header information and
 * data from ds radar beam
 *****************************************/
static int load_ray(void)    
{
    int nGatesCopy;

    /* 
     * Check the beam thresholds
     */
    if (check_minmax()) {
       return(-1);
    }

    /*
     * Do the clipping - i.e. get the number of gates
     * that should be copied to the ray for this beam.
     * The beam will actually be clipped when that number
     * of gates is copied into the ray
     */
    if ((nGatesCopy = clip_beam()) < 0) {
       return(-1);
    }

    /*
     * Set the ray
     */
    ray->setRay(*(Glob->radarMsg), nGatesCopy);

    return(0);
    
}

/******************************************
 * clip_beam()
 *
 * Returns the number of gates that should
 * be copied to the ray after clipping is
 * performed. 
 ******************************************/ 
static int clip_beam(void)
{
    double theta;
    double sinTheta;
    double rLeng;

    int nGatesCopy;

    /*
     * If we want to do clipping and the elevation
     * is such that clipping should be applied here, compute ngates 
     * appropriately, otherwise set ngates to the number of gates 
     * specified in the radar params
     */
    if (Glob->params.clip_beams && 
        (radarBeam->elevation >= Glob->params.min_clipping_elev)) {
         theta = ((double) radarBeam->elevation * PI)/180.0;
         sinTheta = sin(theta);
         if (sinTheta < TOL) {
	    sinTheta = TOL;
         }
         rLeng = Glob->params.clipping_alt/sinTheta;
         nGatesCopy = (int)((rLeng - radarParams->startRange) /
                         radarParams->gateSpacing + 0.5);
         if (nGatesCopy > radarParams->numGates)
            nGatesCopy = radarParams->numGates;
    } else {
       nGatesCopy = radarParams->numGates;
    }

    return(nGatesCopy);
    
}

/**********************************************
 * check_minmax()
 *
 * checks that the parameters are within thresholds
 **********************************************/
static int check_minmax(void) 
{
   if (Glob->params.check_prf) {
      if ((radarParams->pulseRepFreq < Glob->params.min_prf) || 
          (radarParams->pulseRepFreq > Glob->params.max_prf)) {
         if (Glob->params.debug) {
	    fprintf(stderr, "WARNING - %s:check_minmax\n", 
                    Glob->prog_name);
	    fprintf(stderr, "elevation = %f, azimuth = %f - ",
                    radarBeam->elevation, radarBeam->azimuth);
	    fprintf(stderr, "prf out of range\n");
         }
	 return(-1);
      }
   }
   
   if (Glob->params.check_elev) {
      if ((radarBeam->elevation < Glob->params.min_elev) ||
          (radarBeam->elevation > Glob->params.max_elev)) {
         if (Glob->params.debug) {
	    fprintf(stderr, "WARNING - %s:check_minmax\n",
                    Glob->prog_name);
	    fprintf(stderr, "elevation = %f, azimuth = %f - ",
                    radarBeam->elevation, radarBeam->azimuth);
	    fprintf(stderr, "elevation out of range\n");
         }
	 return(-1);
      }
   }
   
   return(0);
   
}


/**********************************************
 * send_ray()
 *
 * sends the ray to the mmq
 **********************************************/
static void send_ray(void)
{
   static int Mmq_id = -1;
   static int first_call = TRUE;
   static int count = 0;
   
   int retval;
   
   if (first_call) {
      Mmq_id = open_mmq();
      first_call = FALSE;
   }
   
   /*
    * check that the message queue is OK
    */

   if (Mmq_id < 0) {
      fprintf(stderr, "ERROR - %s:send_beam\n", Glob->prog_name);
      fprintf(stderr, "Mmq id not valid\n");
      tidy_and_exit(-1);
   }

   /*
    * write to message queue
    */

   retval = MMQ_write (Mmq_id, ray->getRay(), ray->getRayLength());

   if ((count == 0) && (Glob->params.debug)) {
    
     if (retval < 0) {
       fprintf(stderr, "WARNING - %s:send_beam\n",
	       Glob->prog_name);
       fprintf(stderr, "Error on write - retval = %d\n", retval);
     } else if (retval == 0) {
       fprintf(stderr, "WARNING - %s:send_beam\n",
	       Glob->prog_name);
       fprintf(stderr, "Output buffer full\n");
     }
   }
  
   count++;
   if (count >= ERR_COUNT) {
     count = 0;
   }

}

/***************************************
 * print_ray()
 *
 * prints ray information
 ***************************************/
void print_ray()
{
  static int summary_count = 0;

  if (summary_count == 0) {
     ray->printRay();
  }
      
  summary_count++;
    
  if (summary_count == Glob->params.summary_interval)
    summary_count = 0;
    
} 
