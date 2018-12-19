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
 *  
 */
# define PRESERVATION_FILE_NAME "preserved_sweep_files_list.txt"
static char *preservation_file_name=PRESERVATION_FILE_NAME;

/*
 * log files are time stamped at the time they are opened
 * data displays are always preceeded by a "stat" line to identify the data
 * 
 * you can toggle between logging the display and not logging
 * the display by typing "xlog"
 * "xlog" close will cause the file to be closed
 */
# define     LOG_CLOSED 0
# define     LOG_ACTIVE 1
# define  LOG_SUSPENDED 2

static FILE *log_stream=NULL;
static int log_state=LOG_CLOSED;
static char log_directory[128];
static char log_file_name[80];
static char *log_dir_ptr=NULL;

static struct solo_click_info last_click;
static int click_frame=0, focus_frame=0;
static struct se_changez *sxm_spair_changez=NULL;
static struct examine_widget_info *ewi=NULL;
/*
 * routines present in this file
 */
void sxm_apply_changez();
int sxm_append_to_log_stream();
void sxm_change_cell_in_list();
void sxm_clear_changes();
void sxm_click_in_data();
void sxm_click_in_list();
void sxm_close_log_stream();
void sxm_flush_log_stream();
void sxm_gen_all_files_list();
void sxm_gen_delete_lists();
void sxm_get_widget_info();
void sxm_list_beams();
void sxm_list_descriptors();
void sxm_list_edit_hist();
void sxm_list_to_log();
void sxm_log_stat_line();
void sxm_open_log_stream();
struct se_changez *sxm_pop_change();
struct se_changez *sxm_pop_spair_change();
void sxm_print_list();
void sxm_process_click();
void sxm_push_change();
void sxm_push_spair_change();
void sxm_refresh_list();
struct se_changez *sxm_remove_change();
void sxm_scroll_list();
void sxm_set_click_frame();
void sxm_stack_spair_change();
void sxm_stat_line();
void sxm_ui_cmd();
void sxm_undo_last();
void sxm_unlink_files_list();
void sxm_update_examine_data();

/* Sherrie's routines  */
void sp_change_cursor();
void se_dump_examine_widget();
void se_refresh_examine_widgets();
void sp_refresh_delete_sweeps();
void popup_message();

/*
 * external function declarations
 */

void se_set_sfic_from_frame();	/* sp_basics.c */
void se_setup_input_cmds();
void sp_sweep_file_check();	/* sp_clkd.c */
int se_gen_sweep_list();	/* se_proc_data.c */
int solo_nab_next_file();	/* solo_perusal.c */
void solo_list_sort_file_names(); /* dorade_share.c */
void solo_list_remove_dups();	/* dorade_share.c */
void solo_list_remove_entry();	/* dorade_share.c */
char *solo_modify_list_entry();	/* dorade_share.c */
void se_process_click();	/* se_wgt_stf.c */
int sp_ts_ray_list();		/* sp_clkd.c */
void sp_copy_tsri();		/* sp_basics.c */
void solo_set_busy_signal();	/* sp_basics.c */
void solo_clear_busy_signal();	/* sp_basics.c */
int solo_busy();		/* sp_basics.c */
char *fstring();		/* dorade_share.c */
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

void
sxm_apply_changez(frme)
  int frme;
{
    static int count=0;
    int ii, nn, pn, sn_first_change, sn_last_change, mark, ignore_it=NO;
    int time_series, vv, ival, bad, sweep_list_state=0, ignore_count = 0;
    int loop_count = 0;
    long time_now();
    short *ss, *zz;
    struct se_changez *chz;
    struct examine_control *ecs;
    struct cell_d *celv;
    WW_PTR wwptr, wwptrx, solo_return_wwptr();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_input_sweepfiles_v3 *dis, *dd_return_sweepfile_structs_v3();
    struct unique_sweepfile_info_v3 *usi;
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct ts_ray_table *tsrt;
    struct ts_ray_info *tsri;
    double d_start, d_stop, dd_ts_start();
    char file_info[88], file_name[88];


# ifdef obsolete
    if(solo_busy())
	  return;
# endif
    seds = return_sed_stuff();
    wwptr = solo_return_wwptr(frme);
    wwptrx = wwptr->lead_sweep;
    time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;
    if(!wwptr->examine_info->change_count)
	  return;
    solo_set_busy_signal();
    sp_change_cursor(NO);

    ecs = wwptr->examine_control;
    dont_print_dd_open_info();
    dis = dd_return_sweepfile_structs_v3();
    usi = dis->usi_top;
    dis->editing = YES;
    seds->process_ray_count = seds->volume_count = seds->sweep_count = 0;
    seds->setup_input_ndx = 0;
    seds->modified = seds->finish_up = NO;
    dd_output_flag(YES);

    se_set_sfic_from_frame(frme);
    seds->sfic->clicked_frame = 0;
    g_string_truncate (gs_complaints, 0);

# ifdef never_again
    se_setup_input_cmds();
    ssm = seds->setup_inputs[0];
    
    for(; ssm; ssm=ssm->next) {
	if(strstr(ssm->at, "procedure"))
	      continue;
	ui_perform(ssm->at);
    }
# else
    seds->num_radars = 1;
    se_push_all_ssms(&seds->radar_stack);
    ssm = se_pop_spair_string();
    strcpy(ssm->at, seds->sfic->radar_names_text);
    se_append_string(&seds->radar_stack, ssm);
    strcpy (seds->sfic->directory, seds->sfic->directory_text);
# endif

    /*
     * set up to pass through this sweep
     * setting clicked_frame fakes se_gen_sweep_list() into setting up
     * to pass through the sweep in this frme if not time series
     */
    if(!(se_gen_sweep_list()) || seds->punt) {
	sp_change_cursor(YES);
	solo_clear_busy_signal();
	if (strlen (gs_complaints->str))
	  { sii_message (gs_complaints->str); }
	return;
    }
    usi = dis->usi_top;
    dgi = dd_window_dgi(usi->radar_num, "UNK");
    celv = dgi->dds->celvc;
    dgi->compression_scheme = HRD_COMPRESSION;
    dgi->disk_output = YES;
    slash_path(dgi->directory_name, usi->directory);
    /*
     * find the sweep numbers that bracket the changes
     */
    chz=wwptr->changez_list;
    sn_first_change = sn_last_change = chz->sweep_num;

    for(chz = chz->next; chz; chz = chz->next) {
	if(chz->sweep_num < sn_first_change)
	      sn_first_change = chz->sweep_num;
	if(chz->sweep_num > sn_last_change)
	      sn_last_change = chz->sweep_num;
    }

    for(;;) {
	count++;
	ignore_it = NO;
	if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
	    break;
	}
	if(!seds->process_ray_count) { /* first time through */
	    mark = 0;
	}

	if((nn = wwptr->examine_info->change_count) > 0 &&
	   sn_first_change <= usi->swp_count &&
	   usi->swp_count <= sn_last_change) {

	    chz=wwptr->changez_list->last;
	    /*
	     * changes are applied in reverse order
	     * see if we can find a change for this ray
	     */
	    for(; nn--; chz = chz->last) {
		if(chz->ray_num == usi->ray_num-1 &&
		   chz->sweep_num == usi->swp_count) {
		    /*
		     * apply the change...
		     * find the right cell in the right field
		     * scale the new value and stuff it in if it's
		     * not a bad data flag; otherwise stuff in
		     * a bad data flag
		     */

		    pn = chz->field_num;
# ifdef NEW_ALLOC_SCHEME
		    ss = (short *)dgi->dds->qdat_ptrs[pn];
# else
		    ss = (short *)((char *)dgi->dds->rdat[pn] +
				    sizeof(struct paramdata_d));
# endif
		    switch(chz->typeof_change) {

		    case EX_DELETE:
			*(ss + chz->cell_num) =  dgi->dds->parm[pn]->bad_data;
			break;

		    case EX_MINUS_FOLD:
		    case EX_PLUS_FOLD:
		    case EX_REPLACE:
			*(ss + chz->cell_num) = DD_SCALE
			      (chz->f_new_val
			       , dgi->dds->parm[pn]->parameter_scale
			       , dgi->dds->parm[pn]->parameter_bias);
			break;

		    case EX_RAY_PLUS_FOLD:
		    case EX_RAY_MINUS_FOLD:
		    case EX_GT_PLUS_FOLD:
		    case EX_GT_MINUS_FOLD:
			ival = DD_SCALE
			      (chz->f_new_val
			       , dgi->dds->parm[pn]->parameter_scale
			       , dgi->dds->parm[pn]->parameter_bias);
			bad = dgi->dds->parm[pn]->bad_data;

			if(chz->second_cell_num) {
			    zz = ss + chz->second_cell_num;
			    ss += chz->cell_num;
			}
			else if (chz->typeof_change == EX_GT_PLUS_FOLD ||
				 chz->typeof_change == EX_GT_MINUS_FOLD) {
			    zz = ss + dgi->dds->celv->number_cells;
			    ss += chz->cell_num;
			}
			else {
			    zz = ss + dgi->dds->celv->number_cells;
			}
			for(; ss < zz ; ss++) {
			    if(*ss != bad)
				  *ss += ival;
			}
			break;

		    case EX_RAY_IGNORE:
			if(chz->second_cell_num) {
			    bad = dgi->dds->parm[pn]->bad_data;
			    zz = ss + chz->second_cell_num;
			    ss += chz->cell_num;

			    for(; ss < zz ; *ss++ = bad);
			}
			else {
			    ignore_it = YES;
			    ignore_count++;
			}
			break;

		    default:
			break;
		    }
		    sxm_remove_change(&wwptr->changez_list, chz);
		    --wwptr->examine_info->change_count;
		}
	    }
	}
	seds->process_ray_count++;
	if(!ignore_it)
	      dd_dump_ray(dgi);	/* to a new output file */
	    
	if(time_series && ddswp_last_ray(usi)) { /* end of current sweep */
	    dd_flush(usi->radar_num);	 
	}
	if(usi->ray_num <= 1) {
	    sp_sweep_file_check(usi->directory, usi->filename);
	}
	if(dgi->new_sweep) {
	  printf ("%s", return_dd_open_info());
# ifdef obsolete
	    g_string_sprintfa (gs_complaints, "%s", return_dd_open_info());
# endif
	}
	if(usi->swp_count > usi->num_swps)
	      break;
	if( !(dgi->new_sweep && ignore_it)) {
	   dgi->new_sweep = dgi->new_vol = NO;
	}
    }
    dd_flush(usi->radar_num);	 
    seds->time_modified = time_now();
    wwptr->lead_sweep->sweep_file_modified = YES;
    dis->editing = NO;
    printf("Finished!\n");
    if (strlen (gs_complaints->str))
      { sii_message (gs_complaints->str); }
    sp_change_cursor(YES);
    solo_clear_busy_signal();
    return;
}
/* c------------------------------------------------------------------------ */

int
sxm_append_to_log_stream(stuff, nchar)
  char *stuff;
  int nchar;
{
    int nn;

    if(log_state != LOG_ACTIVE){
	return(0);
    }

    sxm_open_log_stream();

    if(!log_stream){
	return(0);
    }
    
    char line[4096];
    sprintf(line,"%s\n",stuff);

    return(nn = fwrite(line, sizeof(char), strlen(line), log_stream));
}
/* c------------------------------------------------------------------------ */

void
sxm_change_cell_in_list(chz, data_row_num, val)
  struct se_changez *chz;
  int data_row_num;
  float val;
{
    int entry_num, nn;
    int col=chz->col_num, cell=chz->row_num;
    struct examine_control *ecs;
    WW_PTR wwptr, solo_return_wwptr();
    struct solo_list_mgmt *slm;
    char *aa, *bb, *line, tmp_val[16], *solo_list_entry();


    wwptr = solo_return_wwptr(chz->window_num);
    ecs = wwptr->examine_control;
    slm = wwptr->examine_list;
    entry_num = data_row_num + ecs->heading_row_count;

    aa = solo_list_entry(slm, entry_num);
    line = ecs->data_line;
    strcpy(line, aa);

    if(val == ecs->bad_data[col]) {
	bb = ecs->del_str;
    }
    else {
	bb = tmp_val;
	sprintf(tmp_val, ecs->actual_data_format, val);
	if((int)strlen(tmp_val) > ecs->num_chars_per_cell) {
	    tmp_val[ecs->num_chars_per_cell-1] = '|';
	    tmp_val[ecs->num_chars_per_cell] = '\0';
	}
    }
    strncpy(ecs->col_ptrs[col], bb, ecs->num_chars_per_cell);
    solo_modify_list_entry(slm, line, strlen(line), entry_num++);

    return;
}
/* c------------------------------------------------------------------------ */

