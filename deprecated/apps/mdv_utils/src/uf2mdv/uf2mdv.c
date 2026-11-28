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
 * uf2mdv.c
 *
 * Driver to read in uf and convert to mdv
 *
 * Dave Albo, RAP, NCAR, Boulder CO, January, 1998
 *
 *********************************************************************/

#include <signal.h>
#define MAIN
#include <input.h>
#include <output.h>
#include <uf2mdv_tdrp.h>
#undef MAIN

/* static TDRPtable  *table;	*/		/* TDRP parsing table */

/*----------------------------------------------------------------*/
static int check_params_for_consistancy(void)
{
    int nelevation, status;

    status = 1;
    if (params.use_vertical_levels)
    {
        nelevation = params.grid_params.nelev;
	{
	    if (params.vertical_levels_n != nelevation)
	    {
	        fprintf(stderr,
			"ERROR vertical level inconsistancy with nelev %d %d\n",
			nelevation,
			params.vertical_levels_n);
		status = 0;
	    }
	}
    }
    return status;
}

/*----------------------------------------------------------------*/
static void
uf2mdv_parse_args(int argc, char **argv,
		  tdrp_override_t *override,
		  char **params_file_name_p,
		  char **override_input_param,
		  char **override_output_param)
{
    int i;
/*     int error_flag = 0; */
    char usage[BUFSIZ];
    char tmp_str[BUFSIZ];
  
    /* set usage */
    sprintf(usage, "%s%s%s",
	    "Usage: ", argv[0], "\n"
	    "      [--, -help, -man] produce this list.\n"
	    "      [-input_params file] input parameter file \n"
	    "      [-output_params file] output parameter file \n"
	    );
    
    /* initialize */
    TDRP_init_override(override);

    *params_file_name_p = NULL;
    *override_input_param = NULL;
    *override_output_param = NULL;

    /* process for special command options */
    for (i =  1; i < argc; i++)
    {
	if (!strcmp(argv[i], "--")    || !strcmp(argv[i], "-h") ||
	    !strcmp(argv[i], "-help") || !strcmp(argv[i], "-usage") ||
	    !strcmp(argv[i], "-man"))
	{
            fprintf(stderr, "%s", usage);
	    TDRP_usage(stderr);
            exit(0);
	}
	else if (!strcmp(argv[i], "-debug"))
	{
	    sprintf(tmp_str, "debug = TRUE;");
	    TDRP_add_override(override, tmp_str);
	}
	else if (!strcmp(argv[i], "-input_params"))
	{
	    sprintf(tmp_str, "input_param_file = \"%s\";",argv[++i] );
	    TDRP_add_override(override, tmp_str);
	}
	else if (!strcmp(argv[i], "-output_params"))
	{
	    sprintf(tmp_str, "output_param_file = \"%s\";", argv[++i]);
	    TDRP_add_override(override, tmp_str);
	}
    }

    return;
}

/*----------------------------------------------------------------*/
static void process_record(input_data_t *uf)
{
    static int last_vol = -1;
    static int last_sweep = -1;
    
    if (uf->analyzed_state.volume == -1 ||
	uf->analyzed_state.sweep == -1)
	/*
	 * Not wanted
	 */
	return;

    /*
     * It's wanted.  Determine changes in volume and sweep
     */
    if (last_vol != -1 && last_sweep != -1)
    {
	/*
	 * There is stuff in progress.
	 * Is this a new sweep, or a new volume?
	 */
	if (last_vol != uf->analyzed_state.volume)
	{
	    /*
	     * This is a new volume.
	     */
	    OUTPUT_finish_1_sweep();
	    OUTPUT_finish_1_volume();
	}
	else 
	{
	    if (last_sweep != uf->analyzed_state.sweep)
	    {
		/*
		 * This is a new sweep..finish old sweep now.
		 */
		OUTPUT_finish_1_sweep();
	    }
	}	    
    }

    /*
     * Process this new data
     */
    OUTPUT_process(uf);
    last_vol = uf->analyzed_state.volume;
    last_sweep = uf->analyzed_state.sweep;
}

/*----------------------------------------------------------------*/
static void uf2mdv_build_output(void)
{
    input_data_t *u;

    INPUT_data_rewind();
    for (;;)
    {
	u = INPUT_get();
	if (u == NULL)
	    return;
	process_record(u);
	if (params.debug_level >= 1)
	    printf("record:%4d volume:%d  sweep:%d file:%d nbyts:%d\n", 
		   u->analyzed_state.record,
		   u->analyzed_state.volume,
		   u->analyzed_state.sweep,
		   u->analyzed_state.file,
		   u->nbytes);
	INPUT_free(u);
    }
}

/*----------------------------------------------------------------*/
static void uf2mdv_init(int argc, char **argv)
{
    tdrp_override_t  override;
    char *file_name, *input_file_name, *output_file_name;
    
    /* the main part of the program! */
    fprintf(stdout,"\nUF2MDV starting....\n");

    /* parse command line arguments */
    uf2mdv_parse_args(argc, argv, &override, &file_name,
		      &input_file_name, &output_file_name);
    
    /* do the tdrp part */
    if (uf2mdv_tdrp_load_from_args(argc, argv, &params,
				   override.list, &file_name))
    {
	fprintf(stderr, "Problems with params file '%s'\n",
		file_name);
	exit(-1);
    }
    
    /*
     * free up override list
     */
    TDRP_free_override(&override);

    if (check_params_for_consistancy() == 0)
        exit(-1);

    INPUT_init();
    switch (params.processing)
    {
    case ANALYZE_BUILD_OUTPUT:
        OUTPUT_init(params.grid_params.naz, 
		    params.grid_params.nelev);
	break;
    case ANALYZE:
    case BROWSE:
    default:
	break;
    }
}

/*----------------------------------------------------------------*/
int main(int argc, char **argv)
{
    /*
     * Init
     */
    uf2mdv_init(argc, argv);

    switch (params.processing)
    {
    case BROWSE:
	printf("Begin browsing input data\n");
	INPUT_browse();
	printf("Finish browsing input data %d records browsed\n",
	       INPUT_num_records());
	INPUT_finish();
	break;
    case ANALYZE_DATA_RANGE:
	printf("Begin analyzing input data\n");
	INPUT_analyze_data_range();
	INPUT_finish();
	printf("Finish analyzing input data\n\t%d records analyzed\n",
	       INPUT_num_records());
	break;
    case ANALYZE:
	printf("Begin analyzing input data\n");
	INPUT_analyze();
	INPUT_finish();
	printf("Finish analyzing input data\n\t%d records analyzed\n",
	       INPUT_num_records());
	printf("\t%d final analyzed records are wanted\n",
	       INPUT_num_analyzed_records_wanted());
	break;
    case ANALYZE_BUILD_OUTPUT:
	printf("Begin analyzing input data\n");
	INPUT_analyze();
	printf("Finish analyzing input data %d records analyzed\n",
	       INPUT_num_records());
	printf("\t%d final analyzed records are wanted\n",
	       INPUT_num_analyzed_records_wanted());
	printf("Begin building output data\n");
	uf2mdv_build_output();
	INPUT_finish();
	OUTPUT_finish();
	printf("Output data built %d records output %d volumes output\n",
	       OUTPUT_num_records(), OUTPUT_num_volumes());
    default:
	break;
    }
    /*
     * Free up
     */

    uf2mdv_tdrp_free_all();

    exit(1);
}

/*----------------------------------------------------------------*/

