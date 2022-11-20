/********************************************************************************/
/*MIT License                                                                   */
/*                                                                              */
/*Copyright (c) 2021 Iwan Holleman, Asko Huuskonen, and Hidde Leijnse           */
/*                                                                              */
/*Permission is hereby granted, free of charge, to any person obtaining a copy  */
/*of this software and associated documentation files (the "Software"), to deal */
/*in the Software without restriction, including without limitation the rights  */
/*to use, copy, modify, merge, publish, distribute, sublicense, and/or sell     */
/*copies of the Software, and to permit persons to whom the Software is         */
/*furnished to do so, subject to the following conditions:                      */
/*                                                                              */
/*The above copyright notice and this permission notice shall be included in all*/
/*copies or substantial portions of the Software.                               */
/*                                                                              */
/*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    */
/*IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      */
/*FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE   */
/*AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        */
/*LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, */
/*OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE */
/*SOFTWARE.                                                                     */
/********************************************************************************/


/******************************************************************************/
/*This program scans one or two byte volume (uncorrected) reflectivity data in*/
/*KNMI-HDF5 format for solar interferences using NCSA high-level lib. The sun */
/*hit power is calculated using a two-stage method as agreed with Asko        */
/*Huuskonen (FMI, personal communications in April 2015). First an estimate is*/
/*made using only data above a certain height (Height 1), well above possible */
/*rain. Then this estimate is used to remove outlying rangebins from the ray, */
/*and the mean sunpower and standard deviation are calculated for all bins    */
/*above a certain height (Height 2).                                          */
/*The retreived power is converted to sunflux in dB at the antenna feed of the*/
/*radar, this radar-independent quantity was agreed earlier between Holleman  */
/*and Huuskonen.                                                              */
/*In case, differential reflectivity data are available they are used only for*/
/*the rangebins that are used for the second-stage, filtered sun power.       */
/*Input parameters for this program are:                                      */
/*'MinHeig1', minimum height in km for first-stage of sunpower calculation    */
/*'RayMin', minimum analysed length of radar rays in km                       */
/*'MinHeig2', minimum height in km for second-stage of sunpower calculation   */
/*'FracMin', minimum fraction of valid rangebins in the rays with a sunhit    */
/*'dBdifX', maximum deviation of rangebin power from first estimate in dB     */
/*The sunhits are printed to stdout in standardized format.                   */
/******************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <time.h>
#include "read_ODIM_vol_h5_functions.h"

/******************************************************************************/
/*Definition of analysis parameters.                                          */
/******************************************************************************/
#define HEIGMIN1   (8.0)          /*Minimum height for first analyses in km.*/
#define HEIGMIN2   (4.0)          /*Minimum height for second analyses in km.*/
#define RAYMIN     (20.0)         /*Minimum length of rays for analyses in km.*/
#define FRACDATA   (0.70)         /*Fraction of rangebins passing the analyses.*/
#define DBDIFX     (2.0)          /*Maximum dev. of power from 1st fit in dB.*/
#define ANGLEDIF   (5.0)          /*Maximum dev. from calculated sun in deg.*/
#define GASATTN    (0.016)        /*Two-way gaseous attenuation in dB/km.*/
#define CWIDTH     (1.2)          /*Factor between bandwidth and 1/pulsewidth.*/

/******************************************************************************/
/*Definition of radar parameters for solar analysis:                          */
/******************************************************************************/
#define ONEPOL         (3.01)    /*Losses due to one-pol RX only in dB.*/
#define AVGATTN        (1.39)    /*Averaging and overlap losses in dB for 1deg.*/
#define DEF_WAVELENGTH (5.333)   /*Default radar wavelength in cm.*/
#define DEF_ANTGAIN    (45.0)    /*Default antenna gain in dB.*/
#define DEF_RXLOSS     (0.0)     /*Default receiver losses in dB.*/
#define DEF_GASATTN    (0.016)   /*Default two-way gaseous attenuation in dB/km.*/
#define DEF_ASTART     (0.0)     /*Default azimuth offset in degrees.*/
#define DEF_RSTART     (0.0)     /*Default range offset in km.*/
#define DEF_RADCONST   (65.0)    /*Default radar constant in dB.*/
#define DEF_PULSEWIDTH (0.8)     /*Default pulse width in us.*/
#define DEF_RPM        (3.0)     /*Default antenna rotation speed in revolutions per minute.*/
#define DEF_A1GATE     (0)       /*Default first azimuth radiated in scan.*/

