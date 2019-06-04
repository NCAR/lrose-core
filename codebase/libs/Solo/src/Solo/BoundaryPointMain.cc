// Contains the operations for boundaries

// code extracted from soloii with these settings:
// # define OLD_XNSx  <== OLD_XNS is NOT defined.
// #define obsolete   <== obsolete is NOT defined.

// trying to figure out the Solo boundary point code

// There must be a list of points, that get translated from display coordinates into
// radar coordinates, then some list of points that are either inside the boundary
// or outside the boundary.
// TODO: how do we know if a point in a RadxVol is inside or outside a boundary?

// The data structures are changed from Soloii.  
// No globals
// No sed structure
//
//

  // determine if point is left or right of line 
int xse_ccw(x0, y0, x1, y1)
  double x0, y0, x1, y1;
{
  /* is point 0 left or right of a line between the origin and point 1                                      
   * form a line between the origin and point 0 called line 0                                               
   */
  if(y0 * x1 > y1 * x0)
    return(1);
  /*                                                                            
   * this says the slope of line 0                                              
   * is greater than the slope of line 1                                        
   * and is counter-clockwise or left                                           
   * of the line.                                                               
   */
  if(y0 * x1 < y1 * x0)
    return(-1);
  /* cw or right */
  return(0);                  /* on the line */
}

/* c------------------------------------------------------------------------ */

int xse_set_intxn(x, y, slope, bpm, ob)
  double x, y;
  double slope;
  struct bnd_point_mgmt *bpm;
  struct one_boundary *ob;
{
    /* compute the intersection of a ray and a boundary segment
     * x & y are the endpoints of the ray and "slope" is
     * the slope of the ray and "bpm" represents the boundary segment
     *
     * when we are doing intersections, we want to use the shifted
     * x,y values because the boundary may be for another radar
     */
    struct bnd_point_mgmt *bpmx;
    double dd, xx, yy, sqrt();

    /*
     * first compute the x coordinate of the intersection
     */
    if(!x) {                    /* the ray is vertical */
        xx = 0;
    }
    if(!bpm->dx) {              /* the segment is vertical */
	xx = bpm->_x;
    }
    else {                      /* remember the origin of the ray is (0,0) */
        xx = (-bpm->slope*bpm->_x +bpm->_y)/(slope -bpm->slope);
    }
    if(x < 0) {                 /* check for an imaginary intersection */
        if(xx < x || xx > 0)
              return(NO);
    }
    else if(xx < 0 || xx > x)
          return(NO);

    if(!y) {                    /* the ray is horizontal */
        yy = 0;
    }
    else if(!bpm->dy) {         /* the segment is horizontal */
        yy = bpm->_y;
    }
    else {
        yy = slope*xx;
    }
    if(y < 0) {                 /* check for an imaginary intersection */
        if(yy < y || yy > 0)
              return(NO);
    }
    else if(yy < 0 || yy > y)
          return(NO);

    ob->num_intxns++;
    bpm->rx = sqrt(((double)SQ(xx)+(double)SQ(yy)));

    bpm->next_intxn = NULL;

    if(!(bpmx = ob->first_intxn)) {     /* first intersection */
        ob->first_intxn = bpm->last_intxn = bpm;
                                /* first intxn always points to
                                 * the last intxn tacked on */
        return(YES);
    }
    /*
     * insert this intxn and preserve the order
     */
    for(; bpmx; bpmx = bpmx->next_intxn) {
        if(bpm->rx < bpmx->rx) {
                                /* insert intxn here */
            bpm->next_intxn = bpmx;
            bpm->last_intxn = bpmx->last_intxn;
            if(bpmx == ob->first_intxn) { /* new first intxn */
                ob->first_intxn = bpm;
            }
            else {
                bpmx->last_intxn->next_intxn = bpm;
            }
            bpmx->last_intxn = bpm;
            break;
        }
    }
    if(!bpmx) {                 /* furthest out...tack it onto the end */
        ob->first_intxn->last_intxn->next_intxn = bpm;
        bpm->last_intxn = ob->first_intxn->last_intxn;
        ob->first_intxn->last_intxn = bpm;
    }
    return(YES);
}



/* c------------------------------------------------------------------------ */

int se_radar_inside_bnd(OneBoundary *ob)
{
  /* determine if the radar is inside or outside the boundary                                               
   */
  BoundaryPointManagement *bpm, *bpmx;
  double dd, sqrt(), atan2(), fmod();
  double r, x, y, theta;
  int ii, nBoundaryPoints = ob->num_points-1, nn, mark, inside_count=0;


  if(ob->bh->force_inside_outside) {
    if(ob->bh->force_inside_outside == BND_FIX_RADAR_INSIDE) {
      return(ob->radar_inside_boundary = YES);
    }
    if(ob->bh->force_inside_outside == BND_FIX_RADAR_OUTSIDE) {
      return(ob->radar_inside_boundary = NO);
    }
  }

  bpm = ob->top_bpm;
  x = ABS(bpm->_x);
  y = ABS(bpm->_y);
  /*                                                                                                        
   * we are using the shifted values of x and y in case this boundary                                       
   * is for a different radar                                                                               
   */
  for(bpm = bpm->next; nBoundaryPoints--; bpm = bpm->next) {
    if(ABS(bpm->_x) > x) x = ABS(bpm->_x);
    if(ABS(bpm->_y) > y) y = ABS(bpm->_y);
  }
  x += 11;
  y += 11;

  for(ii=0; ii < 4; ii++) {
    switch(ii) {
      case 1:
        x = -x;             /* x negative, y positive */
        break;
      case 2:
        y = -y;             /* both negative */
        break;
      case 3:
        x = -x;             /* x postive, y negative */
        break;
      default:                /* case 0: both positive */
        break;
    }
    r = sqrt(SQ(x)+SQ(y));
    theta = atan2(y, x);
    theta = DEGREES(theta);
    theta = CART_ANGLE(theta);
    theta = FMOD360(theta);
    nn = xse_find_intxns(theta, r, ob);
    // there must be side effects to find_intxns; what are they?

    inside_count += (int)(nn & 1); /* i.e. and odd number of intxns */
  }
  ob->radar_inside_boundary = inside_count > 2;
  return(ob->radar_inside_boundary);
}


/* c------------------------------------------------------------------------ */

int xse_find_intxns(angle, range, ob)
  double angle, range;
  struct one_boundary *ob; 
{
    /* this routine creates a list of endpoints of lines that 
     * intersect the current ray
     */   
    struct bnd_point_mgmt *bpm, *bpmx;
    double theta = RADIANS(CART_ANGLE(angle)), cos(), sin();
    double slope=0, xx, yy;
    long x, y;
    int ii, mm, nn, nx=0, mark;


    ob->num_intxns = 0; 
    ob->first_intxn = NULL;
    /*   
     * compute the endpoints of the ray
     */   
    xx = range * cos(theta);
    yy = range * sin(theta);
    x = xx < 0 ? xx -.5 : xx +.5; 
    y = yy < 0 ? yy -.5 : yy +.5; 
    if(xx) slope = yy/xx;

    bpm = ob->top_bpm;          /* find the first point that is not
                                 * on the line */
    /*   
     * when we are doing intersections, we want to use the shifted
     * x,y values because the boundary may be for another radar
     */   
    for(bpmx=NULL; bpm; bpm = bpm->next) {
        bpm->which_side = xse_ccw((double)bpm->_x, (double)bpm->_y, xx, yy); 
        if(bpm->which_side) {
            bpmx = bpm; 
            break;
        }    
    }    
    if(!bpmx)
          return(0);            /* no intersections! */
    mm = ob->num_points;
    /*   
     * since "->last" links all points we loop through all the
     * points using the last pointer
     */   
    for(; mm--; bpm = bpm->last) {
        /*   
         * a return of 0 from se_ccw says this point is
         * colinear with the ray. So we do nothing.
         */   
        bpm->last->which_side = xse_ccw
              ((double)bpm->last->_x, (double)bpm->last->_y, xx, yy); 
        if(bpm->last->which_side) {
            if(bpm->last->which_side != bpmx->which_side) {
                /*   
                 * we may have an intersection between "bpm"
                 * and "bpm->last". See if it's real.
                 */   
                xse_set_intxn(xx, yy, slope, bpm, ob); 
            }    
            bpmx = bpm->last;
        }    
    }    
# ifdef obsolete
    xse_num_segments(ob);
# endif
    return(ob->num_intxns);
}
/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */

//
// this assigns boundary true/false to each data point
// i.e. to each gate of a ray?
//

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
   
   for(;;) {                    /* for each ray */
          
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
// there must be some side effects of this. What are they?
// sets ob->radar_inside_boundary = T/F 
               }
            }
         } /* boundary exists? */
      } /* first time through */

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
// there must be some side effects of this. What are they?
// sets ob->radar_inside_boundary = T/F 
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

      else {                    /* no boundary */
         seds->use_boundary = NO;
         seds->boundary_mask = seds->all_ones_array;
      }
      ssm = seds->fer_cmds;
      nn = seds->num_fer_cmds;

      /*
       * loop through the for-each-ray operations
       */
      se_perform_cmds (seds->first_fer_cmd, seds->num_fer_cmds);

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
   se_perform_cmds (seds->first_fer_cmd, seds->num_fer_cmds);
   se_free_raqs();              /* free running average queues
                                 * if any where set up */
   if(seds->modified) {
      seds->time_modified = time_now();
      ddir_rescan_urgent(seds->se_frame);
   }

   if(seds->boundary_exists) {  /* pack up the current boundary */
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

      if(new_bnd) {             /* put this boundary in the queue */
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



int main(int argc, char *argv[]) {


// Q: So, where is radar_inside_bnd called? and how is it used?
// See dd_edd(time_series, automatic, down, d_ctr)
// dd_edd goes through data and sets boundary_mask to true or false 
// se_nab_segment(..)  <== returns the start and stop range of the requested segment
//    the start and stop are intersection points

}
