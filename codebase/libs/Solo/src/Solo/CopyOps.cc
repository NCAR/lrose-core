/* c------------------------------------------------------------------------ */

int se_cpy_field(arg, cmds)
  int arg;
  struct ui_command *cmds;
{
    /*                                                                                                                              
     * #duplicate#                                                                                                                  
     * #establish-and-reset#                                                                                                        
     * #copy#                                                                                                                       
     * #flagged-copy#                                                                                                               
     * #shift-field#                                                                                                                
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    char *src_name, *dst_name;
    struct dds_structs *dds;
    int ii, jj, nn, fn, fns, fnd, size=0, mark, ns, nd, nc, bad, rescale=NO;
    int fshift, src_bad, dst_bad, inc;
    char *a=NULL, *b=NULL;
    unsigned short *bnd, *flag;
    short *ss, *tt, *zz;
    float srs_scale, rcp_scale, srs_bias, dst_scale, dst_bias, x;

    src_name = (cmdq++)->uc_text;
    dst_name = (cmdq++)->uc_text;
    ns = strlen(src_name);
    nd = strlen(dst_name);

    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dds->celv->number_cells;
    bnd = seds->boundary_mask;

    if((ii=se_establish_field(dgi, src_name, dst_name, &fns, &fnd)) < 0) {
        seds->punt = YES;
        return(ii);
    }
    if(dgi->dds->parm[fns]->parameter_scale !=
       dgi->dds->parm[fnd]->parameter_scale ||
       dgi->dds->parm[fns]->parameter_bias !=
       dgi->dds->parm[fnd]->parameter_bias)
      {
        srs_scale = dgi->dds->parm[fns]->parameter_scale;
        dst_scale = dgi->dds->parm[fnd]->parameter_scale;
        srs_bias = dgi->dds->parm[fns]->parameter_bias;
        dst_bias = dgi->dds->parm[fnd]->parameter_bias;
        rescale = YES;
        rcp_scale = 1./srs_scale;
      }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fns];
    tt = (short *)dds->qdat_ptrs[fnd];
# else
    ss = (short *)((char *)dds->rdat[fns] + sizeof(struct paramdata_d));
    tt = (short *)((char *)dds->rdat[fnd] + sizeof(struct paramdata_d));
# endif
    bad = dds->parm[fnd]->bad_data;
    zz = ss +nc;

    /*                                                                                                                              
     * here's where we finally do the copying                                                                                       
     */
    if(strncmp(cmds->uc_text, "duplicate", 3) == 0) {
      if(rescale) {
        for(; ss < zz; ss++,tt++) { /* dont use boundary */
          if(*ss == bad) {
            *tt = bad;
          }
          else {
            x = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
            *tt = DD_SCALE(x, dst_scale, dst_bias);
          }
        }
      }
      else {
        for(; ss < zz; ss++,tt++) { /* dont use boundary */
          *tt = *ss;
        }
      }
    }

    else if(strncmp(cmds->uc_text, "shift-field", 4) == 0) {
        fshift = seds->gates_of_shift;

        ii = fshift > 0 ? nc -1 : 0;
        inc = fshift > 0 ? -1 : 1;
        nn = fshift > 0 ? nc - fshift : nc + fshift;

        if(!rescale) {
            for( ; nn-- ; ii += inc ) {
                *( tt + ii ) = *( ss + ii - fshift );
            }
        }
        else {
            for( ; nn-- ; ii += inc ) {
                if(*( ss + ii - fshift ) == bad) {
                    *( tt + ii ) = bad;
                }
                else {
                    x = DD_UNSCALE((float)( *ss + ii - fshift )
                                   , rcp_scale, srs_bias);
                    *( tt + ii ) = DD_SCALE(x, dst_scale, dst_bias);
                }
            }
        }
        /* fill in at whichever end                                                                                                 
         */
        ii = fshift < 0 ? nc -1 : 0;
        inc = fshift < 0 ? -1 : 1;
        nn = fshift < 0 ? -fshift : fshift;

        for( ; nn-- ; ii += inc ) {
            *( tt + ii ) = bad;
        }
    }

    else if(strncmp(cmds->uc_text, "establish-and-reset", 3) == 0) {
        zz = tt +nc;
        for(; tt < zz; *tt++ = bad); /* just fill with bad flags */
    }

    else if(strncmp(cmds->uc_text, "flagged-copy", 3) == 0) {
      /* copy using flags and boundary */
      seds->bad_flag_mask = flag = seds->bad_flag_mask_array;

      for(; ss < zz; ss++,tt++,bnd++,flag++) {
# ifdef perhaps
        if(*bnd && *ss != bad && *flag) {
# else
        if(*bnd && *flag) {     /* copies everything including missing data */
# endif
          if(rescale) {
            x = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
            *tt = DD_SCALE(x, dst_scale, dst_bias);
          }
          else {
            *tt = *ss;
          }
        }
      }
    }

    else {                      /* copy using boundary */
      for(; ss < zz; ss++,tt++,bnd++) {
        if(*bnd) {
          if(rescale) {
            if(*ss == bad) {
              *tt = bad;
            }
            else {
              x = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
              *tt = DD_SCALE(x, dst_scale, dst_bias);
            }
          }
          else {
            *tt = *ss;
          }
        }
      }
    }
    return(1);
}

