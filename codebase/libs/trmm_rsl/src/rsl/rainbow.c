/*
    NASA/TRMM, Code 912
    This is the TRMM Office Radar Software Library.
    Copyright (C) 2004
            Bart Kelley
	    George Mason University
	    Fairfax, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <trmm_rsl/rsl.h> 
#include "rainbow.h"

static int get_param_int(char *buf)
{
    /* Returns an integer parameter from a header line. */

    int value;
    char *substr;

    substr = index(buf, ':');
    sscanf(substr, ": %d", &value);
    return value;
}

static float get_param_float(char *buf)
{
    /* Returns a floating point parameter from a header line. */

    float value;
    char *substr;

    substr = index(buf, ':');
    sscanf(substr, ": %f", &value);
    return value;
}


static char *get_param_string(char *buf)
{
    /* Returns a string parameter from a header line. */

    static char string[20];
    char *substr;

    substr = index(buf, ':');
    sscanf(substr, ": %s", string);
    return string;
}

void A_label(Rainbow_hdr *rainbow_header, char* buf)
{
    int labelnum;
    char label;

    sscanf(buf, "%c%d", &label, &labelnum);

    if (labelnum == 3) 
	rainbow_header->az_step = get_param_float(buf);
     else if (labelnum == 9)
	rainbow_header->nsweeps = get_param_int(buf);
}

void F_label(Rainbow_hdr *rainbow_header, char* buf)
{
    int labelnum, day, month, year, sec, minute, hour;
    float lat, lon;

    sscanf(buf, "%*c%d", &labelnum);

    switch (labelnum) {
	case 3:
	    rainbow_header->compressed = get_param_int(buf);
	    break;
	case 4:
	    sscanf(buf, "%*c%*d : %f %f", &lon, &lat);
	    rainbow_header->lon = lon;
	    rainbow_header->lat = lat;
	    break;
	case 5:
	    sscanf(buf, "%*c%*d : %d %d %d", &day, &month, &year);
	    rainbow_header->month = month;
	    rainbow_header->day = day;
	    rainbow_header->year = year;
	    break;
	case 6:
	    sscanf(buf, "%*c%*d : %d %d %d", &sec, &minute, &hour);
	    rainbow_header->hour = hour;
	    rainbow_header->minute = minute;
	    rainbow_header->sec = sec;
	    break;
	case 9:
	    rainbow_header->datatype = get_param_int(buf);
	    break;
    }
}

void H_label(Rainbow_hdr *rainbow_header, char* buf)
{
    int labelnum;
    char label;

    sscanf(buf, "%c%d", &label, &labelnum);

    if (labelnum == 3)
      rainbow_header->filetype = get_param_int(buf);
    else if (labelnum == 8)
      strcpy(rainbow_header->radarname, get_param_string(buf));
}

void P_label(Rainbow_hdr *rainbow_header, char* buf)
{
    int labelnum;
    char label;

    sscanf(buf, "%c%d", &label, &labelnum);

    if (labelnum == 3)
	rainbow_header->range_start= get_param_float(buf);
    else if (labelnum == 4)
	rainbow_header->range_stop = get_param_float(buf);
    else if (labelnum == 5)
	rainbow_header->range_step = get_param_float(buf);
}

void R_label(Rainbow_hdr *rainbow_header, char* buf)
{
    int labelnum;

    sscanf(buf, "%*c%d", &labelnum);

    if (labelnum == 1)
      rainbow_header->nbins = get_param_int(buf);
    else if (labelnum == 2)
      rainbow_header->bin_resolution = get_param_float(buf);
    else if (labelnum == 8)
      rainbow_header->nvalues = get_param_int(buf);
}

