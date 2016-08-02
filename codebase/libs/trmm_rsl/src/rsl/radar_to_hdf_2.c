/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            Mike Kolander
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_LIBTSDISTK
/******************************************************************

	 Subroutines to write one VOS from a RSL radar structure into one
	 1B-51/1C-51 HDF file.

	 A 1B-51/1C-51 HDF file contains multiple VOS's recorded by a radar
	 site during a 1-hour time period, and the HDF file is named using
	 the date/hour of the constituent VOS's.

	 All functions defined herein build the components of the TSDIS
	 toolkit 'L1B_1C_GV' structure using the data from a RSL radar
	 structure. These functions are executed via a call from the top-level
	 RSL function 'RSL_radar_to_hdf()', defined in RSL file
	 'radar_to_hdf_1.c'.

  -----------------------------------------------------------------
	 Libraries required for execution of this code :
      -ltsdistk                    : TSDIS toolkit
      -lmfhdf -ldf -ljpeg -lz      : HDF
      -lrsl                        : rsl
      -lm                          : C math

  -----------------------------------------------------------------
*******************************************************************/

#include <math.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/* TSDIS toolkit function and structure definitions. */
#include "IO.h"
#include "IO_GV.h"
/* RSL function and structure definitions. */
#include <trmm_rsl/rsl.h> 
/* Parameter definitions for 1B-51 and 1C-51 HDF
	 file handling applications using the TSDIS toolkit. */
#include "toolkit_1BC-51_appl.h"


/*************************************************************/
/*                                                           */
/*                    Function Prototypes                    */
/*                                                           */
/*************************************************************/
static int julian(int year, int mo, int day);
int8 ***parmData1byteBuild(Volume *v, int pindex, VosSize *vs);
int16 ***parmData2byteBuild(Radar *radar, PARAMETER_DESCRIPTOR *parmDesc,
														int vindex, int pindex, VosSize *vs);
void cellRangeVectorFill(CELL_RANGE_VECTOR *cellRangeVector,
												 int vindex, int pindex, VosSize *vs);
void parmDescFill(PARAMETER_DESCRIPTOR *parmDesc, Radar *radar, int vindex);
PARAMETER *parmBuild(Radar *radar, VosSize *vs, int vindex,
										 int pindex);
void rayInfoFill(int32 rayInfoInteger[MAX_SWEEP][MAX_RAY][7],
								 float32 rayInfoFloat[MAX_SWEEP][MAX_RAY][4],
								 Radar *radar, VosSize *vs);
void sweepInfoFill(SWEEP_INFO sweepInfo[MAX_SWEEP], Radar *radar, VosSize *vs);
void radarDescFill(RADAR_DESCRIPTOR *radarDesc, Radar *radar, int vindex,
									 VosSize *vs);
void sensorFill(SENSORS *sensor, Radar *radar, VosSize *vs, int productID);
struct tm *timeUTC(void);
void volDesFill(VOLUME_DESCRIPTORS *volDes, Radar_header *h, VosSize *vs);
void commentsFill(char *comments, VosSize *vs, Radar *radar,
									float *qcParm, int productID);
L1B_1C_GV *gvl1Build(Radar *radar, float *qcParm, VosSize *vs,
										 int productID);

extern int nextVolume(Radar *radar, int last_volume);
extern int radar_verbose_flag;
extern Ray *first_ray_in_volume[MAX_RADAR_VOLUMES];

