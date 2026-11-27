/*----------------------------------------------------------------------------*/

# include	"command.h"

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
