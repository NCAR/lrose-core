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
/*----------------------------------------------------------------------------*/

# include	"tdwr2moments.h"

int
get_params (Global *Glob)

{
	char    param_file [MAX_NAME_SIZE];
	char    line [MAX_LINE_SZ];
	FILE    *file_ptr;
	int     ret_val = 1;

	(void) strcpy (param_file, getenv ("TDWR_PARAM_FILE") ?
			getenv ("TDWR_PARAM_FILE") : "tdwr.params");

	if ((file_ptr = fopen (param_file, "r")) == (FILE *) NULL)
	{
		(void) printf ("error opening file %s\n", param_file);
		ret_val = 0;
	}
	else
	{
		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%s", Glob->prog_name);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->debug);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%s", Glob->radar_name);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%s", Glob->site_name);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%ld", &Glob->latitude);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%ld", &Glob->longitude);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%hd", &Glob->altitude);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%hd", &Glob->polarization);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%hd", &Glob->vel_bias);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%hd", &Glob->vel_scale);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->lo_prf_gate_spac);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->cmd_shmem_key);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->input_shmem_key);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->output_shmem_key);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->real_time);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->write_shm_output);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->write_fmq_output);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%ld", &Glob->output_fmq_size);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%ld", &Glob->output_fmq_nslots);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->output_fmq_compress);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%s", &Glob->output_fmq_path);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->caf);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->ctf);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->cvf);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->ccv);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->cv);

		(void) fgets (line, sizeof (line), file_ptr);     /* skip the comment */
		(void) fgets (line, sizeof (line), file_ptr);
		(void) sscanf (line, "%d", &Glob->true_north);

		fclose (file_ptr);
		ret_val = 1;
	}
	return (ret_val);

}

/************************************************************************/