void
sxm_clear_changes(frme)
  int frme;
{
    WW_PTR wwptr, solo_return_wwptr();

    wwptr = solo_return_wwptr(frme);

    for(; wwptr->changez_list;) {
	sxm_undo_last(frme);
    }
}
/* c------------------------------------------------------------------------ */

void
sxm_click_in_data(sci)
  struct solo_click_info *sci;
{
    WW_PTR wwptr, solo_return_wwptr();
    double x=sci->x, y=sci->y, theta, range;

    wwptr = solo_return_wwptr(sci->frame);
    wwptr->examine_control->ctr_on_click = YES;
    wwptr->examine_control->click_angle = wwptr->clicked_angle;
    wwptr->examine_control->click_range =
	  wwptr->view->type_of_plot & SOLO_TIME_SERIES ?
		KM_TO_M(wwptr->field_vals->rng) :
		      KM_TO_M(wwptr->clicked_range);
    sxm_get_widget_info(sci->frame);
    if(wwptr->examine_info->whats_selected ==  EX_RADAR_DATA)
	  sxm_update_examine_data(sci->frame, 0L);
}
/* c------------------------------------------------------------------------ */

void
sxm_click_in_list(sci)
  struct solo_click_info *sci;
{
    /* this routine is called when there is a
     * EX_CLICK_IN_LIST
     */
    int ii, jj, kk, nn, pn, scaled_nyq_interval;
    int ival, cell, rn, col, entry_num, itsa_run=NO, row_num;
    int undo_it, rrn;
    short *ss;
    struct se_changez *chz, *sxm_pop_change();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct examine_control *ecs;
    WW_PTR wwptr, wwptrx, solo_return_wwptr();
    struct dd_general_info *dgi, *dd_window_dgi();
    float *fptr, val, nyq_interval;
    struct solo_list_mgmt *slm;
    struct ts_ray_table *tsrt;
    struct ts_sweep_info *tssi;
    struct ts_ray_info *tsri;
    char *aa, *bb, *line, tmp_val[16], *solo_list_entry();
    double d, nyqv;


    seds = return_sed_stuff();
    wwptr = solo_return_wwptr(sci->frame);
    wwptrx = wwptr->lead_sweep;
    tsrt = wwptrx->tsrt;
    ecs = wwptr->examine_control;
    sxm_get_widget_info(sci->frame);


    /* which column */
    for(col=0; col < ecs->num_cols; col++) {
	if(sci->which_character < ecs->col_lims[col])
	      break;
    }
    if(col == 0 || col == ecs->num_cols) {
	return;
    }
    col--;			/* forget the range annotation column */
    row_num = sci->which_list_entry - ecs->heading_row_count;

    if(row_num < 0 || row_num >= ecs->actual_num_cells) {
	return;
    }

    switch(wwptr->examine_info->typeof_change) {

    case EX_RAY_PLUS_FOLD:
    case EX_RAY_MINUS_FOLD:
    case EX_RAY_IGNORE:
	if((chz = wwptr->changez_list) &&
	   chz->col_num == col && !chz->second_cell_num &&
	   chz->row_num != row_num &&
	   chz->typeof_change == wwptr->examine_info->typeof_change) {
	    /*
	     * the assumption here is when you click a second time
	     * in the same column for one of the above options,
	     * you're defining a run rather than selecting the whole beam
	     */
	    itsa_run = YES;
	}
	break;
    default:
	break;
    }

    if(itsa_run) {		
	if(row_num < chz->row_num) {
	    chz->second_cell_num = ecs->actual_at_cell + chz->row_num +1;
	    /* we add one for the second cell num so differencing
	     * the two cell numbers give the correct number of cells
	     * in the run.
	     */
	    chz->row_num = row_num;
	    chz->cell_num = ecs->actual_at_cell + chz->row_num;
	}
	else
	      chz->second_cell_num = ecs->actual_at_cell + row_num +1;
    }
    if(!itsa_run) {		/* get a new change struct */
	chz = sxm_pop_spair_change();
	chz->typeof_change = wwptr->examine_info->typeof_change;
	chz->col_num = col;
	chz->row_num = row_num;
	/* relative ray number
	 */
	rrn = col/ecs->actual_num_fields;
	tsri = tsrt->tsri_sxm + rrn;
	chz->ray_num = tsri->ray_num;
	chz->sweep_num = tsri->sweep_num;
	chz->field_num =
	      ecs->actual_field_num[col % ecs->actual_num_fields];
	chz->cell_num = ecs->actual_at_cell + chz->row_num;
	chz->window_num = sci->frame;
    }

    dgi = dd_window_dgi(wwptr->lead_sweep->window_num);
    fptr = ecs->data_ptrs[chz->col_num] +chz->row_num;

    nyqv = seds->nyquist_velocity ? seds->nyquist_velocity
	  : dgi->dds->radd->eff_unamb_vel;
    nyq_interval = 2. * nyqv;

    chz->f_old_val = *fptr;

    switch(chz->typeof_change) {

    case EX_MINUS_FOLD:
	*fptr -= nyq_interval;
	chz->f_new_val = *fptr;
	sxm_change_cell_in_list(chz, chz->row_num, *fptr);
	break;
    case EX_PLUS_FOLD:
	*fptr += nyq_interval;
	chz->f_new_val = *fptr;
	sxm_change_cell_in_list(chz, chz->row_num, *fptr);
	break;
    case EX_DELETE:
	*fptr = ecs->bad_data[col];
	chz->flagged_for_deletion = YES;
	chz->f_new_val = *fptr;
	sxm_change_cell_in_list(chz, chz->row_num, *fptr);
	break;
    case EX_REPLACE:
	*fptr = wwptr->examine_info->modify_val;
	chz->f_new_val = *fptr;
	sxm_change_cell_in_list(chz, chz->row_num, *fptr);
	break;
    case EX_REMOVE_AIR_MOTION:
	*fptr += ecs->ac_vel[rrn];
	for(ii=0; FABS(*fptr) > nyqv && ii++ < 11;) {
	    *fptr = *fptr < 0 ? *fptr + nyq_interval : *fptr - nyq_interval;
	}
	chz->f_new_val = *fptr;
	sxm_change_cell_in_list(chz, chz->row_num, *fptr);
	break;

    case EX_RAY_PLUS_FOLD:
    case EX_RAY_MINUS_FOLD:
    case EX_GT_PLUS_FOLD:
    case EX_GT_MINUS_FOLD:
    case EX_RAY_IGNORE:
	if(chz->typeof_change == EX_RAY_IGNORE) {
	    chz->f_new_val = val = ecs->bad_data[chz->col_num];
	    chz->flagged_for_deletion = YES;
	}
	else if(chz->typeof_change == EX_RAY_PLUS_FOLD ||
		chz->typeof_change == EX_GT_PLUS_FOLD)
	      val = nyq_interval;
	else
	      val = -nyq_interval;
	fptr = ecs->data_ptrs[chz->col_num];
	if (chz->typeof_change == EX_GT_PLUS_FOLD) {
	}
	nn = wwptr->examine_control->actual_num_cells;

	if(itsa_run) {		/* we need to but the parts of the ray
				 * that are not in the run back the way
				 * they were */
	    val = -val;
	    jj = chz->row_num;

	    for(ii=0; ii < jj ; ii++, fptr++) {
		if(chz->typeof_change != EX_RAY_IGNORE &&
		   *fptr != ecs->bad_data[chz->col_num])
		      *fptr += val;
		sxm_change_cell_in_list(chz, ii, *fptr);
	    }
	    ii += chz->second_cell_num - chz->cell_num;
	    fptr  += chz->second_cell_num - chz->cell_num;

	    for(; ii < nn; ii++, fptr++) {
		if(chz->typeof_change != EX_RAY_IGNORE &&
		   *fptr != ecs->bad_data[chz->col_num])
		      *fptr += val;
		sxm_change_cell_in_list(chz, ii, *fptr);
	    }
	}
	else {
	   ii = 0;
	   if (chz->typeof_change == EX_GT_PLUS_FOLD ||
	       chz->typeof_change == EX_GT_MINUS_FOLD) {
	      ii = row_num;
	      fptr += row_num;
	   }

	    for(; ii < nn ; ii++, fptr++) {
		if(chz->typeof_change == EX_RAY_IGNORE) {
		    sxm_change_cell_in_list
			  (chz, ii, ecs->bad_data[chz->col_num]);
		}
		else {
		    if(*fptr != ecs->bad_data[chz->col_num])
			  *fptr += val;
		    sxm_change_cell_in_list(chz, ii, *fptr);
		}
	    }
	    chz->f_new_val = val;
	}
	break;			/* ray/run operations */

    default:
	break;
    }

    if(!itsa_run) {    /* push this change onto the change list */
	wwptr->examine_info->change_count++;
	sxm_push_change(chz, &wwptr->changez_list);
    }
    return;
}
/* c------------------------------------------------------------------------ */

void
sxm_close_log_stream()
{
    if(!log_stream)
	  return;
    fclose(log_stream);
    log_stream = NULL;
}
/* c------------------------------------------------------------------------ */

void 
sxm_flush_log_stream()
{
    if(!log_stream){
	printf("sxm_flush_log_stream: log_stream is NULL\n");
	return;
    }
    fflush(log_stream);
}
/* c------------------------------------------------------------------------ */

void 
sxm_set_log_dir (dir)
     char *dir;
{
  char str[2048];
  sprintf (str, "Setting log directory to: %s\n", dir);
  sii_message (str);
  sxm_close_log_stream();
  slash_path(log_directory, dir);
  log_dir_ptr = log_directory;
  log_state = LOG_ACTIVE;
}

/* c------------------------------------------------------------------------ */

void 
sxm_set_tmp_nyq_vel (nyqvel)
     float nyqvel;
{
  struct solo_edit_stuff *seds, *return_sed_stuff();
  seds = return_sed_stuff();
  if (nyqvel >= 0)
    { seds->nyquist_velocity = nyqvel; }
}

/* c------------------------------------------------------------------------ */

void 
sxm_toggle_log_dir (active)
     int active;
{
  char str[2048];
  log_state = (active) ? LOG_ACTIVE : LOG_SUSPENDED;

  sprintf (str, "Toggled log state to: %s!\n"
	   , (active) ? "log active" : "log suspended");
  sii_message (str);
}

/* c------------------------------------------------------------------------ */

void
sxm_gen_all_files_list(frme, slm)
  int frme;
  struct solo_list_mgmt *slm;
{
    int nn, full_file_name=YES;
    WW_PTR wwptr, solo_return_wwptr();

    wwptr = solo_return_wwptr(frme);
    frme = wwptr->lead_sweep->window_num;
    wwptr = solo_return_wwptr(frme);
    ddir_rescan_urgent(frme);

    if((nn = mddir_file_list_v3(frme, wwptr->sweep->directory_name)) < 1 ) {
	g_string_sprintfa (gs_complaints,"Directory %s contains no sweep files\n"
		  , wwptr->sweep->directory_name);
	return;
    }
    mddir_gen_swp_str_list_v3(frme, wwptr->sweep->radar_num
			      , full_file_name, slm);
}
/* c------------------------------------------------------------------------ */

void
sxm_gen_delete_lists(sci)
  struct solo_click_info *sci;
{
    int ii, jj, kk, nn, full_file_name=YES;
    WW_PTR wwptr, wwptrx, solo_return_wwptr();
    struct solo_list_mgmt *slm, *slms;
    struct solo_perusal_info *spi, *solo_return_winfo_ptr();
    char *aa;

    wwptr = solo_return_wwptr(sci->frame);
    wwptr = wwptr->lead_sweep;
    spi = solo_return_winfo_ptr();