/******************************************************************************/
/*Definition of standard parameters.                                          */
/******************************************************************************/

#define DEG2RAD    (0.017453293)  /*Degrees to radians.*/
#define RAD2DEG    (57.29578)     /*Radians to degrees.*/
#define STRLEN     (128)          /*Length of all strings used.*/
#define NELEVX     (64)           /*Maximum number of elevations.*/
#define RADIUS43   (8495.0)       /*Earth radius used for height calculations.*/
#define EPS        (1E-8)         /*Epsilon to account for floating-pojnt rounding errors.*/



/******************************************************************************/
/*Prototypes of local functions:                                              */
/******************************************************************************/
int check_metadata(SCANMETA *meta_init, int verbose, SCANMETA *meta);
float ElevHeig2Rang(double elev, double heig);
void solar_elev_azim(double lon, double lat, char *yyyymmdd, char *hhmmss, double *elev, double *azim);

/******************************************************************************/
/*The main program:                                                           */
/******************************************************************************/
int main(int argc, char **argv)
{
	char *h5file, type[STRLEN], type2[STRLEN], datestr[STRLEN], timestr[STRLEN];
	int id, ia, ir, irn1, irn2, n, ndr, Zv, Zdr, show_Zdr;
	double HeigMin1, HeigMin2, FracData, dBdifX, AngleDif;
	double RayMin, AntAreaH, AntAreaV, Azimuth, Range, SunFirst;
	double SunMean, SunStdd, dBSunFlux, ZdrMean, ZdrStdd, Signal, ElevSun, AzimSun, ascale;
	hid_t h5_in;
	struct tm time;
	SCANMETA meta, meta_init;
	SCANDATA data, Ddata;
	
	/*Extraction of command line parameters.*/
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <h5-volume> [-uZ/-Z] [-hmin1<x>/-hmin2<x>/-rayn<x>/-frac<x>/-difx]!\n", argv[0]);
		exit(1);
	}
	h5file = argv[1];
	sprintf(type, "TH");
	HeigMin1 = HEIGMIN1;
	HeigMin2 = HEIGMIN2;
	FracData = FRACDATA;
	dBdifX = DBDIFX;
	AngleDif = ANGLEDIF;
	RayMin = RAYMIN;
	for (n = 2; n < argc; n++)
	{
		if (strcmp(argv[n], "-uZ") == 0) sprintf(type, "TH");
		if (strcmp(argv[n], "-Z") == 0) sprintf(type, "DBZH");
		sscanf(argv[n], "-hmin1%lf", &HeigMin1);
		sscanf(argv[n], "-hmin2%lf", &HeigMin2);
		sscanf(argv[n], "-rayn%lf", &RayMin);
		sscanf(argv[n], "-frac%lf", &FracData);
		sscanf(argv[n], "-difx%lf", &dBdifX);
	}
	
	/*Opening of HDF5 radar input file.*/
	fprintf(stdout, "#Opening of HDF5 radar input file      : %s\n", h5file);
	h5_in = H5Fopen(h5file, H5F_ACC_RDONLY, H5P_DEFAULT);
	if (h5_in < 0)
	{
		fprintf(stderr, "HDF5 radar input file could not be opened!\n");
		exit(2);
	}
	
	/*Reading metadata from file.*/
	if (!read_scan_metadata(h5_in, 1, &meta, &meta_init))
	{
		fprintf(stderr, "Error: HDF5 input file does not contain scans.\n");
		H5Fclose(h5_in);
		exit(3);
	}
	
	/*Check if all necessary metadata are there.*/
	if (!check_metadata(&meta_init, 1, &meta))
	{
		H5Fclose(h5_in);
		exit(4);
	}
	
	/*Setting effective antenna area.*/
	AntAreaH = (meta.wavelength / 100.0) * (meta.wavelength / 100.0) * pow(10.0, meta.antgainH / 10.0) / (4.0 * M_PI);
	AntAreaV = (meta.wavelength / 100.0) * (meta.wavelength / 100.0) * pow(10.0, meta.antgainV / 10.0) / (4.0 * M_PI);
	
	/*Printing input parameters and settings.*/
	fprintf(stdout, "#Minimum height in 1st analysis (km)               : %5.2f\n", HeigMin1);
	fprintf(stdout, "#Minimum height in 2nd analysis (km)               : %5.2f\n", HeigMin2);
	fprintf(stdout, "#Minimum ray length in analysis (km)               : %5.2f\n", RayMin);
	fprintf(stdout, "#Maximum deviation of sun power (dB)               : %5.3f\n", dBdifX);
	fprintf(stdout, "#Minium fraction of valid rangebins                : %5.3f\n", FracData);
	fprintf(stdout, "#Two-way gaseous attenuation [dB/km]               : %5.3f\n", meta.gasattn);
	fprintf(stdout, "#Max. angular deviation of rays [deg]              : %5.3f\n", AngleDif);
	fprintf(stdout, "#Effective area of radar antenna (H channel) [m2]  : %5.3f\n", AntAreaH);
	fprintf(stdout, "#Effective area of radar antenna (V channel) [m2]  : %5.3f\n", AntAreaV);
	fprintf(stdout, "#Losses between reference and feed (H channel) [dB]: %5.3f\n", meta.RXlossH);
	fprintf(stdout, "#Losses between reference and feed (V channel) [dB]: %5.3f\n", meta.RXlossV);
	
	/*Reading of PPI data of specified type and looking for solar interferences.*/
	id = 0;
	while (read_scan_metadata(h5_in, id + 1, &meta, &meta_init))
	{
		id += 1;
		if (!check_metadata(&meta_init, 0, &meta)) continue;
		if ((meta.nbins * meta.rscale / 1000.0) < RayMin) continue;
		if (!read_scan_data(h5_in, id, type, &meta, &data)) continue;
		
		/*Check if the vertical version of the desired variable is available, otherwise try ZDR.*/
		Zv = 0;
		Zdr = 0;
		strcpy(type2, type);
		type2[strlen(type2) - 1] = 'V';
		if (read_scan_data(h5_in, id, type2, &meta, &Ddata)) Zv = 1;
		else if (read_scan_data(h5_in, id, "ZDR", &meta, &Ddata)) Zdr = 1;
		
		/*Set ray index.*/
	    irn1 = (int) (ElevHeig2Rang(meta.elangle, HeigMin1) / (meta.rscale / 1000.0));
	    if (((meta.nbins - irn1) * meta.rscale / 1000.0) < RayMin) irn1 = meta.nbins - RayMin / (meta.rscale / 1000.0);
		if (irn1 < 0) irn1 = 0;
	    irn2 = (int) (ElevHeig2Rang(meta.elangle, HeigMin2) / (meta.rscale / 1000.0));
	    if (((meta.nbins - irn2) * meta.rscale / 1000.0) < RayMin) irn2 = meta.nbins - RayMin / (meta.rscale / 1000.0);
		if (irn2 < 0) irn2 = 0;
		
		/*Loop over all rays.*/
		ascale = 360.0 / meta.nrays;
		for (ia = 0; ia < meta.nrays; ia++)
		{
			/*Set azimuth variable.*/
			Azimuth = (ia + 0.5) * ascale + meta.astart;
			
			/*First analyses to estimate sunpower at higher altitudes (less rain contamination).*/
			n = 0;
			SunFirst = 0.0;
			for (ir = irn1; ir < meta.nbins; ir++)
			{
				/*Check if data are nodata.*/
				if ((data.data[ir + ia * meta.nbins] == data.nodata) || (data.data[ir + ia * meta.nbins] == data.undetect)) continue;
				
				/*Compute apparent sun signal.*/
				Range = (ir + 0.5) * meta.rscale / 1000.0 + meta.rstart;
				Signal = data.offset + data.gain * data.data[ir + ia * meta.nbins];
				Signal -= meta.radconstH + 20.0 * log10(Range) + meta.gasattn * Range;
				SunFirst += Signal;
				n++;
			}
			
			/*Check if the number of data points are sufficient.*/
			if (!n || (n < (FracData * (meta.nbins - irn1)))) continue;
			SunFirst /= n;
			
			/*Second analysis with removal of outliers, if available also Zdr analysis.*/
			n = 0;
			ndr = 0;
			SunMean = 0.0;
			SunStdd = 0.0;
			ZdrMean = 0.0;
			ZdrStdd = 0.0;
			if (Zv || Zdr) show_Zdr = 1;
			else show_Zdr = 0;
			for (ir = irn2; ir < meta.nbins; ir++)
			{
				/*Check if data are nodata.*/
				if ((data.data[ir + ia * meta.nbins] == data.nodata) || (data.data[ir + ia * meta.nbins] == data.undetect)) continue;
				
				/*Compute apparent sun signal.*/
				Range = (ir + 0.5) * meta.rscale / 1000.0 + meta.rstart;
				Signal = data.offset + data.gain * data.data[ir + ia * meta.nbins];
				Signal -= meta.radconstH + 20.0 * log10(Range) + meta.gasattn * Range;
				
				/*Check if the signal deviates significantly from that computed in the first pass.*/
				if (fabs(Signal - SunFirst) > dBdifX) continue;
				SunMean += Signal;
				SunStdd += Signal * Signal;
				n++;
				
				if (Zv)
				{
					/*Check if data are nodata.*/
					if ((Ddata.data[ir + ia * meta.nbins] != Ddata.nodata) && (Ddata.data[ir + ia * meta.nbins] != Ddata.undetect))
					{
						/*Compute apparent sun signal for ZV.*/
						Signal = Ddata.offset + Ddata.gain * Ddata.data[ir + ia * meta.nbins];
						Signal -= meta.radconstV + 20.0 * log10(Range) + meta.gasattn * Range;
						ZdrMean += Signal;
						ZdrStdd += Signal * Signal;
						ndr++;
					}
				}
				else if (Zdr)
				{
					/*Check if data are nodata.*/
					if ((Ddata.data[ir + ia * meta.nbins] != Ddata.nodata) && (Ddata.data[ir + ia * meta.nbins] != Ddata.undetect))
					{
						/*Compute apparent sun signal for ZDR.*/
						Signal = Ddata.offset + Ddata.gain * Ddata.data[ir + ia * meta.nbins];
						ZdrMean += Signal;
						ZdrStdd += Signal * Signal;
						ndr++;
					}
				}
			}
			
			/*Check if the number of data points are sufficient.*/
			if (!n || (n < (FracData * (meta.nbins - irn2)))) continue;
			
			/*Compute mean and standard deviation of sun signal.*/
			SunMean /= n;
			SunStdd = sqrt(SunStdd / n - SunMean * SunMean + EPS);
			SunMean -= 10.0 * log10(CWIDTH / meta.pulsewidth);
			if (Zv || Zdr)
			{
				/*Check if the number of data points for ZV or ZDR are sufficient.*/
				if (!ndr || (ndr < (FracData * (meta.nbins - irn2)))) show_Zdr = 0;
				else
				{
					/*Compute mean and standard deviation of sun signal for ZV or ZDR.*/
					ZdrMean /= ndr;
					ZdrStdd = sqrt(ZdrStdd / ndr - ZdrMean * ZdrMean + EPS);
				}
			}
			if (Zv) ZdrMean -= 10.0 * log10(CWIDTH / meta.pulsewidth);
			
			/*Compute exact time of sun hit.*/
			ODIMstring2time(meta.startdate, meta.starttime, &time);
			time.tm_sec += ((ia - meta.a1gate + meta.nrays) % meta.nrays) * ascale / (6.0 * meta.rpm);
			timegm(&time);
			time2ODIMstring(&time, datestr, timestr);
			
			/*Compute exact location of the sun at the location of the radar at the time of the sun hit.*/
			solar_elev_azim(meta.lon, meta.lat, datestr, timestr, &ElevSun, &AzimSun);
			
			/*Check if the sun hit is too far from the expected sun position.*/
			if (fabs(meta.elangle - ElevSun) > AngleDif || fabs(Azimuth - AzimSun) > AngleDif) continue;
			
			/*Conversion to SunFlux and printing of results.*/
			dBSunFlux = 130 + SunMean + meta.RXlossH - 10.0 * log10(AntAreaH) + AVGATTN + ONEPOL;
			fprintf(stdout, "#Date    Time        Elevatn Azimuth   ElevSun   AzimSun     N  dBSunFlux   SunMean SunStdd   ZdrMean  ZdrStdd  Refl   ZDR\n");
			fprintf(stdout, "%s %s.000 %8.3f %7.2f %9.4f %9.4f %5d %10.2f %9.2f %7.3f", datestr, timestr, meta.elangle, Azimuth, ElevSun, AzimSun, n, dBSunFlux, SunMean, SunStdd);
			
			/*Plot part of output for ZV or ZDR.*/
			if (Zv && show_Zdr)
			{
				/*Compute sun flux for vertical channel and plot results.*/
				dBSunFlux = 130 + ZdrMean + meta.RXlossV - 10.0 * log10(AntAreaV) + AVGATTN + ONEPOL;
				fprintf(stdout, " %9.3f %8.4f %5s %5s\n", dBSunFlux, ZdrStdd, type, type2);
			}
			else if (Zdr && show_Zdr) fprintf(stdout, " %9.3f %8.4f %5s   ZDR\n", ZdrMean, ZdrStdd, type);
			else fprintf(stdout, "        NA       NA %5s    NA\n", type);
		}
		
		/*Cleaning memory.*/
		free(data.data);
		if (Zv || Zdr) free(Ddata.data);
	}
	
	/*Close HDF5 file.*/
	H5Fclose(h5_in);
	
	/*End of program.*/
	return 0;
}



