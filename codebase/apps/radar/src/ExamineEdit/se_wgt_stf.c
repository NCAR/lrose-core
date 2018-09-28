/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# include <dorade_headers.h>
# include <input_sweepfiles.h>
# include <solo_window_structs.h>
# include <solo_editor_structs.h>
# include <solo_list_widget_ids.h>
# include <dd_math.h>
# include <ui.h>
# include <ui_error.h>
# include <errno.h>
# include <dirent.h>
# include <function_decl.h>
# include <dgi_func_decl.h>
# include <glib.h>

extern GString *gs_complaints;
/*
 * 
 * se_process_click
 * se_update_list
 * 
 *  
 */

/* Sherrie's functions */
void solo_refresh_list();
void se_refresh_sed_files_widget();
void config_from_frame_states_file();
void solo_refresh_command_buffer();
void se_dump_sfic_widget();
void se_refresh_sfic_widget();
void se_dump_bnd_files_widget();
void se_refresh_bnd_files_widget();
void se_dump_sed_files_widget();
void se_dump_state_widget();
void popup_message();


/* external functions */
void xse_add_bnd_pt();		/* se_bnd.c */
void se_draw_bnd();		/* se_bnd.c */
struct bnd_point_mgmt *se_zap_last_bpm(); /* se_bnd.c */
void se_clr_all_bnds();		/* se_bnd.c */
void se_clr_current_bnd();	/* se_bnd.c */
void se_cycle_bnd();		/* se_bnd.c */
void se_erase_all_bnds();	/* se_bnd.c */
void se_redraw_all_bnds();	/* se_bnd.c */
void xse_save_bnd();		/* se_bnd.c */
void se_write_sed_cmds();	/* se_proc_data.c */
void se_prev_bnd_set();		/* se_bnd.c */
void solo_save_window_info();	/* sp_save_frms.c */
int se_absorb_strings();	/* se_for_each.c */
void se_push_all_ssms();	/* se_utils.c */
int se_crack_ed_file_name();	/* se_utils.c */
int xse_absorb_bnd();		/* se_bnd.c */
int solo_absorb_window_info();	/* sp_save_frms.c */
void se_push_spair_string();	/* se_utils.c */
void se_append_string();	/* se_utils.c */
void se_insert_ssm_string();	/* se_utils.c */
void se_process_data();		/* se_proc_data.c */
void solo_add_list_entry();	/* dorade_share.c */

void se_really_readin_cmds();	/* se_for_each.c */
void solo_sort_strings();	/* se_basics.c */
void solo_message();		/* solo.c */

void     ddir_rescan_urgent();	/* dd_files.c */
int      mddir_file_list_v3();	/* dd_files.c */
int      mddir_num_radars_v3();
int      mddir_radar_num_v3();
int      mddir_gen_swp_list_v3();


/* internal files */
void se_process_click();
struct solo_list_mgmt *se_update_list();
int se_nab_files();
int se_nab_all_files();

static struct solo_str_mgmt *help_items = NULL;
static char * default_help_file = "hlp.";

/* c------------------------------------------------------------------------ */

void se_process_click(sci)
  struct solo_click_info *sci;
{
    static int hclicks = 0;
    int arg=0;
    struct ui_command *cmds=NULL;
    int ii, nn, vv, mark, id, entry, mouse, wig_but;
    int time_series=NO, automatic, down;
    int sp_max_frames();
    long lnn;
    double d, d_ctr, dtime, ddfnp_list_entry();
    char *a, *b, *c, *e, str[256], line[88], *dd_whiteout(), *dd_delimit();
    char string_space[256], *str_ptrs[16];
    char *dir, ts[24], *se_string_time(), *se_return_example_dir();
    struct sed_command_files *scf, *se_return_cmd_files_struct();
    struct solo_frame_state_files *sfsf, *se_return_state_files_struct();
    struct bnd_point_mgmt *bpm, *se_pop_bpms();
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct one_boundary *ob;
    struct solo_list_mgmt *solo_malloc_list_mgmt(), *slm
	  , *se_update_list();
    static struct solo_list_mgmt *slm_keeper=NULL;
    struct solo_str_mgmt *ssm, *ssmx, *ssmy, *se_remove_ssm_string()
	  , *se_pop_spair_string();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    WW_PTR wwptr, solo_return_wwptr();


