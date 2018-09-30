/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

/*
 * 
 * clear_click_info
 * fgetz
 * 
 * return_solo_click_ptr
 * return_sed_stuff
 * return_swp_fi_in_cntrl
 * 
 * se_all_fer_cmds
 * se_all_other_commands
 * se_append_string
 * se_bracket_procedure
 * se_clone_ssm
 * se_insert_ssm_string
 * 
 * se_pop_spair_string
 * se_print_ssm_strings
 * se_push_all_ssms
 * se_push_spair_string
 * se_remove_ssm_string
 * se_replace_prcdr_items
 * se_replace_ssm_string
 * se_unquote_string 
 * 
 * solo_print_list
 * 
 * 
 */
# include <dorade_headers.h>
# include <input_sweepfiles.h>
# include <solo_editor_structs.h>
# include <dgi_func_decl.h>
# include <function_decl.h>
# include <glib.h>

extern GString *gs_complaints;

static struct solo_str_mgmt *se_spair_strings=NULL;
static struct solo_edit_stuff *seds=NULL;
static struct sed_command_files *scf=NULL;
static struct solo_frame_state_files *sfsf=NULL;

/* Sherrie's functions */
void yes_exit();

/* internally used functions */
void se_all_fer_cmds();
void se_all_other_commands();
void se_insert_ssm_string();
void se_push_all_ssms();
void se_append_string();
int dd_tokens();

int se_BB_setup();
int se_BB_ac_unfold();
int se_absolute_value();
int se_ac_surface_tweak();
int se_add_fields();
int se_add_const();
int se_assert_bad_flags();
int se_assign_value();
int se_bad_flags_logic();
int se_clear_bad_flags();
int se_clr_current_bnd();
int se_copy_bad_flags();
int se_cpy_field();
int se_despeckle();
int se_dir();
int se_fix_vortex_vels();
int se_flag_freckles();
int se_flagged_add();
int se_for_each_ray();
int se_funfold();
int se_hard_zap();
int se_header_value();
int se_histo_ray();
int se_histog_setup();
int se_mult_const();
int se_once_only();
int se_radial_shear();
int se_readin_cmds();
int se_remove_ac_motion();
int se_remove_storm_motion();
int se_remove_field();
int se_rescale_field();
int se_ring_zap();
int se_rewrite();
int se_set_bad_flags();
int se_threshold_field();
int se_use_bnd();
int se_write_sed_cmds();
int se_xy_stuff();
struct ui_cmd_mgmt *se_malloc_ucm();
char *solo_list_entry (struct solo_list_mgmt  *slm, int jj);
void se_all_input_commands(struct solo_list_mgmt  *slm);

# ifdef notyet
int ();
int ();
# endif

static char * syn_sugar =
" to of in with from by put-in is scale bias when and gates on km. degrees deg. deg mps m. low high increment around milliseconds meters-per-second";


/* c------------------------------------------------------------------------ */

int se_cmd_tokens (char * line, struct ui_cmd_mgmt *ucm)
{
  char str[256], *sptrs[32], *aa, token[64], *bb;
  char *dq=0, *dq2=0, *end_str;
  int nn, nt, ii, jj = 0;
  struct ui_command * cmd = ucm->cmds;

  strcpy( token, " " );
  bb = token +1;
  strcpy( str, line );
  aa = str;
  end_str = str + strlen( str );

  while( aa < end_str ) {
    dq = strstr( aa, "\"" );	/* beware of double quoted strings */

    if( dq )
      { *dq = '\0'; }

    nt = dd_tokens( aa, sptrs ); /* tokens so far */

    for( ii = 0; ii < nt; ii++ ) { /* remove syntactic sugar */
      strcpy( bb, sptrs[ii] );
      strcat( bb, " " );
      if( strstr( syn_sugar, token )) {
	continue;
      }
      jj++;
      cmd->uc_ctype = UTT_KW;
      strcpy( (cmd++)->uc_text, sptrs[ii] );
      /*
       * tokens will be checked and interpreted later
       */
    }

    if( dq ) {			/* first set of double quotes */

      if( dq2 = strstr( dq+1, "\"" )) /* second set? */
	{ *dq2 = '\0'; }
      else
	{ dq2 = end_str; }	/* suck up the rest of the line */

      jj++;
      cmd->uc_ctype = UTT_KW;
      strcpy( (cmd++)->uc_text, dq+1 );
      aa = dq2 +1; 
    }
    else
      { break; }		/* should have processed the rest of the
				 * tokens if no double quotes found
				 */
  }
  cmd->uc_ctype = UTT_END;

  return( jj );
}

/* c------------------------------------------------------------------------ */

struct ui_cmd_mgmt *ok_cmd( char * line, struct ui_cmd_mgmt *the_ucm )
{
  struct solo_edit_stuff *seds, *return_sed_stuff();
  struct ui_cmd_mgmt *ucm, *ucm_save = NULL;
  struct ui_command *cmd;
  int ii, mm, nn, nt, hits = 0, nk;
  char *aa, *ee;
  struct solo_list_mgmt *slm;
  
  if(( the_ucm->num_tokens = se_cmd_tokens( line, the_ucm )) < 1 )
    {
       return( ucm_save );
    }
  
  seds = return_sed_stuff();
  the_ucm->keyword = aa = the_ucm->cmds->uc_text;
  nn = strlen( aa );
  ucm = seds->all_cmds;

  /* loop through the list of commands */

