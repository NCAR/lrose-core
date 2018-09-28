/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# include <dorade_headers.h>
# include <input_sweepfiles.h>
# include <solo_editor_structs.h>
# include <solo_list_widget_ids.h>
# include <solo_window_structs.h>
# include <ui.h>
# include <ui_error.h>
# include <dd_math.h>
# include <function_decl.h>
# include <dgi_func_decl.h>
# include <glib.h>

extern GString *gs_complaints;
/*
 * 
 * 
 * dd_edd
 * se_edit_summary
 * se_gen_sweep_list
 * se_process_data
 * se_setup_input_cmds
 * se_shift_setup_inputs
 * se_tack_on_cmds
 * se_write_sed_cmds
 * 
 *  
 *  
 */

typedef enum {
   SED_ZERO,
   SED_FIELD,
   SED_INTEGER,
   SED_REAL,
   SED_WHERE
} cmd_data_types;

/*
 * se_set_sfic_from_frame is in sp_basics.c (update_sfic)
 */

int se_dump_sfic_widget();
void se_dump_sed_files_widget();
void se_refresh_sed_files_widget();
void se_refresh_sfic_widget();


/* external routines */
void se_set_sfic_from_frame();	/* sp_basics.c */
void dd_new_vol();		/* dd_swp_files.c */
int ddswp_last_ray();		/* swp_file_acc.c */
void sp_sweep_file_check();	/* sp_clkd.c */
int se_free_raqs();		/* dorade_share.c */
void ddir_rescan_urgent();	/* dd_files.c */
int se_sizeof_bnd_set();	/* se_bnd.c */
void se_pack_bnd_set();		/* se_bnd.c */
int se_compare_bnds();		/* se_bnd.c */
void se_clr_all_bnds();		/* se_bnd.c */
void se_append_string();	/* se_utils.c */
int ddir_files_v3();		/* dd_files.c */
void se_fix_comment();		/* se_utils.c */
void se_push_spair_string();	/* se_utils.c */
int se_radar_inside_bnd();	/* se_bnd.c */
void se_shift_bnd();		/* se_bnd.c */
void se_ts_find_intxns();	/* se_bnd.c */
int xse_find_intxns();		/* se_bnd.c */
int xse_num_segments();		/* se_bnd.c */
void se_nab_segment();		/* se_bnd.c */
void se_push_all_ssms();	/* se_utils.c */
int se_absorb_strings();	/* se_utils.c */
int solo_halt_flag();		/* sp_basics.c */
void solo_clear_halt_flag();	/* sp_basics.c */


/* internal routines */
void se_cull_setup_input();
void dd_edd();
void se_edit_summary();
int se_gen_sweep_list();
int se_process_data();
int se_really_readin_cmds();
void se_setup_input_cmds();
void se_shift_setup_inputs();
void se_tack_on_cmds();
double ts_pointing_angle();
void se_write_sed_cmds();
struct ui_cmd_mgmt *se_malloc_ucm();
struct ui_cmd_mgmt *ok_cmd( char * line, struct ui_cmd_mgmt *the_ucm );
int sii_cmd_tmplt_tokens (struct ui_cmd_mgmt *the_ucm);


/* c------------------------------------------------------------------------ */

int se_ucm_errlen()
{
   return 512;
}

/* c------------------------------------------------------------------------ */

int se_ucm_cmdlen()
{
   return 128;
}

/* c------------------------------------------------------------------------ */

void se_cull_setup_input(top_ssm)
  struct solo_str_mgmt **top_ssm;
{  
    /* routine to cull setup-input commands from a list of commands
     */
    struct sed_command_files *scf, *se_return_cmd_files_struct();
    struct solo_str_mgmt *ssmx, *next=(*top_ssm), *se_pull_ssm_string();
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    char *a;
    
    seds = return_sed_stuff();
    scf = se_return_cmd_files_struct();

    for(; next;) {
	ssm = next;
	next = next->next;
				/* ignore leading blanks */
	for(a=ssm->at; *a && (*a == ' ' || *a == '\t'); a++);
	if(strncmp(a, "sweep-directory", 7) == 0 ||
	   strncmp(a, "radar-names", 7) == 0 ||
	   strncmp(a, "first-sweep", 7) == 0 ||
	   strncmp(a, "last-sweep", 7) == 0 ||
	   strncmp(a, "version", 4) == 0 ||
	   strncmp(a, "new-version", 7) == 0 ||
	   strncmp(a, "no-new-version", 9) == 0)
	      {
# ifdef never_again
		  if(!scf->omit_source_file_info) {
		      ui_perform(ssm->at);
		  }
# endif
		  ssmx = se_pull_ssm_string(top_ssm, ssm);
		  se_push_spair_string(ssmx);
	      }
    }
}
/* c------------------------------------------------------------------------ */