/*************************************************************/
/*                                                           */
/*                          julian                           */
/*                                                           */
/*************************************************************/
static int daytab[2][13] = {
  {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
  {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

static int julian(int year, int mo, int day)
{
/* Converts a calendar date (month, day, year) to a Julian date. 
	 Returns:
	   Julian day.
*/
  int leap;

  leap = (year%4 == 0 && year%100 != 0) || year%400 == 0;
  return(day + daytab[leap][mo-1]);
}

/*************************************************************/
/*                                                           */
/*                   parmData1byteBuild                      */
/*                                                           */
/*************************************************************/
int8 ***parmData1byteBuild(Volume *v, int pindex, VosSize *vs)
/* Move all data from one mask volume of the RSL structure into
	 the array parmData1byte[][][] .
*/
{
	int sindex, rindex, bindex;  /* Indices for rsl arrays. */
	int tk_sindex, tk_rindex;    /* Indices for toolkit arrays. */
	int ncell;
	int8 ***data1byte;
	Ray *ray;
	
	ncell = vs->rsl.ncell[pindex][0];
	data1byte = (int8 ***)TKnewParmData1byte(vs->tk.nsweep, vs->rsl.maxNray,
																					 ncell);
	/* Move data values from all non-NULL rsl sweeps into the
		 'parmData1byte' array. */
	sindex = -1;
	for (tk_sindex=0; tk_sindex<vs->tk.nsweep; tk_sindex++)
	{
		if (vs->tk.ncell[tk_sindex][pindex] == 0)
		{
			/* No data from this parm in this physical sweep.
				 Fill bins of this 'parmData1byte' sweep with 0 */
			for (rindex=0; rindex<vs->rsl.maxNray; rindex++)
		    for (bindex=0; bindex<ncell; bindex++)
			    data1byte[tk_sindex][rindex][bindex] = (int8) 0;
			continue;
		}
		sindex++;
		tk_rindex = -1;
		/* Move data values from all non-NULL rsl rays into the
			 'parmData1byte' array. */
		for (rindex=0; rindex<v->sweep[sindex]->h.nrays; rindex++)
		{
			if (v->sweep[sindex]->ray[rindex] == NULL) continue;
			tk_rindex++;
			ray = v->sweep[sindex]->ray[rindex];
			/* Move the rsl bin values which exist into the 'parmData1byte' ray.*/
			for (bindex=0; bindex<vs->tk.ncell[tk_sindex][pindex]; bindex++)
			{
			  if (bindex >= ray->h.nbins)  /* Short rsl ray? */
				data1byte[tk_sindex][tk_rindex][bindex] = (int8) 0;
			  else  /* Valid bin */
			    data1byte[tk_sindex][tk_rindex][bindex] = (int8) ray->range[bindex];
			} /* end for (bindex=0... */
			/* Fill all remaining bins of 'parmData1byte' ray with 0. */
			for (bindex=vs->tk.ncell[tk_sindex][pindex]; bindex<ncell; bindex++)
			  data1byte[tk_sindex][tk_rindex][bindex] = (int8) 0;
		} /* end (rindex=0;... */

		/* Fill bins of all remaining rays of this sweep with 0. */
		for (rindex=tk_rindex+1; rindex<vs->rsl.maxNray; rindex++)
		  for (bindex=0; bindex<ncell; bindex++)
			  data1byte[tk_sindex][rindex][bindex] = (int8) 0;
	} /* end for (tk_sindex=0;...*/

	return(data1byte);
}

/*************************************************************/
/*                                                           */
/*                   parmData2byteBuild                      */
/*                                                           */
/*************************************************************/
int16 ***parmData2byteBuild(Radar *radar, PARAMETER_DESCRIPTOR *parmDesc,
														 int vindex, int pindex, VosSize *vs)
/* Move all ray data from one volume of the RSL structure into
	 the array parmData2byte[][][] . Data values placed into 
	 parmData2byte[][][] are scaled. 
*/
{
  int sindex, rindex, bindex;  /* Indices for rsl arrays. */
  int tk_sindex, tk_rindex;    /* Indices for toolkit arrays. */
  int ncell;
  float value;
  Volume *v;
  Ray *ray;
  int16 ***data2byte;	

  ncell = vs->rsl.ncell[pindex][0];
  data2byte = (int16 ***)TKnewParmData2byte(vs->tk.nsweep, vs->rsl.maxNray, ncell);
  v = radar->v[vindex];
  /* Move data values from all non-NULL rsl sweeps into the
	 'parmData2byte' array. */
  sindex = -1;
  for (tk_sindex=0; tk_sindex<vs->tk.nsweep; tk_sindex++)
  {
	if (vs->tk.ncell[tk_sindex][pindex] == 0)
	{
	  /* No data from this parm in this physical sweep.
		 Fill bins of this 'parmData2byte' sweeps with NO_VALUE. */
	  for (rindex=0; rindex<vs->rsl.maxNray; rindex++)
		for (bindex=0; bindex<ncell; bindex++)
		  data2byte[tk_sindex][rindex][bindex] = (int16) NO_VALUE;
	  continue;
	}
	sindex++;
	tk_rindex = -1;
	/* Move data values from all non-NULL rsl rays into the
	   'parmData2byte' array. */
	for (rindex=0; rindex<v->sweep[sindex]->h.nrays; rindex++)
	{
	  if (v->sweep[sindex]->ray[rindex] == NULL) continue;
	  tk_rindex++;
	  ray = v->sweep[sindex]->ray[rindex];
	  /* Move the rsl bin values which exist into the 'parmData2byte' ray.*/
	  for (bindex=0; bindex<vs->tk.ncell[tk_sindex][pindex]; bindex++)
	  {
		/* if short rsl ray, fill cell with NO_VALUE */
		if (bindex >= ray->h.nbins)
		{
		  data2byte[tk_sindex][tk_rindex][bindex] = (int16) NO_VALUE;
		  continue;
		}
		value = v->h.f(ray->range[bindex]);
		if (value >= NOECHO)  /* Handle anomalous condition flags */
		{
		  if (value == BADVAL)
			data2byte[tk_sindex][tk_rindex][bindex] = (int16) NO_VALUE;
		  else if (value == RFVAL)
			data2byte[tk_sindex][tk_rindex][bindex] = (int16) RNG_AMBIG_VALUE;
		  else if (value == APFLAG)
			data2byte[tk_sindex][tk_rindex][bindex] = (int16) AP_VALUE;
		  else
			data2byte[tk_sindex][tk_rindex][bindex] = (int16) NOECHO_VALUE;
		}
		else  /* Valid rsl data */
		{
		  if (vindex == CZ_INDEX)  /* Corrected Z data */
			{
			  /* CZ = DZ + dzCal - mask_val * X  ... From Ferrier memo. */
			  value = (value + v->h.calibr_const -
					   radar->v[MZ_INDEX]->sweep[sindex]->ray[rindex]->range[bindex] * X);
			}
		  else if (vindex == CD_INDEX)  /* Corrected differential Z data */
			{
			  /* CD = ZD - mask_val * X */
			  value = value -
				radar->v[MD_INDEX]->sweep[sindex]->ray[rindex]->range[bindex] * X;
			}
		  /* Apply scale and offset factors, then store value in 
			 parmData2byte structure. */
		  data2byte[tk_sindex][tk_rindex][bindex] = (int16)
			(value * parmDesc->scaleFactor + parmDesc->offsetFactor);
		} /* end else Valid rsl data */
	  } /* end for (bindex=0;... */

	  /* Fill all remaining bins of 'parmData2byte' ray with NO_VALUE. */
	  for (bindex=vs->tk.ncell[tk_sindex][pindex]; bindex<ncell; bindex++)
		data2byte[tk_sindex][tk_rindex][bindex] = (int16) NO_VALUE;
	} /* end (rindex=0;... */

	/* Fill bins of all remaining rays of this sweep with NO_VALUE. */
	for (rindex=tk_rindex+1; rindex<vs->rsl.maxNray; rindex++)
	  for (bindex=0; bindex<ncell; bindex++)
		data2byte[tk_sindex][rindex][bindex] = (int16) NO_VALUE;
  } /* end for (sindex=0;... */
	
  return(data2byte);
}

/*************************************************************/
/*                                                           */
/*                   cellRangeVectorFill                     */
/*                                                           */
/*************************************************************/
void cellRangeVectorFill(CELL_RANGE_VECTOR *cellRangeVector,
												 int vindex, int pindex, VosSize *vs)
{
/* Fill in the cellRangeVector values.
	 cellRangeVector[j] = distance(m) from radar to center of cell[j] .
*/
	int j;
	Ray_header *ray_head;
	
	cellRangeVector->numOfCells = (int32) vs->rsl.ncell[pindex][0];
	ray_head = &first_ray_in_volume[vindex]->h;
	
	/* Do first cell (m) */
	cellRangeVector->distanceToCell[0] = (float32) (ray_head->range_bin1 +
																									0.5 * ray_head->gate_size);

	/* Do remaining cells. Just add gate_size to previous cellRange value. */
	for (j=1; j<cellRangeVector->numOfCells; j++)
	{
		cellRangeVector->distanceToCell[j] = (float32)
		     (cellRangeVector->distanceToCell[j-1] + ray_head->gate_size);
	}
}

/*************************************************************/
/*                                                           */
/*                      parmDescFill                         */
/*                                                           */
/*************************************************************/
void parmDescFill(PARAMETER_DESCRIPTOR *parmDesc, Radar *radar, int vindex)
{
	static char *parm_list[20][3] = 
	{
		{ "Z",      "dBz",    "Reflectivity"                     },
		{ "V",      "m/s",    "Radial Velocity"                  },
		{ "SW",     "m2/s2",  "Spectral Width"                   },
		{ "QCZ",    "dBz",    "QC'ed Reflectivity"               },
		{ "ZT",     "dBz",    "Total Reflectivity"               },
		{ "DR",     "?",      "Differential reflectivity"        },
		{ "LR",     "?",      "Differential reflectivity"        },
		{ "ZDR",    "dB",     "Differential Reflectivity"        },
		{ "DM",     "dBm",    "Received power"                   },
		{ "RH",     "-",      "Correlation Coefficient"          },
		{ "PH",     "?",      "Phi"                              },
		{ "XZ",     "dBz",    "X-band Reflectivity"              },
		{ "QCZDR",  "dB",     "QC'ed Differential Reflectivity"  },
		{ "QCMZ",   "-",      "Z Mask"                           },
		{ "QCMZDR", "-",      "ZDR Mask"                         },
		{ "ZE",     "-",      "Edited Reflectivity"              },
		{ "VE",     "-",      "Edited Velocity"                  },
		{ "--",     "-",      "*******"                          },
		{ "--",     "-",      "*******"                          },
		{ "--",     "-",      "*******"                          }
	};

  strncpy(parmDesc->parmName, parm_list[vindex][0], 7);
	strncpy(parmDesc->parmDesc, parm_list[vindex][2], 39); 
	strncpy(parmDesc->parmUnits, parm_list[vindex][1], 7);
	parmDesc->interPulsePeriod = (int16) 0;
	parmDesc->transFreq = (int16) 1;
	/* Receiver Bandwidth (MHz) */
	parmDesc->receiverBandwidth = (float32) 0.0;
	/* Pulse width (m) */
  parmDesc->pulseWidth = (int16) (300.0 * 
												  first_ray_in_volume[vindex]->h.pulse_width);
	parmDesc->polarTransWave = (int16) 0;
	parmDesc->numOfsamples = (int16) 0;
	/* No thresholding done in 1B-51, 1C-51 HDF files. */
	strncpy(parmDesc->thresholdField, "NONE", 8);
	parmDesc->thresholdValue = (float32) 0.0;
	parmDesc->offsetFactor = (float32) 0.0;
	if ((vindex == MZ_INDEX) || (vindex == MD_INDEX))
	{
	  parmDesc->parmDataType = (int16) 1;   /* 1_byte mask value. */
		parmDesc->scaleFactor = (float32) 1.0;
		parmDesc->deletedOrMissDataFlag = (int32) 0;
	}
	else  /* 2_byte data value. */
	{
	  parmDesc->parmDataType = (int16) 2;
		parmDesc->scaleFactor = (float32) SCALE_FACTOR;
		parmDesc->deletedOrMissDataFlag = (int32) NO_VALUE;
	}
}

/*************************************************************/
/*                                                           */
/*                       parmBuild                           */
/*                                                           */
/*************************************************************/
PARAMETER *parmBuild(Radar *radar, VosSize *vs, int vindex,
										 int pindex)
{
	PARAMETER *parm;
	
	/* Allocate memory for a new parameter structure. */
	parm = (PARAMETER *)TKnewGVL1parm();

	parmDescFill(&parm->parmDesc, radar, vindex);
	cellRangeVectorFill(&parm->cellRangeVector, vindex, pindex, vs);
	if ((vindex == MZ_INDEX) || (vindex == MD_INDEX)) /* Mask? */
	  parm->parmData1byte = (int8 ***)parmData1byteBuild(radar->v[vindex],
																											pindex, vs);
	else
	  parm->parmData2byte = (int16 ***)parmData2byteBuild(radar, &parm->parmDesc,
																											 vindex, pindex, vs);
	return(parm);
}

/*************************************************************/
/*                                                           */
/*                         rayInfoFill                       */
/*                                                           */
/*************************************************************/
void rayInfoFill(int32 rayInfoInteger[MAX_SWEEP][MAX_RAY][7],
								 float32 rayInfoFloat[MAX_SWEEP][MAX_RAY][4],
								 Radar *radar, VosSize *vs)
/* For each ray in the rsl structure, move ray header info into the 
	 arrays rayInfoInteger[][][] and rayInfoFloat[][][]. 
*/
{
	int tk_sindex, rindex, tk_rindex;
	double second;
	Ray_header *ray_head;
	Sweep *sweep;
	static int32 julday;
	static int day=-1;
	
	/* For each physical sweep...*/
	for (tk_sindex=0; tk_sindex<vs->tk.nsweep; tk_sindex++)
	{
		sweep = vs->rsl.sweep[tk_sindex];
		tk_rindex = -1;
		for (rindex=0; rindex<sweep->h.nrays; rindex++)
		{
			if (sweep->ray[rindex] == NULL) continue;
			tk_rindex++;
				/***********  Fill in ray info fields.  ***************/
			ray_head = &sweep->ray[rindex]->h;
			/* No. of sweep which contains this ray. */
			rayInfoInteger[tk_sindex][tk_rindex][0] = (int32)(tk_sindex + 1);
			/* Compute Julian Day. Usually do only once per vos. */
			if (day != ray_head->day)  
			{
				day = ray_head->day;  /* Note day & julday are static. */
				julday = (int32) 
					         julian(ray_head->year, ray_head->month, ray_head->day);
			}
			rayInfoInteger[tk_sindex][tk_rindex][1] = (int32) julday;
			rayInfoInteger[tk_sindex][tk_rindex][2] = (int32) ray_head->hour;
			rayInfoInteger[tk_sindex][tk_rindex][3] = (int32) ray_head->minute;
			rayInfoInteger[tk_sindex][tk_rindex][5] = (int32) (1000.0 *
				                                modf(ray_head->sec, &second));
			rayInfoInteger[tk_sindex][tk_rindex][4] = (int32) second;
			/* Ray status. 0:Normal , 1:Tansition , 2:Bad , 3:Questionable */
/* 			rayInfoInteger[tk_sindex][tk_rindex][6] = (int32) 0; */

			rayInfoFloat[tk_sindex][tk_rindex][0] = (float32) ray_head->azimuth;
			rayInfoFloat[tk_sindex][tk_rindex][1] = (float32) ray_head->elev;
			/* Store num_of_samples here instead of power. */
			rayInfoFloat[tk_sindex][tk_rindex][2] = (float32) ray_head->pulse_count;
			/* Store prf here instead of Sweep Rate */
			rayInfoFloat[tk_sindex][tk_rindex][3] = (float32) ray_head->prf;
		} /* end (rindex=0;... */
	} /* end for (tk_sindex=0;...*/
}

/*************************************************************/
/*                                                           */
/*                      sweepInfoFill                        */
/*                                                           */
/*************************************************************/
void sweepInfoFill(SWEEP_INFO sweepInfo[MAX_SWEEP], Radar *radar, VosSize *vs)
{
	int rindex, tk_sindex;
	Sweep *sweep;
	
	/* For each physical sweep...*/
	for (tk_sindex=0; tk_sindex<vs->tk.nsweep; tk_sindex++)
	{
		sweep = vs->rsl.sweep[tk_sindex];
		/* Fill in sweep info from the 1st non-NULL rsl ray we find. */
		for (rindex=0; rindex<sweep->h.nrays; rindex++)
		{
			if (sweep->ray[rindex] == NULL) continue;
			strncpy(sweepInfo[tk_sindex].radarName, radar->h.radar_name, 8);
			/* 1st sweep number (for tsdis structures) is 1 */
			sweepInfo[tk_sindex].sweepNum = (int32) (tk_sindex + 1);
			/* No. of rays in sweep */
			sweepInfo[tk_sindex].numRays = (int32) vs->tk.nray[tk_sindex];
			sweepInfo[tk_sindex].trueStartAngle = (float32) 
			                                       sweep->ray[rindex]->h.azimuth;
			if (sweep->ray[sweep->h.nrays-1] != NULL)
			  sweepInfo[tk_sindex].trueStopAngle = (float32) 
		                             sweep->ray[sweep->h.nrays-1]->h.azimuth;
			else
			  sweepInfo[tk_sindex].trueStopAngle = (float32) 
				                              sweepInfo[tk_sindex].trueStartAngle;
			/* degrees. Only for PPI scans. */
			sweepInfo[tk_sindex].fixedAngle = (float32) sweep->h.elev;
			/* Filter Flag. 0: No filtering, 1: filtered (descr in comment block) */
/*			sweepInfo[tk_sindex].filterFlag = (int32) 0; */
			break;
		}
	} /* end for (tk_sindex=0;... */
}

/*************************************************************/
/*                                                           */
/*                      radarDescFill                        */
/*                                                           */
/*************************************************************/
void radarDescFill(RADAR_DESCRIPTOR *radarDesc, Radar *radar, int vindex,
									 VosSize *vs)
{
	strncpy(radarDesc->radarName, radar->h.name, 8);
/*radarDesc->radarConstant = (float32) 0.0;
	radarDesc->nomPeakPower = (float32) 0.0;
	radarDesc->nomNoisePower = (float32) 0.0;
	radarDesc->receiverGain = (float32) 0.0;
	radarDesc->antennaGain = (float32) 0.0;
	radarDesc->radarSystemGain = (float32) 0.0;
*/
	radarDesc->horBeamWidth = (float32) first_ray_in_volume[vindex]->h.beam_width;
	radarDesc->verBeamWidth = (float32) first_ray_in_volume[vindex]->h.beam_width;
	radarDesc->radarType = (int16) 0;            /* 0: Ground-based radar */
	radarDesc->scanMode = (int16) 1;             /* 1: PPI  ,  3: RHI  */
	/* radar sweep rate (deg/sec) */
	radarDesc->nomScanRate = (float32) 
	           (first_ray_in_volume[vindex]->h.sweep_rate * 6.0);
	/* Following holds only for PPI scans. ??? */
	radarDesc->nomStartAngle = (float32) 
	           first_ray_in_volume[vindex]->h.azimuth;
	radarDesc->nomStopAngle = (float32) radarDesc->nomStartAngle;
	radarDesc->numParmDesc = (int16) vs->tk.nparm;
	radarDesc->numDesc = (int16) vs->tk.nparm;
	/* Data compression. Always 0 . Data compression done by HDF.*/
	radarDesc->dataComp = (int16) 0;
	/* Data reduction algorithm. */
	radarDesc->dataReductAlg = (int16) 0;          /* 0: No reduction */
	radarDesc->dataReductParm1 = (float32) 4.0;    /* TBD */
	radarDesc->dataReductParm2 = (float32) 4.0;    /* TBD */
	radarDesc->radarLon = (float32) (radar->h.lond + radar->h.lonm/60.0 +
																	 radar->h.lons/3600.0);
	radarDesc->radarLat = (float32) (radar->h.latd + radar->h.latm/60.0 +
																	 radar->h.lats/3600.0);
	/* altitude above Mean Sea Level (km) */
	radarDesc->radarAlt = (float32)((float)(radar->h.height) / 1000.0); 
	/* Effective unambiguous velocity (m/s) Leave blank. See range comments
	   below. */
/*
	if (radar->v[VR_INDEX] != NULL)
	  radarDesc->velocity = (float32) 
	                         first_ray_in_volume[VR_INDEX]->h.nyq_vel;
	else
	  radarDesc->velocity = (float32)0.0;
*/
	/* Effective unambiguous range (km). Leave blank. For wsr88d,
		 unambig_range varies with sweep. See range_info_float block. */
/*
	radarDesc->range = (float32) 
	                    first_ray_in_volume[vindex]->h.unam_rng;
*/
	/* No. of transmitted freqs */
	radarDesc->numTransfreqency = (int16) 1;
/*	radarDesc->numInterPulsePeriods = (int16) 0; */
	/* Freq. GHz. */
	radarDesc->frequency1 = (float32) first_ray_in_volume[vindex]->h.frequency;
}

/*************************************************************/
/*                                                           */
/*                      sensorFill                           */
/*                                                           */
/*************************************************************/
void sensorFill(SENSORS *sensor, Radar *radar, VosSize *vs, int productID)
/* Fill the substructures of the sensor data structure using
	 data from the radar structure. */
{
	int pindex, tk_sindex, vindex;
	
	vindex = nextVolume(radar, -1);  /* Find 1st non-NULL rsl volume. */
	
	radarDescFill(&sensor->radarDesc, radar, vindex, vs);
	sweepInfoFill(sensor->sweepInfo, radar, vs);
	rayInfoFill(sensor->rayInfoInteger, sensor->rayInfoFloat, radar, vs);

	/* Move data from each of the radar structure volumes into a
		 corresponding L1GV parameter structure. */
	if (productID == TK_L1B_GV)
	{
	  for (pindex=0; pindex<vs->tk.nparm; pindex++)
		{
			sensor->parm[pindex] = (PARAMETER	*)parmBuild(radar, vs, vindex, pindex);
			/* Find the next non-NULL volume in radar structure. */
			vindex = nextVolume(radar, vindex);
		}  /* end for (pindex=0; ... */
	}
	else  /* 1C-51 */
	{
		/* This is a hatchet job to conform with newest toolkit. The toolkit
			 arbitrarily assumes that the mask volume precedes the corresponding
			 data volume.
		*/
	  for (pindex=0; pindex<vs->tk.nparm/2; pindex++)
		{
			if (vindex == CZ_INDEX)
			  sensor->parm[pindex] = (PARAMETER *)parmBuild(radar, vs, MZ_INDEX,
																										 pindex);
			else if (vindex == CD_INDEX)
			  sensor->parm[pindex*2] = (PARAMETER *)parmBuild(radar, vs, MD_INDEX,
																											 pindex);
			else
			  continue;
			sensor->parm[pindex*2+1] = (PARAMETER *)parmBuild(radar, vs, vindex,
																											 pindex);
			/* Find the next non-NULL volume in radar structure. */
			vindex = nextVolume(radar, vindex);
		}  /* end for (pindex=0... */
	}  /* end else 1C-51 */

	if (radar_verbose_flag)
	{
		for (pindex=0; pindex<vs->tk.nparm; pindex++)
		{
			fprintf(stderr, "Toolkit parameter type : %s '%s'\n",
							sensor->parm[pindex]->parmDesc.parmDesc,
							sensor->parm[pindex]->parmDesc.parmName);
		  for (tk_sindex=0; tk_sindex<vs->tk.nsweep; tk_sindex++)
			  if (vs->tk.ncell[tk_sindex][pindex] == 0)
				  fprintf(stderr, "  tk_sweep[%.2d]  elev=%4.1f  nrays=%3d  cells/ray=%d\n",
									tk_sindex, sensor->sweepInfo[tk_sindex].fixedAngle,
									(int)0,
									vs->tk.ncell[tk_sindex][pindex]);
				else
				  fprintf(stderr, "  tk_sweep[%.2d]  elev=%4.1f  nrays=%3d  cells/ray=%d\n",
									tk_sindex, sensor->sweepInfo[tk_sindex].fixedAngle,
									(int)sensor->sweepInfo[tk_sindex].numRays,
									vs->tk.ncell[tk_sindex][pindex]);
		} /* end for (pindex=0;... */
	} /* end if (radar_verbose_flag) */
}

/*************************************************************/
/*                                                           */
/*                  timeUTC                                  */
/*                                                           */
/*************************************************************/
struct tm *timeUTC(void)
{
/* Find the current time (UTC). 
	 If success: return pointer to filled time_t structure. 
	 If failure: return NULL.
*/
	time_t time_current;

	/* Get the current system clock time */
	time_current = (time_t) time(NULL);
	if (time_current != -1)      /* valid time? */
		return(gmtime(&time_current));  /* Convert to UTC and return. */
	else
	  return(NULL);         /* Couldn't get the current time. */
}


/*************************************************************/
/*                                                           */
/*                      volDesFill                           */
/*                                                           */
/*************************************************************/
void volDesFill(VOLUME_DESCRIPTORS *volDes, Radar_header *h, VosSize *vs)
{
	struct tm *time_utc;
	int32 max_ncell;
	int pindex;
	
	/* Version no. of DORADE specifications used. Currently 1 */
	volDes->verNum = (int16) 1;  
	/* No. of this volume scan in granule */
	volDes->volNum = (int16) (vs->vos_num + 1);
	/* Max size of DORADE data record in this VOS. 2bytes x max(ncell) */
	max_ncell = 0;
	for (pindex=0; pindex<vs->tk.nparm; pindex++)
	  if (vs->tk.ncell[0][pindex] > max_ncell)
	    max_ncell = vs->tk.ncell[0][pindex];
	volDes->sizeDataRec = (int32) (2 * max_ncell);  
	strncpy(volDes->projectName, "TRMM GV", 20);
	volDes->year = (int16) h->year; 	    /* Year of volume scan */
	volDes->month = (int16) h->month;    /* Month of volume scan */
	volDes->day = (int16) h->day;        /* Day ... */
	volDes->hour = (int16) h->hour;      /* Hour ... */
	volDes->minute = (int16) h->minute;  /* Minute ... */
	volDes->second = (int16) floor((double)h->sec);  /* Second ... */
	/* Flight no. for airborne radar, or IOP no. for ground radar */
	strncpy(volDes->flightNum, "***", 8);  
	/* Data product generation facility name */
	strncpy(volDes->facName, "TSDIS", 8); 
	/* Get the current time; ie, the time of creation of this hdf file. */
	time_utc = timeUTC();
	if (time_utc != NULL)   /* Valid time? */
	{
		volDes->genYear = (int16) (1900 + time_utc->tm_year);
		volDes->genMonth = (int16) (time_utc->tm_mon + 1);
		volDes->genDay = (int16) time_utc->tm_mday;
	}
	else  /* Couldn't get valid time. */
	{
		volDes->genYear = (int16) 0;
		volDes->genMonth = (int16) 0;
		volDes->genDay = (int16) 0;
	}		
	/* No. of sensor descriptors in this volume scan */
	volDes->numSensorDesc = (int16) 1;     /* 1 for ground-based radar */
}

/*************************************************************/
/*                                                           */
/*                      commentsFill                         */
/*                                                           */
/*************************************************************/
void commentsFill(char *comments, VosSize *vs, Radar *radar,
									float *qcParm, int productID)
{
/* Write the following into the comments field of the gvl1
	 structure:
	   1. VOS comment_field header line.
	   2. cell/ray/sweep count.
	   3. 1C-51 QC parameters, if we're writing a 1C-51 HDF file.
*/
	char buf[256];
	int pindex, tk_sindex;
		
	/* Write out the dimensions of the toolkit structure which contains
		 this VOS. */
	sprintf(buf, "nSweep=%d\n", vs->tk.nsweep);
	strcat(comments, buf);
	for (tk_sindex=0; tk_sindex<vs->tk.nsweep; tk_sindex++)
	{
		sprintf(buf, "sweep[%.2d]--\n nRay=%d\n", tk_sindex,
						vs->tk.nray[tk_sindex]);
		strcat(comments, buf);
	  for (pindex=0; pindex<vs->tk.nparm; pindex++)
		{
			sprintf(buf, "  nCell_parm[%d]=%d\n", pindex,
							vs->tk.ncell[tk_sindex][pindex]);
			strcat(comments, buf);
		}
		strcat(comments, "\n");
	} /* end for (pindex=0 ... */

	strcat(comments, "********\n");
	/* If 1C-51 file, write out the QC parameters. */
	if (productID == TK_L1C_GV)
	{
		sprintf(buf, "-hThresh1 %.2f  -hThresh2 %.2f  -hThresh3 %.2f  -zThresh0 %.2f  -zThresh1 %.2f  -zThresh2 %.2f  -zThresh3 %.2f  -hFreeze %.2f  -dbzNoise %.2f  -zCal %.2f\n\n",
						qcParm[HTHRESH1], qcParm[HTHRESH2], qcParm[HTHRESH3],
						qcParm[ZTHRESH0], qcParm[ZTHRESH1], qcParm[ZTHRESH2], qcParm[ZTHRESH3],
						qcParm[HFREEZE], qcParm[DBZNOISE], qcParm[ZCAL]);
		strcat(comments, buf);
	} /* end if (productID == TK_L1C_GV) */
}

/*************************************************************/
/*                                                           */
/*                        gvl1Build                          */
/*                                                           */
/*************************************************************/
L1B_1C_GV *gvl1Build(Radar *radar, float *qcParm, VosSize *vs,
										 int productID)
{
/* Build the components of the Toolkit 'L1B_1C_GV' structure using
	 the data from the RSL radar structure. 
*/
	L1B_1C_GV *gvl1;

	/* Allocate memory for a TSDIS level_1 structure. */
	gvl1 = (L1B_1C_GV *)TKnewGVL1();

	/* Fill the structure, using data from the radar structure. */
	commentsFill(gvl1->comments, vs, radar, qcParm, productID);
	volDesFill(&gvl1->volDes, &radar->h, vs);
	sensorFill(&gvl1->sensor, radar, vs, productID);
	return(gvl1);
}

#endif