  for( ; ucm; ucm = ucm->next ) {
    if( strncmp( ucm->keyword, aa, nn ) == 0 ) {
      hits++;
      ucm_save = ucm;
      if (nn == strlen (ucm->keyword)) {
	 hits = 1;	/* deals with the copy & copy--bad-flags cmds */
	 break;
      }
    }
  }
  if (hits != 1) {
     if (!the_ucm->error) {
	the_ucm->error = (char *)malloc(se_ucm_errlen());
	memset (the_ucm->error, 0, se_ucm_errlen());
     }
     ee = the_ucm->error;
     sprintf (ee, "%s\n %s %s", the_ucm->cmd_line, the_ucm->keyword
	      , (hits < 1) ? "not supported" : "ambiguous");
  }
  else {
    the_ucm->keyword = ucm_save->keyword;
    the_ucm->tmplt = NULL;
    the_ucm->cmd_proc = ucm_save->cmd_proc;
    the_ucm->xtra_tokens = ucm_save->xtra_tokens;

    /* look for the cmd template */

    nk = strlen (ucm_save->keyword);
    slm = seds->all_templates;
    nn = slm->num_entries;
    for (ii = 0; ii < nn; ii++) {
      aa = solo_list_entry (slm, ii);
      if (!strlen (aa))
	{ continue; }
      if (strncmp (aa, ucm_save->keyword, nk) == 0) {
	the_ucm->tmplt = aa;
	break;
      }
    }
  }

  return((hits == 1) ? ucm_save : NULL);
}
/* c------------------------------------------------------------------------ */

int se_absorb_strings(path_name, top_ssm)
  char *path_name;
  struct solo_str_mgmt **top_ssm;
{
    int ii, nn, mark;
    char *a, str[256], *fgetz();
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    FILE *stream;


    if(!(stream = fopen(path_name, "r"))) {
	uii_printf("Unable to open %s", path_name);
	return(-1);
    }
    se_push_all_ssms(top_ssm);

    /* read in the new strings
     */
    for(nn=0;; nn++) {
	if(!(a = fgetz(str, (int)88, stream))) {
	    break;
	}
	ssm = se_pop_spair_string();
	strcpy(ssm->at, str);
	se_append_string(top_ssm, ssm);
    }
    fclose(stream);
    return(nn);
}
/* c------------------------------------------------------------------------ */

void se_util_dummy()
{
    int ii, mark;
    mark = 0;
}
/* c------------------------------------------------------------------------ */

void se_fix_comment(comment)
  char *comment;
{				/* remove blanks and slashed from comment */
    char *a=comment;
    for(; *a; a++)
	  if(*a == ' ' || *a == '/')
		*a = '_';
}
/* c------------------------------------------------------------------------ */

int se_crack_ed_file_name(name, file, comment)
  char *name, *file, *comment;
{
    int ii, jj=0, kk=0, nn, mark, nt, dd_tokenz();
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    char string_space[256], *str_ptrs[32];

    sebs = return_se_bnd_struct();
    if(!(*name))
	  return(NO);

    strcpy(string_space, name);
    if((nt = dd_tokenz(string_space, str_ptrs, ".")) > 3) {
	/* Does the time stamp contain a number */
	jj = sscanf(str_ptrs[1], "%d", &ii);
	/* Does the version number contain a number */
	kk = sscanf(str_ptrs[3], "%d", &ii);
    }
    *comment = '\0';

    if(jj == 1 && kk == 1) {	/* this is probably standard file name */
	/* append the comment if there is one
	 * and try to deal with the fact that
	 * it may contain additional dots
	 */
	for(ii=4; ii < nt; ii++) { 
	    if(ii > 4) strcat(comment, ".");
	    strcat(comment, str_ptrs[ii]);
	}
	strcpy(file, str_ptrs[0]);
	for(ii=1; ii < 4; ii++) {
	    strcat(file, ".");
	    strcat(file, str_ptrs[ii]);
	}
	return(YES);
    }
    else {			/* punt! */
	strcpy(file, name);
	return(NO);
    }
}
/* c------------------------------------------------------------------------ */

void se_crack_ed_path_name(name, dir, file, comment)
  char *name, *dir, *file, *comment;
{
    int ii, nn, mark;
    char *a=name, *b, *c, *e, *strrchr();
    struct boundary_stuff *sebs, *return_se_bnd_struct();

    sebs = return_se_bnd_struct();
    /* get the directory
     */
    if(!(*a))
	  return;
    if(b = strrchr(name, '/')) {
	for(b++,c=dir; a < b; *c++ = *a++);
	*c++ = '\0';
    }
    else {			/* no directory name in path */
	strcpy(dir, "./");
	b = a;
    }
    se_crack_ed_file_name(b, file, comment);
}
/* c------------------------------------------------------------------------ */

struct sed_command_files *
se_return_cmd_files_struct()
{
    int nt, dd_tokens();
    char *aa, string_space[88], *strptrs[16];
    struct swp_file_input_control *sfic, *return_swp_fi_in_cntrl();


    if(!scf) {
	scf = (struct sed_command_files *)
	      malloc(sizeof(struct sed_command_files));
	memset(scf, 0, sizeof(struct sed_command_files));
	strcpy(scf->comment_text, "no_comment");
	scf->omit_source_file_info = YES;
	strcpy(scf->comment_text, "no_comment");
	sfic = return_swp_fi_in_cntrl();
	strcpy(scf->directory_text, sfic->directory_text);
    }
    return(scf);
}
/* c------------------------------------------------------------------------ */

struct solo_frame_state_files *
se_return_state_files_struct()
{
    if(!sfsf) {
	sfsf = (struct solo_frame_state_files *)
	      malloc(sizeof(struct solo_frame_state_files));
	memset(sfsf, 0, sizeof(struct solo_frame_state_files));
	strcpy(sfsf->comment_text, "no_comment");
    }
    return(sfsf);
}
/* c------------------------------------------------------------------------ */

char *
se_string_time(t, str)
  double t;
  char *str;
{
    double d, d_time_stamp();
    DD_TIME dts, *d_unstamp_time();

    dts.time_stamp = t;
    d_unstamp_time(&dts);
    sprintf(str, "%02d/%02d/%02d:%02d:%02d:%02d.%03d"
	   , dts.month
	   , dts.day
	   , dts.year-1900
	   , dts.hour
	   , dts.minute
	   , dts.second
	   , dts.millisecond
	   );
    return(str);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */
/* c...mark */
/* c------------------------------------------------------------------------ */

struct solo_click_info *
clear_click_info()
{
    struct solo_click_info *sci, *return_solo_click_ptr();

    sci = return_solo_click_ptr();
    memset(sci, 0, sizeof(struct solo_click_info));
    return(sci);
}
/* c------------------------------------------------------------------------ */

char *
fgetz(aa, nn, stream)
  char *aa;
  int nn;
  FILE *stream;
{
    int ii, jj=0;
    char *c=aa;

    for(; nn--; jj++,*c++ = ii) {
	ii = fgetc(stream);
	if(ii == EOF) {
	    if(!jj) return(NULL);
	    return(aa);
	}
	if(ii == '\n') {	/* don't include the \n */
	    *c = '\0';
	    return(aa);
	}
    }
    return(aa);
}
/* c------------------------------------------------------------------------ */

struct solo_click_info *
return_solo_click_ptr()
{
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();
    if(!seds->click) {
	seds->click = (struct solo_click_info *)
	      malloc(sizeof(struct solo_click_info));
	memset(seds->click, 0, sizeof(struct solo_click_info));
    }
    return(seds->click);
}
/* c------------------------------------------------------------------------ */

int
se_malloc_flagged_arrays(seds, max_gates)
  struct solo_edit_stuff *seds;
  int max_gates;
{
    int ii, nn=max_gates;
    unsigned short *us;

    if(max_gates < seds->max_gates)
	  return(seds->max_gates);

    if(seds->all_zeroes_array) free(seds->all_zeroes_array);
    if(seds->all_ones_array) free(seds->all_ones_array);
    if(seds->bad_flag_mask_array) free(seds->bad_flag_mask_array);
    if(seds->boundary_mask_array) free(seds->boundary_mask_array);

    if(!(seds->all_zeroes_array =
	 (unsigned short *)malloc(max_gates * sizeof(unsigned short)))) {
	nn = 0;
    }
    else {
	memset(seds->all_zeroes_array, 0, max_gates * sizeof(unsigned short));
    }

    if(!(seds->all_ones_array =
	 (unsigned short *)malloc(max_gates * sizeof(unsigned short)))) {
	nn = 0;
    }
    else {
	for(ii=max_gates, us = seds->all_ones_array; ii--; *us++ = 1);
    }

    if(!(seds->bad_flag_mask_array =
	 (unsigned short *)malloc(max_gates * sizeof(unsigned short)))) {
	nn = 0;
    }
    else {
	memset(seds->bad_flag_mask_array, 0
	       , max_gates * sizeof(unsigned short));
    }

    if(!(seds->boundary_mask_array =
	 (unsigned short *)malloc(max_gates * sizeof(unsigned short)))) {
	nn = 0;
    }
    else {
	memset(seds->boundary_mask_array, 0
	       , max_gates * sizeof(unsigned short));
    }

    if(!nn) {
	uii_printf("Unable to malloc flagged arrays!\n");
    }
    else {
	seds->max_gates = max_gates;
    }
    return(nn);
}
/* c------------------------------------------------------------------------ */

struct ui_cmd_mgmt *malloc_cmd_mgmt()
{
  int llen = 128;
  struct ui_cmd_mgmt *ucm;
  ucm = (struct ui_cmd_mgmt *)malloc( sizeof( struct ui_cmd_mgmt ));
  memset( ucm, 0, sizeof( struct ui_cmd_mgmt ));

  return( ucm );
}
/* c------------------------------------------------------------------------ */

struct solo_edit_stuff *
return_sed_stuff()
{
    int ii, jj, mm, nn, llen;
    struct solo_list_mgmt *slm, *solo_malloc_list_mgmt();
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct ui_cmd_mgmt *ucm, *ucm_prev;
    struct ui_command *cmd;
    static char *a, b, *deep6=0;

    if(!seds) {
	if(!(seds = (struct solo_edit_stuff *)
	      malloc(sizeof(struct solo_edit_stuff)))) {
	    uii_printf("Unable to malloc solo editor struct!\n");
	    yes_exit();
	}
	memset(seds, 0, sizeof(struct solo_edit_stuff));
	if(!(se_malloc_flagged_arrays(seds, SE_MAX_GATES))) {
	    yes_exit();
	}
 
	seds->sfic = (struct swp_file_input_control *)
	      malloc(sizeof(struct swp_file_input_control));
	memset(seds->sfic, 0, sizeof(struct swp_file_input_control));
	strcpy (seds->sfic->first_sweep_text, " ");
	strcpy (seds->sfic->last_sweep_text, " ");
	strcpy (seds->sfic->version_text, "last");
	seds->sfic->version = 99999;
	seds->se_frame = SE_FRAME;
	/*
	 * set default parameters   c...mark
	 */
	seds->BB_avg_count = 4;
	seds->BB_max_neg_folds = 999;
	seds->BB_max_pos_folds = 999;
	strcpy(seds->BB_unfold_field, "VU");

	seds->a_speckle = 3;
	strcpy(seds->despeckle_field, "VD");

	seds->threshold_val = 0.33;
	strcpy(seds->threshold_target, "VD");
	strcpy(seds->threshold_field, "NCP");
	
	seds->freckle_threshold = 5.0;
	seds->dual_average_offset = 1;
	seds->freckle_avg_count = 5;
	strcpy(seds->defreckle_field, "DZ");

	seds->sidelobe_ring_gate_count = 3;
	strcpy(seds->ac_sidelobe_ring_field, "VU");

	seds->optimal_beamwidth = 0;

	strcpy(seds->remove_ac_motion_field, "VG");
	
	strcpy(seds->histo_comment, "no_comment");
	/*
	 * generate a list state parameter commands
	 */
	seds->all_other_cmds = solo_malloc_list_mgmt(SE_MAX_STRING);
	seds->all_fer_cmds = solo_malloc_list_mgmt(SE_MAX_STRING);
	se_all_fer_cmds(seds->all_fer_cmds);
	se_all_other_commands(seds->all_other_cmds);
	seds->current_cmds = solo_malloc_list_mgmt(SE_MAX_STRING);
	seds->all_templates = solo_malloc_list_mgmt(SE_MAX_STRING);
	se_all_fer_cmds(seds->all_templates);
	se_all_other_commands(seds->all_templates);
	se_all_input_commands(seds->all_templates);

	seds->surface_gate_shift = -3;

	ucm = se_malloc_ucm();
	seds->first_oto_cmd = ucm; /* one time only commands */
	ucm->max_cmds = 1;
	
	ucm = se_malloc_ucm();
	seds->first_fer_cmd = ucm; /* for each ray commands */
	ucm->max_cmds = 1;
	
	ucm = se_malloc_ucm();
	seds->first_input_cmd = ucm; /* for each ray commands */
	ucm->max_cmds = 1;

	/*
	 * exhaustive list of commands with the
	 * subroutine that will process each command
	 * 
	 * routines and files to be modified when adding a new command
	 * 
	 * edit this routine and  "se_all_fer_cmds()" or
	 #    "se_all_other_commands()" in this file.
	 * generate code to perform this command
	 * add help for this command in ../perusal/help_edit.h
	 * add an entry in the routine "edit_cmds_help_widget ()"
	 *    in the file ../perusal/sii_edit_widget.c
	 * 
	 */


	ucm = malloc_cmd_mgmt();

	ucm->keyword = "BB-gates-averaged";
	ucm->cmd_proc = se_BB_setup;
	ucm->one_time_only = YES;
	ucm_prev = ucm;
	seds->all_cmds = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "BB-max-neg-folds";
	ucm->cmd_proc = se_BB_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "BB-max-pos-folds";
	ucm->cmd_proc = se_BB_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "BB-unfolding";
	ucm->cmd_proc = se_BB_ac_unfold;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "BB-use-ac-wind";
	ucm->cmd_proc = se_BB_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "BB-use-first-good-gate";
	ucm->cmd_proc = se_BB_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "BB-use-local-wind";
	ucm->cmd_proc = se_BB_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "a-speckle";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "absolute-value";
	ucm->cmd_proc = se_absolute_value;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

# ifdef notyet
	ucm = malloc_cmd_mgmt();

	ucm->keyword = "ac-sidelobe-ring";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;
# endif

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "add-field";
	ucm->cmd_proc = se_add_fields;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "add-value";
	ucm->cmd_proc = se_add_const;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "and-bad-flags";
	ucm->xtra_tokens = 1;
	ucm->cmd_proc = se_bad_flags_logic;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "append-histogram-to-file";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "area-histogram";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "assert-bad-flags";
	ucm->cmd_proc = se_assert_bad_flags;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "assign-value";
	ucm->cmd_proc = se_assign_value;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "clear-bad-flags";
	ucm->cmd_proc = se_clear_bad_flags;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

# ifdef obsolete
	ucm = malloc_cmd_mgmt();

	ucm->keyword = "clear-boundary";
	ucm->cmd_proc = se_clr_current_bnd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;
# endif

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "complement-bad-flags";
	ucm->cmd_proc = se_clear_bad_flags;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "copy";
	ucm->cmd_proc = se_cpy_field;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "copy-bad-flags";
	ucm->cmd_proc = se_copy_bad_flags;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "count-histogram";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "deglitch-min-gates";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "deglitch-radius";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "deglitch-threshold";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "do-histogram";
	ucm->cmd_proc = se_histo_ray;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "despeckle";
	ucm->cmd_proc = se_despeckle;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "dont-append-histogram-to-file";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "dont-use-boundary";
	ucm->cmd_proc = se_use_bnd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "duplicate";
	ucm->cmd_proc = se_cpy_field;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "establish-and-reset";
	ucm->cmd_proc = se_cpy_field;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "ew-wind";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "exponentiate";
	ucm->cmd_proc = se_mult_const;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "first-good-gate";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "first-sweep";
	ucm->cmd_proc = se_dir;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "fix-vortex-velocities";
	ucm->cmd_proc = se_fix_vortex_vels;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "flag-freckles";
	ucm->cmd_proc = se_flag_freckles;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "flag-glitches";
	ucm->cmd_proc = se_flag_freckles;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "flagged-add";
	ucm->cmd_proc = se_flagged_add;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "flagged-assign";
	ucm->cmd_proc = se_assign_value;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "flagged-copy";
	ucm->cmd_proc = se_cpy_field;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "flagged-multiply";
	ucm->cmd_proc = se_flagged_add;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "for-each-ray"; /* this should be ignored */
	ucm->cmd_proc = se_for_each_ray;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "forced-unfolding";
	ucm->cmd_proc = se_funfold;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "freckle-average";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "freckle-threshold";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "gates-shifted";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

# ifdef obsolete
	ucm = malloc_cmd_mgmt();

	ucm->keyword = "get-commands";
	ucm->cmd_proc = se_readin_cmds;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;
# endif

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "header-value";
	ucm->cmd_proc = se_header_value;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "histogram-comment";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "histogram-directory";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "histogram-flush";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "ignore-field";
	ucm->cmd_proc = se_remove_field;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "irregular-histogram-bin";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "last-sweep";
	ucm->cmd_proc = se_dir;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "map-boundary";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "merge-field";
	ucm->cmd_proc = se_add_fields;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "min-bad-count";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "min-notch-shear";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "mult-fields";
	ucm->cmd_proc = se_add_fields;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "multiply";
	ucm->cmd_proc = se_mult_const;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "new-histogram-file";
	ucm->cmd_proc = se_xy_stuff;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "new-version";
	ucm->cmd_proc = se_dir;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "no-new-version";
	ucm->cmd_proc = se_dir;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "notch-max";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "ns-wind";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "nyquist-velocity";
	ucm->cmd_proc = se_BB_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "offset-for-radial-shear";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "omit-source-file-info";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "optimal-beamwidth";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "or-bad-flags";
	ucm->xtra_tokens = 1;
	ucm->cmd_proc = se_bad_flags_logic;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "radar-names";
	ucm->cmd_proc = se_dir;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "radial-shear";
	ucm->cmd_proc = se_radial_shear;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "rain-rate";
	ucm->cmd_proc = se_mult_const;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "regular-histogram-parameters";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "remove-aircraft-motion";
	ucm->cmd_proc = se_remove_ac_motion;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "remove-only-second-trip-surface";
	ucm->cmd_proc = se_ac_surface_tweak;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "remove-only-surface";
	ucm->cmd_proc = se_ac_surface_tweak;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "remove-ring";
	ucm->cmd_proc = se_ring_zap;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "remove-surface";
	ucm->cmd_proc = se_ac_surface_tweak;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "remove-storm-motion";
	ucm->cmd_proc = se_remove_storm_motion;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "rescale-field";
	ucm->cmd_proc = se_rescale_field;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "rewrite";
	ucm->cmd_proc = se_rewrite;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "save-commands";
	ucm->cmd_proc = se_write_sed_cmds;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "select-site-list";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "set-bad-flags";
	ucm->xtra_tokens = 1;
	ucm->cmd_proc = se_set_bad_flags;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "shift-field";
	ucm->cmd_proc = se_cpy_field;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "show-site-values";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "site-list";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "subtract";
	ucm->cmd_proc = se_add_fields;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "surface-gate-shift";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

# ifdef notyet
	ucm = malloc_cmd_mgmt();

	ucm->keyword = "surface-shift";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;
# endif

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "sweep-directory";
	ucm->cmd_proc = se_dir;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "threshold";
	ucm->cmd_proc = se_threshold_field;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "unconditional-delete";
	ucm->cmd_proc = se_hard_zap;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "use-boundary";
	ucm->cmd_proc = se_use_bnd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "version";
	ucm->cmd_proc = se_dir;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "vert-wind";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xor-bad-flags";
	ucm->xtra_tokens = 1;
	ucm->cmd_proc = se_bad_flags_logic;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xy-directory";
	ucm->cmd_proc = se_histog_setup;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xy-listing";
	ucm->cmd_proc = se_xy_stuff;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

# ifdef notyet

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "panner-toggle";
	ucm->cmd_proc = sp_ui_plot_cmds;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = set-angular-fill"";
	ucm->cmd_proc = sp_ui_plot_cmds;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "set-x-tic-mag";
	ucm->cmd_proc = sp_ui_plot_cmds;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "set-y-tic-mag";
	ucm->cmd_proc = sp_ui_plot_cmds;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "screen-clear-mode";
	ucm->cmd_proc = sp_ui_plot_cmds;
	ucm_prev->next = ucm;
	ucm_prev = ucm;
	ucm = malloc_cmd_mgmt();

	ucm->keyword = "gif-file";
	ucm->cmd_proc = sp_ui_plot_cmds;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xwd-file";
	ucm->cmd_proc = sp_ui_plot_cmds;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "png-file";
	ucm->cmd_proc = sp_ui_plot_cmds;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "jpeg-file";
	ucm->cmd_proc = sp_ui_plot_cmds;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "postscript-file";
	ucm->cmd_proc = sp_ui_plot_cmds;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "nexrad-mode";
	ucm->cmd_proc = sp_ui_plot_cmds;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "surface-gate-shift";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "surface-shift";
	ucm->cmd_proc = se_once_only;
	ucm->one_time_only = YES;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xdata";
	ucm->key = EX_RADAR_DATA;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xbeams";
	ucm->key = EX_BEAM_INVENTORY;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xedit";
	ucm->key = EX_EDIT_HIST;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xheaders";
	ucm->key = EX_DESCRIPTORS;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xleft";
	ucm->key = EX_SCROLL_LEFT;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xright";
	ucm->key = EX_SCROLL_RIGHT;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xup";
	ucm->key = EX_SCROLL_UP;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xdown";
	ucm->key = EX_SCROLL_DOWN;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xminus-fold";
	ucm->key = EX_MINUS_FOLD;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xplus-fold";
	ucm->key = EX_PLUS_FOLD;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xdelete";
	ucm->key = EX_DELETE;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xreplace";
	ucm->key = EX_REPLACE;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xundo";
	ucm->key = EX_UNDO;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xclear_changes";
	ucm->key = EX_CLEAR_CHANGES;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xcommit";
	ucm->key = EX_COMMIT;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xannotation";
	ucm->key = EX_TOGGLE_ANNOTATION;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xat";
	ucm->key = EX_CHANGE_LOCATION;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xpick";
	ucm->key = EX_CLICK_IN_LIST;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xfields";
	ucm->key = EX_WHICH_FIELDS;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xray-count";
	ucm->key = EX_RAY_COUNT;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xcell-count";
	ucm->key = EX_CELL_COUNT;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xscroll-inc";
	ucm->key = EX_SCROLL_INC;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xformat";
	ucm->key = EX_DISPLAY_FMT;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xcenter-on-click";
	ucm->key = EX_CLICK_IN_DATA;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xframe-num";
	ucm->key = EX_FRAME_NUM;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xlog";
	ucm->key = EX_DISPLAY_LOG;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xlist-selects";
	ucm->key = XX_LIST_SELECTED_FILES;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xpreserve-file";
	ucm->key = XX_PRESERVE_FILE;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "xzap-files";
	ucm->key = XX_ZAP_FILES;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "next-boundary";
	ucm->key = SE_NEXT_BOUNDARY;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "time-series";
	ucm->key = TS_TIME_SERIES;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "ts-up-down";
	ucm->key = TS_UP_DOWN;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "ts-contiguous";
	ucm->key = TS_CONTIGUOUS;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "ts-ground-relative";
	ucm->key = TS_RELATIVE;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "ts-start-time";
	ucm->key = TS_START_TIME;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;

	ucm = malloc_cmd_mgmt();

	ucm->keyword = "ts-stop-time";
	ucm->key = TS_STOP_TIME;
	ucm->cmd_proc = sxm_ui_cmd;
	ucm_prev->next = ucm;
	ucm_prev = ucm;





	ucm = malloc_cmd_mgmt();

	ucm->keyword = "";
	ucm->cmd_proc = ;
	ucm_prev->next = ucm;
	ucm_prev = ucm;
# endif

    }
    return(seds);
}
/* c------------------------------------------------------------------------ */

struct swp_file_input_control *
return_swp_fi_in_cntrl()
{
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();
    return(seds->sfic);
}
/* c------------------------------------------------------------------------ */

void se_all_fer_cmds(which)
  struct solo_list_mgmt *which;
{
    char *a, str[SE_MAX_STRING];

    a = "absolute-value of <field>";
    solo_add_list_entry(which, a, strlen(a));
# ifdef notyet
    a = "BB-ac-unfolding of <field>";
    solo_add_list_entry(which, a, strlen(a));
# endif
    a = "BB-unfolding of <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "add-field <field> to <field> put-in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "add-value <real> to <field> put-in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "and-bad-flags with <field> <where> <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "assert-bad-flags in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "assign-value <real> to <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "clear-bad-flags";
    solo_add_list_entry(which, a, strlen(a));

    a = "complement-bad-flags";
    solo_add_list_entry(which, a, strlen(a));

    a = "copy <field> to <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "copy-bad-flags from <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "despeckle <field>";
    solo_add_list_entry(which, a, strlen(a));
# ifdef obsolete
    a = "do-defreckling in <field>";
    solo_add_list_entry(which, a, strlen(a));
# endif
    a = "do-histogram";
    solo_add_list_entry(which, a, strlen(a));

    a = "dont-use-boundary";
    solo_add_list_entry(which, a, strlen(a));

    a = "duplicate <field> in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "establish-and-reset <field> to <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "exponentiate <field> by <real> put-in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "fix-vortex-velocities in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "flag-freckles in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "flag-glitches in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "flagged-add of <real> in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "flagged-assign of <real> in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "flagged-copy <field> to <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "flagged-multiply by <real> in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "forced-unfolding in <field> around <real>";
    solo_add_list_entry(which, a, strlen(a));

# ifdef obsolete
    a = "for-each-ray";
    solo_add_list_entry(which, a, strlen(a));
# endif
    a = "header-value <name> is <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "ignore-field <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "merge-field <field> with <field> put-in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "mult-fields <field> by <field> put-in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "multiply <field> by <real> put-in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "or-bad-flags with <field> <where> <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "radial-shear in <field> put-in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "rain-rate <field> by <real> put-in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "remove-aircraft-motion in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "remove-only-surface in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "remove-only-second-trip-surface in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "remove-ring in <field> from <real> to <real> km.";
    solo_add_list_entry(which, a, strlen(a));

    a = "remove-storm-motion in <field> of <real> deg <real> mps";
    solo_add_list_entry(which, a, strlen(a));

    a = "remove-surface in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "rescale-field <field> <real> <real> scale and bias";
    solo_add_list_entry(which, a, strlen(a));

    a = "rewrite";
    solo_add_list_entry(which, a, strlen(a));

    a = "set-bad-flags when <field> <where> <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "shift-field <field> put-in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "subtract <field> from <field> put-in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "threshold <field> on <field> <where> <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "unconditional-delete in <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "use-boundary";
    solo_add_list_entry(which, a, strlen(a));

    a = "xor-bad-flags with <field> <where> <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "xy-listing of <field> and  <field>";
    solo_add_list_entry(which, a, strlen(a));

# ifdef obsolete

    a = "";
    solo_add_list_entry(which, a, strlen(a));

    a = "";
    solo_add_list_entry(which, a, strlen(a));
# endif
}
/* c------------------------------------------------------------------------ */

void se_all_input_commands(which)
  struct solo_list_mgmt *which;
{
   char *a, str[SE_MAX_STRING];
   
   a = "first-sweep date-time";
   solo_add_list_entry(which, a, strlen(a));
   a = "generate-sweep-list for radar-name";
   solo_add_list_entry(which, a, strlen(a));
   
   a = "last-sweep date-time";
   solo_add_list_entry(which, a, strlen(a));
   
   a = "new-version";
   solo_add_list_entry(which, a, strlen(a));
   
   a = "process-data";
   solo_add_list_entry(which, a, strlen(a));
   
   a = "source-version-number is version-indicator";
   solo_add_list_entry(which, a, strlen(a));
   
   a = "sweep-directory is directory-name";
   solo_add_list_entry(which, a, strlen(a));
   
}
/* c------------------------------------------------------------------------ */

void se_all_other_commands(which)
  struct solo_list_mgmt *which;
{
    char *a, str[SE_MAX_STRING];

# ifdef obsolete
    a = "first-sweep date-time";
    solo_add_list_entry(which, a, strlen(a));
    a = "generate-sweep-list for radar-name";
    solo_add_list_entry(which, a, strlen(a));

    a = "last-sweep date-time";
    solo_add_list_entry(which, a, strlen(a));

    a = "new-version";
    solo_add_list_entry(which, a, strlen(a));

    a = "process-data";
    solo_add_list_entry(which, a, strlen(a));

    a = "source-version-number is version-indicator";
    solo_add_list_entry(which, a, strlen(a));

    a = "sweep-directory is directory-name";
    solo_add_list_entry(which, a, strlen(a));

    a = "one-time commands";
    solo_add_list_entry(which, a, strlen(a));

    a = "!";
    solo_add_list_entry(which, a, strlen(a));
# endif

    a = "BB-gates-averaged is <integer> gates";
    solo_add_list_entry(which, a, strlen(a));

    a = "BB-max-neg-folds is <integer>";
    solo_add_list_entry(which, a, strlen(a));

    a = "BB-max-pos-folds is <integer>";
    solo_add_list_entry(which, a, strlen(a));

    a = "BB-use-ac-wind";
    solo_add_list_entry(which, a, strlen(a));

    a = "BB-use-first-good-gate";
    solo_add_list_entry(which, a, strlen(a));

    a = "BB-use-local-wind";
    solo_add_list_entry(which, a, strlen(a));

    a = "a-speckle is <integer> gates";
    solo_add_list_entry(which, a, strlen(a));
# ifdef notyet
    a = "ac-sidelobe-ring is <integer> gates";
    solo_add_list_entry(which, a, strlen(a));
# endif
    a = "append-histogram-to-file";
    solo_add_list_entry(which, a, strlen(a));

    a = "area-histogram on <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "count-histogram on <field>";
    solo_add_list_entry(which, a, strlen(a));

    a = "deglitch-min-gates is <integer>";
    solo_add_list_entry(which, a, strlen(a));

    a = "deglitch-radius is <integer>";
    solo_add_list_entry(which, a, strlen(a));

    a = "deglitch-threshold is <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "dont-append-histogram-to-file";
    solo_add_list_entry(which, a, strlen(a));

    a = "ew-wind is <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "first-good-gate is <integer>";
    solo_add_list_entry(which, a, strlen(a));

    a = "freckle-average is <integer> gates";
    solo_add_list_entry(which, a, strlen(a));

    a = "freckle-threshold is <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "gates-shifted is <integer>";
    solo_add_list_entry(which, a, strlen(a));

    a = "histogram-comment <comment>";
    solo_add_list_entry(which, a, strlen(a));

    a = "histogram-directory <directory>";
    solo_add_list_entry(which, a, strlen(a));

    a = "histogram-flush";
    solo_add_list_entry(which, a, strlen(a));

    a = "irregular-histogram-bin from <real> to <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "min-bad-count is <integer> gates";
    solo_add_list_entry(which, a, strlen(a));

    a = "min-notch-shear is <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "notch-max is <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "new-histogram-file";
    solo_add_list_entry(which, a, strlen(a));

    a = "ns-wind is <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "nyquist-velocity is <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "optimal-beamwidth is <real> degrees";
    solo_add_list_entry(which, a, strlen(a));

    a = "offset-for-radial-shear is <integer> gates";
    solo_add_list_entry(which, a, strlen(a));

    a = "regular-histogram-parameters low <real> high <real> increment <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "surface-gate-shift is <integer> gates";
    solo_add_list_entry(which, a, strlen(a));

    a = "vert-wind is <real>";
    solo_add_list_entry(which, a, strlen(a));

    a = "xy-directory <directory>";
    solo_add_list_entry(which, a, strlen(a));

# ifdef obsolete
    a = "inside-boundaries file-name";
    solo_add_list_entry(which, a, strlen(a));

    a = "outside-boundaries file-name";
    solo_add_list_entry(which, a, strlen(a));

    a = "punt";
    solo_add_list_entry(which, a, strlen(a));


    a = "";
    solo_add_list_entry(which, a, strlen(a));
# endif
}
/* c------------------------------------------------------------------------ */

void se_append_string(top_ssm, ssm)
  struct solo_str_mgmt **top_ssm, *ssm;
{
    /*
     * this routine appends a ssm to a list of ssms
     * the "last" branch of the top node points to the
     * last node of the list
     */
    if(!(*top_ssm)) {			/* no list yet */
	*top_ssm = ssm;
	(*top_ssm)->last = ssm;
    }
    else {
	(*top_ssm)->last->next = ssm; /* last ssm on list should point
				       * to this one */
	ssm->last = (*top_ssm)->last;
	(*top_ssm)->last = ssm;
    }
    ssm->next = NULL;
    return;
}
/* c------------------------------------------------------------------------ */

struct solo_str_mgmt *
se_bracket_procedure(top_ssm, name, ncmds, item_num)
  struct solo_str_mgmt **top_ssm;
  char *name;
  int *ncmds, *item_num;
{
    int ii, nn=0;
    struct solo_str_mgmt *ssmx=*top_ssm, *ssm=NULL, *se_pop_spair_string();

    *ncmds = *item_num = 0;

    for(; ssmx; ssmx=ssmx->next, (*item_num)++) {
	if(strstr(ssmx->at, "procedure")) {
	    if(strstr(ssmx->at, name)) {
		break;
	    }
	}
    }
    if(!ssmx)
	  return(ssmx);
    ssm = ssmx;
    ssmx = ssmx->next;

    for(*ncmds=1;  ssmx; ssmx=ssmx->next, (*ncmds)++) {
	if(strstr(ssmx->at, "endp"))
	      break;
	else if(strstr(ssmx->at, "procedure"))
	      break;
    }
    (*ncmds)++;
    /*
     * may need to tack on an "endprocedure"
     */
    if(!ssmx) {			
	ssmx = se_pop_spair_string();
	strcpy(ssmx->at, "endprocedure");
	se_append_string(top_ssm, ssmx);
    }
    else if(!strstr(ssmx->at, "endp")) {
	ssmx = se_pop_spair_string();
	strcpy(ssmx->at, "endprocedure");
	se_insert_ssm_string(top_ssm, (*item_num)+(*ncmds), ssmx);
    }
    return(ssm);
}
/* c------------------------------------------------------------------------ */

struct solo_str_mgmt *
se_clone_ssm(ssm)
  struct solo_str_mgmt *ssm;
{
    struct solo_str_mgmt *ssmx, *se_pop_spair_string();

    ssmx = se_pop_spair_string();
    strcpy(ssmx->at, ssm->at);
    return(ssmx);
}
/* c------------------------------------------------------------------------ */

void se_insert_ssm_string(top_ssm, item, ssm)
  struct solo_str_mgmt **top_ssm, *ssm;
  int item;
{
    /* this routine inserts a ssm before ssm indicated
     * item 0 is the first string
     */
    int ii;
    struct solo_str_mgmt *last, *ssmx=(*top_ssm);

    if(!(*top_ssm) || item == 0) {		/* first item on the list */
	if(item == 0) {
	    ssm->last = (*top_ssm)->last;
	    (*top_ssm)->last = ssm;
	    ssm->next = (*top_ssm);
	    *top_ssm = ssm;
	}
	else {
	    *top_ssm = ssm;
	    (*top_ssm)->last = ssm;
	    (*top_ssm)->next = NULL;
	}
	return;
    }

    for(ii=0; ssmx && ii < item; ii++,ssmx=ssmx->next);

    if(ii < item) {		/* just append it */
	se_append_string(top_ssm, ssm);
    }
    else {
	ssmx->last->next = ssm;
	ssm->last = ssmx->last;
	ssmx->last = ssm;
	ssm->next = ssmx;
    }
    return;
}
/* c------------------------------------------------------------------------ */

struct solo_str_mgmt *
se_pop_spair_string()
{
    struct solo_str_mgmt *ssm = se_spair_strings;

    if(!ssm) {
	ssm = (struct solo_str_mgmt *)malloc(sizeof(struct solo_str_mgmt));
	memset(ssm, 0, sizeof(struct solo_str_mgmt));
	ssm->at = (char *)malloc(SE_MAX_STRING);
    }
    else {
	se_spair_strings = ssm->next;
    }
    *ssm->at = '\0';
    ssm->last = ssm->next = NULL;
    return(ssm);
}
/* c------------------------------------------------------------------------ */

void se_print_ssm_strings(top_ssm)
  struct solo_str_mgmt *top_ssm;
{
    int nn;
    char *a, str[256];
    struct solo_str_mgmt *ssm=top_ssm;
    
    for(; ssm; ssm=ssm->next) {
       strcpy (str, ssm->at);
	a = str +strlen(str)-1;
	if(*a == '\n')
	  { *a = '\0'; }
       g_message (str);
    }
}
/* c------------------------------------------------------------------------ */

struct solo_str_mgmt *
se_pull_ssm_string(top_ssm, ssmx)
  struct solo_str_mgmt **top_ssm, *ssmx;
{
    int ii;

    if(ssmx == *top_ssm) {	/* empties the list */
	if(ssmx->next) {
	    ssmx->next->last = ssmx->last;
	    *top_ssm = ssmx->next;
	}
	else {
	    *top_ssm = NULL;
	}
    }
    else if(ssmx == (*top_ssm)->last) {	/* last member of the list */
	(*top_ssm)->last = ssmx->last;
	ssmx->last->next = NULL;
    }
    else {
	ssmx->next->last = ssmx->last;
	ssmx->last->next = ssmx->next;
    }
    return(ssmx);
}
/* c------------------------------------------------------------------------ */

void se_push_all_ssms(ssmptr)
  struct solo_str_mgmt **ssmptr;
{
    struct solo_str_mgmt *ssm=(*ssmptr);

    if(!ssm)
	  return;

    ssm->last->next = se_spair_strings;
    se_spair_strings = ssm;

    *ssmptr = NULL;
    return;
}
/* c------------------------------------------------------------------------ */

void se_push_spair_string(ssm)
  struct solo_str_mgmt *ssm;
{
    ssm->next = se_spair_strings;
    se_spair_strings = ssm;
    return;
}
/* c------------------------------------------------------------------------ */

struct solo_str_mgmt *
se_remove_ssm_string(top_ssm, item)
  struct solo_str_mgmt **top_ssm;
  int item;
{
    int ii;
    struct solo_str_mgmt *next, *ssmx=(*top_ssm);

    for(ii=0; ssmx && ii < item; ii++, ssmx=ssmx->next);

    if(ii < item || !ssmx)
	  return(NULL);

    if(ssmx == *top_ssm) {	/* empties the list */
	if(ssmx->next) {
	    ssmx->next->last = ssmx->last;
	    *top_ssm = ssmx->next;
	}
	else {
	    *top_ssm = NULL;
	}
    }
    else if(ssmx == (*top_ssm)->last) {	/* last member of the list */
	(*top_ssm)->last = ssmx->last;
	ssmx->last->next = NULL;
    }
    else {
	ssmx->next->last = ssmx->last;
	ssmx->last->next = ssmx->next;
    }
    return(ssmx);
}
/* c------------------------------------------------------------------------ */

void se_replace_prcdr_items(ssma,ssmb)
  struct solo_str_mgmt **ssma, **ssmb;
{
    /* for each command in "b", find its counterpart in "a"
     * and replace it
     */
    int ii, mm, nn;
    struct solo_str_mgmt *ssmx, *ssmy=(*ssmb), *ssmz;
    char *a, *b, *c, *dd_whiteout(), *dd_delimit();

    for(; ssmy; ssmy=ssmy->next) {
	c = a = dd_whiteout(ssmy->at);
	b = dd_delimit(a);	
	for(mm=0; c++ < b; mm++); /* count characters in command name */

	for(ssmx=(*ssma),ssmz=NULL; ssmx; ssmx=ssmx->next) {
	    if(*ssmx->at == '!')
		  continue;
	    b = dd_whiteout(ssmx->at);
	    if(strncmp(a, b, mm) == 0) {
		ssmz = ssmx;
		strcpy(ssmx->at, ssmy->at);
		break;
	    }
	}
	if(!ssmz) {		/* insert this item after the
				 * first line of the procedure */
	    se_insert_ssm_string(ssma, 1, ssmy);
	}
    }
}
/* c------------------------------------------------------------------------ */

int se_replace_ssm_string(top_ssm, item, ssm)
  struct solo_str_mgmt **top_ssm, *ssm;
  int item;
{
    int ii;
    se_remove_ssm_string(top_ssm, item);
    se_insert_ssm_string(top_ssm, item, ssm);
    return(1);
}
/* c------------------------------------------------------------------------ */

char *
se_unquote_string(uqs, qs)
char *qs, *uqs;
{
# ifdef obsolete
    char *a=uqs;
    /*
     * remove the double quotes from either end of the string
     */
    for(; *qs; qs++)
	  if(*qs != '"') *a++ = *qs;
    *a = '\0';
# else
    char *dd_unquote_string();

    dd_unquote_string(uqs, qs);
# endif
    return(uqs);
}
/* c------------------------------------------------------------------------ */

void solo_print_list(slm)
  struct solo_list_mgmt *slm;
{
    /* print the entries in the list
     */
    int ii;
    char **lx;

    if(!slm)
	  return;
    lx = slm->list;

    for(ii=0; ii < slm->num_entries; ii++,lx++) {
	uii_printf("%s\n", *lx);
    }
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