void dd_edd(time_series, automatic, down, d_ctr)
  int time_series, automatic, down;
  double d_ctr;
{
   /* c...mark */
   int ii, jj, mm, nn, nc,  mark, nx, g1, g2, num_segments=0, bflag;
   int boundary_exists, not_aligned, shift_bnd, old_usi_swp_count=-1;
   int new_bnd, size, airborne, stopped = NO;
   struct bnd_point_mgmt *bpm;
   long time_now();
   static int count=0;
   static char *cptrs[32], *deep6=0;
   char *aa, *bbuf, *return_dd_open_info(), *getenv();
   float r1, r2, rr, ff;
   static float anga=58., angb=63., angc=118., angd=123.;
   double range1, range2, drr, dalt;
   double d, rota, rotl;
   unsigned short *bnd;
   
   struct one_boundary *ob, *se_return_current_bnd();
   struct solo_str_mgmt *ssm;
   struct dd_input_sweepfiles_v3 *dis, *dd_return_sweepfile_structs_v3();
   struct boundary_stuff *sebs, *return_se_bnd_struct();
   
   struct unique_sweepfile_info_v3 *usi;
   struct dd_general_info *dgi, *dd_window_dgi();
   struct cell_d *celv;
   static struct point_in_space *radar=NULL;
   struct point_in_space *solo_malloc_pisp();
   struct solo_edit_stuff *seds, *return_sed_stuff();
   
   if(!radar) radar = solo_malloc_pisp();
   sebs = return_se_bnd_struct();
   seds = return_sed_stuff();
   dis = dd_return_sweepfile_structs_v3();
   dis->editing = YES;
   usi = dis->usi_top;
   dd_output_flag(YES);
   dgi = dd_window_dgi(usi->radar_num, "UNK");
   celv = dgi->dds->celvc;
   dgi->compression_scheme = HRD_COMPRESSION;
   dgi->disk_output = YES;
   slash_path(dgi->directory_name, usi->directory);
   seds->process_ray_count = seds->volume_count = seds->sweep_count = 0;
   seds->setup_input_ndx = 0;
   seds->modified = seds->finish_up = NO;
   ob = sebs->first_boundary;
   for(nn=0; ob; ob = ob->next) {
      if(ob->num_points > 2)
	nn += ob->num_points;
   }
   seds->boundary_exists = nn ? YES : NO;
   
   
   for(;;) {			/* for each ray */
      
      count++;

      if(solo_halt_flag()){
	  g_string_sprintfa (gs_complaints,"HALTING\n");
	  stopped = YES;
	  break;
      } 

      if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
	 break;
      }
      if(!seds->process_ray_count) { /* first time through
				      * tack on the seds cmds */
	 se_tack_on_cmds(&dgi->seds, &dgi->sizeof_seds);
	 
	 if(seds->boundary_exists) {
	    radar->latitude = dd_latitude(dgi);
	    radar->longitude = dd_longitude(dgi);
	    radar->altitude = dd_altitude(dgi);
	    radar->earth_radius = dd_earthr(radar->latitude);
	    radar->tilt = dd_tilt_angle(dgi);
	    radar->tilt = dgi->dds->swib->fixed_angle;
	    
	    dd_latlon_relative(sebs->origin, radar);
	    /*
	     * radar->x is now the x coordinate of the boundary origin
	     * relative to the radar
	     * check to see if the boundary origin and the radar origin
	     * are within 100m of each other
	     */
	    not_aligned = (SQ(radar->x) + SQ(radar->y)) > .1;
	    airborne = dgi->dds->radd->scan_mode == RHI ||
	      !(dgi->dds->radd->radar_type == GROUND ||
		dgi->dds->radd->radar_type == SHIP);
	    
	    for(ob = sebs->first_boundary; ob ; ob = ob->next) {
	       if(ob->num_points > 2) {
		  bpm = ob->top_bpm;
		  mm = ob->num_points;

		  for(; mm--; bpm = bpm->next) {
		     bpm->_x = bpm->x;
		     bpm->_y = bpm->y;
		  }
		  se_radar_inside_bnd(ob);
	       }
	    }
	 } /* boundary exists? */
      }	/* first time through */

      if(dgi->new_vol) {
	 dd_new_vol(dgi);
	 seds->volume_count++;
	 seds->volume_ray_count = 0;
	 dgi->new_sweep = YES;
      }
      if(dgi->new_sweep) {
	 seds->sweep_count++;
	 seds->sweep_ray_count = 0;
      }
      seds->process_ray_count++;
      seds->sweep_ray_count++;
      seds->volume_ray_count++;
      nc = dgi->clip_gate+1;
      seds->num_segments = 0;
      
      if(seds->boundary_exists) {
	 seds->use_boundary = YES;
	 ob = sebs->current_boundary = sebs->first_boundary;
	 rota = d = dd_rotation_angle(dgi);
	 drr = celv->dist_cells[celv->number_cells-1];
	 seds->boundary_mask = 
	   bnd = seds->boundary_mask_array;
	 nn = nc +1;
	 if(sebs->operate_outside_bnd) {
	    bflag = 0;
	    for(ii=0; ii < nn; *(bnd+ii++) = 1);
	 }
	 else {
	    bflag = 1;
	    memset(bnd, 0, nn * sizeof(*seds->boundary_mask));
	 }
	 if(dgi->new_sweep) {
	    radar->latitude = dd_latitude(dgi);
	    radar->longitude = dd_longitude(dgi);
	    radar->altitude = dd_altitude(dgi);
	    radar->earth_radius = dd_earthr(radar->latitude);
	    radar->tilt = dd_tilt_angle(dgi);
	    radar->tilt = dgi->dds->swib->fixed_angle;
	    radar->elevation = dd_elevation_angle(dgi);
	    radar->azimuth = dd_azimuth_angle(dgi);
	    radar->heading = dd_heading(dgi);

	    if(airborne) {
	       shift_bnd = NO;
	    }
	    else if(not_aligned) {
	       shift_bnd = YES;
	    }
	    else if(FABS(radar->tilt - sebs->origin->tilt) > .2) {
	       /* boundary and radar origin are the same but
		* not the same tilt
		*/
	       shift_bnd = YES;
	    }
	    else {
	       shift_bnd = NO;
	    }
	 }
	 /* for each boundary, set up the mask
	  */
	 for(ob = sebs->first_boundary; ob ; ob = ob->next) {
	    if(ob->num_points < 3)
	      continue;

	    if(dgi->new_sweep) {
	       if(shift_bnd && !time_series) {
		  se_shift_bnd(ob, sebs->origin, radar
			       , dgi->dds->radd->scan_mode
			       , dd_tilt_angle(dgi));
	       }
	       if(time_series || (not_aligned && !airborne)) {
		  /* see if radar inside this boundary
		   */
		  se_radar_inside_bnd(ob);
	       }
	    }
	    if(time_series) {
	       se_ts_find_intxns(dd_altitude(dgi), drr, ob
				 , dgi->time, ts_pointing_angle(dgi)
				 , automatic, down, d_ctr);
	    }
	    else {
	       xse_find_intxns(d, drr, ob);
	    }
	    xse_num_segments(ob);
	    seds->num_segments += ob->num_segments;

	    for(ii=0; ii < ob->num_segments; ii++) {
	       se_nab_segment(ii, &range1, &range2, ob);
	       if(range2 <= 0)
		 continue;
	       r1 = range1; r2 = range2;
	       g1 = dd_cell_num(dgi->dds, 0, r1);
	       g2 = dd_cell_num(dgi->dds, 0, r2) +1;

	       for(; g1 < g2; g1++) { /* set boundary flags */
		  *(bnd + g1) = bflag;
	       }
	    } /* end segments loop */

	 } /* end boundary for loop */
	 
      } /* boundary exists */

      else {			/* no boundary */
	 seds->use_boundary = NO;
	 seds->boundary_mask = seds->all_ones_array;
      }
      ssm = seds->fer_cmds;
      nn = seds->num_fer_cmds;

      /*
       * loop through the for-each-ray operations
       */
# ifdef obsolete
      for(; nn--; ssm=ssm->next) {
	 ui_perform(ssm->at);
	 if(seds->punt)
	   break;
      }
# else
      se_perform_cmds (seds->first_fer_cmd, seds->num_fer_cmds);
# endif

      if(seds->punt) {
	 break;
      }
      if(seds->modified) {
	 dd_dump_ray(dgi);

	 if (dgi->sweep_fid < 0) { /* unable to open the new sweepfile */
	    seds->punt = YES;
	    break;
	 }
	    
	 if(ddswp_last_ray(usi)) { /* end of current sweep */
	    dd_flush(usi->radar_num);	 
	 }
	 if(usi->ray_num <= 1) {
	    sp_sweep_file_check(usi->directory, usi->filename);
	 }
	 if(dgi->new_sweep) {
	    g_string_sprintfa (gs_complaints,"%s", return_dd_open_info());
	 }
      }
      dgi->new_sweep = dgi->new_vol = NO;
      
      if(usi->swp_count > usi->num_swps) {
	  break;
      }
   } /* end of loop through data */


   dis->editing = NO;

   if(seds->punt)
     return;
   seds->finish_up = YES;
   ssm = seds->fer_cmds;
   nn = seds->num_fer_cmds;
   /*
    * make a final pass through all operations in case they
    * need to finish up
    */