    slm = spi->list_all_files;
    slms = spi->list_select_files;
    slms->num_entries = 0;
    /*
     * produce a list of all the files for this radar
     * in the current directory
     */
    sxm_gen_all_files_list(wwptr->window_num, slm);
    /*
     * locate the current file in this list
     */
    for(jj=0; jj < slm->num_entries; jj++) {
	if(!strcmp(*(slm->list +jj), wwptr->sweep->file_name)) {
	    break;
	}
    }
    kk = sci->ival0 >= 0 ? sci->ival0 : 0; /* sweep count comes in ival0 */

    for(; wwptr; wwptr = wwptr->next_sweep) { /* broadcast the sweep count */
	wwptr->sweep->sweep_keep_count = kk;
    }
    /*
     * copy all the previous file name except for a few
     * into the select list
     */
    if( jj < slm->num_entries && jj > kk) {
	jj -= kk;
	solo_reset_list_entries(slms);

	for(ii=0; ii <= jj; ii++) {
	    aa = *(slm->list +ii);
	    solo_add_list_entry(slms, aa, strlen(aa));
	}
    }
}
/* c------------------------------------------------------------------------ */

void
sxm_get_widget_info(frme)
  int frme;
{
    int ii, width, frac;
    char *aa, *bb, *cc, str[2048];
    float f;
    WW_PTR wwptr, solo_return_wwptr();
    struct examine_control *ecs;

    if(!ewi) {
	ewi = (struct examine_widget_info *)
	      malloc(sizeof(struct examine_widget_info));
	memset(ewi, 0, sizeof(struct examine_widget_info));
    }
    /* nab the user modifiable contents of the examine widget
     */
    se_dump_examine_widget(frme, ewi);
    wwptr = solo_return_wwptr(frme);
    ecs = wwptr->examine_control;

    if(ewi->ray_num >= 0)
	  wwptr->examine_info->ray_num = ewi->ray_num;
    if(ewi->at_cell >= 0)
	  wwptr->examine_info->at_cell = ewi->at_cell;
    if(ewi->ray_count > 0)
	  wwptr->examine_info->ray_count = ewi->ray_count;
    if(ewi->cell_count > 0)
	  wwptr->examine_info->cell_count = ewi->cell_count;
    if(ewi->scroll_increment > 0)
	  wwptr->examine_info->scroll_increment = ewi->scroll_increment;

    switch(ewi->typeof_change) {
    case EX_MINUS_FOLD:
    case EX_PLUS_FOLD:
    case EX_DELETE:
    case EX_REPLACE:
	wwptr->examine_info->typeof_change = ewi->typeof_change;
	break;
    default:
	break;
    }
    switch(ewi->row_annotation) {
    case EX_VIEW_RANGES:
    case EX_VIEW_CELL_NUMS:
	wwptr->examine_info->row_annotation = ewi->row_annotation;
	break;
    default:
	break;
    }
    switch(ewi->col_annotation) {
    case EX_VIEW_ROT_ANGS:
    case EX_VIEW_RAY_NUMS:
	wwptr->examine_info->col_annotation = ewi->col_annotation;
	break;
    default:
	break;
    }
    wwptr->examine_info->row_annotation =
      (ewi->row_annotation == EX_VIEW_CELL_NUMS) ? YES : NO;
    wwptr->examine_info->col_annotation =
      (ewi->col_annotation == EX_VIEW_RAY_NUMS) ? YES : NO;

    strcpy(wwptr->examine_info->fields_list, ewi->fields_list);
    if(sscanf(ewi->modify_val, "%f", &f) == 1) {
	wwptr->examine_info->modify_val = f;
    }
    /*
     * now do the display format
     */
    aa = bb = ewi->display_format;
    if(strlen(aa)) {
	for(; *aa && !isdigit(*aa); aa++);
	if((ii = sscanf(aa, "%d.%d", &width, &frac)) == 2) {
	    if(frac >= 0 && width > frac +2) {
		sprintf(wwptr->examine_info->display_format
			, "%d.%df", width, frac);
	    }
	}
    }
    return;
}
/* c------------------------------------------------------------------------ */

void
sxm_list_beams(frme)
  int frme;
{
    int nn;
    struct solo_list_mgmt *slm;
    WW_PTR wwptr, solo_return_wwptr();
    struct dd_general_info *dgi, *dd_window_dgi();
    int file_action=TIME_NEAREST, version=LATEST_VERSION;
    int sweep_skip=1, replot=YES, nr;
    char *aa, str[2048];

    wwptr = solo_return_wwptr(frme);
    dgi = dd_window_dgi(wwptr->lead_sweep->window_num);
    slm = wwptr->examine_list;
    solo_reset_list_entries(slm);


    if(wwptr->lead_sweep->sweep_file_modified) {
	/* if it's been edited, reopen the file
	 */
	solo_nab_next_file(wwptr->lead_sweep->window_num
			   , file_action, version, sweep_skip, replot);
	wwptr->lead_sweep->sweep_file_modified = NO;
    }
    dgi_buf_rewind(dgi);
    dd_absorb_header_info(dgi);
    nr = dgi->dds->swib->num_rays;

    for(; nr-- ;) {
	if((nn=dd_absorb_ray_info(dgi)) < 1 ) {
	    break;
	}
	if(dgi->source_ray_num >= dgi->dds->swib->num_rays) {
	    /*
	    break;
	     */
	}
	sxm_stat_line(dgi, NO, "", "", str);
	solo_add_list_entry(slm, str, strlen(str));
    }
    se_refresh_examine_widgets(frme, slm);
}
/* c------------------------------------------------------------------------ */

void
sxm_list_descriptors(frme)
  int frme;
{
    int pn;
    struct solo_list_mgmt *slm;
    WW_PTR wwptr, solo_return_wwptr();
    struct dd_general_info *dgi, *dd_window_dgi();

    wwptr = solo_return_wwptr(frme);
    dgi = dd_window_dgi(wwptr->lead_sweep->window_num);
    slm = wwptr->examine_list;
    solo_reset_list_entries(slm);

    dor_print_vold(dgi->dds->vold, slm);
    dor_print_radd(dgi->dds->radd, slm);
    for(pn=0; pn < dgi->num_parms; pn++) {
	dor_print_parm(dgi->dds->parm[pn], slm);
    }
    if (dgi->dds->frib)
      { dor_print_frib(dgi->dds->frib, slm); }
    dor_print_cfac(dgi->dds->cfac, slm);
    dor_print_celv(dgi->dds->celvc, slm);
    dor_print_swib(dgi->dds->swib, slm);
    dor_print_ryib(dgi->dds->ryib, slm);
    dor_print_asib(dgi->dds->asib, slm);
    dor_print_sswb(dgi->dds->sswb, slm);
    dor_print_rktb((struct rot_ang_table *)dgi->source_rat, slm);
    se_refresh_examine_widgets(frme, slm);
}
/* c------------------------------------------------------------------------ */

void
sxm_list_edit_hist(frme)
  int frme;
{
    int ii, nn;
    struct solo_list_mgmt *slm;
    WW_PTR wwptr, solo_return_wwptr();
    struct dd_general_info *dgi, *dd_window_dgi();
    char *aa, *bb, *cc, *zz, str[2048];

    wwptr = solo_return_wwptr(frme);
    dgi = dd_window_dgi(wwptr->lead_sweep->window_num);
    slm = wwptr->examine_list;
    solo_reset_list_entries(slm);

    if(!dgi->sizeof_seds || !dgi->seds) {
	cc = " ";
	solo_add_list_entry(slm, cc, strlen(cc));
	cc = "No edit history for this sweep";
	solo_add_list_entry(slm, cc, strlen(cc));
	cc = " ";
	solo_add_list_entry(slm, cc, strlen(cc));
	return;
    }
    aa = bb = dgi->seds;
    zz = aa + dgi->sizeof_seds;

    for(; bb < zz; bb++) {	/* find the next line feed */
	cc = str;
	for(aa=bb; bb < zz && *bb != '\n'; *cc++ = *bb++); *cc++ = '\0';
	if(nn = strlen(str))
	      solo_add_list_entry(slm, str, nn);
	if(bb >= zz) break;
    }
    se_refresh_examine_widgets(frme, slm);
}
/* c------------------------------------------------------------------------ */

void
sxm_list_to_log(slm, entry, num)
  struct solo_list_mgmt *slm;
  int entry, num;
{
    int nn = entry + num;

    if(log_state != LOG_ACTIVE)
	  return;

    sxm_open_log_stream();

    if(!log_stream)
	  return;
    
    if(!slm || entry < 0 || entry >= slm->num_entries)
	  return;

    if(nn > slm->num_entries)
	  nn = slm->num_entries;
		
    for(; entry < nn; entry++) {
	fprintf(log_stream, "%s\n", *(slm->list +entry));
    }
}
/* c------------------------------------------------------------------------ */

void
sxm_log_stat_line(dgi)
     struct dd_general_info *dgi;
{
    char str[2048];

    if(log_state != LOG_ACTIVE)
	  return;

    sxm_open_log_stream();

    sxm_stat_line(dgi, YES, "", "", str);

    fprintf(log_stream, "%s", str);
}
/* c------------------------------------------------------------------------ */

void
sxm_open_log_stream()
{
    char str[2048], *aa, *getenv();
    long time_now();

    if(log_stream)
	  return;

    if(!log_dir_ptr) {
	if(aa = getenv("LOG_DIRECTORY")) {
	    strcpy(log_directory, aa);
	}
	else {
	    strcpy(log_directory, "./");
	}
	log_dir_ptr = log_directory;
    }
    strcpy(str, log_dir_ptr);
    aa = str + strlen(str);
    dd_file_name("log", time_now(), "SED_LOG", 0, aa);
    
    g_string_sprintfa (gs_complaints,"Opening display log file: %s\n", str);
    if(!(log_stream = fopen(str, "w"))) {
	g_string_sprintfa (gs_complaints,"**** Unable to open this file! ****\n");
	return;
    }
    strcpy(log_file_name, aa);
}
/* c------------------------------------------------------------------------ */

struct se_changez *
sxm_pop_change(topptr)
  struct se_changez **topptr;
{
    /* pops a change struct off a stack
     */
    struct se_changez *top=(*topptr);

    if(!top)
	  return(NULL);

    if(top->next) {
	top->next->last = top->last;
    }
    *topptr = top->next;
    return(top);
}
/* c------------------------------------------------------------------------ */

struct se_changez *
sxm_pop_spair_change()
{
    /* pops a changez struct off the spairs stack
     */
    struct se_changez *chz=sxm_spair_changez;

    if(!chz) {
	chz = (struct se_changez *)malloc(sizeof(struct se_changez));
	memset(chz, 0, sizeof(struct se_changez));
    }
    else {
	sxm_spair_changez = chz->next; 
	memset(chz, 0, sizeof(struct se_changez));
    }
    return(chz);
}
/* c------------------------------------------------------------------------ */

void
sxm_print_list(slm)
  struct solo_list_mgmt *slm;
{
    int ii=0;

    for(; ii < slm->num_entries; ii++) {
	g_string_sprintfa (gs_complaints,"%s\n", *(slm->list +ii));
    }
}
/* c------------------------------------------------------------------------ */

void
sxm_process_click(sci)
  struct solo_click_info *sci;
{
    int ii, jj, kk, nd, nn, mark, full_file_name=YES;
    int sp_max_frames();
    WW_PTR wwptr, solo_return_wwptr();
    struct solo_list_mgmt *slm, *slms, *slmx;
    struct solo_perusal_info *spi, *solo_return_winfo_ptr();
    char *aa, *bb, line[256], name[256], *keep;
    FILE *stream;

    if(solo_busy())
	  return;

    if(!sci) {
	g_string_sprintfa (gs_complaints,"sxm_process_click...struct solo_click_info *sci=NULL\n");
	return;
    }
    else if(sci->frame < 0 || sci->frame > sp_max_frames()) {
	g_string_sprintfa (gs_complaints,"sxm_process_click...illegal frame number: %d\n"
		  , sci->frame);
	return;
    }
    solo_set_busy_signal();
    wwptr = solo_return_wwptr(sci->frame);
    slm = wwptr->examine_list;
    spi = solo_return_winfo_ptr();

