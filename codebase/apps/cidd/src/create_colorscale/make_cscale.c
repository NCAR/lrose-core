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
/******************************************************************************
 *  Make a colorscale 
 * metar station.lst and reformates int into a easier format
 */
#include <stdio.h>
#include <math.h>

/******************************************************************************
 * MAIN :   Open files and send the output to STDOUT
 *
 */
main(argc,argv)
	int	argc;
	char	*argv[];
{

	int i;
	double min_val, max_val;
	double log_val, val;
	double log_delta;


	/* .001 to 40 mm/hr log scale */
    log_delta = (3.68 + 6.9) / 256.0;
	log_val = -6.9;


	if(argc != 1) { 
		fprintf(stderr,"Usage: make_cscale > outputfile\n");
		exit(-1);
	}

	for( i = 0; i < 256; i++) {
		min_val = exp(log_val);
		max_val = exp(log_val + log_delta);
		log_val += log_delta;
		printf("%.5f %.5f #%x%x%x\n",
				min_val,max_val,i,i,i);
	}
}