/******************************************************************************/
/*LOCAL FUNCTIONS:                                                            */
/******************************************************************************/


/**********************************************************************/
/* function check_metadata                                            */
/* IN: SCANMETA *meta_init: pointer to structure containing metadata  */
/*     initialization information.                                    */
/* IN: int verbose: boolean whether to warn when a default value is   */
/*     used.                                                          */
/* OUT: SCANMETA *meta: pointer to structure containing metadata.     */
/* RETURNS: 1 if all necessary metadata is present, 0 otherwise.      */
/*                                                                    */
/* Checks if all necessary metadata are available in the metadata     */
/* structure. If possible, it inserts default values when missing.    */
/**********************************************************************/
int check_metadata(SCANMETA *meta_init, int verbose, SCANMETA *meta)
{
	/*Check for metadata that have no default values.*/
	if ((*meta_init).nbins == 0)
	{
		if (verbose) fprintf(stderr, "Error: missing metadata nbins!\n");
		return 0;
	}
	if ((*meta_init).nrays == 0)
	{
		if (verbose) fprintf(stderr, "Error: missing metadata nrays!\n");
		return 0;
	}
	if ((*meta_init).startdate[0] == 0)
	{
		if (verbose) fprintf(stderr, "Error: missing metadata startdate!\n");
		return 0;
	}
	if ((*meta_init).starttime[0] == 0)
	{
		if (verbose) fprintf(stderr, "Error: missing metadata starttime!\n");
		return 0;
	}
	if ((*meta_init).rscale < 0.5)
	{
		if (verbose) fprintf(stderr, "Error: missing metadata rscale!\n");
		return 0;
	}
	if ((*meta_init).elangle < 0.5)
	{
		if (verbose) fprintf(stderr, "Error: missing metadata elangle!\n");
		return 0;
	}
	if ((*meta_init).lat < 0.5)
	{
		if (verbose) fprintf(stderr, "Error: missing metadata lat!\n");
		return 0;
	}
	if ((*meta_init).lon < 0.5)
	{
		if (verbose) fprintf(stderr, "Error: missing metadata lon!\n");
		return 0;
	}
	
	/*Check for metadata that have default values.*/
	if ((*meta_init).wavelength < 0.5)
	{
		(*meta).wavelength = DEF_WAVELENGTH;
		if (verbose) fprintf(stderr, "Warning: missing metadata wavelength. Using default value (%f).\n", (*meta).wavelength);
	}
	if ((*meta_init).antgainH < 0.5)
	{
		(*meta).antgainH = DEF_ANTGAIN;
		if (verbose) fprintf(stderr, "Warning: missing metadata antgainH. Using default value (%f).\n", (*meta).antgainH);
	}
	if ((*meta_init).antgainV < 0.5)
	{
		(*meta).antgainV = (*meta).antgainH;
		if (verbose) fprintf(stderr, "Warning: missing metadata antgainV. Using default value (%f).\n", (*meta).antgainV);
	}
	if ((*meta_init).RXlossH < 0.5)
	{
		(*meta).RXlossH = DEF_RXLOSS;
		if (verbose) fprintf(stderr, "Warning: missing metadata RXlossH. Using default value (%f).\n", (*meta).RXlossH);
	}
	if ((*meta_init).RXlossV < 0.5)
	{
		(*meta).RXlossV = (*meta).RXlossH;
		if (verbose) fprintf(stderr, "Warning: missing metadata RXlossV. Using default value (%f).\n", (*meta).RXlossV);
	}
	if ((*meta_init).gasattn < 0.5)
	{
		(*meta).gasattn = DEF_GASATTN;
		if (verbose) fprintf(stderr, "Warning: missing metadata gasattn. Using default value (%f).\n", (*meta).gasattn);
	}
	if ((*meta_init).astart < 0.5)
	{
		(*meta).astart = DEF_ASTART;
		if (verbose) fprintf(stderr, "Warning: missing metadata astart. Using default value (%f).\n", (*meta).astart);
	}
	if ((*meta_init).rstart < 0.5)
	{
		(*meta).rstart = DEF_RSTART;
		if (verbose) fprintf(stderr, "Warning: missing metadata rstart. Using default value (%f).\n", (*meta).rstart);
	}
	if ((*meta_init).radconstH < 0.5)
	{
		(*meta).radconstH = DEF_RADCONST;
		if (verbose) fprintf(stderr, "Warning: missing metadata radconstH. Using default value (%f).\n", (*meta).radconstH);
	}
	if ((*meta_init).radconstV < 0.5)
	{
		(*meta).radconstV = (*meta).radconstH;
		if (verbose) fprintf(stderr, "Warning: missing metadata radconstV. Using default value (%f).\n", (*meta).radconstV);
	}
	if ((*meta_init).pulsewidth < 0.5)
	{
		(*meta).pulsewidth = DEF_PULSEWIDTH;
		if (verbose) fprintf(stderr, "Warning: missing metadata pulsewidth. Using default value (%f).\n", (*meta).pulsewidth);
	}
	if ((*meta).pulsewidth < 1.0E-6)
	{
		if (verbose) fprintf(stderr, "Warning: incorrect metadata pulsewidth (%f). Using default value (%f).\n", (*meta).pulsewidth, DEF_PULSEWIDTH);
		(*meta).pulsewidth = DEF_PULSEWIDTH;
	}
	if ((*meta_init).rpm < 0.5)
	{
		(*meta).rpm = DEF_RPM;
		if (verbose) fprintf(stderr, "Warning: missing metadata rpm. Using default value (%f).\n", (*meta).rpm);
	}
	if ((*meta_init).a1gate == 0)
	{
		(*meta).a1gate = DEF_A1GATE;
		if (verbose) fprintf(stderr, "Warning: missing metadata a1gate. Using default value (%ld).\n", (*meta).a1gate);
	}
	
	/*Return 1 on success.*/
	return 1;
}

/******************************************************************************/
/*This function calculates the height and range from the Radar corresponding  */
/*to a point with a known elevation and on-ground distance from the Radar.    */
/*The formulae used are exact.                                                */
/******************************************************************************/
float ElevHeig2Rang(double elev, double heig)
{
	double rang;
	
	rang = RADIUS43 * sin(DEG2RAD * elev);
	rang = sqrt(rang * rang + 2.0 * RADIUS43 * heig + heig * heig) - RADIUS43 * sin(DEG2RAD * elev);
	return rang;
}

/******************************************************************************/
/*This function calculates the solar elevation and azimuth using the          */
/*geographical position, date, and time. The equations and constants are taken*/
/*from the WMO guide on Meteorological Instruments and Methods of Observations*/
/*(CIMO, WMO no. 8), annex 7.D. The equations have been slightly modified and */
/*extended to include the calculation of both the sine and cosine of the      */
/*azimuth.                                                                    */
/******************************************************************************/
void solar_elev_azim(double lon, double lat, char *yyyymmdd, char *hhmmss, double *elev, double *azim)
{
	double MeanLon, MeanAnom, EclipLon, Obliquity, RightAsc, Declinat;
	double GMST, angleH;
	double hour, dt;
	struct tm time;
	time_t time1, time2;
	
	/*Conversion of lon,lat.*/
	lon *= DEG2RAD;
	lat *= DEG2RAD;
	
	/*Set time1 to current time.*/
	sscanf(yyyymmdd, "%04d%02d%02d", &(time.tm_year), &(time.tm_mon), &(time.tm_mday));
	sscanf(hhmmss, "%02d%02d%02d", &(time.tm_hour), &(time.tm_min), &(time.tm_sec));
	time.tm_year -= 1900;
	time.tm_mon -= 1;
	time1 = timegm(&time);
	hour = ((double) time.tm_hour) + ((double) time.tm_min) / 60.0 + ((double) time.tm_sec) / 3600.0;
	
	/*Set time2 to noon at Jan 1, 2000.*/
	time.tm_year = 100;
	time.tm_mon = 0;
	time.tm_mday = 1;
	time.tm_hour = 12;
	time.tm_min = 0;
	time.tm_sec = 0;
	time2 = timegm(&time);
	
	/*Compute time difference in days.*/
	dt = difftime(time1, time2) / 86400.0;
	
	/*Calculation of eclips coordinates.*/
	MeanLon = 280.460 + 0.9856474 * dt;
	MeanAnom = 357.528 + 0.9856003 * dt;
	EclipLon = MeanLon + 1.915 * sin(MeanAnom * DEG2RAD) + 0.020 * sin(2.0 * MeanAnom * DEG2RAD);
	EclipLon *= DEG2RAD;
	Obliquity = 23.439 - 0.0000004 * dt;
	Obliquity *= DEG2RAD;
	
	/*Calculation of the celestial coordinates of the sun.*/
	RightAsc = atan2(cos(Obliquity) * sin(EclipLon), cos(EclipLon));
	Declinat = asin(sin(Obliquity) * sin(EclipLon));
	
	/*Calculation of current, local hour angle.*/
	GMST = 6.697375 + 0.0657098242 * dt + hour;
	angleH = GMST * 15 * DEG2RAD + lon - RightAsc;
	
	/*Calculation of elevation and azimuth.*/
	*elev = asin(sin(Declinat) * sin(lat) + cos(Declinat) * cos(lat) * cos(angleH));
	*azim = atan2(-sin(angleH), tan(Declinat) * cos(lat) - cos(angleH) * sin(lat));
	
	/*Scaling and shifting of values.*/
	(*elev) *= RAD2DEG;
	(*azim) *= RAD2DEG;
	if ((*azim) < 0) (*azim) += 360;
}
