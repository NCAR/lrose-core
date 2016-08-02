/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*************************************************************************
 * C2S_WRITE_CLASS_FILE.c - Writes a class file based on data in global arrays 
 */

#define C2S_WRITE_CLASS_FILE

#include "cidd2skewt.h"

#define BAD_VAL 999.0
#define BVAL     99.0
#define GVAL     0.0
#define DEG_PER_RAD 57.29577951 /* degrees per radian */

char *NA = "NOT AVAILABLE";

/*************************************************************************
 * write_class_file
 */
void write_class_file(char* dir, char* fname)
{
    int i;
    double angle,speed;  /* winds conversions */

    char    buf[256];
    char    path[MAXPATHLEN];

    FILE *outfile;

    sprintf(path,"%s/%s",dir,fname);
    if((outfile = fopen(path,"w")) == NULL) {
	perror(path);
	return;
    }

    fprintf(outfile,"Data Type:                         %s\n","MODEL SOUNDING");
    fprintf(outfile,"Project ID:                        %s\n","MODEL SOUNDING");
    fprintf(outfile,"Launch Site Type/Site ID:          %s\n","MODEL  CCM    ");
    fprintf(outfile,"Launch Location (lon,lat,alt):     %g, %g, %g\n",gd.lon,gd.lat,0.0);

    strftime(buf,256,"%Y, %m, %d, %T [nominal]",gmtime(&gd.t_mid));
     
    fprintf(outfile,"GMT Launch Time (y,m,d,h,m,s):     %s\n",buf);
    fprintf(outfile,"Sonde Type/ID/Sensor ID/Tx Freq:   %s\n",NA);
    fprintf(outfile,"Met Processor/Met Smoothing:       %s\n",NA);
    fprintf(outfile,"Winds Type/Processor/Smoothing:    %s\n",NA);
    fprintf(outfile,"Pre-launch Surface Obs Source:     %s\n",NA);
    fprintf(outfile,"System Operator/Comments:          %s\n",NA);
    fprintf(outfile,"\\\n\\\n");
    fprintf(outfile," Time  Press  Temp  Dewpt  RH    Uwind  Vwind  Wspd  Dir   dZ       Lon       ");
    fprintf(outfile,"Lat     Rng   Ang    Alt    Qp   Qt   Qh   Qu   Qv   Quv\n");
    fprintf(outfile,"  sec    mb     C     C     %%     m/s    m/s   m/s   deg   m/s      deg       ");
    fprintf(outfile,"deg      km   deg     m     mb   C    %%    m/s  m/s  m/s\n");
    fprintf(outfile,"------ ------ ----- ----- ----- ------ ------ ----- ----- ----- ----------");
    fprintf(outfile," --------- ----- ----- ------- ---- ---- ---- ---- ---- ----\n");

    for(i=0; i< gd.num_levels; i++) {
        fprintf(outfile,"%6.1f %6.1f ",i * 10.0, gd.Press.val[i]); /* Time and Pressure */

        fprintf(outfile,"%5.1f %5.1f ",gd.Temper.val[i], gd.Dewpt.val[i]); /* Temperature and Dewpoint */
	 
        fprintf(outfile,"%5.1f %6.1f ",gd.Relhum.val[i], gd.U_wind.val[i]); /* RH and U winds */

        fprintf(outfile,"%6.1f ",gd.V_wind.val[i]); /* V Winds */

	speed = sqrt((gd.U_wind.val[i] * gd.U_wind.val[i]) + (gd.V_wind.val[i] * gd.V_wind.val[i]));
	angle = atan2( -gd.V_wind.val[i], -gd.U_wind.val[i]) * DEG_PER_RAD;
	angle = 90.0 - angle; /* convert to North = 0 */
	if (angle > 360.0) angle -= 360;
	if (angle < 0.0) angle += 360.0;

        fprintf(outfile,"%5.1f %5.1f ",speed,angle); /* Wind Speed and Direction */

        fprintf(outfile,"%5.1f ",BVAL); /* dZ */

        fprintf(outfile,"%10.5f %9.5f ",gd.lon, gd.lat); /* Position longitude, latitude */

        fprintf(outfile,"%5.1f %5.1f ",BVAL, BVAL); /* Range and Angle */

        fprintf(outfile,"%7.1f ",gd.alt_msl[i]); /* Altitude */

        fprintf(outfile,"%4.1f %4.1f %4.1f %4.1f %4.1f %4.1f\n",GVAL,GVAL,GVAL,GVAL,GVAL,GVAL);
    }

    fclose(outfile);
}