void W_label(Rainbow_hdr *rainbow_hdr, char* buf)
{
    int labelnum, az_start, az_stop, pw_code, prf_high, prf_low, zdata;
    int vdata, wdata, unfolding, cdata, ddata, uzdata;
    float elev, az_step, az_rate, range_stop;


    sscanf(buf, "%*c%d : %f %d %d %f %f %d %d %d %d %d %d %d %f %d %d %d",
	    &labelnum, &elev, &az_start, &az_stop, &az_step,
	    &az_rate, &pw_code, &prf_high, &prf_low, &zdata, &vdata,
	    &wdata, &unfolding, &range_stop, &cdata, &ddata, &uzdata);

    /* Note: Only need to collect parameters 1, 5, 7, 8, and 13 for each
     * elevation.  Parameters 2, 3, and 4 are fixed at 0, 359, and 1 for volume
     * scan.  The remaining parameters can be taken once from label number 1
     * (first elevation).  Also, don't need az_step; got it from A3. 
     */

    if (labelnum == 1) {
	rainbow_hdr->az_start = az_start;
	rainbow_hdr->az_stop =  az_stop;
	rainbow_hdr->pulse_width_code = pw_code;
	rainbow_hdr->zdata = zdata;
	rainbow_hdr->vdata = vdata;
	rainbow_hdr->wdata = wdata;
	rainbow_hdr->unfolding = unfolding;
	rainbow_hdr->cdata = cdata;
	rainbow_hdr->ddata = ddata;
	rainbow_hdr->uzdata = uzdata;
    }
    rainbow_hdr->elev_params[labelnum-1] =
	(struct elev_params *) malloc(sizeof(struct elev_params));
    rainbow_hdr->elev_params[labelnum-1]->elev_angle = elev;
    rainbow_hdr->elev_params[labelnum-1]->az_rate = az_rate;
    rainbow_hdr->elev_params[labelnum-1]->prf_high = prf_high;
    rainbow_hdr->elev_params[labelnum-1]->prf_low = prf_low;
    rainbow_hdr->elev_params[labelnum-1]->maxrange = range_stop;
}

/**********************************************************/
/*                                                        */
/*                    read_hdr_line                       */
/*                                                        */
/**********************************************************/

static int read_hdr_line(char *buf, int maxchars, FILE *fp)
{
    /* Read a line from the Rainbow file header into character buffer.
     * Function returns the first character in buffer (the "label") if
     * the line was successfully read, -1 otherwise.
     *
     * Note: the following control characters (defined in rainbow.h) are used
     * in the Rainbow header:
     *   CR  - Carriage Return: end of a line.
     *   ETB - End of Transmission Block: divides header into sections (we can
     *         ignore this one.)
     *   ETX - End of Text: end of header.
     */

    int c = 0;
    int badline = 0, i;

    i = 0;
    while ((c = getc(fp)) != CR && c != ETX) {
	if (c == ETB) {               
	    c = getc(fp);                /* Read past both <ETB> and the */
	    if (c == CR) c = getc(fp);   /* combination <ETB><CR>.       */
	}

	buf[i++] = c;

	if (i == maxchars) {
	    badline = 1;
	    break;
	}
    }

    if (badline) {
	fprintf(stderr,"A header line exceeded buffer size %d.\n",maxchars);
	fprintf(stderr,"Did not find end-of-line character 0x%02x.\n",CR);
	buf[maxchars - 1] = '\0';  /* Make it a legal string anyway. */
	return -1;
    }

    buf[i] = '\0';

    if (c != ETX) c = (int) buf[0];
    return c;
}

/**********************************************************/
/*                                                        */
/*                  read_rainbow_header                   */
/*                                                        */
/**********************************************************/

#define BUFSIZE 128

void read_rainbow_header(Rainbow_hdr *rainbow_header, FILE *fp)
{
    /* Reads parameters from Rainbow file header into a rainbow header
       structure. */

    char buf[BUFSIZE];
    int label;

    /* Read each line of the header and extract parameters according to the
     * label.  The label is a single alphabetic character at the beginning
     * of the line which indicates a category of parameters.
     */

    while ((label = read_hdr_line(buf, BUFSIZE, fp)) != ETX && label > 0) {
	switch (label) {
	    case 'H': H_label(rainbow_header, buf);
		      break;
	    case 'P': P_label(rainbow_header, buf);
		      break;
	    case 'W': W_label(rainbow_header, buf);
		      break;
	    case 'F': F_label(rainbow_header, buf);
		      break;
	    case 'R': R_label(rainbow_header, buf);
		      break;
	    case 'A': A_label(rainbow_header, buf);
		      break;
	    case 'N': rainbow_header->product = get_param_int(buf);
		      break;
	    case 'X': /* X_label(rainbow_header, buf); */
		      break;
	}
    }
}