    switch(sci->which_widget_button) {

    case EX_RADAR_DATA:
    case EX_BEAM_INVENTORY:
    case EX_EDIT_HIST:
    case EX_DESCRIPTORS:
	wwptr->examine_info->whats_selected = sci->which_widget_button;
    case EX_REFRESH_LIST:
	sxm_refresh_list(sci);
	sxm_list_to_log(slm, 0, slm->num_entries);
	break;

    case EX_SCROLL_LEFT:
    case EX_SCROLL_RIGHT:
    case EX_SCROLL_UP:
    case EX_SCROLL_DOWN:
	sxm_scroll_list(sci);
	sxm_list_to_log(slm, 0, slm->num_entries);
	break;

    case EX_TOGGLE_ANNOTATION:
	wwptr->examine_info->row_annotation =
	      wwptr->examine_info->row_annotation ? NO : YES;
	wwptr->examine_info->col_annotation =
	      wwptr->examine_info->col_annotation ? NO : YES;
	sxm_update_examine_data(sci->frame, 0L);
	sxm_list_to_log(slm, 0, slm->num_entries);
	break;

    case EX_MINUS_FOLD:
    case EX_PLUS_FOLD:
    case EX_DELETE:
    case EX_REPLACE:
    case EX_REMOVE_AIR_MOTION:
    case EX_RAY_IGNORE:
    case EX_RAY_PLUS_FOLD:
    case EX_RAY_MINUS_FOLD:
    case EX_GT_PLUS_FOLD:
    case EX_GT_MINUS_FOLD:
	sxm_get_widget_info(sci->frame);
	wwptr->examine_info->typeof_change = sci->which_widget_button;
	break;

    case EX_UNDO:
	sxm_undo_last(sci->frame);
	sxm_list_to_log(slm, 0, slm->num_entries);
	se_refresh_examine_widgets(sci->frame, slm);
	break;

    case EX_CLEAR_CHANGES:
	sxm_clear_changes(sci->frame);
	sxm_list_to_log(slm, 0, slm->num_entries);
	se_refresh_examine_widgets(sci->frame, slm);
	break;

    case EX_CLICK_IN_LIST:
	sxm_click_in_list(sci);
	sxm_list_to_log(slm, 0, slm->num_entries);
	se_refresh_examine_widgets(sci->frame, slm);
	break;

    case EX_CLICK_IN_DATA:
	sxm_click_in_data(sci);
	sxm_list_to_log(slm, 0, slm->num_entries);
	break;

    case EX_COMMIT:
	sxm_apply_changez(sci->frame);
	sxm_update_examine_data (sci->frame, 0);
	se_refresh_examine_widgets(sci->frame, slm);
	break;





    case OK_TO_DELETE:
	/* delete files in the select list and regenerate the all files list
	 */
	slms = spi->list_select_files;
	sxm_unlink_files_list(slms
			      , wwptr->lead_sweep->sweep->directory_name
			      , wwptr->lead_sweep->window_num);
	solo_reset_list_entries(slms);
	sp_refresh_delete_sweeps();
	break;

    case ADD_TO_PRESERVATION_LIST:
	/* clicked from the sweep widget to add the current sweep file
	 * to the preservation list
	 */
	slms = spi->list_select_files;
	solo_reset_list_entries(slms);
	aa = wwptr->lead_sweep->sweep->file_name;
	solo_add_list_entry(slms, aa, strlen(aa));

    case MARK_FOR_PRESERVATION:
	/* add file names in the select list to the preservation list file
	 * first append existing preserved file names to the list,
	 * then sort it, andxh then write it back out again
	 */
	slms = spi->list_select_files;
	if(slms->num_entries) {
	    slash_path(name, wwptr->lead_sweep->sweep->directory_name);
	    strcat(name, preservation_file_name);
	    if(stream = fopen(name, "r")) {
		for(; aa = fstring(line, 88, stream);) {
		    solo_add_list_entry(slms, aa, strlen(aa));
		}
		fclose(stream);
		solo_list_sort_file_names(slms);
		solo_list_remove_dups(slms);
	    }
	    if(stream = fopen(name, "w")) {
		for(ii=0; ii < slms->num_entries; ii++) {
		    fprintf(stream, "%s\n", *(slms->list +ii));
		}
		fclose(stream);
	    }
	}
	break;

    case LIST_ALL_SWEEP_FILES:
	/* generate a list of all the sweep files for this radar in
	 * this directory
	 * if the save count is > 0 put the files to be delete in
	 * the select list
	 */
	sxm_gen_delete_lists(sci);
	sp_refresh_delete_sweeps();
	break;

    case CLEAR_SELECT_LIST:
	/* clears out the select list
	 */
	solo_reset_list_entries(spi->list_select_files);
	sp_refresh_delete_sweeps();
	break;

    case LIST_PRESERVED_FILES:
	slms = spi->list_select_files;
	solo_reset_list_entries(slms);

	slash_path(name, wwptr->lead_sweep->sweep->directory_name);
	strcat(name, preservation_file_name);
	if(stream = fopen(name, "r")) {
	    for(; aa = fstring(line, 88, stream);) {
		solo_add_list_entry(slms, aa, strlen(aa));
	    }
	    fclose(stream);
	}
	sp_refresh_delete_sweeps();
	break;

    case REMOVE_PRESERVATION:
	/* removes the files in the select list from the preservation list file
	 */
	slms = spi->list_select_files;
	slmx = spi->list_text;
	solo_reset_list_entries(slmx);

	slash_path(name, wwptr->lead_sweep->sweep->directory_name);
	strcat(name, preservation_file_name);
	if(stream = fopen(name, "r")) {
	    for(; aa = fstring(line, 88, stream);) {
		for(ii=0; ii < slms->num_entries; ii++) {
		    if(!strcmp(aa, *(slms->list +ii))) {
			break; /* ignore this file name */
		    }
		}
		if(ii == slms->num_entries) {
		    solo_add_list_entry(slmx, aa, strlen(aa));
		}
	    }
	    fclose(stream);
	    if(stream = fopen(name, "w")) {
		for(ii=0; ii < slmx->num_entries; ii++) {
		    fprintf(stream, "%s\n", *(slmx->list +ii));
		}
		fclose(stream);
	    }
	}
	else {
	    g_string_sprintfa (gs_complaints,"Unable to open preservation file in dir: %s\n"
		      , wwptr->lead_sweep->sweep->directory_name);
	}
	sp_refresh_delete_sweeps();
	break;

    case NOT_SELECTED_FILES:
	/* remove all files that are not in the select list
	 */
	slm = spi->list_all_files;
	slms = spi->list_select_files;
	slmx = spi->list_text;
	solo_reset_list_entries(slmx);
	/*
	 * copy select files into the scratch list
	 */
	for(ii=0; ii < slms->num_entries; ii++) {
	    aa = *(slms->list +ii);
	    solo_add_list_entry(slmx, aa, strlen(aa));
	}
	solo_reset_list_entries(slms);
	/*
	 * now rebuild the select list from the all files list
	 * omitting entries that were in the select list and are now
	 * in the scratch list
	 */
	for(jj=0; jj < slm->num_entries; jj++) {
	    for(ii=0; ii < slmx->num_entries; ii++) {
		if(!strcmp(*(slm->list +jj), *(slmx->list +ii))) {
		    break;	/* found */
		}
	    }
	    if(ii == slmx->num_entries) {
		aa = *(slm->list +jj);
		solo_add_list_entry(slms, aa, strlen(aa));
	    }
	}
	sxm_unlink_files_list(slms, wwptr->lead_sweep->sweep->directory_name
			      , wwptr->lead_sweep->window_num);
	solo_reset_list_entries(slms);
	sp_refresh_delete_sweeps();
	break;

    case CLICKED_IN_ALL_FILES_LIST:
	/* copy file name(s) into the select list
	 */
	ii = sci->which_list_entry;
	jj = sci->second_list_entry > ii ? sci->second_list_entry : ii;
	slm = spi->list_all_files;
	slms = spi->list_select_files;

	for(; ii <= jj; ii++) {
	    aa = *(slm->list +ii);
	    solo_add_list_entry(slms, aa, strlen(aa));
	}
	solo_list_sort_file_names(slms);
	solo_list_remove_dups(slms);
	sp_refresh_delete_sweeps();
	break;

    case CLICKED_IN_SELECT_LIST:
	/* just remove file name(s) from the select list
	 */
	slms = spi->list_select_files;
	solo_list_remove_entry(slms, sci->which_list_entry
			       , sci->second_list_entry);
	sp_refresh_delete_sweeps();
	break;

    case PETERS_QUICKIE_DELETE:
	/* find the current file in the all files list,
	 * back up "wwptr->sweep->keep_sweep_count"
	 * files and delete everything before that point
	 */
	sxm_gen_delete_lists(sci);
	sxm_unlink_files_list(spi->list_select_files
			      , wwptr->lead_sweep->sweep->directory_name
			      , wwptr->lead_sweep->window_num);
	break;

    default:
	g_string_sprintfa (gs_complaints,"sxm_process_click...unused case no: %d\n"
		  , sci->which_widget_button);
	break;
    }
    solo_clear_busy_signal();
    return;
}
/* c------------------------------------------------------------------------ */

void
sxm_push_change(chz, topptr)
  struct se_changez *chz, **topptr;
{
    struct se_changez *top=(*topptr);

    if(!chz)
	  return;

    if(!top) {
	chz->last = chz;
	chz->next = NULL;
    }
    else {
	chz->next = top;
	chz->last = top->last;
	top->last = chz;
    }
    *topptr = chz;
    return;
}
/* c------------------------------------------------------------------------ */

void
sxm_push_spair_change(chz)
  struct se_changez *chz;
{
    /* pushes an individual struct on to the spairs stack
     */
    if(!chz)
	  return;
    chz->next = sxm_spair_changez;
    sxm_spair_changez = chz;
    return;
}
/* c------------------------------------------------------------------------ */

void
sxm_refresh_list(sci)
  struct solo_click_info *sci;
{
    WW_PTR wwptr, solo_return_wwptr();

    wwptr = solo_return_wwptr(sci->frame);

    switch(wwptr->examine_info->whats_selected) {

    case EX_RADAR_DATA:
	sxm_get_widget_info(sci->frame);
	sxm_update_examine_data(sci->frame, 0L);
	break;
    case EX_BEAM_INVENTORY:
	sxm_list_beams(sci->frame);
	break;
    case EX_EDIT_HIST:
	sxm_list_edit_hist(sci->frame);
	break;
    case EX_DESCRIPTORS:
	sxm_list_descriptors(sci->frame);	
	break;
    default:
	break;
    }
}
/* c------------------------------------------------------------------------ */

struct se_changez *
sxm_remove_change(topptr, this)
  struct se_changez **topptr;
  struct se_changez *this;
{
    /* pops a change struct off a stack
     */
    struct se_changez *top=(*topptr);

    if(!top || !this)
	  return(NULL);

    if(this == top) {
	if(top->next) {
	    top->next->last = top->last;
	}
	*topptr = top->next;
    }
    else {
	this->last->next = this->next;
	if(this->next) {
	    this->next->last = this->last;
	}
	else { /* at the end */
	    top->last = this->last;
	}
    }
    return(this);
}
/* c------------------------------------------------------------------------ */

void
sxm_scroll_list(sci)
  struct solo_click_info *sci;
{
    int wig_but, frme;
    struct examine_control *ecs;
    WW_PTR wwptr, solo_return_wwptr();
    struct solo_examine_info *sei;
    
    wwptr = solo_return_wwptr(sci->frame);
    ecs = wwptr->examine_control;
    sei = wwptr->examine_info;
    sxm_get_widget_info(sci->frame);

