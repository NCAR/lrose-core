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
/*********************************************************************
 * input.c
 *
 * Read input file(s)
 * Defaults contained in paramdef.input
 *
 * Dave Albo, RAP, NCAR, Boulder CO, January, 1998
 *
 *********************************************************************/

#include <signal.h>
#include <input.h>
#include <toolsa/umisc.h>
#include <input_data.h>
#include <input_analyze.h>
#include <input_analyze_data_range.h>

/*----------------------------------------------------------------*/
/*
 * Initialize the input module
 */
void INPUT_init(void)
{
    /* Initialize the objects */
    input_data_init();
}

/*----------------------------------------------------------------*/
/*
 * Read in and display the input data
 */
void INPUT_browse(void)
{
    input_data_t *uf;

    input_data_rewind();
    while ((uf = input_data_get_next()) != NULL)
    {
	input_browse_data(uf);
	input_data_free(uf);
    }
}

/*----------------------------------------------------------------*/
/*
 * Figure out data ranges
 */
void INPUT_analyze_data_range(void)
{
    input_data_t *uf;

    input_analyze_data_range_init();

    /*
     * First pass...just load in the records
     * For each uf_data_get_next, there is a
     * record in the analysis.
     */
    input_data_rewind();
    while ((uf = input_data_get_next()) != NULL)
    {
	input_analyze_data_range_load_next(uf);
	input_data_free(uf);
    }

    /*
     * Next build the tables internally
     */
    input_analyze_data_range_summarize();

}

/*----------------------------------------------------------------*/
/*
 * Build internal tables of UF data
 */
void INPUT_analyze(void)
{
    input_data_t *uf;

    input_analyze_init();

    /*
     * First pass...just load in the records
     * For each uf_data_get_next, there is a
     * record in the analysis.
     */
    input_data_rewind();
    while ((uf = input_data_get_next()) != NULL)
    {
	input_analyze_load_next(uf);
	input_data_free(uf);
    }

    /*
     * Next build the tables internally
     */
    input_analyze_build_tables();

    /*
     * Optionally print out the results
     */
    if (params.analyze_feedback != ANALYZE_FEEDBACK_NONE)
	input_analyze_print("Final results");
    
}

/*----------------------------------------------------------------*/
void INPUT_data_rewind(void)
{
    input_data_rewind();
}

/*----------------------------------------------------------------*/
/*
 * Retrieve next wanted input data.
 */
input_data_t *INPUT_get(void)
{
    return (input_analyze_next_wanted());
}


/*----------------------------------------------------------------*/
/*
 * free uf data.
 */
void INPUT_free(input_data_t *uf)
{
    input_data_free(uf);
}

/*----------------------------------------------------------------*/
/*
 * Finish the input module
 */
void INPUT_finish(void)
{
    if (params.debug_level > 1)
	fprintf(stdout,"input finished\n");
}

/*----------------------------------------------------------------*/
extern int INPUT_num_records(void)
{
    return (input_data_num_records());
}

/*----------------------------------------------------------------*/
extern int INPUT_num_analyzed_records_wanted(void)
{
    return (input_analyze_num_records());
}