# ifdef obsolete
   for(; nn--; ssm=ssm->next) {
      ui_perform(ssm->at);
      if(seds->punt)
	break;
   }
# else
   se_perform_cmds (seds->first_fer_cmd, seds->num_fer_cmds);
# endif
   se_free_raqs();		/* free running average queues
				 * if any where set up */
   if(seds->modified) {
      seds->time_modified = time_now();
      ddir_rescan_urgent(seds->se_frame);
   }

   if(seds->boundary_exists) {	/* pack up the current boundary */
      size = se_sizeof_bnd_set();
      bbuf = (char *)malloc(size);
      memset(bbuf, 0, size);
      se_pack_bnd_set(bbuf);

      if(seds->last_pbs && size == seds->last_pbs->sizeof_set) {
	 /* see if this boundary is different from the last boundary
	  */
	 new_bnd = se_compare_bnds(seds->last_pbs->at, bbuf, size);
      }
      else {
	 new_bnd = YES;
      }

      if(new_bnd) {		/* put this boundary in the queue */
	 if(seds->num_prev_bnd_sets < 7) {
	    /* grow the circular queue till it reaches 7 boundaries
	     */
	    seds->num_prev_bnd_sets++;
	    seds->pbs = (struct prev_bnd_sets *)
	      malloc(sizeof(struct prev_bnd_sets));
	    memset(seds->pbs, 0, sizeof(struct prev_bnd_sets));
	    if(!seds->last_pbs) {
	       seds->pbs->last = seds->pbs->next = seds->pbs;
	    }
	    else {
	       seds->pbs->last = seds->last_pbs;
	       seds->pbs->next = seds->last_pbs->next;
	       seds->last_pbs->next->last = seds->pbs;
	       seds->last_pbs->next = seds->pbs;
	    }
	 }
	 else {
	    seds->pbs = seds->last_pbs->next;
	 }
	 seds->last_pbs = seds->pbs;
	 if(seds->pbs->at) free(seds->pbs->at);

	 seds->pbs->at = bbuf;
	 seds->pbs->sizeof_set = size;
      }
      if(!getenv("SOLO_DONT_CLEAR_BOUNDARIES"))
	se_clr_all_bnds();

      /* we should now be ready to draw the next boundary or to
       * retreive  the last boundary we just put in the queue
       */

   } /* end packing up current boundary */

   printf ("Finished!\n");
   return;
}
/* c------------------------------------------------------------------------ */