    switch(wwptr->examine_info->whats_selected) {

    case EX_RADAR_DATA:
	switch(sci->which_widget_button) {
	case EX_SCROLL_LEFT:
# ifdef obsolete
	    sei->ray_num--;
# endif
	    break;
	case EX_SCROLL_RIGHT:
# ifdef obsolete
	    sei->ray_num++;
# endif
	    break;
	case EX_SCROLL_UP:
	    sei->at_cell -= sei->scroll_increment;
	    break;
	case EX_SCROLL_DOWN:
	    sei->at_cell += sei->scroll_increment;
	    break;
	default:
	    break;
	}
	sxm_update_examine_data(sci->frame, sci->which_widget_button);
	break;
    case EX_BEAM_INVENTORY:

	break;
    case EX_EDIT_HIST:

	break;
    case EX_DESCRIPTORS:

	break;
    default:
	break;
    }
}
/* c------------------------------------------------------------------------ */

void 
sxm_set_click_frame(frme, theta, range)
  int frme;
  float theta, range;
{
    WW_PTR wwptr, solo_return_wwptr();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct solo_list_mgmt *slm;
    char *aa, str[2048];

    if(frme >= 0 && frme < SOLO_MAX_WINDOWS) {
	seds = return_sed_stuff();
	aa = str;
	seds->focus_frame = 
	      focus_frame = click_frame = frme;
	wwptr = solo_return_wwptr(frme);
	sprintf(aa,
		"Click at angle, range, and frame | %7.2f | %8.3f | %d |\n"
		, theta, range, frme);
	sxm_append_to_log_stream(aa, strlen(aa));
    }
}
/* c------------------------------------------------------------------------ */

void
sxm_stack_spair_change(topptr)
  struct se_changez **topptr;
{
    /* pushes a stack of changes onto the spairs stack
     */
    struct se_changez *chz=(*topptr);

    if(!chz)
	  return;
    chz->last->next = sxm_spair_changez;
    sxm_spair_changez = chz;
    *topptr = NULL;
    return;
}
/* c------------------------------------------------------------------------ */

void
sxm_stat_line(dgi, verbosity, preamble, postamble, line)
  struct dd_general_info *dgi;
  int verbosity;
  char *preamble, *postamble, *line;
{
    /* 
     * passing in the swib is necessary because there is more than
     * on around during DORADE input and this routine is used by 
     * other front ends such as UF output from sweep files.
     */
    int ii, jj, nn;
    char str[2048], *a, *dts_print(), *str_terminate(), radar_name[12];
    DD_TIME *d_unstamp_time();
    struct dds_structs *dds=dgi->dds;
    struct sweepinfo_d *swib=dgi->dds->swib;
    struct platform_i *asib=dgi->dds->asib;
    struct ray_i *ryib=dgi->dds->ryib;
    char *aa=line, *bb, *cc;
    double dd_elevation_angle(), dd_azimuth_angle(), dd_roll(), dd_pitch();
    double dd_heading();

    *aa = '\0';

    sprintf(aa+strlen(aa), "%s ", preamble);
    sprintf(aa+strlen(aa), "%s ", dts_print(d_unstamp_time(dds->dts)));
    strcpy(radar_name, str_terminate(str, dds->radd->radar_name, 8));
    sprintf(aa+strlen(aa), "%s ", radar_name);
    aa += strlen(aa);


# ifdef obsolete
    if(dds->radd->scan_mode == AIR || dds->radd->radar_type == AIR_LF ||
       dds->radd->radar_type == AIR_NOSE
	) {
	sprintf(aa+strlen(aa), "az:%6.1f ", dd_azimuth_angle(dgi));
	sprintf(aa+strlen(aa), "el:%6.2f ", dd_elevation_angle(dgi));
	sprintf(aa+strlen(aa), "h:%6.1f ", dd_heading(dgi));
	sprintf(aa+strlen(aa), "rl:%5.1f ", dd_roll(dgi));
	sprintf(aa+strlen(aa), "al:%6.3f ", dd_altitude(dgi));
    }    
# endif
    if(dds->radd->scan_mode == AIR) {
	sprintf(aa+strlen(aa), "rt:%6.1f "
		, FMOD360(dd_rotation_angle(dgi)));
	sprintf(aa+strlen(aa), "t:%5.1f ", dd_tilt_angle (dgi));
	sprintf(aa+strlen(aa), "rl:%5.1f ", asib->roll);
	sprintf(aa+strlen(aa), "h:%6.1f ", asib->heading);
	sprintf(aa+strlen(aa), "al:%6.3f ", asib->altitude_msl);
    }
    else if(dds->radd->radar_type == AIR_LF) {
	sprintf(aa+strlen(aa), "az:%6.1f ", dd_azimuth_angle(dgi));
	sprintf(aa+strlen(aa), "el:%6.2f ", dd_elevation_angle(dgi));
	sprintf(aa+strlen(aa), "h:%6.1f ", dd_heading(dgi));
	sprintf(aa+strlen(aa), "rl:%5.1f ", dd_roll(dgi));
	sprintf(aa+strlen(aa), "p:%5.1f ", dd_pitch(dgi));
    }
    else {
	sprintf(aa+strlen(aa), "fx:%6.1f ", swib->fixed_angle);
	sprintf(aa+strlen(aa), "az:%6.1f ", dd_azimuth_angle(dgi));
	sprintf(aa+strlen(aa), "el:%6.2f ", dd_elevation_angle(dgi));
    }
    aa += strlen(aa);
    sprintf(aa+strlen(aa), "swp: %2d ", ryib->sweep_num);
    sprintf(aa+strlen(aa), "%s ", postamble);
    if(verbosity) {
	sprintf(aa+strlen(aa), "\n");
    }   

    if(verbosity) {
	sprintf(aa+strlen(aa), "la:%9.4f ", asib->latitude);
	sprintf(aa+strlen(aa), "lo:%9.4f ", asib->longitude);
	if(dds->radd->scan_mode == AIR) {
	    sprintf(aa+strlen(aa), "p:%5.1f ", asib->pitch);
	    sprintf(aa+strlen(aa), "d:%5.1f ", asib->drift_angle);
	    sprintf(aa+strlen(aa), "ag:%6.3f ", asib->altitude_agl);
	    sprintf(aa+strlen(aa), "\n");
	    sprintf(aa+strlen(aa), "ve:%6.1f ", asib->ew_velocity);
	    sprintf(aa+strlen(aa), "vn:%6.1f ", asib->ns_velocity);
	    sprintf(aa+strlen(aa), "vv:%6.1f ", asib->vert_velocity);
	    sprintf(aa+strlen(aa), "we:%6.1f ", asib->ew_horiz_wind);
	    sprintf(aa+strlen(aa), "wn:%6.1f ", asib->ns_horiz_wind);
	    sprintf(aa+strlen(aa), "wv:%6.1f ", asib->vert_wind);
	}
	else {
	    sprintf(aa+strlen(aa), "al:%6.3f ", asib->altitude_msl);
	}
	sprintf(aa+strlen(aa), "\n");
	sprintf(aa+strlen(aa), "Num parameters: %d  ", dgi->source_num_parms);
	for(ii=0; ii < dgi->source_num_parms; ii++) {
	    sprintf(aa+strlen(aa), "%s ", str_terminate
		   (str, dds->parm[ii]->parameter_name, 8));
	}
	sprintf(aa+strlen(aa), "\n");
    }
}
/* c------------------------------------------------------------------------ */

void 
sxm_ui_cmd(arg, cmds, key)
  int arg;
  struct ui_command *cmds;	
  int key;
{
    int ii, jj, ok=YES, col, ray, cell, field, width, frac, mark;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    WW_PTR wwptr, solo_return_wwptr();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct solo_perusal_info *spi, *solo_return_winfo_ptr();
    struct examine_control *ecs;
    struct solo_examine_info *sei;
    struct solo_list_mgmt *slm;
    char *aa, *bb, tmp_str[2048], *strstr(), *se_unquote_string();


    last_click.which_widget_button = key;
    last_click.frame = focus_frame;
    wwptr = solo_return_wwptr(focus_frame);
    sei = wwptr->examine_info;
    ecs = wwptr->examine_control;
    slm = wwptr->examine_list;
    spi = solo_return_winfo_ptr();
    seds = return_sed_stuff();


    switch(key) {
    case EX_RADAR_DATA:
    case EX_BEAM_INVENTORY:
    case EX_EDIT_HIST:
    case EX_DESCRIPTORS:
	sxm_process_click(&last_click);
	sxm_print_list(wwptr->examine_list);
	break;

    case EX_SCROLL_LEFT:
    case EX_SCROLL_RIGHT:
    case EX_SCROLL_UP:
    case EX_SCROLL_DOWN:
	if(cmdq->uc_ctype != UTT_END && (ii = cmdq->uc_v.us_v_int)) {
	    switch(key) {
	    case EX_SCROLL_LEFT:
		sei->ray_num -= ii;
		break;
	    case EX_SCROLL_RIGHT:
		sei->ray_num += ii;
		break;
	    case EX_SCROLL_UP:
		sei->at_cell -= ii;
		break;
	    case EX_SCROLL_DOWN:
		sei->at_cell += ii;
		break;
	    }
	    sxm_refresh_list(&last_click);
	}
	else {
	    sxm_process_click(&last_click);
	}
	sxm_print_list(wwptr->examine_list);
	break;

    case EX_MINUS_FOLD:
    case EX_PLUS_FOLD:
    case EX_DELETE:
    case EX_REPLACE:
    case EX_CLEAR_CHANGES:
    case EX_COMMIT:
	sxm_process_click(&last_click);
	break;

    case EX_UNDO:
	sxm_process_click(&last_click);
	sxm_print_list(wwptr->examine_list);
	break;
    case EX_CLICK_IN_LIST:
	/* need to fake row and character number */
	ray = (cmdq++)->uc_v.us_v_int -1;
	if(ray < ecs->actual_ray_num || ray
	   >= ecs->actual_ray_num + ecs->actual_ray_count) {
	    ok = NO;
	    g_string_sprintfa (gs_complaints,"Ray: %d  out of range of display: %d - %d\n"
		      , ray+1, ecs->actual_ray_num +1
		      , ecs->actual_ray_num + ecs->actual_ray_count);
	}
	cell = (cmdq++)->uc_v.us_v_int -1;
	if(cell < ecs->actual_at_cell || cell
	   >= ecs->actual_at_cell + ecs->actual_num_cells) {
	    ok = NO;
	    g_string_sprintfa (gs_complaints,"Cell: %d  out of range of display: %d - %d\n"
		      , cell+1, ecs->actual_at_cell +1
		      , ecs->actual_at_cell + ecs->actual_num_cells);
	}
	field = (cmdq++)->uc_v.us_v_int -1;
	if(field < 0 || field >= ecs->actual_num_fields) {
	    ok = NO;
	    g_string_sprintfa (gs_complaints,"Field: %d out of range of display: %d - %d\n"
		      , field+1, 1, ecs->actual_num_fields);
	}
	if(ok) {
	    ray -= ecs->actual_ray_num;
	    cell -= ecs->actual_at_cell;
	    col = ecs->non_data_col_count
		  + ray * ecs->actual_num_fields +field;
	    last_click.which_character = ecs->col_lims[col] -1;
	    last_click.which_list_entry = cell + ecs->heading_row_count;
	    sxm_process_click(&last_click);
	}
	sxm_print_list(wwptr->examine_list);
	break;

    case EX_CLICK_IN_DATA:
	ecs->click_angle = wwptr->clicked_angle;
	ecs->click_range = wwptr->clicked_range * 1000.;
	ecs->ctr_on_click = YES;
	last_click.which_widget_button = EX_RADAR_DATA;
	sxm_update_examine_data(focus_frame, 0L);
	sxm_print_list(wwptr->examine_list);
	sxm_list_to_log(slm, 0, slm->num_entries);
	break;

    case EX_TOGGLE_ANNOTATION:
# ifdef obsolete
	wwptr->examine_info->row_annotation =
	      wwptr->examine_info->row_annotation ? NO : YES;
	wwptr->examine_info->col_annotation =
	      wwptr->examine_info->col_annotation ? NO : YES;
	last_click.which_widget_button = EX_RADAR_DATA;
	sxm_update_examine_data(focus_frame, 0L);
	sxm_print_list(wwptr->examine_list);
	sxm_list_to_log(slm, 0, slm->num_entries);
# else
	sxm_process_click(&last_click);
	sxm_print_list(wwptr->examine_list);
# endif
	break;

    case EX_CHANGE_LOCATION:
	ray = (cmdq++)->uc_v.us_v_int -1;
	if(ray >= 0)
	      wwptr->examine_info->ray_num = ray;
	cell = (cmdq++)->uc_v.us_v_int -1;
	if(cell >= 0)
	      wwptr->examine_info->at_cell = cell;
	last_click.which_widget_button = EX_RADAR_DATA;
	sxm_update_examine_data(focus_frame, 0L);
	sxm_print_list(wwptr->examine_list);
	sxm_list_to_log(slm, 0, slm->num_entries);
	break;

    case EX_WHICH_FIELDS:
	tmp_str[0] = '\0';
	for(; cmdq->uc_ctype != UTT_END;) {
	    aa = (cmdq++)->uc_text;
	    if(strlen(aa)) {
		strcat(tmp_str, aa);
		strcat(tmp_str, " ");
	    }
	}
	if(strlen(tmp_str))
	      strcpy(wwptr->examine_info->fields_list, tmp_str);
	last_click.which_widget_button = EX_RADAR_DATA;
	sxm_update_examine_data(focus_frame, 0L);
	sxm_print_list(wwptr->examine_list);
	sxm_list_to_log(slm, 0, slm->num_entries);
	break;

    case EX_RAY_COUNT:
	ii = (cmdq++)->uc_v.us_v_int;
	if(ii > 0)
	      wwptr->examine_info->ray_count = ii;
	sxm_update_examine_data(focus_frame, 0L);
	sxm_print_list(wwptr->examine_list);
	sxm_list_to_log(slm, 0, slm->num_entries);
	break;

    case EX_CELL_COUNT:
	ii = (cmdq++)->uc_v.us_v_int;
	if(ii > 0)
	      wwptr->examine_info->cell_count = ii;
	sxm_update_examine_data(focus_frame, 0L);
	sxm_print_list(wwptr->examine_list);
	sxm_list_to_log(slm, 0, slm->num_entries);
	break;

    case EX_SCROLL_INC:
	ii = (cmdq++)->uc_v.us_v_int;
	if(ii > 0)
	      wwptr->examine_info->scroll_increment = ii;
	break;

    case EX_DISPLAY_FMT:
	aa = (cmdq++)->uc_text;
	for(; *aa && !isdigit(*aa); aa++);
	if(*aa && (ii = sscanf(aa, "%d.%d", &width, &frac)) == 2) {
	    if(frac >= 0 && width > frac +2) {
		sprintf(wwptr->examine_info->display_format
			, "%d.%df", width, frac);
	    }
	}
	sxm_update_examine_data(focus_frame, 0L);
	sxm_print_list(wwptr->examine_list);
	sxm_list_to_log(slm, 0, slm->num_entries);
	break;

    case EX_FRAME_NUM:
	ii = (cmdq++)->uc_v.us_v_int -1;
	if(0 <= ii && ii < SOLO_MAX_WINDOWS) {
	    seds->focus_frame = focus_frame = ii;
	}
	sxm_update_examine_data(focus_frame, 0L);
	sxm_print_list(wwptr->examine_list);
	sxm_list_to_log(slm, 0, slm->num_entries);
	break;

    case EX_DISPLAY_LOG:
	if(strncmp(cmdq->uc_text, "close", 2) == 0) {
	    sxm_close_log_stream();
	}
	else if(strncmp(cmdq->uc_text, "flush", 2) == 0) {
	    sxm_flush_log_stream();
	}
	else if(strncmp(cmdq->uc_text, "directory", 2) == 0) {
	    if((++cmdq)->uc_ctype != UTT_END) {
		if(strlen(aa = cmdq->uc_text)) {
		    if(strchr(aa, '/')) {
			g_string_sprintfa (gs_complaints,"Setting log directory to: %s\n", aa);
			slash_path(log_directory, se_unquote_string(aa));
			log_dir_ptr = log_directory;
			log_state = LOG_ACTIVE;
		    }
		}
	    }
	}
	else {			/* toggle logging */
	    log_state = log_state == LOG_ACTIVE ? LOG_SUSPENDED : LOG_ACTIVE;
	    aa = log_state == LOG_ACTIVE ? "log active" : "log suspended";
	    g_string_sprintfa (gs_complaints,"Toggled log state to: %s!\n", aa);
	}
	mark = 0;
	break;

    case XX_ZAP_FILES:
	last_click.which_widget_button = PETERS_QUICKIE_DELETE;
	last_click.ival0 = wwptr->sweep->sweep_keep_count;
	if(cmdq->uc_ctype != UTT_END) {
	    ii = -1;
	    if((jj = sscanf(cmdq->uc_text, "%d", &ii)) == 1) {
		if(ii >= 0)
		     last_click.ival0 = ii; 
	    }
	}
	sxm_process_click(&last_click);
	break;
    case XX_LIST_SELECTED_FILES:
	last_click.which_widget_button = LIST_ALL_SWEEP_FILES;
	last_click.ival0 = wwptr->sweep->sweep_keep_count;
	if(cmdq->uc_ctype != UTT_END) {
	    ii = -1;
	    if((jj = sscanf(cmdq->uc_text, "%d", &ii)) == 1) {
		if(ii >= 0)
		     last_click.ival0 = ii; 
	    }
	}
	sxm_process_click(&last_click);
	sxm_print_list(spi->list_select_files);
	break;

    case XX_PRESERVE_FILE:
	last_click.which_widget_button = ADD_TO_PRESERVATION_LIST;
	sxm_process_click(&last_click);
	break;

    case SE_NEXT_BOUNDARY:
	last_click.which_widget_button = MOVE_TO_NEXT_BOUNDARY;
	se_process_click(&last_click);
	break;

    default:
	break;
    }

# ifdef obsolete
    g_string_sprintfa (gs_complaints,"Key: %d\n", key);

    for(cmdq=cmds; cmdq->uc_ctype != UTT_END; cmdq++) {
	g_string_sprintfa (gs_complaints,"%s ", cmdq->uc_text);
    }
    g_string_sprintfa (gs_complaints,"\n");
# endif
}
/* c------------------------------------------------------------------------ */

void
sxm_undo_last(frme)
{
    int ii, jj, kk, nn, col, cell, rn, pn;
    struct se_changez *chz, *sxm_pop_change();
    struct examine_control *ecs;
    WW_PTR wwptr, solo_return_wwptr();
    struct dd_general_info *dgi, *dd_window_dgi();
    float *fptr, val;
    struct solo_list_mgmt *slm;
    char *aa, *bb, *line, tmp_val[16], *solo_list_entry();


    wwptr = solo_return_wwptr(frme);
    ecs = wwptr->examine_control;
    if(!(chz = sxm_pop_change(&wwptr->changez_list)))
	  return;
    /* decrement the changes count
     */
    wwptr->examine_info->change_count--;

    switch(chz->typeof_change) {

    case EX_MINUS_FOLD:
    case EX_PLUS_FOLD:
    case EX_DELETE:
    case EX_REPLACE:
    case EX_REMOVE_AIR_MOTION:
	fptr = ecs->data_ptrs[chz->col_num] + chz->row_num;
	*fptr = chz->f_old_val;
	sxm_change_cell_in_list(chz, chz->row_num, *fptr);
	break;

    case EX_RAY_PLUS_FOLD:
    case EX_RAY_MINUS_FOLD:
    case EX_GT_PLUS_FOLD:
    case EX_GT_MINUS_FOLD:
    case EX_RAY_IGNORE:

	if(chz->second_cell_num) { /* it's a run */
	    ii = chz->row_num;
	    nn = ii + chz->second_cell_num - chz->cell_num;
	}
	else if (chz->typeof_change == EX_GT_PLUS_FOLD ||
		chz->typeof_change == EX_GT_MINUS_FOLD ) {
	    ii = chz->row_num; nn = wwptr->examine_control->actual_num_cells;
	}
	else {
	    ii = 0; nn = wwptr->examine_control->actual_num_cells;
	}
	fptr = ecs->data_ptrs[chz->col_num] + ii;

	for(; ii < nn; ii++, fptr++) {
	    if(chz->typeof_change != EX_RAY_IGNORE &&
	       *fptr != ecs->bad_data[chz->col_num])
		  *fptr -= chz->f_new_val;
	    sxm_change_cell_in_list(chz, ii, *fptr);
	}
	break;
    }
    /*
     * pop this change back onto spair changes
     */
    sxm_push_spair_change(chz);
}
/* c------------------------------------------------------------------------ */

void
sxm_unlink_files_list(slm, dir, frme)
  struct solo_list_mgmt *slm;
  char *dir;
  int frme;
{
    /* removes the files listed
     */
    int ii, jj, nn;
    char *aa, *bb, name[256], line[256];
    struct solo_list_mgmt *slmx;
    struct solo_perusal_info *spi, *solo_return_winfo_ptr();
    FILE *stream;

    if((nn = slm->num_entries) <= 0 || !dir || !strlen(dir))
	  return;
    /*
     * read in the preserved files list
     */
    spi = solo_return_winfo_ptr();
    slmx = spi->list_text;
    solo_reset_list_entries(slmx);

    slash_path(name, dir);
    strcat(name, preservation_file_name);
    if(stream = fopen(name, "r")) {
	for(; aa = fstring(line, 88, stream);) {
	    solo_add_list_entry(slmx, aa, strlen(aa));
	}
	fclose(stream);
    }
    aa = name;
    slash_path(aa, dir);
    bb = aa + strlen(aa);

    for(jj=0; jj < slm->num_entries; jj++) {
	for(ii=0; ii < slmx->num_entries; ii++) {
	    if(!strcmp(*(slm->list +jj), *(slmx->list +ii)))
		  break;
	}
	if(ii == slmx->num_entries) {
	    strcpy(bb, *(slm->list +jj));
	    g_string_sprintfa (gs_complaints,"Unlinking %s\n", aa);
	    nn = unlink(name);
	}
	else {
	    g_string_sprintfa (gs_complaints,"**** %s is in the preserved file list!\n"
		      , *(slm->list +jj));
	}
    }
    sxm_gen_all_files_list(frme, spi->list_all_files);
}
/* c------------------------------------------------------------------------ */