    if(!sci) {
	uii_printf("se_process_click...struct solo_click_info *sci=NULL\n");
	return;
    }
    else if(sci->frame < 0 || sci->frame > sp_max_frames()) {
	uii_printf("se_process_click...illegal frame number: %d\n"
		  , sci->frame);
	return;
    }

    seds = return_sed_stuff();
    id = sci->which_list;
    entry = sci->which_list_entry;
    mouse = sci->which_mouse_button;
    wig_but = sci->which_widget_button;
    sebs = return_se_bnd_struct();

    if(wig_but == BND_DATA_POINT) {
	sebs->last_operation = BND_POINT_APPENDED;
	xse_add_bnd_pt(sci, sebs->current_boundary);
    }
    else if(wig_but == DELETE_LAST_POINT) {
	ob = sebs->current_boundary;
	if(ob->num_points > 1)
	      se_draw_bnd(ob->top_bpm->last, 2, YES); /* erase it first */
	se_zap_last_bpm(&ob->top_bpm);
	if(ob->num_points > 0)
	      ob->num_points--;
	sebs->last_operation = BND_POINT_DELETED;
    }
    else if(wig_but == CLOSE_BOUNDARY) {
	sebs->last_operation = BND_OTHER;
    }
    else if(wig_but == CLEAR_BOUNDARY) {
	if(sebs->last_operation == BND_CLEARED) {
	    sebs->last_operation = BND_OTHER;
	    se_clr_all_bnds();
	}
	else {
	    sebs->last_operation = BND_CLEARED;
	    se_clr_current_bnd();
	}
# ifdef obsolete
	se_redraw_all_bnds();
# endif
    }
    else if(wig_but == MOVE_TO_NEXT_BOUNDARY) {
	se_cycle_bnd();
	sebs->last_operation = MOVE_TO_NEXT_BOUNDARY;
    }
    else if(wig_but == REDRAW_BOUNDARIES) {
	if(sebs->last_operation == REDRAW_BOUNDARIES) {
	    sebs->last_operation = BND_OTHER;
	    se_erase_all_bnds();
	    ob = sebs->current_boundary;
	    if(ob->num_points > 1) {
		se_draw_bnd(ob->top_bpm->last, ob->num_points, NO);
	    }
	}
	else {
	    sebs->last_operation = REDRAW_BOUNDARIES;
	    se_redraw_all_bnds();
	}
    }
    else if(wig_but == SAVE_BOUNDARY) {
	sebs->last_operation = BND_OTHER;
	xse_save_bnd();
    }
    else if(wig_but == SAVE_SED_CMD_FILES) {
	se_write_sed_cmds();
	sebs->last_operation = BND_OTHER;
    }
    else if(wig_but == PREV_BND_SET) {
	se_prev_bnd_set();
	sebs->last_operation = PREV_BND_SET;
    }
    else if(wig_but == SAVE_FRAME_STATES) {
	solo_save_window_info();
	sebs->last_operation = BND_OTHER;
    }
    else if(wig_but == IGNORE_SWEEP_INFO) {
	sebs->last_operation = BND_OTHER;
	sfsf = se_return_state_files_struct();
	sfsf->omit_sweep_info = !sfsf->omit_sweep_info;
	if(sfsf->omit_sweep_info)
	      uii_printf("IGNORING sweep info in the frame states file\n");
	else
	      uii_printf("NOT ignoring sweep info in the frame states file\n");
    }
    else if(wig_but == IGNORE_SOURCE_FILE_INFO) {
	sebs->last_operation = BND_OTHER;
	scf = se_return_cmd_files_struct();
	scf->omit_source_file_info = !scf->omit_source_file_info;
	if(scf->omit_source_file_info)
	      uii_printf("IGNORING source_file info in the frame states file\n");
	else
	      uii_printf("NOT ignoring source_file info in the frame states file\n");
    }
    else if(wig_but == INSIDE_BOUNDARY) {
	sebs->last_operation = BND_OTHER;
	seds->boundary_use = SE_INSIDE_BOUNDARY;
    }
    else if(wig_but == OUTSIDE_BOUNDARY) {
	sebs->last_operation = BND_OTHER;
	seds->boundary_use = SE_OUTSIDE_BOUNDARY;
    }
# ifdef FIXME
# ifdef obsolete
    else if(id == EXAMPLE_OPS_LIST) {
# else
    else if(id == NON_FER_CMDS_LIST) {
# endif
	sebs->last_operation = BND_OTHER;
	slm = seds->cmd_examples;
	if(entry < 0 ||
	   entry >= slm->num_entries) {
	    return;
	}
	a = *(slm->list +entry);
				/* this is the name of the file containing
				 * the example command sequence */
	slash_path(str, se_return_example_dir(str));
	strcat(str, a);
	if((nn=se_absorb_strings(str, &seds->scratch1)) > 0) {
	    /*
	     * move scratch1 to cmdz
	     */
	    se_push_all_ssms(&seds->cmdz);
	    seds->cmdz = seds->scratch1;
	    seds->scratch1 = NULL;
	    slm = se_update_list(CURRENT_CMDS_LIST);
	    solo_refresh_list(CURRENT_CMDS_LIST, slm);
	}
    }
# endif 

    else if(id == SED_CMD_FILES_LIST) {
	sebs->last_operation = BND_OTHER;
	slm = seds->list_ed_cmd_files;
# ifdef obsolete
	if(entry < 0 ||
	   entry >= slm->num_entries) {
	    return;
	}
	/* nab this entry and put it in the command buffer
	 */
	scf = se_return_cmd_files_struct();
	a = *(slm->list +entry);
	se_crack_ed_file_name(a, scf->file_name_text, scf->comment_text);
# endif
	se_really_readin_cmds();
	slm = se_update_list(CURRENT_CMDS_LIST);
	solo_refresh_list(CURRENT_CMDS_LIST, slm);
	se_refresh_sed_files_widget(scf);
    }
    else if(id == BND_FILES_LIST) {
	sebs->last_operation = BND_OTHER;
	se_clr_all_bnds();
	se_redraw_all_bnds();
# ifdef obsolete
	slm = seds->list_boundary_files;
	sebs = return_se_bnd_struct();
	if(entry < 0 ||
	   entry >= slm->num_entries) {
	    return;
	}
	/* nab this entry and put it in the command buffer
	 */
	a = *(slm->list +entry);
	se_crack_ed_file_name(a, sebs->file_name_text, sebs->comment_text);
# endif
	xse_absorb_bnd();
    }
    else if(id == FRAME_STATES_LIST) {
	sebs->last_operation = BND_OTHER;
	slm = seds->list_winfo_files;
	if(entry < 0 ||
	   entry >= slm->num_entries) {
	    return;
	}
	a = *(slm->list +entry);
	sfsf = se_return_state_files_struct();
	se_crack_ed_file_name(a, sfsf->file_name_text, sfsf->comment_text);
	if(solo_absorb_window_info() > 0) {
	    config_from_frame_states_file();
	}
    }
# ifdef obsolete
    else if(id == NON_FER_CMDS_LIST || id == FER_CMDS_LIST) {
# else
    else if(id == FER_CMDS_LIST) {
# endif
	sebs->last_operation = BND_OTHER;
	slm = id == NON_FER_CMDS_LIST ? seds->all_other_cmds :
	      seds->all_fer_cmds;
	if(entry < 0 || entry >= slm->num_entries) {
	    return;
	}
	/* nab this entry and put it in the command buffer
	 */
	a = *(slm->list +entry);
	strcpy(sci->command_buffer, a);
	solo_refresh_command_buffer(sci);
    }
    else if(id == CURRENT_CMDS_LIST) {
	sebs->last_operation = BND_OTHER;
	slm = seds->current_cmds;
	if(mouse == LEFT_MOUSE_BUTTON) {
	    if(entry < 0 ||
	       entry >= slm->num_entries) {
		return;
	    }
	    /* remove this entry and put it into the command buffer
	     */
	    ssm = se_remove_ssm_string(&seds->cmdz, entry);
	    strcpy(sci->command_buffer, ssm->at);
	    se_push_spair_string(ssm);
	    if(!seds->batch_mode)
		  solo_refresh_command_buffer(sci);
	}
	else if(mouse == RIGHT_MOUSE_BUTTON) {
	    ssm = se_pop_spair_string();
	    a = dd_whiteout(sci->command_buffer);
	    strcpy(ssm->at, "    ");
	    strcat(ssm->at, a);
	    if(entry < 0 ||
	       entry >= slm->num_entries) {
		se_append_string(&seds->cmdz, ssm);
	    }
	    else
		  se_insert_ssm_string(&seds->cmdz, entry, ssm);
	}
	else {
	    return;
	}
	slm = se_update_list(id);
	solo_refresh_list(CURRENT_CMDS_LIST, slm);
    }
    else if(id == HELP_LIST) {
	if( !help_items ) {	/* construct a circular que for strings */
	    for( ii = 0; ii < 64; ii++ ) {
		ssm = se_pop_spair_string();
		if( !ii ) {
		    help_items = ssm;
		    ssm->last = ssm->next = ssm;
		    continue;
		}
		strcpy( help_items->at, default_help_file );
		ssm->next = help_items;
		ssm->last = help_items->last;
		help_items->last->next = ssm;
		help_items->last = ssm;
	    }
	}
	sebs->last_operation = BND_OTHER;
	slm = seds->help_info;
	a = default_help_file;

	if( mouse == RIGHT_MOUSE_BUTTON ) {
	    help_items = help_items->last;
	    a = help_items->at;
	}
	else {
	    a = entry >= 0 ? *(slm->list +entry) : "";
	    help_items = help_items->next;
	    strcpy( help_items->at, a );
	}
	strcpy(sci->command_buffer, a);
	if(!seds->batch_mode)
	      solo_refresh_command_buffer(sci);
	sebs->last_operation = LIST_HELP_INFO;
	slm = se_update_list(HELP_LIST);
	solo_refresh_list(HELP_LIST, slm);
    }
    else if(id == RADARS_LIST) {
	sebs->last_operation = BND_OTHER;
	se_dump_sfic_widget(seds->sfic);
	slm = seds->list_radars;
	a = *(slm->list +entry);
	b = seds->sfic->radar_names_text;
	/*
	 * find the last non-blank character in the list
	 * and append this name after it
	 */
	for(c = b +strlen(b)-1; b < c && *c == ' '; c--);
	if(c != b) {*(++c) = ' '; c++;}
	strcpy(c, a);
	seds->sfic->clicked_frame = 0;
	se_refresh_sfic_widget(seds->sfic);
    }
    else if(wig_but == FIRST_SWEEP_BUTTON || wig_but == LAST_SWEEP_BUTTON) {
	sebs->last_operation = BND_OTHER;
	lnn = dtime = ddfnp_list_entry
	      (seds->se_frame, seds->sfic->radar_num, entry, &vv, line, str);

	if(wig_but == FIRST_SWEEP_BUTTON) {
	    seds->sfic->start_time = dtime;
	    strcpy(seds->sfic->first_sweep_text
		   , se_string_time(dtime, ts));
	}
	else {
	    seds->sfic->stop_time = dtime;
	    strcpy(seds->sfic->last_sweep_text
		   , se_string_time(dtime, ts));
	}
	seds->sfic->clicked_frame = 0;
	se_refresh_sfic_widget(seds->sfic);
    }
    else if(wig_but == LIST_HELP_INFO) {
	if(sebs->last_operation == LIST_HELP_INFO) {
	    /* need a routine I can call that pops down the
	     * list help info widget if it is popped up. */
	    sebs->last_operation = BND_OTHER;
# ifdef obsolete	    
	    popdown_helplist();
# endif
	}
	else {
	    sebs->last_operation = LIST_HELP_INFO;
	    slm = se_update_list(HELP_LIST);
	    solo_refresh_list(HELP_LIST, slm);
	}
    }
    else if(wig_but == NEW_COMMAND_SEQ) {
	sebs->last_operation = BND_OTHER;
	se_push_all_ssms(&seds->cmdz);
	ssm = se_pop_spair_string();
	strcpy(ssm->at, "!");
	se_append_string(&seds->cmdz, ssm);
	ssm = se_pop_spair_string();
	strcpy(ssm->at,
	       "! for-each-ray (put one-time cmds before this line)");
	se_append_string(&seds->cmdz, ssm);
	ssm = se_pop_spair_string();
	strcpy(ssm->at, "!");
	se_append_string(&seds->cmdz, ssm);
	slm = se_update_list(CURRENT_CMDS_LIST);
	solo_refresh_list(CURRENT_CMDS_LIST, slm);
# ifdef obsolete
	/*
	 * test of new message widget
	 */
	if(!slm_keeper) {
	    slm_keeper = solo_malloc_list_mgmt(SE_MAX_STRING);
	}
	slm = slm_keeper;
	solo_reset_list_entries(slm);
	a = "This is a test of the message widget";
	solo_add_list_entry(slm, a, strlen(a));
	a = "This is a only a test";
	solo_add_list_entry(slm, a, strlen(a));
	a = "But I hope it works!";
	solo_add_list_entry(slm, a, strlen(a));
	popup_message(slm);
# endif
    }
    else if(wig_but == START_PASS_THRU_DATA) {
	sebs->last_operation = BND_OTHER;
	seds->pbs = NULL;
	wwptr = solo_return_wwptr(sci->frame);
	time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;
	automatic = wwptr->view->type_of_plot & TS_AUTOMATIC;
	down = wwptr->view->type_of_plot & TS_PLOT_DOWN;
	d_ctr = wwptr->view->ts_ctr_km;

	se_process_data(arg, cmds, time_series, automatic, down, d_ctr);
    }
    else if(wig_but == REWRITE_SAME_VERSION) {
	sebs->last_operation = BND_OTHER;
	seds->sfic->new_version_flag = NO;
    }
    else if(wig_but == CREATE_NEW_VERSION) {
	sebs->last_operation = BND_OTHER;
	seds->sfic->new_version_flag = YES;
    }
    else if(wig_but == USE_LAST_SWEEP_SET) {
	sebs->last_operation = BND_OTHER;
	nn = seds->setup_input_ndx++;
	seds->setup_input_ndx %= MAX_SETUP_INPUTS;
	if(!(ssm = seds->setup_inputs[nn])) {
	    ssm = seds->setup_inputs[0];
	    seds->setup_input_ndx = 1;
	}

	for(; ssm; ssm=ssm->next) {
	    if(strstr(ssm->at, "procedure"))
		  continue;
	    ui_perform(ssm->at);
	    if(seds->punt)
		  return;
	}
	seds->sfic->clicked_frame = 0;
	se_refresh_sfic_widget(seds->sfic);
    }
    else {
	uii_printf("se_process_click...unused button no: %d or list id: %d\n"
		  , sci->which_widget_button, sci->which_list);
	mark = 0;
    }
    return;
}
/* c------------------------------------------------------------------------ */

struct solo_list_mgmt *
se_update_list(list_id)
  int list_id;
{
    /* update various lists so they can be refreshed on the screen
     * solo_refresh_list(list_id)
     */
    int ii, nn, mark, rn;
    FILE *stream;
    char *dir;
    char *a, *b, *c, *e, str[128], *dd_whiteout(), *dd_delimit();
    char command[256];
    char *fgetz(), *strrchr(), mess[256], *mddir_radar_name_v3();
    char *se_return_help_dir(), *se_return_example_dir(), cmd_name[64];
    struct sed_command_files *scf, *se_return_cmd_files_struct();
    struct solo_frame_state_files *sfsf, *se_return_state_files_struct();
    struct bnd_point_mgmt *bpm, *se_pop_bpms();
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct one_boundary *ob;
    struct solo_click_info *sci, *return_solo_click_ptr();
    struct solo_list_mgmt *solo_malloc_list_mgmt(), *slm=NULL;
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    double d, d_x, d_y;

    seds = return_sed_stuff();
    sebs = return_se_bnd_struct();
    sci = return_solo_click_ptr();


    if(list_id == CURRENT_CMDS_LIST) {
	ssm = seds->cmdz;
	if(!ssm) {		/* put in the basics */
	    ssm = se_pop_spair_string();
	    strcpy(ssm->at, "!");
	    se_append_string(&seds->cmdz, ssm);
	    ssm = se_pop_spair_string();
	    strcpy(ssm->at,
		   "! for-each-ray (put one-time cmds before this line)");
	    se_append_string(&seds->cmdz, ssm);
	    ssm = se_pop_spair_string();
	    strcpy(ssm->at, "!");
	    se_append_string(&seds->cmdz, ssm);
	    ssm = seds->cmdz;
	}
	slm = seds->current_cmds;
	if(!slm) {
	    slm = solo_malloc_list_mgmt(SE_MAX_STRING);
	    seds->current_cmds = slm;
	}
	solo_reset_list_entries(slm);

	for(; ssm; ssm=ssm->next) {
	    solo_add_list_entry(slm, ssm->at, strlen(ssm->at));
	}
	return(slm);
    }
    else if(list_id == HELP_LIST) {
# ifdef FIXME
	slm = seds->help_info;
	if(!slm) {
	    slm = solo_malloc_list_mgmt(SE_MAX_STRING);
	    seds->help_info = slm;
	}
	solo_reset_list_entries(slm);
	se_return_help_dir(str);
	strcat(str, default_help_file);
	/*
	 * nab the first item from the command buffer and
	 * see if there is a help file for it
	 */
	a = sci->command_buffer;
	b = a +strlen(a);
	for(; a < b &&
	    (*a == ' ' || *a == '!' || *a == '*' || *a == '\t'); a++)
	      {
		  if(*a == '*')
			dir = se_return_example_dir(str);
	      }
	c = e = str +strlen(str);

	if(*a) {
	    for(b=dd_delimit(a);  a < b;  *c++ = *a++); *c++ = '\0';
	}
	/* open the file and dump it into the list
	 */
	for(ii=0;; ii++) {
	    if(stream = fopen(str, "r")) {
		break;
	    }
	    if(!ii) {
# ifdef obsolete
		sprintf(mess, "! Sorry, there is no help file for \'%s\'"
			, e);
		solo_add_list_entry(slm, mess, strlen(mess));
# endif
		/* go for general info or the "hlp." file
		 */
		*sci->command_buffer = *e = '\0';
	    }
	    if(ii > 1) {
		return(NULL);
	    }
	}

	for(;;) {
	    if(!(a = fgetz(str, (int)88, stream))) {
		break;
	    }
	    solo_add_list_entry(slm, str, strlen(str));
	}
	fclose(stream);
	return(slm);
# endif 
    }
    else if(list_id == FER_CMDS_LIST) {
	return(seds->all_fer_cmds);
    }
    else if(list_id == RADARS_LIST) {
	slm = seds->list_radars;
	se_dump_sfic_widget(seds->sfic);
	if(!slm) {
	    slm = solo_malloc_list_mgmt(SE_MAX_STRING);
	    seds->list_radars = slm;
	}
	slash_path(str, seds->sfic->directory_text);
	ddir_rescan_urgent(seds->se_frame);

	if((nn = mddir_file_list_v3(seds->se_frame, str)) < 1) {
	    uii_printf("No sweep files in dir: %s\n", str);
	    return(NULL);
	}
	strcpy(seds->sfic->directory, str);
	nn = mddir_num_radars_v3(seds->se_frame);
	solo_reset_list_entries(slm);

	for(ii=0; ii < nn; ii++) {
	    a = mddir_radar_name_v3(seds->se_frame, ii);
	    solo_add_list_entry(slm, a, strlen(a));
	}
	return(slm);
    }
    else if(list_id == SWEEPS_LIST) {
	slm = seds->list_sweeps;
	if(!slm) {
	    slm = solo_malloc_list_mgmt(SE_MAX_STRING);
	    seds->list_sweeps = slm;
	}
	se_dump_sfic_widget(seds->sfic);
	slash_path(str, seds->sfic->directory_text);
	ddir_rescan_urgent(seds->se_frame);

	if((nn = mddir_file_list_v3(seds->se_frame, str)) < 1) {
	    uii_printf("No sweep files in dir: %s\n", str);
	    return(NULL);
	}
	strcpy(seds->sfic->directory, str);
	solo_reset_list_entries(slm);
	/*
	 * use the first radar name in the list
	 */
	a = dd_whiteout(seds->sfic->radar_names_text);
	b = dd_delimit(a);
	for(c=str; a < b; *c++ = *a++); *c++ = '\0';

	seds->sfic->radar_num = rn = mddir_radar_num_v3(seds->se_frame, str);
	if(rn < 0) {
	    uii_printf("No sweep files for radar: %s in %s\n"
		      , str, seds->sfic->directory);
	    return(NULL);
	}
	mddir_gen_swp_list_v3(seds->se_frame, rn, slm);

	return(slm);
    }
    else if(list_id == BND_FILES_LIST) {
	se_dump_bnd_files_widget(sebs);
	slm = seds->list_boundary_files;
	if(!slm) {
	    slm = solo_malloc_list_mgmt(SE_MAX_STRING);
	    seds->list_boundary_files = slm;
	}
	a = "bnd";
	if(!strlen(sebs->directory_text)) {
	    strcpy(sebs->directory_text, seds->sfic->directory_text);
	}
	se_nab_files(sebs->directory_text, slm, a, strlen(a));
	solo_sort_strings(slm->list, slm->num_entries);
	se_refresh_bnd_files_widget(sebs);
	return(slm);
    }
    else if(list_id == SED_CMD_FILES_LIST) {
	slm = seds->list_ed_cmd_files;
	if(!slm) {
	    slm = solo_malloc_list_mgmt(SE_MAX_STRING);
	    seds->list_ed_cmd_files = slm;
	}
	a = "sed";
	scf = se_return_cmd_files_struct();
	se_dump_sed_files_widget(scf);
	if(!strlen(scf->directory_text)) {
	    strcpy(scf->directory_text, seds->sfic->directory_text);
	}
	se_nab_files(scf->directory_text, slm, a, strlen(a));
	solo_sort_strings(slm->list, slm->num_entries);
	return(slm);
    }
    else if(list_id == FRAME_STATES_LIST) {
	slm = seds->list_winfo_files;
	if(!slm) {
	    slm = solo_malloc_list_mgmt(SE_MAX_STRING);
	    seds->list_winfo_files = slm;
	}
	sfsf = se_return_state_files_struct();
	se_dump_state_widget(sfsf);
	a = "wds";
	se_nab_files(sfsf->directory_text, slm, a, strlen(a));
	solo_sort_strings(slm->list, slm->num_entries);
	return(slm);
    }
# ifdef FIXME
# ifdef obsolete
    else if(list_id == EXAMPLE_OPS_LIST) {
# else
    else if(list_id == NON_FER_CMDS_LIST) { /* klooge! */
# endif
	slm = seds->cmd_examples;
	if(!slm) {
	    slm = solo_malloc_list_mgmt(SE_MAX_STRING);
	    seds->cmd_examples = slm;
	}
	dir = se_return_example_dir(str);
	se_nab_all_files(dir, seds->cmd_examples);
	return(slm);
    }
# endif 
    else if(list_id == BND_POINTS_LIST) {
	slm = seds->boundary_points;
	if(!slm) {
	    slm = solo_malloc_list_mgmt(SE_MAX_STRING);
	    seds->boundary_points = slm;
	}
	solo_reset_list_entries(slm);
	ob = sebs->current_boundary;

	if(ob && ob->num_points) {
	    bpm = ob->top_bpm;
	    for(; bpm; bpm = bpm->next) {
		sprintf(str, "x:%8.3fkm y:%8.3fkm"
			, M_TO_KM((float)bpm->x)
			, M_TO_KM((float)bpm->y));
		solo_add_list_entry(slm, str, strlen(str));
	    }
	}
	return(slm);
    }
    return(slm);
}
/* c------------------------------------------------------------------------ */

int se_nab_files(dir, slm, type, nt)
  char *dir, *type;
  struct solo_list_mgmt *slm;
  int nt;
{
    int ii, jj, nn, mark;
    DIR *dir_ptr;
    struct dirent *dp;
    char *fn, *keep, str[256], mess[256];

    if(!(dir_ptr = opendir(dir))) {
	sprintf(mess, "Cannot open directory %s\n", dir);
	solo_message(mess);
	return(0);
    }
    solo_reset_list_entries(slm);

    for(;;) {
	dp = readdir(dir_ptr);
	if(dp == NULL ) {
	    break;
	}
	fn =  dp->d_name;
	if(strncmp(fn, type, nt)) {	/* not a match */
	    continue;
	}
	solo_add_list_entry(slm, fn, strlen(fn));
    }
    closedir(dir_ptr);
# ifdef obsolete
    /*
     * sort by time (simple brute force sort)
     */
    solo_list_sort_file_names(slm);
# endif
    return(slm->num_entries);
}
/* c------------------------------------------------------------------------ */

int se_nab_all_files(dir, slm)
  char *dir;
  struct solo_list_mgmt *slm;
{
    int ii, jj, nn, mark;
    DIR *dir_ptr;
    struct dirent *dp;
    char *a, *b, *fn, *keep, mess[256];

    if(!(dir_ptr = opendir(dir))) {
	sprintf(mess, "Cannot open directory %s\n", dir);
	solo_message(mess);
	return(0);
    }
    solo_reset_list_entries(slm);

    for(;;) {
	dp = readdir(dir_ptr);
	if(dp == NULL ) {
	    break;
	}
	fn =  dp->d_name;
	b = fn +strlen(fn)-1;	/* point to last character */
	if(*fn == '.' || *b == '~')
	      continue;
	solo_add_list_entry(slm, fn, strlen(fn));
    }
    closedir(dir_ptr);
    return(slm->num_entries);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

