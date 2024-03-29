
// TODO: this will take some translation, from Solo structures to Radx and IpsEye structures
//       maybe this can be the world point translation code of Mike's

void PointInSpace::LocateThisPoint(SoloClickInfo *sci, BoundaryPointManagement *bpm) {

  /* this routine loads up a pisp struct with all the positioning                                           
   * info for this boundary point                                                                           
   */
  int frme, ww, ii, jj, kk, nn, nr, ypos;
  int file_action=TIME_NEAREST, version=LATEST_VERSION;
  int sweep_skip=1, replot=YES;
  struct dd_general_info *dgi, *dd_window_dgi();
  WW_PTR wwptrld, wwptr, solo_return_wwptr();
  struct ts_ray_info *tsri;
  struct ts_sweep_info *tssi;
  struct ts_ray_table *tsrt;
  double theta, range, x, y, scale;
  double dd_rotation_angle(), dd_tilt_angle(), dd_azimuth_angle()
    , dd_elevation_angle(), dd_earthr(), dd_nav_tilt_angle();
  double dd_latitude(), dd_longitude(), dd_altitude();
  double dd_heading(), dd_roll(), dd_pitch(), dd_drift();
  struct rot_table_entry *entry1, *dd_return_rotang1();
  struct rot_ang_table *rat;
  struct point_in_space *pisp=bpm->pisp;
  char str[256];

  frme = sci->frame;
  wwptr = solo_return_wwptr(frme);
  wwptrld = wwptr->lead_sweep;
  ww = wwptrld->window_num;
  dgi = dd_window_dgi(ww, "");
  rat = dgi->source_rat;      /* rotation angle table */
  entry1 = dd_return_rotang1(rat);

  strcpy(pisp->id, "BND_PT_V1");
  pisp->state = PISP_AZELRG | PISP_EARTH;


  if(wwptr->view->type_of_plot & SOLO_TIME_SERIES) {
    /*                                                                                                    
     * for time series the center of the screen is assumed to be                                          
     * the specified number in km. above msl                                                              
     */
    scale = M_TO_KM(sp_meters_per_pixel())/wwptr->view->ts_magnification;
    ypos = wwptr->view->height_in_pix -1 - sci->y;

    /* set msl altitude of clicked location */
        pisp->range =
          pisp->z = wwptr->view->ts_ctr_km -
          ((.5*wwptr->view->height_in_pix -.5) -ypos) * scale;
        pisp->x = sci->x;       /* pixel values */
        pisp->y = sci->y;
        pisp->state |= PISP_TIME_SERIES;
        nr = sp_ts_ray_list(ww, (int)sci->x, 3);
        tsrt = wwptrld->tsrt;
        if(!tsrt || !tsrt->tsri_top) {
          uii_printf("NULL tsrt pointer in sp_locate_this_point()  frme: %d\n"
                     , frme);
          return;
        }
        tsri = *(tsrt->tsri_list + nr/2); /* center ray info struct */
        kk = tsri->ray_num;
        tssi = tsrt->tssi_top + tsri->sweep_num;
        slash_path(str, tssi->directory);
        strcat(str, tssi->filename);
        if(wwptrld->file_id)
          close(wwptrld->file_id);
        if((wwptrld->file_id = open(str, O_RDONLY, 0)) < 0) {
          uii_printf("Unable to open sweep %s\n", str);
          return;
        }
        slash_path(dgi->directory_name, tssi->directory);
        dgi->in_swp_fid = wwptrld->file_id;
        dd_absorb_header_info(dgi);
        wwptrld->sweep_file_modified = NO;
  }
  else {
    x = sci->x;
    y = sci->y;
    theta = atan2(y, x);
    theta = FMOD360(CART_ANGLE(DEGREES(theta)));
    range = M_TO_KM(sqrt(SQ(x) + SQ(y))); /* km. */

    if(wwptr->lead_sweep->sweep_file_modified) {
      solo_nab_next_file(ww, file_action, version, sweep_skip, replot);
      wwptr->lead_sweep->sweep_file_modified = NO;
    }
    kk = dd_rotang_seek(rat, (float)theta);
    pisp->x = M_TO_KM(bpm->x);
    pisp->y = M_TO_KM(bpm->y);
    pisp->range = range;
    bpm->r = KM_TO_M(range);

    if(wwptr->lead_sweep->scan_mode == AIR)
      pisp->state |= PISP_PLOT_RELATIVE;
  }

  dgi_buf_rewind(dgi);
  nn = lseek(dgi->in_swp_fid, (long)(entry1+kk)->offset, 0L);
  dd_absorb_ray_info(dgi);

  pisp->time = dgi->time;
  pisp->rotation_angle = dd_rotation_angle(dgi);
  pisp->tilt = dd_tilt_angle(dgi);
  pisp->tilt = dgi->dds->swib->fixed_angle;

  pisp->azimuth = dd_azimuth_angle(dgi);
  pisp->elevation = dd_elevation_angle(dgi);

  pisp->latitude = dd_latitude(dgi);
  pisp->longitude = dd_longitude(dgi);
  pisp->altitude = dd_altitude(dgi);

  if(wwptr->view->type_of_plot & SOLO_TIME_SERIES) {
    /* true range of clicked location from radar                                                          
     */
    if(wwptr->view->type_of_plot & TS_MSL_RELATIVE) {
      pisp->range = (pisp->z - pisp->altitude) * sin(pisp->elevation);
    }
    else if(wwptr->view->type_of_plot & TS_PLOT_DOWN) {
      pisp->range = 2. * wwptr->view->ts_ctr_km -pisp->z;
    }
    else {                  /* plot up */
      pisp->range = pisp->z;
    }
    bpm->r = KM_TO_M(pisp->range);
  }
  else {
    /* msl altitude of clicked location */
    pisp->z = (pisp->altitude + range *
               sin((double)RADIANS(pisp->elevation)));
  }
  pisp->earth_radius = dd_earthr(pisp->latitude);

  pisp->heading = dd_heading(dgi);
  pisp->roll = dd_roll(dgi);
  pisp->pitch = dd_pitch(dgi);
  pisp->drift = dd_drift(dgi);



}

void PointInSpace::print(stream out) {

  //  stream, num, pisp)
  //    FILE *stream;
  //int num;
  //struct point_in_space *pisp;
{
  fprintf(out
          , "bnd:%3d; lon:%.4f; lat:%.4f; alt:%.4f; "
          , num, pisp->longitude, pisp->latitude, pisp->altitude);
  fprintf(out
          , "az:%.4f; el:%.4f; rng:%.4f;"
          , pisp->azimuth, pisp->elevation, pisp->range);
  fprintf(out
          , "hdg:%.4f; tlt:%.4f; dft:%.4f; "
          , pisp->heading, pisp->tilt, pisp->drift);
  fprintf(out, "\n");

}