int
sxm_ray_list(frme, wgt_btn)
  int frme, wgt_btn;
{
    /* assemble a list of ts_ray_info structs representing
     * the requisit number of rays to be displayed
     */
    int ii, jj, kk, mm, nn, mark, time_series, nurl=0, nurr=0, blip=0;
    int offset, incre, sweep_num, ray_num;
    WW_PTR wwptr, solo_return_wwptr();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct rot_table_entry *entry1, *dd_return_rotang1();
    struct rot_ang_table *rat;
    struct ts_ray_table *tsrt;
    struct ts_sweep_info *tssi;
    struct ts_ray_info *tsri, *tsrix, *tsri_right, *tsri_last, tsri_tmp;
    struct examine_control *ecs;
    struct solo_examine_info *sei;
    float rng;

    wwptr = solo_return_wwptr(frme);
    tsrt = wwptr->lead_sweep->tsrt;
    dgi = dd_window_dgi(wwptr->lead_sweep->window_num);
    ecs = wwptr->examine_control;
    sei = wwptr->examine_info;
    time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;
    /*
     * make sure there is space the list the requisite number of rays
     */
    if(tsrt->max_sxm_entries < 2 *sei->ray_count) {
	if(tsrt->tsri_sxm) free(tsrt->tsri_sxm);
	tsrt->max_sxm_entries = 2 * sei->ray_count;
	tsrt->tsri_sxm = (struct ts_ray_info *)
	      malloc(tsrt->max_sxm_entries * sizeof(struct ts_ray_info));
	memset(tsrt->tsri_sxm, 0
	       , tsrt->max_sxm_entries * sizeof(struct ts_ray_info));
    }

    if(!(time_series)) {
	if(wgt_btn == EX_SCROLL_LEFT) {
	    sei->ray_num--;
	}
	else if(wgt_btn == EX_SCROLL_RIGHT) {
	    sei->ray_num++;
	}
	rat = dgi->source_rat;	/* rotation angle table */

	if(ecs->ctr_on_click) {
	    sei->ray_num = dd_rotang_seek(rat, (float)ecs->click_angle)
		  - sei->ray_count/2;
	    sei->at_cell = dd_cell_num(dgi->dds, 0, (float)ecs->click_range)
		  - sei->cell_count/2;
	    ecs->ctr_on_click = NO;
	}
	ecs->actual_ray_count = sei->ray_count > rat->num_rays
	      ? rat->num_rays : sei->ray_count;

	if(sei->ray_num + sei->ray_count >= rat->num_rays) {
	    sei->ray_num = rat->num_rays - ecs->actual_ray_count;
	}
	if(sei->ray_num < 0) {
	    sei->ray_num = 0;
	}
	ecs->actual_ray_num = sei->ray_num;
	tsri = tsrt->tsri_sxm;
	/*
	 * the sweep info was placed in tsrt->tssi_top by
	 * solo_nab_next_file()
	 */
	for(ii=0; ii < ecs->actual_ray_count; ii++, tsri++) {
	    tsri->ray_num = ecs->actual_ray_num +ii;
	    tsri->sweep_num = 1;
	}
	return(ecs->actual_ray_count);
    }
    /* time series data!
     */
    tsri_right = tsrt->tsri_top + wwptr->view->width_in_pix;

    if(ecs->ctr_on_click) {
	/* assemble a list of rays based on the clicked x pixel
	 */
	nn = sp_ts_ray_list(frme, (int)wwptr->clicked_x_pxl
			    , (int)3);
	tsrix = *(tsrt->tsri_list +(nn-1)/2); /* center ray info struct */
	/*
	 * see how many rays we can actually do
	 */
	mm = (sei->ray_count-1)/2; /* in case ray count is an even number */
	nn = sei->ray_count/2;
	/*
	 * put the current center in the center of the new list for now
	 */
	sp_copy_tsri(tsrix, tsrt->tsri_sxm +sei->ray_count-1); 
	tsrix = tsrt->tsri_sxm +sei->ray_count-1;
	/*
	 * see if we can find the number of rays we need to the left
	 */
	sweep_num = tsrix->sweep_num;
	ray_num = tsrix->ray_num -1;
	tssi = tsrt->tssi_top +sweep_num;
	tsri = tsrix -1;

	for(nurl=0; nurl < sei->ray_count-1 ;) {
	    if(ray_num < 0) {
		if(--sweep_num < 1)
		      break;
		tssi--;
		ray_num = tssi->ray_count-1;
	    }
	    nurl++;
	    tsri->sweep_num = sweep_num;
	    tsri->ray_num = ray_num--;
	    tsri--;
	}
	/*
	 * now look to the right
	 */
	sweep_num = tsrix->sweep_num;
	ray_num = tsrix->ray_num +1;
	tssi = tsrt->tssi_top +sweep_num;
	tsri = tsrix +1;

	for(nurr=0; nurr < sei->ray_count-1 ;) {
	    if(ray_num >= tssi->ray_count) {
		if(++sweep_num > tsrt->sweep_count)
		      break;
		tssi++;
		ray_num = 0;
	    }
	    nurr++;
	    tsri->sweep_num = sweep_num;
	    tsri->ray_num = ray_num++;
	    tsri++;
	}
	ecs->actual_ray_count = sei->ray_count;
	ray_num = sei->ray_count-1 -mm;
	/*
	 * move them over to the start of the array
	 */
	if((nurl +1 +nurr) < sei->ray_count) {
	    ray_num = sei->ray_count-1 -nurl;
	    ecs->actual_ray_count = nurl +1 +nurr;
	}
	else if(nurl < mm) {
	    ray_num = sei->ray_count-1 -nurl;
	}
	else if(nurr < nn) {
	    ray_num -= nn -nurr;
	}
	
	tsri = tsrt->tsri_sxm;
	for(ii=0; ii++ < ecs->actual_ray_count; tsri++) {
	    sp_copy_tsri(tsri+ray_num, tsri);
	}
	sei->ray_num = tsrt->tsri_sxm->ray_num;
	sei->sweep_num = tsrt->tsri_sxm->sweep_num;
	rng = KM_TO_M(wwptr->field_vals->rng);

	sei->at_cell = dd_cell_num(dgi->dds, 0, rng) - sei->cell_count/2;
	ecs->ctr_on_click = NO;
	return(ecs->actual_ray_count);
    }

    /*
     * if this is just a refresh or a first time display,
     * make sure the starting sweep and ray numbers make sense
     * i.e. see if we can assemble a list based on the starting
     * ray number and sweep number
     */

    if(sei->sweep_num < 1) {
	if(wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT) {
	    tsri = tsrt->tsri_top + wwptr->view->width_in_pix -1;
	    incre = -1;
	}
	else {
	    tsri = tsrt->tsri_top;
	    incre = 1;
	}
	/* find the first ray plotted
	 */
	mm = wwptr->view->width_in_pix;
	for(; mm-- ; tsri += incre) {
	    if(tsri->ray_num >= 0 && tsri->sweep_num > 0)
		  break;
	}
	sei->sweep_num = tsri->sweep_num;
	sei->ray_num = tsri->ray_num;
    }
    if(sei->sweep_num > tsrt->sweep_count) {
	/* give it the last ray number of the last sweep */
	sei->sweep_num = tsrt->sweep_count;
	tssi = tsrt->tssi_top +sei->sweep_num;
	sei->ray_num = tssi->ray_count -1;
    }
    if(wgt_btn == EX_SCROLL_LEFT || wgt_btn == EX_SCROLL_RIGHT) {
	blip = wgt_btn == EX_SCROLL_RIGHT ? 1 : -1;
    }
    else {
	blip = 0;
    }
    if(wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT) {
	blip *= -1;
    }
    sei->ray_num += blip;
    /*
     * get to a legitimate starting sweep and ray number
     */
    tssi = tsrt->tssi_top +sei->sweep_num;

    if(sei->ray_num >= tssi->ray_count) {
	if(sei->sweep_num < tsrt->sweep_count) {
	    sei->sweep_num++;
	    sei->ray_num = 0;
	    tssi = tsrt->tssi_top +sei->sweep_num;
	}
	else {
	    sei->ray_num = tssi->ray_count -1;
	}
    }
    else if(sei->ray_num < 0) {
	if(sei->sweep_num > 1) {
	    sei->sweep_num--;
	    tssi = tsrt->tssi_top +sei->sweep_num;
	    sei->ray_num = tssi->ray_count -1;
	}
	else {
	    sei->ray_num = 0;
	}
    }
    if(wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT) {
	tsri = tsrt->tsri_sxm +sei->ray_count -1;
	incre = -1;
    }
    else {
	tsri = tsrt->tsri_sxm;
	incre = 1;
    }
    sweep_num = sei->sweep_num;
    ray_num = sei->ray_num;
    tssi = tsrt->tssi_top +sweep_num;

    for(mm=0; mm < sei->ray_count && sweep_num <= tsrt->sweep_count;) {
	if(ray_num >= tssi->ray_count) {
	    sweep_num++; tssi++;
	    ray_num = 0;
	    continue;
	}
	mm++;
	tsri->sweep_num = sweep_num;
	tsri->ray_num = ray_num++;
	tsri += incre;
    }
    ecs->actual_ray_count = mm;

# ifdef obsolete
    if((ray_num = sei->ray_count -mm) > 0 &&
       wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT) {
	/* shift the data over */
	tsri = tsrt->tsri_sxm;
	for(; mm--; tsri++) {
	    sp_copy_tsri(tsri+ray_num, tsri);
	}
    }
# endif
    return(ecs->actual_ray_count);
}
/* c------------------------------------------------------------------------ */

void sxm_update_examine_data(frme, wgt_btn)
  int frme, wgt_btn;
{
    int max_char_per_line = 256, fw;
    int ii, jj, kk, rc, pn, ww, mm, nn, nt, dd_tokens(), col, ncol, entry_num;
    int ndx, g1, ray_num, cell_num, ncells, data_ndx, nb, time_series;
    int incre, roff;
    struct examine_widget_info *ewi;
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct examine_control *ecs;
    struct solo_examine_info *sei;
    struct solo_list_mgmt *slm, *slmx;
    WW_PTR wwptr, wwptrx, solo_return_wwptr();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct rot_table_entry *entry1, *dd_return_rotang1();
    struct rot_ang_table *rat;
    struct ts_ray_table *tsrt;
    struct ts_sweep_info *tssi;
    struct ts_ray_info *tsri;
    int file_action=TIME_NEAREST, version=LATEST_VERSION;
    int sweep_skip=1, replot=YES;
    char *aa, *bb, *cc, *ee, *line, tmp_val[32];
    char *str_terminate(), tmp_flds[32], str[2048];
    double d, d_start, et, dd_rotation_angle(), dd_ac_vel();
    double dd_elevation_angle();
    float bad_val, val, *fptr;
    struct solo_perusal_info *spi, *solo_return_winfo_ptr();

    spi = solo_return_winfo_ptr();

# ifdef obsolete
    if(solo_busy())
	  return;
# endif
    wwptr = solo_return_wwptr(frme);
    wwptrx = wwptr->lead_sweep;
    tsrt = wwptr->lead_sweep->tsrt;

    if(wwptr->examine_info->change_count > 0) {
	slm = spi->list_a_message;
	solo_reset_list_entries(slm);
	aa = str;
	sprintf(aa, "You have %d changes outstanding!"
		  , wwptr->examine_info->change_count);
	solo_add_list_entry(slm, aa, strlen(aa));
	aa = "You cannot change the display";
	solo_add_list_entry(slm, aa, strlen(aa));
	aa = "until you either commit or clear them";
	solo_add_list_entry(slm, aa, strlen(aa));
	popup_message(slm);
	return;
    }
    ecs = wwptr->examine_control;
    mm = sxm_ray_list(frme, wgt_btn);
    if((time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES)) {
	d_start = wwptrx->sweep->start_time;
    }

# ifdef obsolete
    solo_set_busy_signal();
# endif

    sp_change_cursor(NO);
    sei = wwptr->examine_info;
    dgi = dd_window_dgi(wwptr->lead_sweep->window_num);


    /* Be sure to check for an outrageous number of columns!
     * first calculate the sizeof the range label
     */
    strcpy(ecs->rng_fmt, "%7.2f_|");
    line = ecs->data_line;
    sprintf(line, ecs->rng_fmt, 1.0);
    ecs->num_chars_rng_label = strlen(line);
    ecs->col_lims[0] = strlen(line);
    /*
     * then the size of a data colume
     */
    aa = ecs->actual_data_format;
    strcpy(aa, "%");
    strcat(aa, wwptr->examine_info->display_format);
    strcat(aa, "|");
    sprintf(line, ecs->actual_data_format, 1.0);
    ecs->num_chars_per_cell = strlen(line);
    /*
     * next get the number of fields per ray
     */
    strcpy(ecs->fields_list, sei->fields_list);
    if((nt = dd_tokens(ecs->fields_list, ecs->fld_ptrs)) < 1) nt = 2;
    mm = max_char_per_line -ecs->num_chars_rng_label;

    if((nn = mm/(nt * ecs->num_chars_per_cell)) < sei->ray_count) {
	sei->ray_count = nn;
    }


    if(wwptr->lead_sweep->sweep_file_modified) {
	/* if it's been edited, reopen the file
	 */
	solo_nab_next_file(wwptr->lead_sweep->window_num
			   , file_action, version, sweep_skip, replot);
	wwptr->lead_sweep->sweep_file_modified = NO;
    }

    /* nab the first ray
     */
    tsri = tsrt->tsri_sxm;
    incre = 1;
    tssi = tsrt->tssi_top +tsri->sweep_num;

    slash_path(str, tssi->directory);
    strcat(str, tssi->filename);
    
    if(wwptrx->file_id > 0)
	      close(wwptrx->file_id);

    if((wwptrx->file_id = open(str, O_RDONLY, 0)) < 0) {
	g_string_sprintfa (gs_complaints, "Unable to open sweep %s\n", str);
	return;
    }
    slash_path(dgi->directory_name, tssi->directory);
    dgi->in_swp_fid = wwptrx->file_id;
    dgi_buf_rewind(dgi);
    dd_absorb_header_info(dgi);
    rat = dgi->source_rat;	/* rotation angle table */
    entry1 = dd_return_rotang1(rat); /* first entry in rat */
    dgi_buf_rewind(dgi);
    nn = lseek(dgi->in_swp_fid
	       , (long)(entry1 +tsri->ray_num)->offset, 0L);
    dd_absorb_ray_info(dgi);

    sxm_log_stat_line(dgi);


    /*
     * check for fields present
     */
    aa = tmp_flds; *aa = '\0';
    /* nab at least two fields incase sei->fields_present is empty
     */
    for(pn=0; pn < 2 && pn < dgi->num_parms ; pn++) {
	str_terminate(tmp_val, dgi->dds->parm[pn]->parameter_name, 8);
	strcat(aa, tmp_val); strcat(aa, " ");
    }

    for(jj=2; jj-- ;) {
	strcpy(ecs->fields_list, sei->fields_list);
	nt = dd_tokens(ecs->fields_list, ecs->fld_ptrs);
	ecs->actual_num_fields = 0;

	if(strlen(sei->fields_list)) { /* this could be zero the first time through */
	    for(ii=0; ii < nt; ii++) {
		if((pn = dd_find_field(dgi, ecs->fld_ptrs[ii])) >= 0) {
		    ecs->actual_field_num[ecs->actual_num_fields++] = pn;
		}
	    }
	}
	if(ecs->actual_num_fields)
	      break;
	/* or else try the set of default fields generated earlier
	 */
	strcpy(sei->fields_list, tmp_flds);
    }
    ecs->num_data_cols = ecs->actual_ray_count * ecs->actual_num_fields;
    ecs->num_cols = ecs->num_data_cols +1;
    /*
     * see if we can print all the rows we want
     */
# ifdef obsolete
    if(sei->at_cell + sei->cell_count >= dgi->dds->celvc->number_cells) {
	ecs->actual_at_cell = dgi->dds->celvc->number_cells
	      - sei->cell_count -1;
	ecs->actual_num_cells = sei->cell_count;
    }
    else if(sei->at_cell < 0) {
	ecs->actual_at_cell = 0;
	ecs->actual_num_cells = sei->cell_count;
    }
    else {
	ecs->actual_at_cell = sei->at_cell;
	ecs->actual_num_cells = sei->cell_count;
    }
    sei->at_cell = ecs->actual_at_cell;
# else
    ecs->actual_at_cell = 0;
    ecs->actual_num_cells = dgi->dds->celvc->number_cells;
# endif
    
    slm = wwptr->examine_list;
    solo_reset_list_entries(slm);

    /*
     * c...mark
     * the following array will be used to determine
     * which column contains an arbitrary character number
     */
    /*
     * pointers to the beginning of each data column
     */
    ecs->col_ptrs[0] = line + ecs->num_chars_rng_label;
    /*
     * set up the rotation angle header
     */
    aa = ecs->rotang_header;
    for(ii=0; ii < ecs->num_chars_rng_label-1; ii++, *aa++ = ' ');
    *aa++ = '|'; *aa++ = '\0'; 
    /*
     * generate the array of range values
     */
    if(ecs->actual_num_cells > ecs->max_rngs) {
	if(ecs->ranges) free(ecs->ranges);
	ecs->max_rngs = ecs->actual_num_cells;
	ecs->ranges = (float *)malloc(ecs->max_rngs * sizeof(float));
	memset(ecs->ranges, 0, ecs->max_rngs * sizeof(float));
    }
    fptr = ecs->ranges;
    ii = ecs->actual_at_cell;
    mm = ii + ecs->actual_num_cells;
    for(; ii < mm; ii++) {
	*fptr++ = M_TO_KM(dgi->dds->celvc->dist_cells[ii]);
    }
    aa = ecs->del_str;		/* set up the delete pattern for the display
				 */
    for(ii=0; ii < ecs->num_chars_per_cell-1; ii++, *aa++ = '-');
    *aa++ = '|'; *aa++ = '\0';
    /*
     * calculate column limits and pointers
     */
    for(ii=1; ii < ecs->num_cols; ii++) {
	ecs->col_lims[ii] = ecs->col_lims[ii-1] + ecs->num_chars_per_cell;
	ecs->col_ptrs[ii] = ecs->col_ptrs[ii-1] + ecs->num_chars_per_cell;
    }
    ecs->non_data_col_count = 1; /* first col contains range */
    /*
     * set up the field names line
     */
    nn = ecs->num_chars_rng_label-2;
    cc = sei->row_annotation ? " " : "KM.";
    mm = strlen(cc);
    aa = bb = ecs->names_header;
    for(ii=0; ii < nn-mm; ii++, *aa++ = ' '); *aa = '\0';
    strcat(aa, cc);
    strcat(aa, " |");
    aa += strlen(aa);
    nn = ecs->num_chars_per_cell -1;

    for(bb=aa,ii=0; ii < ecs->actual_num_fields; ii++) {
	for(cc=aa,jj=0; jj++ < nn; *cc++ = ' '); /* blank it out */
	*cc++ = '|'; *cc++ = '\0'; 
	cc = dgi->dds->parm[ecs->actual_field_num[ii]]->parameter_name;
	cc = str_terminate(tmp_val, cc, 8);
	mm = strlen(cc);
	if(mm > nn) mm = nn;
	nb = nn - mm;
	jj = nb/2;
	if(nb && !jj) jj++;
	strncpy(aa+jj, cc, mm);
	aa += ecs->num_chars_per_cell;
    }
    *aa = '\0';
    mm = strlen(bb);
    for(ii=1; ii < ecs->actual_ray_count; ii++, aa += mm, *aa = '\0') {
	strncpy(aa, bb, mm);	/* duplicate fields for each
				 * additional ray */
    }
    /*
     * check arrays that hold data
     */
    ncells = ecs->actual_ray_count *
	  ecs->actual_num_fields *
		ecs->actual_num_cells;
	  
    if(ecs->max_cells < ncells) {
	if(ecs->data_space) free(ecs->data_space);
	ecs->data_space = (float *)
	      malloc(ncells * sizeof(float));
	memset(ecs->data_space, 0, ncells * sizeof(float));
	ecs->max_cells = ncells;
    }
    fptr = ecs->data_space;
    for(ii=0; ii < ecs->num_data_cols; ii++, fptr += ecs->actual_num_cells) {
	ecs->data_ptrs[ii] = fptr;
    }
    /*
     * we now know what ray to start at and how many rays to display
     * what cell to start at and how many cells to display
     * the number fields and which fields to display
     */
    for(rc=0;; rc++) {
	/* rotation angles...first blank out the space
	 */
	aa = bb = ecs->rotang_header + strlen(ecs->rotang_header);
	mm = nn = ecs->actual_num_fields * ecs->num_chars_per_cell -1;
	for(; mm--; *aa++ = ' '); *aa++ = '|'; *aa++ = '\0';

	if(sei->col_annotation) {
	    sprintf(tmp_val, "%d"
		    , rc + ecs->actual_ray_num +1);
	}
	else if(time_series) {
	    et = fmod(dgi->time -d_start, (double)1000.);
	    sprintf(tmp_val, "%.2f", et);
	    if((mm = strlen(tmp_val)) > nn) {
		jj = mm -nn;
		ee = tmp_val;
		for(mm=nn+1; mm--; ee++) { /* don't forget the null */
		    *ee = *(ee +jj);
		}
	    }
	}
	else {
	    d = dgi->dds->radd->scan_mode == RHI
		  ? dd_elevation_angle(dgi) : dd_rotation_angle(dgi);
	    sprintf(tmp_val, "%.2f", d);
	}
	if((mm = strlen(tmp_val)) > nn) mm = nn;
	nb = nn - mm;
	ii = nb/2;		/* center it */
	if(nb && !ii) ii++;	/* shift it right one char */
	strncpy(bb+ii, tmp_val, mm);

	g1 = ecs->actual_at_cell+1; /* equivalent fortran subscript */
	/*
	 * nab the data for each ray and keep it around
	 */
	roff = wwptr->lead_sweep->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT
	      ? ecs->actual_ray_count -rc -1 : rc;
	roff = rc;
	ndx = roff * ecs->actual_num_fields;
	for(ii=0; ii < ecs->actual_num_fields; ii++) {
	    dd_givfld(dgi, &ecs->actual_field_num[ii]
		      , &g1, &ecs->actual_num_cells
		      , ecs->data_ptrs[ndx+ii], &bad_val);
	    ecs->bad_data[ndx+ii] = bad_val;
	}
	ecs->ac_vel[rc] = dd_ac_vel(dgi);

	if(rc >= ecs->actual_ray_count -1) {
	    break;
	}
	tsri += incre;
	if(tsri->sweep_num != (tsri -incre)->sweep_num) {
	    /* need to open a new file
	     */
	    tssi = tsrt->tssi_top +tsri->sweep_num;

	    slash_path(str, tssi->directory);
	    strcat(str, tssi->filename);
    
	    if(wwptrx->file_id > 0)
		  close(wwptrx->file_id);

	    if((wwptrx->file_id = open(str, O_RDONLY, 0)) < 0) {
		g_string_sprintfa (gs_complaints, "Unable to open sweep %s\n", str);
		return;
	    }
	    slash_path(dgi->directory_name, tssi->directory);
	    dgi->in_swp_fid = wwptrx->file_id;
	    dgi_buf_rewind(dgi);
	    dd_absorb_header_info(dgi);
	    rat = dgi->source_rat;	/* rotation angle table */
	    entry1 = dd_return_rotang1(rat); /* first entry in rat */
	    dgi_buf_rewind(dgi);
	    nn = lseek(dgi->in_swp_fid
		       , (long)(entry1 +tsri->ray_num)->offset, 0L);
	    dd_absorb_ray_info(dgi);
	}
	else {
	    dd_absorb_ray_info(dgi);
	}
    }	
    entry_num = 0;
    solo_modify_list_entry(slm, ecs->rotang_header
			   , strlen(ecs->rotang_header), entry_num++);
    sxm_append_to_log_stream(ecs->rotang_header,strlen(ecs->rotang_header));

    /* second row: field names
     */
    solo_modify_list_entry(slm, ecs->names_header
			   , strlen(ecs->names_header), entry_num++);
    sxm_append_to_log_stream(ecs->names_header,strlen(ecs->names_header));

    ecs->heading_row_count = entry_num;

    for(cell_num=0; cell_num < ecs->actual_num_cells; cell_num++) {
	/* first do the range value
	 */
	if(sei->row_annotation) {
	    for(aa=cc=line,jj=0; jj++ < ecs->num_chars_rng_label -2;
		*cc++ = ' '); *cc++ = '_'; *cc++ = '|'; *cc++ = '\0';
	    sprintf(tmp_val, "%4d", cell_num + ecs->actual_at_cell +1);
	    strncpy(line+1, tmp_val, strlen(tmp_val));
	}
	else {
	    sprintf(tmp_val, ecs->rng_fmt, *(ecs->ranges + cell_num));
	    if((int)strlen(tmp_val) > ecs->num_chars_rng_label) {
		strcpy(tmp_val + ecs->num_chars_rng_label -1, "|");
	    }
	    strcpy(line, tmp_val);
	}
	/* now do the data for this line
	 */
	for(col=0; col < ecs->num_data_cols; col++) {
	    val = *(ecs->data_ptrs[col] + cell_num);
	    if(val != ecs->bad_data[col]) {
		sprintf(tmp_val, ecs->actual_data_format, val);
		if((int)strlen(tmp_val) > ecs->num_chars_per_cell) {
		    tmp_val[ecs->num_chars_per_cell-1] = '|';
		    tmp_val[ecs->num_chars_per_cell] = '\0';
		}
		strcpy(ecs->col_ptrs[col], tmp_val);
	    }
	    else {
		strcpy(ecs->col_ptrs[col], ecs->del_str);
	    }
	}
	/* send this line back to the list management tool
	 */
	solo_modify_list_entry(slm, line, strlen(line), entry_num++);
	sxm_append_to_log_stream(line,strlen(line));
    }
    se_refresh_examine_widgets(frme, slm);
    sp_change_cursor(YES);
# ifdef obsolete
    solo_clear_busy_signal();
# endif
    return;
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */



