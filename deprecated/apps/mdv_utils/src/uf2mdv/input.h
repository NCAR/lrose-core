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
#ifndef INPUT_H
#define INPUT_H
#include <uf2mdv.h>
#include <input_data.h>

#ifdef MAIN
uf2mdv_tdrp_struct params;
#else
extern uf2mdv_tdrp_struct params;
#endif

/*
 * Initialize the input module
 */
extern void INPUT_init(void);

/*
 * Read in and display the input data
 */
extern void INPUT_browse(void);

/*
 * Build internal tables of INPUT data
 */
extern void INPUT_analyze(void);

/*
 * Figure out data ranges
 */
extern void INPUT_analyze_data_range(void);

extern void INPUT_data_rewind(void);

/*
 * Retrieve next wanted input data.
 */
extern input_data_t *INPUT_get(void);

/*
 * free input data.
 */
extern void INPUT_free(input_data_t *input);

/*
 * Finish the input module
 */
extern void INPUT_finish(void);


extern int INPUT_nazimuth(void);
extern int INPUT_nelevation(void);

extern int INPUT_num_records(void);
extern int INPUT_num_analyzed_records_wanted(void);

#endif