void se_edit_summary()
{
    /* saves the all the current editor commands 
     */
    int ii, nn, mark;
    char *dts_print(), str[88];
    DD_TIME dts, *d_unstamp_time();
    long tn, time_now();
    struct solo_str_mgmt *ssm, *ssmx, *se_clone_ssm(), *se_pop_spair_string();
    FILE *stream;
    struct solo_edit_stuff *seds, *return_sed_stuff();
    char *a;

    seds = return_sed_stuff();
    tn = time_now();
    se_push_all_ssms(&seds->edit_summary); 
    ssm = se_pop_spair_string();

    dts.time_stamp = time_now();
    /*
     * time stamp this set of cmds
     */
    sprintf(ssm->at, "!   time-now %s GMT\n", dts_print(d_unstamp_time(&dts)));
    se_append_string(&seds->edit_summary, ssm);
    /*
     * dump out the setup-input cmds
     */
    ssm = seds->setup_inputs[0];
    for(; ssm; ssm=ssm->next) {
	ssmx = se_clone_ssm(ssm);
	strcat(ssmx->at, "\n");
	se_append_string(&seds->edit_summary, ssmx);
    }
    /* dump out the current cmds
     */
    ssm = seds->once_cmds;
    for(; ssm; ssm=ssm->next) {
	ssmx = se_clone_ssm(ssm);
	strcat(ssmx->at, "\n");
	se_append_string(&seds->edit_summary, ssmx);
    }
    ssm = se_pop_spair_string();
    strcpy(ssm->at, "! for-each-ray\n");
    se_append_string(&seds->edit_summary, ssm);

    ssm = seds->fer_cmds;
    for(; ssm; ssm=ssm->next) {
	ssmx = se_clone_ssm(ssm);
	strcat(ssmx->at, "\n");
	se_append_string(&seds->edit_summary, ssmx);
    }
    return;
}
/* c------------------------------------------------------------------------ */

int se_gen_sweep_list()
{
    int ii,jj, nn, mark, rn;
    char str[64], *a, *mddir_radar_name_v3();
    struct solo_str_mgmt *ssm , *se_remove_ssm_string();
    struct dd_input_sweepfiles_v3 *dis, *dd_return_sweepfile_structs_v3();
    struct unique_sweepfile_info_v3 *usi;

    struct swp_file_input_control *sfic;
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();
    if(!seds->radar_stack)
	  return(0);

    seds->punt = NO;
    sfic = seds->sfic;
# ifdef obsolete
    if(sfic->clicked_frame) {
	/* update the sfic based on the current state of perusal frame
	 * zero implies to use the sfic as is.  i.e. the user
	 * has entered his own start and stop time.
	 */
       se_set_sfic_from_frame(sfic->clicked_frame-1);
       se_refresh_sfic_widget(seds->sfic);
    }
# endif
    ddir_rescan_urgent(seds->se_frame);
    if((nn=ddir_files_v3(SE_FRAME, sfic->directory)) < 1) {
	g_string_sprintfa (gs_complaints,"No sweep files in dir: %s\n", sfic->directory);
	seds->punt = YES;
	return(nn);
    }
    /* nab next radar name
     */
    ssm = se_remove_ssm_string(&seds->radar_stack, 0);
    strcpy(sfic->radar_name, ssm->at);
    se_push_spair_string(ssm);

    if((rn = mddir_radar_num_v3(SE_FRAME, sfic->radar_name)) < 0) {
	g_string_sprintfa (gs_complaints,"No sweep files for radar: %s\n", sfic->radar_name);
	seds->punt = YES;
	return(rn);
    }
    a = mddir_radar_name_v3(SE_FRAME, rn);
    strcpy(sfic->radar_name, a);

    dis = dd_return_sweepfile_structs_v3();
    usi = dis->usi_top;
    usi->dir_num = usi->radar_num = seds->se_frame;
    usi->ddir_radar_num = sfic->radar_num = rn;
    usi->version = sfic->version;
    usi->new_version_flag = sfic->new_version_flag;
    strcpy(usi->directory, sfic->directory);

    if((nn=dd_sweepfile_list_v3(usi, sfic->start_time, sfic->stop_time))
       < 0) {
	g_string_sprintfa (gs_complaints,"Unable to produce sweep file list for radar: %s in %s\n"
		  , sfic->radar_name, sfic->directory);
	seds->punt = YES;
	return(nn);
    }

    return(nn);
}
/* c------------------------------------------------------------------------ */

struct ui_cmd_mgmt *se_malloc_ucm()
{
  struct ui_cmd_mgmt *ucm;
  struct ui_command *cmds;
  int ii, nn;
  int llen = 128, max_tokens = 66, token_len = 48;

  ucm = (struct ui_cmd_mgmt *)malloc (sizeof(struct ui_cmd_mgmt));
  memset (ucm, 0, sizeof(struct ui_cmd_mgmt));
  ucm->max_tokens = max_tokens;
  ucm->max_token_size = token_len;

  ucm->cmd_line = (char *)malloc(llen);
  memset( ucm->cmd_line, 0, llen);

  ucm->cmds = cmds = 
    (struct ui_command *)malloc(max_tokens * sizeof(struct ui_command));
  memset(cmds, 0, max_tokens * sizeof(struct ui_command ));
  
  for (ii=0; ii < max_tokens; ii++, cmds++) {
    cmds->uc_text = (char * )malloc( token_len );
    memset( cmds->uc_text, 0, token_len );
  }

  return ucm;
}

/* c------------------------------------------------------------------------ */

int se_interpret_commands (struct solo_str_mgmt *first_ssm
			   , struct ui_cmd_mgmt *first_ucm, int *cmds_count)
{
  static char *buf = NULL;
  static int buf_size = 0;
  static struct ui_command *cmd, *cmds;
  struct solo_edit_stuff *seds, *return_sed_stuff();
  struct solo_str_mgmt *ssm;
  struct ui_cmd_mgmt *ucm = first_ucm, *prev_ucm;
  char *aa, *bb, *cc, str[512], *lines[64], *sptrs[32];
  int ii, jj, kk, mm = 0, nn = 0, nlines=0, nt, max, ok = YES;

  *cmds_count = 0;
  
  seds = return_sed_stuff();
  
  for(ssm=first_ssm; ssm ; ssm=ssm->next, nlines++);
  
  if(ucm->max_cmds < nlines) {
    for (ii=0; ii < nlines-1; ii++) {
       if (ii < first_ucm->max_cmds-1)
	 { ucm = ucm->next; }
       else {
	  ucm->next = se_malloc_ucm();
	  ucm = ucm->next;
       }
    }
    ucm = first_ucm;
    ucm->max_cmds = nlines;
  }
  ssm = first_ssm;

  for (ii=0; ii < nlines; ii++, ssm=ssm->next) {
    cc = ssm->at;
    kk = strlen (cc);
    if (kk > 79)
      { kk = 79; }

    strncpy(ucm->cmd_line, cc, kk);
    ucm->cmd_line[kk] = '\0';
    
    /* see if we can break the command into tokens
     * and see if it is a recognizable command */
    
    if (!ok_cmd (ssm->at, ucm)) {
      ok = NO;
      sii_message (ucm->error);
    }
    else if (!sii_cmd_tmplt_tokens (ucm)) {
      /* see if it has the right tokens */
      ok = NO;
      sii_message (ucm->error);
    }
    else
      { (*cmds_count)++; ucm = ucm->next; }
  }
  return ok;
}

/* c------------------------------------------------------------------------ */

int se_text_to_ssms (char *txt, struct solo_str_mgmt **top_ssm)
{
  static char *buf = NULL;
  static int buf_size = 0;
  struct solo_str_mgmt *ssm, *se_pop_spair_string();
  int ii, nc=0, nn, nt;
  char *aa, *lines[128], str[256], *sptrs[66];
 

  nn = strlen (txt);
  if (nn+1 > buf_size) {
    if (buf)
      { free (buf); }
    buf_size = (int)(1.5 * nn);
    buf = (char *)malloc (buf_size);
    memset (buf, 0, buf_size);
  }
  strcpy (buf, txt);
  nn = dd_tokenz (buf, lines, "\n");
  
  for (ii=0; ii < nn; ii++) {
     if (aa = strchr (lines[ii], '!')) /* comment */
	{ *aa = '\0'; }

     strcpy (str, lines[ii]);
     nt = dd_tokens (str, sptrs);
     if (nt < 1)		/* no tokens in line */
       { continue; }

    ssm = se_pop_spair_string();
    strcpy (ssm->at, lines[ii]);
    se_append_string(top_ssm, ssm);
     nc++;
  }
  return nc;
}

/* c------------------------------------------------------------------------ */

int sii_cmd_tmplt_tokens (struct ui_cmd_mgmt *the_ucm)
{
   int jj, kk, mm=0, nn=0, nt = 0, nx, ok = YES, noks=0;
   int prev_uc_ctype;
   char *aa, *bb, *ee, str[256], *sptrs[32], mess[384];
   struct ui_command *cmds = the_ucm->cmds;
   float ff;

   aa = the_ucm->tmplt;
   bb = str;
   ee = mess;
   *ee = '\0';

   /* isolate the variable symbols */

   for (; *aa; aa++) {
     if (*aa == ' ')
       { continue; }
     if (*aa == '<') {
       mm = 1;
       sptrs[nt++] = bb;
     }
     if (mm)
       { *bb++ = *aa; }
     if (*aa == '>') {
       mm = 0;
       *bb++ = '\0';
     }
   }
   cmds++;
   strcpy (ee, the_ucm->cmd_line);
   ee += strlen(ee);
   nx = the_ucm->num_tokens -1;

   if (nx < nt) {
      ok = NO;
      sprintf (ee, "\nmissing at least %d argument(s)"
	       , nt -nx);
      ee += strlen(ee);
      noks++;
   }
   else if (!the_ucm->xtra_tokens && nx > nt) {
      ok = NO;
      sprintf (ee, "\n%d extra tokens not expected"
	       , nx -nt);
      ee += strlen(ee);
      noks++;
      nx = nt;
   }

   for (kk=0; kk < nx; kk++, cmds++, ok = YES) {
      /*
       * (kk >= nt) => extra optional arguments of the same type as the last
       */

      if ((kk >= nt && prev_uc_ctype == UTT_WHERE))
	{
	   ok = NO;
	   noks++;
	   sprintf (ee, "/n%s should not be a <where> ", cmds->uc_text);
	   ee += strlen(ee);
	}
      else if ((kk >= nt && prev_uc_ctype == UTT_FIELD) ||
	       (kk < nt && strstr( sptrs[kk], "<field>" )))
	{
	   /* already defined as a keyword type */
	   if (kk < nt)
	     { prev_uc_ctype = UTT_FIELD; }
	}
      else if((kk >= nt && prev_uc_ctype == UTT_INT) ||
	      (kk < nt && strstr( sptrs[kk], "<integer>" )))
	{
	   jj = sscanf (cmds->uc_text, "%d", &nn);
	   if (jj == 1) {
	      cmds->uc_v.us_v_int = nn;
	      cmds->uc_ctype = UTT_VALUE;
	      if (kk < nt)
		{ prev_uc_ctype = UTT_INT; }
	   }
	   else {
	      ok = NO;
	      noks++;
	      sprintf (ee, "\n%s is not an <integer> ", cmds->uc_text);
	      ee += strlen(ee);
	   }
	}
      else if((kk >= nt && prev_uc_ctype == UTT_REAL) ||
	      (kk < nt && strstr( sptrs[kk], "<real>" )))
	{
	   jj = sscanf (cmds->uc_text, "%f", &ff);
	   if (jj == 1) {
	      cmds->uc_v.us_v_float = ff;
	      cmds->uc_ctype = UTT_VALUE;
	      if (kk < nt)
		{ prev_uc_ctype = UTT_REAL; }
	   }
	   else {
	      ok = NO;
	      noks++;
	      sprintf (ee, "\n%s is not a <real> ", cmds->uc_text);
	      ee += strlen(ee);
	   }
	}
      else if( strstr( sptrs[kk], "<where>" )) {
	 /* see if it's one of the wheres (above, below, between)*/
	 if (strncmp (cmds->uc_text, "above", 2) == 0 ||
	     strncmp (cmds->uc_text, "below", 3) == 0 ||
	     strncmp (cmds->uc_text, "between", 3) == 0 )
	   {
	      prev_uc_ctype = UTT_WHERE;
	      if (strncmp (cmds->uc_text, "between", 3) == 0 && kk >= nx -2) {
		 ok = NO;
		 noks++;
		 sprintf (ee, "\nthere needs to be 2 arguments following %s"
			  , cmds->uc_text);
		 ee += strlen(ee);
	      }
	   }
	 else {
	    ok = NO;
	    noks++;
	    sprintf (ee, "\n%s is not a <where> (above,below,between)"
		     , cmds->uc_text);
	    ee += strlen(ee);
	 }
      }
   }
   if (noks) {
      if (!the_ucm->error) {
	 the_ucm->error = (char *)malloc(se_ucm_errlen());
	 memset (the_ucm->error, 0, se_ucm_errlen());
      }
      strcpy (the_ucm->error, mess);
   }
   return noks == 0;
}

/* c------------------------------------------------------------------------ */

int se_perform_cmds (struct ui_cmd_mgmt *the_ucm, int num_cmds) 
{
  typedef int (*UI_FUNC)(int, struct ui_command *);
  
  struct ui_cmd_mgmt *ucm = the_ucm;
  int nn = num_cmds;
  struct solo_edit_stuff *seds, *return_sed_stuff();

  seds = return_sed_stuff();
  if (num_cmds < 1)
    { return 1; }
  
  for (; nn--; ucm = ucm->next) {
    (*ucm->cmd_proc)(ucm->num_tokens, ucm->cmds);
    if (seds->punt)
      { break; }
  }
  return 1;
}

/* c------------------------------------------------------------------------ */

int se_process_data(arg, cmds, time_series, automatic, down, d_ctr, frame_num)
  int arg, frame_num;
  struct ui_command *cmds;
  int time_series, automatic, down;
  double d_ctr;
{
    int ii, jj, nn, mark, ncmds, item_num, nc;
    int oto_count, fer_count;
    short yy, mon, dd, hh, mm, ss, ms;
    double d, d_time_stamp();
    DD_TIME dts;
    char *a, *get_tagged_string(), str[88], *se_unquote_string(), *fgetz();
    struct solo_str_mgmt *ssm, *ssmx, *ssmy, *ssm_fer, *se_pop_spair_string()
	  , *se_bracket_procedure();
    struct solo_str_mgmt *se_clone_ssm();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    FILE *stream;
    struct ui_cmd_mgmt *ucm;


    g_string_truncate (gs_complaints, 0);
    dont_print_dd_open_info();
    seds = return_sed_stuff();
    seds->punt = NO;

    se_push_all_ssms(&seds->fer_cmds); 
    fer_count = 0;
    if (nc = se_text_to_ssms (seds->fer_lines, &seds->fer_cmds)) {
       if (!se_interpret_commands (seds->fer_cmds, seds->first_fer_cmd
				   , &fer_count))
	 { fer_count = 0; }
    }
    seds->num_fer_cmds = fer_count;

    if(!fer_count) {
# ifdef obsolete
	sii_message(" No for-each-ray commands ");
# endif
	seds->punt = YES;
	return(TRUE);
    }

    se_push_all_ssms(&seds->once_cmds); 
    oto_count = 0;
    if (nc = se_text_to_ssms (seds->oto_lines, &seds->once_cmds)) {
       if (!se_interpret_commands (seds->once_cmds, seds->first_oto_cmd
				, &oto_count)) {
	  seds->punt = YES;
	  return(TRUE);
       }
    }
    seds->num_once_cmds = oto_count;

    /*
     * generate the setup-input cmds and loop through them
     */
    se_dump_sfic_widget(seds->sfic, frame_num);
    se_setup_input_cmds();

    seds->num_radars = 1;	/* just do one radar at a time now */
    se_push_all_ssms (&seds->radar_stack);
    ssm = se_pop_spair_string ();
    strcpy (ssm->at, seds->sfic->radar_names_text);
    se_append_string (&seds->radar_stack, ssm);

    ssm = seds->setup_inputs[0];
    se_setup_input_cmds();
# ifdef never_again
    ii = se_interpret_commands (ssm, seds->first_input_cmd, &jj);
# endif
    seds->num_setup_input_cmds = jj;


    /* save all the edit commands in the edit summary list
     * and dump them to a temporary file
     */
    se_edit_summary();
    /*
     * loop through the stack of radars
     */
    for(;;) {
	/*
	 * try to create a list of sweeps for the next radar
	 */
	if(!(se_gen_sweep_list()) || seds->punt) {
	    if(!seds->batch_mode) {
		se_set_sfic_from_frame(seds->popup_frame);
		se_refresh_sfic_widget(seds->sfic);
	    }
	    return(TRUE);
	}
	/*
	 * bracket and execute the one-time commands procedure
	 */
	se_perform_cmds (seds->first_oto_cmd, seds->num_once_cmds);
	/*
	 * now try to loop through it
	 */
	dd_edd(time_series, automatic, down, d_ctr);
	mark = 0;
    }
    if (gs_complaints->len)
      { sii_message (gs_complaints->str); }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_really_readin_cmds()
{
    int ii, nn, mark, ncmds, item_num;
    short yy, mon, dd, hh, mm, ss, ms;
    double d, d_time_stamp();
    DD_TIME dts;
    char *a, *get_tagged_string(), str[256], *se_unquote_string(), *fgetz();
    struct sed_command_files *scf, *se_return_cmd_files_struct();
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct solo_str_mgmt *ssmx, *se_bracket_procedure()
	  , *se_remove_ssm_string();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    FILE *stream;

    seds = return_sed_stuff();
    scf = se_return_cmd_files_struct();

    slash_path(str, scf->directory_text);
    strcat(str, scf->file_name_text);
# ifdef obsolete
    if(*scf->comment_text) {
	strcat(str, ".");
	strcat(str, scf->comment_text);
    }
# endif
    se_absorb_strings(str, &seds->scratch1);
    se_cull_setup_input(&seds->scratch1);
    /*
     * move scratch1 to cmdz
     */
    se_push_all_ssms(&seds->cmdz);
    seds->cmdz = seds->scratch1;
    seds->scratch1 = NULL;

    if(!seds->batch_mode)
	  se_refresh_sfic_widget(seds->sfic);
    return(1);
}
/* c------------------------------------------------------------------------ */

void se_setup_input_cmds()
{
    /* this routine takes the contents of the sfic widget and constructs
     * input control commands from it
     */
    int ii, nn, nt, mark, ncmds, item_num;
    struct solo_str_mgmt *ssm, *se_pop_spair_string(), *se_remove_ssm_string();
    struct solo_str_mgmt *ssmx, *ssmy, *ssmz, *se_bracket_procedure();
    struct solo_str_mgmt *se_clone_ssm();
    struct swp_file_input_control * sfic;
    struct solo_edit_stuff *seds, *return_sed_stuff();
    char *a, *b, *c, *e, str[88], *sptrs[16], *dd_whiteout(), *dd_delimit();
    char *dts_print(), *dq="\"";
    DD_TIME dts, *d_unstamp_time();

    seds = return_sed_stuff();
    sfic = seds->sfic;
    se_push_all_ssms(&seds->scratch1); /* clear a scratch list */
    ssm = se_pop_spair_string();
    c = ssm->at;
    strcpy(c, "    sweep-directory ");
    strcat(c, dq);
    strcat(c, sfic->directory_text);
    slash_path(seds->last_directory, sfic->directory_text);
    strcat(c, dq);
    se_append_string(&seds->scratch1, ssm);


    ssm = se_pop_spair_string();
    c = ssm->at;
    strcpy(c, "    radar-names ");
    strcat(c, sfic->radar_names_text);
    se_append_string(&seds->scratch1, ssm);

    se_push_all_ssms(&seds->radar_stack);
    se_push_all_ssms(&seds->last_radar_stack);
    /*
     * also create the list of radar names for processing
     */
    strcpy( str, sfic->radar_names_text);
    nt = dd_tokens( str, sptrs );

    for( ii = 0; ii < nt; ii++ ) {
      ssm = se_pop_spair_string();
      strcpy( ssm->at, sptrs[ii] );
      se_append_string(&seds->radar_stack, ssm);
      se_append_string(&seds->last_radar_stack, se_clone_ssm(ssm));

    }
    ssm = se_pop_spair_string();
    c = ssm->at;
    strcpy(c, "    first-sweep ");
    if(strstr(sfic->first_sweep_text, "first")) {
	strcat(c, "first");
    }
    else if(strstr(sfic->first_sweep_text, "last")) {
	strcat(c, "last");
    }
    else {
	dts.time_stamp = sfic->start_time;
	d_unstamp_time(&dts);
	sprintf(c+strlen(c), "%02d/%02d/%02d:%02d:%02d:%02d.%03d"
		, dts.month
		, dts.day
		, dts.year-1900
		, dts.hour
		, dts.minute
		, dts.second
		, dts.millisecond
		);
    }
    se_append_string(&seds->scratch1, ssm);

    ssm = se_pop_spair_string(); c = ssm->at;
    strcpy(c, "    last-sweep ");
    if(strstr(sfic->last_sweep_text, "first")) {
	strcat(c, "first");
    }
    else if(strstr(sfic->last_sweep_text, "last")) {
	strcat(c, "last");
    }
    else {
	dts.time_stamp = sfic->stop_time;
	d_unstamp_time(&dts);
	sprintf(c+strlen(c), "%02d/%02d/%02d:%02d:%02d:%02d.%03d"
		, dts.month
		, dts.day
		, dts.year-1900
		, dts.hour
		, dts.minute
		, dts.second
		, dts.millisecond
		);
    }
    se_append_string(&seds->scratch1, ssm);

    ssm = se_pop_spair_string(); c = ssm->at;
    strcpy(c, "    version ");
    if(strstr(sfic->version_text, "first")) {
	strcat(c, "first");
    }
    else if(strstr(sfic->version_text, "last")) {
	strcat(c, "last");
    }
    else {
	sprintf(c+strlen(c), "%d", sfic->version);
    }
    se_append_string(&seds->scratch1, ssm);

    ssm = se_pop_spair_string();
    if(sfic->new_version_flag) {
	strcpy(ssm->at, "new-version");
    }
    else {
	strcpy(ssm->at, "no-new-version");
    }
    se_append_string(&seds->scratch1, ssm);
    seds->last_new_version_flag = sfic->new_version_flag;
    /* now pop this on top of the setup_inputs stack
     */
    se_shift_setup_inputs(seds->scratch1);

    seds->scratch1 = NULL;
    return;
}
/* c------------------------------------------------------------------------ */

void se_shift_setup_inputs(ssm)
  struct solo_str_mgmt *ssm;
{
    /* routine to pop the current set of setup-input cmds on top
     * of the stack of previous inputs
     */
    int ii, nn=MAX_SETUP_INPUTS, mark;
    struct solo_str_mgmt **ssmptr;
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();
    ssmptr = &seds->setup_inputs[--nn];
    se_push_all_ssms(ssmptr);	/* if there are cmds here,
				 * push them onto the spairs stack
				 */
    /* now move them all down one
     */
    for(; nn--; *ssmptr = *(ssmptr-1), ssmptr--) {
	mark = 0;
    }
    *ssmptr = ssm;
}
/* c------------------------------------------------------------------------ */

void se_tack_on_cmds(where, how_much)
  char **where;
  int *how_much;
{
    int ii, mm, nn, mark, size;
    char *a, *b, *c;
    struct generic_descriptor *gd;
    struct solo_str_mgmt *ssm;
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();
				/* add up the number of chars to be appended */
    ssm = seds->edit_summary;
    for(size=0; ssm; ssm=ssm->next)
	  size += strlen(ssm->at);


    mm = LONGS_TO_BYTES(BYTES_TO_LONGS(size)); /* force size to an integer
						* number of longs */
    /* don't save more than 64K of edit information
     */
    if(*how_much + mm >= K64) { return; }

    nn = mm - size;		/* padding */

    if(*how_much <= 0) {
	mm += sizeof(struct generic_descriptor);
	*where = (char *)malloc(mm);
	c = *where +sizeof(struct generic_descriptor);
	gd = (struct generic_descriptor *)(*where);
	strncpy(gd->name_struct, "SEDS", 4);
    }
    else {
	mm += *how_much;
	*where = (char *)realloc(*where, mm);
	c = *where + (*how_much);
	gd = (struct generic_descriptor *)(*where);
    }
    *how_much = gd->sizeof_struct = mm;
    ssm = seds->edit_summary;

    for(; ssm; ssm=ssm->next) {
	for(b=ssm->at; *b; *c++ = *b++); /* copy this line */
    }

    if(nn) {			/* need to pad it out with blanks */
	for(nn--; nn--; *c++ = ' ');
	*c++ = '\n';
    }
    mark = 0;
}
/* c------------------------------------------------------------------------ */

double ts_pointing_angle(dgi, wn)
  struct dd_general_info *dgi;
  int wn;
{
    double d_rot;

    switch(dgi->dds->radd->scan_mode) {
	
    case AIR:
	d_rot = dd_tilt_angle(dgi);
	break;
    case RHI:
	d_rot = dd_azimuth_angle(dgi);
	break;
    case TAR:
    case VER:
	d_rot = dd_rotation_angle(dgi);
	break;
    default:
	d_rot = dd_tilt_angle(dgi);
	break;
    }
    return(d_rot);
}
/* c------------------------------------------------------------------------ */

void se_write_sed_cmds()
{
    /* saves the editor commands before beginning a pass through
     * the data
     */
    int ii, nn, mark;
    char *a, *b, *c, *dts_print(), str[256], radar_name[12];
    char *dd_whiteout(), *dd_delimit();
    DD_TIME dts, *d_unstamp_time();
    long tn, time_now();
    struct solo_str_mgmt *ssm;
    FILE *stream;
    struct sed_command_files *scf, *se_return_cmd_files_struct();
    struct swp_file_input_control *sfic, *return_swp_fi_in_cntrl();
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();
    sfic = return_swp_fi_in_cntrl();
    scf = se_return_cmd_files_struct();
    tn = time_now();
    /*
     * create the file name, append the comment and open it
     */
    se_dump_sed_files_widget(scf);
    if(!(*scf->directory_text)) { /* get the dir name from the sfic */
	strcpy(scf->directory_text, sfic->directory_text);
    }
    slash_path(str, scf->directory_text);
    if(seds->last_radar_stack && strlen(seds->last_radar_stack->at)) {
	strcpy(radar_name, seds->last_radar_stack->at);
    }
    else if(strlen(sfic->radar_names_text)) {
	a = dd_whiteout(sfic->radar_names_text);
	b = dd_delimit(a);
	for(c=radar_name; a < b; *c++ = *a++); *c++ = '\0';
    }
    else {
	strcpy(radar_name, "UNK");
    }
    a = str +strlen(str);
    dd_file_name("sed", tn, radar_name, getuid(), a);
    strcpy(scf->file_name_text, a);

    if(strlen(scf->comment_text)) {
	se_fix_comment(scf->comment_text);
	strcat(str, ".");
	strcat(str, scf->comment_text);
    }
    se_refresh_sed_files_widget(scf);

    if(!(stream = fopen(str, "w"))) {
	g_string_sprintfa (gs_complaints,"Unable to save editor commands to %s\n", str);
	return;
    }

    ssm = seds->edit_summary;
    for(; ssm; ssm=ssm->next) {
	nn = fputs(ssm->at, stream);
	if(nn <= 0 || nn == EOF) {
	    /* we have an error */
	    break;
	}
    }
    fclose(stream);
    return;
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */
