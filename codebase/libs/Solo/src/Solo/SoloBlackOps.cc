
//
// SoloBlackOps is the BlackOps box for Soloii functions and boundaries
//  SoloBlackOps(function(ray,bounday))
//

void SoloBlackOps::addBoundary() {}
void SoloBlackOps::deleteBoundary() {}

float *SoloBlackOps::applyFunction(string functionName, float *ray) { 
  //if (functionName.compare("remove_ac_motion") == 0) {
  // actually, use a dictionary to look up name, function
  // short *bnd;
  // return f(bnd, ray);
  return NULL;
}

//
// calls dd_edd to set boundary flag for given ray data & boundary sets
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

    seds->num_radars = 1;       /* just do one radar at a time now */
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

