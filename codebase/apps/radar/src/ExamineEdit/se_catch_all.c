/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME

# include <dorade_headers.h>
# include <input_sweepfiles.h>
# include <solo_editor_structs.h>
# include <seds.h>
# include <solo_list_widget_ids.h>
# include <dd_math.h>
# include <solo_window_structs.h>
# include <dd_files.h>
# include <ui.h>
# include <ui_error.h>
# include <function_decl.h>
# include <dgi_func_decl.h>
# include <glib.h>

extern GString *gs_complaints;

/*
 * 
 * 
 *  
 */

/* external functions */
void se_crack_ed_path_name();	/* se_utils.c */
int se_really_readin_cmds();	/* se_proc_data.c */
void se_push_all_ssms();	/* se_utils.c */
void se_append_string();	/* se_utils.c */
int dcdatime();			/* dorade_share.c */

/* internal functions */


/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

int se_readin_cmds(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    struct sed_command_files *scf, *se_return_cmd_files_struct();
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    char str[256], *se_unquote_string();

    se_unquote_string(str, cmdq->uc_text); /* full path name */
    scf = se_return_cmd_files_struct();
    se_crack_ed_path_name(str, scf->directory_text
			  , scf->file_name_text, scf->comment_text);
    se_really_readin_cmds();
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_establish_field(dgi, src_name, dst_name, fns, fnd)
  struct dd_general_info *dgi;
  char *dst_name, *src_name;
  int *fns, *fnd;
{
    /* established a possible new field but does not copy the
     * data from the source field into the new field
     */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dds_structs *dds;
    int ii, jj, fn, fnsx, fndx, size=0, mark, ns, nd, nc, first_empty;
    int ncopy;

    ns = strlen(src_name);
    nd = strlen(dst_name);
    dds = dgi->dds;
    /*
     * find the source field
     */
    if((fnsx = dd_find_field(dgi, src_name)) < 0) {	
      g_string_sprintfa
	  (gs_complaints, "Source parameter %s not found for establis field\n"
		  , src_name);
	return(-1);
    }
    /*
     * deal with the destination field (it may already be there)
     */
    first_empty = fndx = -1;	

    for(fn=0; fn < MAX_PARMS; fn++) {
	if(!dds->field_present[fn]) {
	    if(first_empty < 0)
		  first_empty = fn;
	    continue;
	}
	if(!strncmp(dst_name, dds->parm[fn]->parameter_name, nd)) {
	    /* destination field! */
	    fndx = fn;
	    break;
	}
    }
    if(fndx < 0) {
	/* create the new parameter
	 */
	if(first_empty < 0) {
	  g_string_sprintfa
	    (gs_complaints, "You have exceeded the %d parameter limit!\n"
	     , (int)MAX_PARMS);
	    exit(1);
	}
	fndx = fn = first_empty;

	memcpy((char *)dds->parm[fn], (char *)dds->parm[fnsx]
	       , sizeof(struct parameter_d));
	strncpy(dds->parm[fn]->parameter_name, dst_name, 8);

	ncopy = *dds->qdat[fn]->pdata_desc == 'R' ?
	      sizeof(struct paramdata_d)
		    : dds->parm[fn]->offset_to_data;
# ifdef NEW_ALLOC_SCHEME
	memcpy(dds->qdat[fn], dds->qdat[fnsx], ncopy);
	strncpy(dds->qdat[fn]->pdata_name, dst_name, 8);
	dd_alloc_data_field(dgi, fndx);
# else
	memcpy(dds->rdat[fn], dds->rdat[fnsx], sizeof(struct paramdata_d));
	strncpy(dds->rdat[fn]->pdata_name, dst_name, 8);
# endif

	dds->field_id_num[fn] =
	      dd_return_id_num(dds->parm[fn]->parameter_name);
	dgi->parm_type[fn] = dgi->parm_type[fnsx];
	dds->number_cells[fn] = dds->number_cells[fnsx];
	dds->field_present[fn] = YES;
	dds->last_cell_num[fn] = dds->last_cell_num[fnsx];
    }
    *fns = fnsx; *fnd = fndx;
    return(fndx);
}
/* c------------------------------------------------------------------------ */

int se_once_only(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    /*
     * #a-speckle#
     * #ac-sidelobe-ring#
     * #first-good-gate#
     * #freckle-average#
     * #freckle-threshold#
     * #min-bad-count#
     * #min-notch-shear#
     * #notch-max#
     * #offset-for-radial-shear#
     * #ew-wind#
     * #ns-wind#
     * #vert-wind#
     * #optimal-beamwidth#
     * #omit-source-file-info#
     * #surface-shift#
     * #surface-gate-shift#
     * #deglitch-radius#
     * #deglitch-threshold#
     * #deglitch-min-gates#
     * ##
     * ##
     * ##
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct sed_command_files *scf, *se_return_cmd_files_struct();
    int ii, nn, mark;
    short yy, mon, dd, hh, mm, ss, ms;
    double d, d_time_stamp();
    DD_TIME dts;
    char *aa, *get_tagged_string(), str[128], *se_unquote_string();

    struct swp_file_input_control *sfic;
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct one_boundary *ob;



    seds = return_sed_stuff();
    sfic = seds->sfic;
    sebs = return_se_bnd_struct();
    ob = sebs->current_boundary;

    if(strncmp(cmds->uc_text, "a-speckle", 7) == 0) {
	seds->a_speckle = (cmdq++)->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "ac-sidelobe-ring", 7) == 0) {
	seds->sidelobe_ring_gate_count = (cmdq++)->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "first-good-gate", 11) == 0) {
	seds->first_good_gate = (cmdq++)->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "freckle-average", 9) == 0) {
	seds->freckle_avg_count = (cmdq++)->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "deglitch-radius", 11) == 0) {
	seds->deglitch_radius = (cmdq++)->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "deglitch-min_gates", 11) == 0) {
	seds->deglitch_min_bins = (cmdq++)->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "freckle-threshold", 9) == 0) {
	seds->freckle_threshold = (cmdq++)->uc_v.us_v_float;
    }
    else if(strncmp(cmds->uc_text, "deglitch-threshold", 11) == 0) {
	seds->deglitch_threshold = (cmdq++)->uc_v.us_v_float;
    }
    else if(strncmp(cmds->uc_text, "gates-shifted", 7) == 0) {
	seds->gates_of_shift = (cmdq++)->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "min-bad-count", 7) == 0) {
	seds->min_bad_count = (cmdq++)->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "min-notch-shear", 9) == 0) {
	seds->notch_shear = (cmdq++)->uc_v.us_v_float;
    }
    else if(strncmp(cmds->uc_text, "notch-max", 9) == 0) {
	seds->notch_max = (cmdq++)->uc_v.us_v_float;
    }
    else if(strncmp(cmds->uc_text, "offset-for-radial-shear", 9) == 0) {
	seds->gate_diff_interval = (cmdq++)->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "ew-wind", 5) == 0) {
	seds->ew_wind = (cmdq++)->uc_v.us_v_float;
    }
    else if(strncmp(cmds->uc_text, "ns-wind", 5) == 0) {
	seds->ns_wind = (cmdq++)->uc_v.us_v_float;
    }
    else if(strncmp(cmds->uc_text, "vert-wind", 7) == 0) {
	seds->ud_wind = (cmdq++)->uc_v.us_v_float;
    }
    else if(strncmp(cmds->uc_text, "optimal-beamwidth", 11) == 0) {
	seds->optimal_beamwidth = (cmdq++)->uc_v.us_v_float;
    }
    else if(strncmp(cmds->uc_text, "omit-source-file-info", 7) == 0) {
	scf = se_return_cmd_files_struct();
	scf->omit_source_file_info = !scf->omit_source_file_info;
	g_string_sprintfa
	  (gs_complaints, "Source file info will be %s\n"
	   , scf->omit_source_file_info ? "IGNORED" : "USED");
    }
    else if(strncmp(cmds->uc_text, "surface-shift", 11) == 0) {
	seds->surface_shift = (cmdq++)->uc_v.us_v_float;
    }
    else if(strncmp(cmds->uc_text, "surface-gate-shift", 11) == 0) {
	seds->surface_gate_shift = (cmdq++)->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "radar-", 6) == 0) {
	if(strstr(cmds->uc_text, "inside")) {
	    ob->bh->force_inside_outside = BND_FIX_RADAR_INSIDE;
	}
	else if(strstr(cmds->uc_text, "outside")) {
	    ob->bh->force_inside_outside = BND_FIX_RADAR_OUTSIDE;
	}
	else if(strstr(cmds->uc_text, "unforced")) {
	    ob->bh->force_inside_outside = 0;
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_dir(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    /* this routine will process all commands associated with
     * the setup-input procedure. It also updates the sfic texts
     * this routine resides in .../editor/se_catch_all.c
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    int ii, nn, mark;
    short yy, mon, dd, hh, mm, ss, ms;
    double d, d_time_stamp();
    DD_TIME dts;
    char *a, *get_tagged_string(), str[128], *se_unquote_string();

    struct swp_file_input_control *sfic;
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct solo_edit_stuff *seds, *return_sed_stuff();


    seds = return_sed_stuff();
    sfic = seds->sfic;

    if(strncmp(cmds->uc_text, "sweep-dir", 7) ==0) {
	se_unquote_string(str, cmdq->uc_text);

	if(strncmp(str, "dorade-directory", 8) == 0) {
	    if(a=get_tagged_string("DORADE_DIR")) {
		slash_path(seds->sfic->directory, a);
	    }
	    else {
	      g_string_sprintfa
		(gs_complaints,  "DORADE_DIR undefined\n");
	      slash_path(seds->sfic->directory, cmdq->uc_text);
	    }
	}
	else {
	    strcpy(seds->sfic->directory, str);
	}
	strcpy(sfic->directory_text, sfic->directory);
    }
    else if(strncmp(cmds->uc_text, "radar-names", 7) ==0) {
	seds->num_radars = 0;
	se_push_all_ssms(&seds->radar_stack);
	*sfic->radar_names_text = '\0';
	for(ii=0; cmdq->uc_ctype != UTT_END; cmdq++,ii++) {
	    ssm = se_pop_spair_string();
	    strcpy(ssm->at, cmdq->uc_text);
	    se_append_string(&seds->radar_stack, ssm);
	    seds->num_radars++;
	    if(ii) strcat(sfic->radar_names_text, " ");
	    strcat(sfic->radar_names_text, cmdq->uc_text);
	}
	mark = 0;
    }
    else if(strncmp(cmds->uc_text, "first-sweep", 7) ==0) {
	if(strcmp(cmdq->uc_text, "first") == 0) {
	    seds->sfic->start_time = DAY_ZERO;   
	    strcpy(sfic->first_sweep_text, "first");
	}
	else if(strcmp(cmdq->uc_text, "last") == 0) {
	    seds->sfic->start_time = ETERNITY;   
	    strcpy(sfic->first_sweep_text, "last");
	}
	else {			/* try to interpret this as a date
				 *  of the form mm/dd/yy:hh:mm:ss.ms
				 */
	    strcpy(sfic->first_sweep_text, cmdq->uc_text);
	    dcdatime(cmdq->uc_text, strlen(cmdq->uc_text)
		     , &yy, &mon, &dd, &hh, &mm, &ss, &ms);
	    dts.year = yy > 1900 ? yy : yy+1900;
	    dts.month = mon;
	    dts.day = dd;
	    dts.day_seconds = D_SECS(hh, mm, ss, ms);
	    seds->sfic->start_time = d_time_stamp(&dts);
	}
	mark = 0;
    }
    else if(strncmp(cmds->uc_text, "last-sweep", 7) ==0) {
	if(strcmp(cmdq->uc_text, "first") == 0) {
	    seds->sfic->stop_time = DAY_ZERO;   
	    strcpy(sfic->last_sweep_text, "first");
	}
	else if(strcmp(cmdq->uc_text, "last") == 0) {
	    seds->sfic->stop_time = ETERNITY;   
	    strcpy(sfic->last_sweep_text, "last");
	}
	else {			/* try to interpret this as a number */
	    strcpy(sfic->last_sweep_text, cmdq->uc_text);
	    dcdatime(cmdq->uc_text, strlen(cmdq->uc_text)
		     , &yy, &mon, &dd, &hh, &mm, &ss, &ms);
	    dts.year = yy > 1900 ? yy : yy+1900;
	    dts.month = mon;
	    dts.day = dd;
	    dts.day_seconds = D_SECS(hh, mm, ss, ms);
	    seds->sfic->stop_time = d_time_stamp(&dts);
	}
	mark = 0;
    }
    else if(strncmp(cmds->uc_text, "version", 4) ==0) {
	if(strcmp(cmdq->uc_text, "first") == 0) {
	    seds->sfic->version = -1;
	    strcpy(sfic->version_text, "first");
	}
	else if(strcmp(cmdq->uc_text, "last") == 0) {
	    seds->sfic->version = 99999;
	    strcpy(sfic->version_text, "last");
	}
	else {			/* try to interpret this as a number */
	    seds->sfic->version = 99999;
	    if(!(nn=sscanf(cmdq->uc_text, "%d", &seds->sfic->version))) {
		/* not a number
		 */
	      g_string_sprintfa
		(gs_complaints, 
		 "\nCould not interpret version number: %s  Using: %d\n"
		 , cmdq->uc_text, seds->sfic->version);
	    }
	    sprintf(sfic->version_text, "%d", seds->sfic->version);
	    mark = 0;
	}
    }
    else if(strncmp(cmds->uc_text, "new-version", 7) ==0) {
	seds->sfic->new_version_flag = YES;
    }
    else if(strncmp(cmds->uc_text, "no-new-version", 9) ==0) {
	seds->sfic->new_version_flag = NO;
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_version_change(dir, radar)
  char *dir, *radar;
{
    /* this routine returns a non-zero value if the last pass
     * of the created a new version of some data for this radar
     * in this directory
     *
     */
    char dirx[128];
    struct solo_str_mgmt *ssm;
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();
    if(!seds->last_new_version_flag)
	  return(NO);		/* last pass did not create a new version */
    slash_path(dirx, dir);
    if(strcmp(dirx, seds->last_directory) != 0)
	  return(NO);		/* different directories! */
    

    ssm = seds->last_radar_stack;

    for(; ssm; ssm->next) {
	if(strcmp(radar, ssm->at) == 0)
	      return(YES);
    }
    return(NO);			/* no match for radar name */
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */





