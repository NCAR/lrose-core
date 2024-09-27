// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*************************************************************************
 * DATA_IO.C : Collect incoming data and report progress
 *
 *  How does this mess work, you ask?  Well, just read the following.
 *
 *  CIDD operates using three cooperating threads of execution;
 *  
 *   1. The GUI thread. This is the "main" thread in all XView
 *       applications. This thread must have control for widgets
 *       to change, labels to update and images to be repainted.
 *       When users click buttons, etc, this thread simply sets 
 *       state information in memory, and calls event handler
 *       routines when the user clicks/drags in the image areas.
 *
 *   2. The Timer thread. This thread is spawned from the GUI
 *       thread using an interval timer. This thread is responsible for
 *       traversing the state table (The various structures in the
 *       global data container struct - 'gd'.) and determining
 *       what actions to take. It looks for images marked as needing rendered
 *       and calls the rendering routines, rendering the highest
 *       priority image. Once the highest priority image is complete, it will
 *       render other, lower priority images if they've been marked to
 *       be rendered in the background. The rendering routines call the
 *       gather_data routines if their data is invalid, which ultimately
 *       generates data requests.
 *       The gd.io_info struct is used to indicate that a pending request 
 *       has been made, where the data should go and what its type is.
 *       While IO is pending, control is returned to the main thread.
 *       The timer thread also checks for incoming data. As data comes in,
 *       it is read and progress indicators are updated. Control returns
 *       to the timer thread, which quickly returns it to the GUI thread.
 *       Once the incoming data is has arrived, the data is decompressed,
 *       stuffed into internal structs, color mappings recalculated, etc,
 *       and then its marked as valid. Once all data is valid, the image
 *       will then be rendered and displayed.
 *      
 *    3. The Data IO thread. This thread is responsible for reading in data
 *       once a request has been made.  The DSMdvX and DSSpdb protocol threads
 *       spawn directly from their get_data methods. 
 *       
 *    If you think this is all crazy, you're right! This program was written
 *    before OS's had true threads. I had to roll my own until
 *    things caught up. Why do all this? So the user doesn't perceive the
 *    GUI freezing up. Data might take many seconds to arrive and the
 *    user wants to hit buttons, or move the window, etc. Users really
 *    do not like their GUI's to freeze up, even for a second, so I had
 *    go to these extreme ends to minimize the GUI freezes.
 *
 * - Frank Hage.
 */

#define   DATA_IO    1

#include "cidd.h"

extern int errno;

/*****************************************************************
 * AUTOSCALE_VCM: Autoscale the value-color mapping
 *        color to use
 *
 */

void autoscale_vcm( Valcolormap_t *vcm, double min, double max)
{
    int    i;
    double delta;

    delta = (max - min) / (double) vcm->nentries;

    if(delta == 0.0) delta = 0.1;

    for(i=0; i < vcm->nentries; i++) {
	vcm->vc[i]->min = min + (delta * i);
	vcm->vc[i]->max = vcm->vc[i]->min + delta;
    }

}

/**********************************************************************
 * CHECK_FOR_IO: This routine checks to see if any new data
 *  has arrived and updates the frame message
 */

void check_for_io()
{
  int i;
  double delta;
  met_record_t *mr;
  double comp;
  long n_read;
  double bps;
  struct timeval tm;
  struct timezone tz;
  char label[256];

  switch (gd.io_info.mode ) {
    default:  // do nothing
      break;

    case DSMDVX_DATA:  // Handle MDV data
      mr = gd.io_info.mr;

      switch(gd.io_info.request_type) {
        case HORIZ_REQUEST:
          // Check if our data access thread  is done
          if( mr->h_mdvx->getThreadDone() ) { // Yes
            // OK - Data is now in.

            if(mr->h_mdvx->getThreadRetVal()) { // Error
              mr->h_data = NULL;
              mr->h_fl32_data = NULL;

              // Save the master header for the file, even if we couldn't
              // get data for this field.  This is needed in case we are
              // dealing with forecast data

              mr->h_mhdr = mr->h_mdvx->getMasterHeader();

              // If No data was available, mark this condition valid
              //if(mr->h_mdvx->getNoFilesFoundOnRead()) mr->h_data_valid = 1;
              mr->h_data_valid = 1;
              mr->last_collected = time(0);

              // Indicate data is no longer pending
              gd.io_info.busy_status = 0;
              gd.io_info.outstanding_request = 0;
              gd.io_info.request_type = 0;
              gd.h_win.redraw[gd.io_info.page] = 1;

              if(gd.debug || gd.debug1) {
                fprintf(stderr,"Aborted Read: Error %d - %s\n",
                        mr->h_mdvx->getThreadRetVal(),
                        mr->h_mdvx->getErrStr().c_str());
              }
              if(_params.show_data_messages) gui_label_h_frame("No Data Received",-1);
              add_message_to_status_win("Aborted Read",1);
              add_message_to_status_win((char *) mr->h_mdvx->getErrStr().c_str(),0);

              return;
            }
		  
            if(gd.debug || gd.debug1 || gd.debug2) {
              fprintf(stderr,"Data Returned from: %s\n", mr->h_mdvx->getPathInUse().c_str());
            }
            // Decompress the Data
            MdvxField *hfld;
            if((hfld = mr->h_mdvx->getFieldByNum(0)) == NULL) {
              fprintf(stderr,"Aborted Read: Error %d - %s\n", 
                      mr->h_mdvx->getThreadRetVal(),
                      mr->h_mdvx->getErrStr().c_str());
              gd.io_info.busy_status = 0;
              gd.io_info.outstanding_request = 0;
              gd.io_info.request_type = 0;
              gd.h_win.redraw[gd.io_info.page] = 1;
              mr->h_data_valid = 1;
              mr->last_collected = time(0);
              return;
            }
            // Indicate data update is in porgress.
            gd.io_info.busy_status = 1;

            Mdvx::field_header_t fh = hfld->getFieldHeader();
            if(fh.encoding_type != Mdvx::ENCODING_RGBA32) {
              *mr->h_mdvx_int16 = *hfld; // Copy for INT16 data

              if(mr->h_vcm.nentries < 2 || fh.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
                mr->h_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE);
              } else {

                // Convert the copy to - Decompressed INT16 - Covering the range of the colorscale
                double range = (mr->h_vcm.vc[mr->h_vcm.nentries-1]->max - mr->h_vcm.vc[0]->min);
                double scale = range / (MAX_COLOR_CELLS -2);
                double bias = mr->h_vcm.vc[0]->min - (2 * scale); // Preserve 0, 1 as legitimate NAN values

                mr->h_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE,
                                              Mdvx::SCALING_SPECIFIED,scale,bias);
              }

              // Record where int8 data is in memory. - Used for fast polygon fills.
              mr->h_data = (unsigned short *) mr->h_mdvx_int16->getVol();

              // Convert the AS-IS to 32 bits float. - Used for Contouring, Data reporting.
              (mr->h_mdvx->getFieldByNum(0))->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
              // Record where float data is in memory.
              mr->h_fl32_data = (fl32  *) mr->h_mdvx->getFieldByNum(0)->getVol();

              // Find Headers for quick reference
              mr->h_mhdr = mr->h_mdvx->getMasterHeader();
              mr->h_fhdr = mr->h_mdvx_int16->getFieldHeader();
              mr->h_vhdr = mr->h_mdvx_int16->getVlevelHeader();
            } else {

              // Decompress
              hfld->convertType(Mdvx::ENCODING_ASIS, Mdvx::COMPRESSION_NONE);

              // Record where data is in memory. 
              mr->h_data = (unsigned short *) hfld->getVol();

              // Record where float data is in memory.
              mr->h_fl32_data = (fl32  *) hfld->getVol();

              // Find Headers for quick reference
              mr->h_mhdr = mr->h_mdvx->getMasterHeader();
              mr->h_fhdr = hfld->getFieldHeader();
              mr->h_vhdr = hfld->getVlevelHeader();
            }
		  
            // Init projection
            mr->proj->init(mr->h_fhdr);

            // condition longitudes to be in same hemisphere
            // as origin
            mr->proj->setConditionLon2Origin(true);

            // Implemented for MOBILE RADARS - 
            if(_params.domain_follows_data && mr == gd.mrec[gd.h_win.page] ) { // Only for the primary field
              double dx,locx;
              double dy,locy;
              int index = gd.h_win.zoom_level;

              if(mr->h_fhdr.proj_origin_lat != gd.h_win.origin_lat || mr->h_fhdr.proj_origin_lon != gd.h_win.origin_lon) {

                dx = gd.h_win.zmax_x[index] - gd.h_win.zmin_x[index];
                dy = gd.h_win.zmax_y[index] - gd.h_win.zmin_y[index];

                switch(gd.display_projection) {
                  case Mdvx::PROJ_LATLON:
                    gd.h_win.origin_lat = mr->h_fhdr.proj_origin_lat;
                    gd.h_win.origin_lon = mr->h_fhdr.proj_origin_lon;

                    gd.h_win.zmin_x[index] = mr->h_fhdr.proj_origin_lon - (dx / 2.0);
                    gd.h_win.zmax_x[index] = mr->h_fhdr.proj_origin_lon + (dx / 2.0);
                    gd.h_win.zmin_y[index] = mr->h_fhdr.proj_origin_lat - (dy / 2.0);
                    gd.h_win.zmax_y[index] = mr->h_fhdr.proj_origin_lat + (dy / 2.0);

                    break;
                        
                  default:
                    gd.proj.latlon2xy(mr->h_fhdr.proj_origin_lat,mr->h_fhdr.proj_origin_lon,locx,locy);

                    gd.h_win.zmin_x[index] = locx - (dx / 2.0);
                    gd.h_win.zmax_x[index] = locx + (dx / 2.0);
                    gd.h_win.zmin_y[index] = locy - (dy / 2.0);
                    gd.h_win.zmax_y[index] = locy + (dy / 2.0);

                    break;

                }
                /* Set current area to our indicated zoom area */
                gd.h_win.cmin_x = gd.h_win.zmin_x[index];
                gd.h_win.cmax_x = gd.h_win.zmax_x[index];
                gd.h_win.cmin_y = gd.h_win.zmin_y[index];
                gd.h_win.cmax_y = gd.h_win.zmax_y[index];
		      
                reset_time_list_valid_flags();
                reset_data_valid_flags(1,0);
                reset_terrain_valid_flags(1,0);
                set_redraw_flags(1,0);
                mr->h_data_valid = 1;  // This field is still valid, though
                mr->time_list_valid = 1;
		      
		      
              }
            }

            if (gd.debug1) {
              cerr << "nx, ny: "
                   << mr->h_fhdr.nx << ", "
                   << mr->h_fhdr.ny << endl;
              cerr << "dx, dy: "
                   << mr->h_fhdr.grid_dx << ", "
                   << mr->h_fhdr.grid_dy << endl;
              cerr << "minx, miny: "
                   << mr->h_fhdr.grid_minx << ", "
                   << mr->h_fhdr.grid_miny << endl;
              cerr << "maxx, maxy: "
                   << mr->h_fhdr.grid_minx + (mr->h_fhdr.nx - 1) * mr->h_fhdr.grid_dx << ", "
                   << mr->h_fhdr.grid_miny + (mr->h_fhdr.ny - 1) * mr->h_fhdr.grid_dy << endl;
            }

            // Punt and use the field headers if the file headers are not avail
            if(hfld->getFieldHeaderFile() == NULL) 
              mr->ds_fhdr = (hfld->getFieldHeader());
            else 
              mr->ds_fhdr = *(hfld->getFieldHeaderFile());

            if(hfld->getVlevelHeaderFile() == NULL) 
              mr->ds_vhdr = (hfld->getVlevelHeader());
            else 
              mr->ds_vhdr = *(hfld->getVlevelHeaderFile());

            // Recompute the color scale lookup table if necessary
            if(mr->h_fhdr.scale != mr->h_last_scale ||
               mr->h_fhdr.bias != mr->h_last_bias   ||
               mr->h_fhdr.missing_data_value != mr->h_last_missing   ||
               mr->h_fhdr.bad_data_value != mr->h_last_bad ||
               mr->h_fhdr.transform_type != mr->h_last_transform ) {

              if(mr->auto_scale)
                autoscale_vcm(&(mr->h_vcm), mr->h_fhdr.min_value, mr->h_fhdr.max_value);

              if(fh.encoding_type != Mdvx::ENCODING_RGBA32) {
#ifdef NOTYET
                /* Remap the data values onto the colorscale */
                setup_color_mapping(&(mr->h_vcm),
                                    mr->h_fhdr.scale,
                                    mr->h_fhdr.bias,
                                    mr->h_fhdr.transform_type,
                                    mr->h_fhdr.bad_data_value,
                                    mr->h_fhdr.missing_data_value);
#endif
              }
              // Update last values
              mr->h_last_scale = mr->h_fhdr.scale;
              mr->h_last_bias = mr->h_fhdr.bias;
              mr->h_last_missing = mr->h_fhdr.missing_data_value;
              mr->h_last_bad = mr->h_fhdr.bad_data_value;
              mr->h_last_transform = mr->h_fhdr.transform_type;
            }

            // Compute the vertical levels 
            mr->plane = 0;
            for(i=0; i < mr->ds_fhdr.nz; i++) { 
              // Find out which plane we received
              if(mr->ds_vhdr.level[i] == mr->h_vhdr.level[0]) mr->plane = i;
              if(i == 0) { // Lowest plane 
                delta = (mr->ds_vhdr.level[i+1] - mr->ds_vhdr.level[i]) / 2.0;
                mr->vert[i].min = mr->ds_vhdr.level[0] - delta;
                mr->vert[i].cent = mr->ds_vhdr.level[0];
                mr->vert[i].max = mr->ds_vhdr.level[0] + delta;

              } else if (i == mr->ds_fhdr.nz -1) { // highest plane
                delta = (mr->ds_vhdr.level[i] - mr->ds_vhdr.level[i-1]) / 2.0;
                mr->vert[i].min = mr->ds_vhdr.level[i] - delta;
                mr->vert[i].cent = mr->ds_vhdr.level[i];
                mr->vert[i].max = mr->ds_vhdr.level[i] + delta;

              } else { // Middle planes
                delta = (mr->ds_vhdr.level[i] - mr->ds_vhdr.level[i-1]) / 2.0;
                mr->vert[i].min = mr->ds_vhdr.level[i] - delta;
                mr->vert[i].cent = mr->ds_vhdr.level[i];

                delta = (mr->ds_vhdr.level[i+1] - mr->ds_vhdr.level[i]) / 2.0;
                mr->vert[i].max = mr->ds_vhdr.level[i] + delta;
              }
            }


            // Record the dimensional Units of the volume
            STRcopy(mr->units_label_cols,
                    mr->h_mdvx->projType2XUnits(mr->h_fhdr.proj_type),LABEL_LENGTH);
            STRcopy(mr->units_label_rows,
                    mr->h_mdvx->projType2YUnits(mr->h_fhdr.proj_type),LABEL_LENGTH);
            STRcopy(mr->units_label_sects,
                    mr->h_mdvx->vertTypeZUnits(mr->h_vhdr.type[0]),LABEL_LENGTH);

            // Record the date
            UTIMunix_to_date(mr->h_mhdr.time_centroid,&mr->h_date);

            mr->h_data_valid = 1;
            mr->last_collected = time(0);
            gd.h_win.redraw[gd.io_info.page] = 1;
            // Indicate its safe to use the data
            gd.io_info.busy_status = 0;
            // Indicate data is no longer pending
            gd.io_info.outstanding_request = 0;
            gd.io_info.request_type = 0;

            if(_params.show_data_messages) gui_label_h_frame("Done",-1);
            else set_busy_state(0);
          } else { // Display a progress message
            comp = mr->h_mdvx->getPercentReadComplete();
            n_read = mr->h_mdvx->getNbytesReadDone();

            if(n_read <= 1) {
              snprintf(label,256,"Waiting for %s service  %ld -  secs before timeout",
                      mr->legend_name,
                      (gd.io_info.expire_time - time(0)));
            } else {
              if(n_read > gd.io_info.last_read) {
                gettimeofday(&tm,&tz);

                bps = (n_read - gd.io_info.last_read) /  elapsed_time(gd.io_info.last_time,tm);
                if( bps < 512) {
                  snprintf(label,256,"READING %s Data  %.0f%% complete  - %.2g bytes/sec",
                          mr->legend_name,
                          comp,bps);
                } else {
                  snprintf(label,256,"READING %s Data  %.0f%% complete  - %.2g Kb/sec",
                          mr->legend_name,
                          comp,bps/1000);
                }
                gd.io_info.expire_time++;
                gd.io_info.last_read = n_read;
                gd.io_info.last_time = tm;
              } else {
                snprintf(label,256,"READING %s Data  %.0f%% complete",
                        mr->legend_name,comp);
              }
            }
            if(_params.show_data_messages) gui_label_h_frame(label,1);
          }
	  break;

        case VERT_REQUEST:
          if( mr->v_mdvx->getThreadDone() ) { // Yes
            // OK - Data is now in.
            if(mr->v_mdvx->getThreadRetVal() || mr->v_mdvx->getNFields() < 1) {

              mr->v_data = NULL;
              mr->v_fl32_data = NULL;

              // If No data was available, mark this condition valid
              //if(mr->v_mdvx->getNoFilesFoundOnRead()) mr->v_data_valid = 1;
              mr->v_data_valid = 1;

              // Indicate data is no longer pending
              gd.io_info.busy_status = 0;
              gd.io_info.outstanding_request = 0;
              gd.io_info.request_type = 0;
              gd.h_win.redraw[gd.io_info.page] = 1;

              if(gd.debug || gd.debug1) {
                fprintf(stderr,"Aborted Read: Error %d - %s\n",
                        mr->v_mdvx->getThreadRetVal(),
                        mr->v_mdvx->getErrStr().c_str());
              }
              if(_params.show_data_messages) gui_label_h_frame("No Cross Section Data Received - Aborting",-1);
              add_message_to_status_win("No Cross Section Data Received",0);
              add_message_to_status_win((char *) mr->v_mdvx->getErrStr().c_str(),1);

              return;
            }

            *mr->v_mdvx_int16 = *mr->v_mdvx->getFieldByNum(0); // Copy for INT16 data
            if(mr->v_vcm.nentries < 2 || mr->v_fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
              mr->v_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE);
            } else {
              // Convert the copy to - Decompressed INT16 - Covering the range of the colorscale
              double range = (mr->v_vcm.vc[mr->v_vcm.nentries-1]->max - mr->v_vcm.vc[0]->min);
              double scale = range / (MAX_COLOR_CELLS -2);
              double bias = mr->v_vcm.vc[0]->min - (2 * scale); // Preserve 0, 1 as legitimate NAN values
              mr->v_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE,
                                            Mdvx::SCALING_SPECIFIED,scale,bias);
            }
            gd.io_info.busy_status = 1;

            // Record where the data are.
            mr->v_data = (unsigned short *) mr->v_mdvx_int16->getVol();

            // Convert the AS-IS Encoding to fl32 for Contouring and reporting.
            (mr->v_mdvx->getFieldByNum(0))->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
            // Record where the fl32 data are.
            mr->v_fl32_data = (fl32 *) mr->v_mdvx->getFieldByNum(0)->getVol();

            mr->v_mhdr = mr->v_mdvx->getMasterHeader();
            mr->v_fhdr = mr->v_mdvx_int16->getFieldHeader();
            mr->v_vhdr = mr->v_mdvx_int16->getVlevelHeader();

            // Punt and use the field headers if the file headers are not avail
            /* if(mr->v_mdvx_int16->getFieldHeaderFile() == NULL) 
               mr->ds_fhdr = (mr->v_mdvx_int16->getFieldHeader());
               else 
               mr->ds_fhdr = *(mr->v_mdvx_int16->getFieldHeaderFile());
            */

            if(mr->v_fhdr.scale != mr->v_last_scale ||
               mr->v_fhdr.bias != mr->v_last_bias ||
               mr->v_fhdr.bad_data_value !=  mr->v_last_bad ||
               mr->v_fhdr.missing_data_value !=  mr->v_last_missing ||
               mr->v_fhdr.transform_type != mr->v_last_transform ) {

              if(mr->auto_scale)
                autoscale_vcm(&(mr->v_vcm), mr->v_fhdr.min_value, mr->v_fhdr.max_value);

              /* Remap the data values onto the colorscale */
#ifdef NOTYET
              setup_color_mapping(&(mr->v_vcm),
                                  mr->v_fhdr.scale,
                                  mr->v_fhdr.bias,
                                  mr->v_fhdr.transform_type,
                                  mr->v_fhdr.bad_data_value,
                                  mr->v_fhdr.missing_data_value);
#endif

              // Update last values
              mr->v_last_scale = mr->v_fhdr.scale;
              mr->v_last_bias = mr->v_fhdr.bias;
              mr->v_last_bad = mr->v_fhdr.bad_data_value;
              mr->v_last_missing = mr->v_fhdr.missing_data_value;
              mr->v_last_transform = mr->v_fhdr.transform_type;
            }

            // Record the date
            UTIMunix_to_date(mr->v_mhdr.time_centroid,&mr->v_date);

            // Record the actual way points returned, if it's the key field .
            if (gd.mrec[gd.h_win.page] == mr) {
              const vector<Mdvx::vsect_waypt_t> way_pts = mr->v_mdvx->getVsectWayPts();

              gd.h_win.route.num_segments = way_pts.size() -1;
              if(gd.debug || gd.debug1 || gd.debug2) fprintf(stderr,"Returned %d WayPoints\n",(int)way_pts.size());
              gd.h_win.route.total_length = 0;
              for(int i=0; i < (int) way_pts.size(); i++) {
                if(gd.debug1 || gd.debug2)
                  fprintf(stderr,"WayPoint %d: Lat,Lon: %g,%g  \n",
                          i, way_pts[i].lat,  way_pts[i].lon);

                gd.proj.latlon2xy(way_pts[i].lat,  way_pts[i].lon,
                                  gd.h_win.route.x_world[i],gd.h_win.route.y_world[i]);
                if(i > 0) {
                  gd.h_win.route.seg_length[i-1] =
                    disp_proj_dist(gd.h_win.route.x_world[i],
                                   gd.h_win.route.y_world[i],
                                   gd.h_win.route.x_world[i-1],
                                   gd.h_win.route.y_world[i-1]);
                  gd.h_win.route.total_length += gd.h_win.route.seg_length[i-1];
                }
              }
            }

            mr->v_data_valid = 1;
            //gd.v_win.cmin_x = 0.0;
            //gd.v_win.cmax_x = gd.h_win.route.total_length;
            //gd.v_win.cmin_y =  gd.v_win.min_ht;
            //gd.v_win.cmax_y =  gd.v_win.max_ht;
		  
            gd.v_win.redraw[gd.io_info.page] = 1;
            gd.io_info.busy_status = 0;
            gd.io_info.outstanding_request = 0;
            gd.io_info.request_type = 0;

            if(_params.show_data_messages) gui_label_h_frame("Done",-1);
            else set_busy_state(0);

          } else {
            comp = mr->v_mdvx->getPercentReadComplete();
            n_read = mr->v_mdvx->getNbytesReadDone();

            if(n_read <= 1) {
              snprintf(label,256,"Waiting for %s service  %ld -  secs before timeout",
                      mr->legend_name,
                      (gd.io_info.expire_time - time(0)));
            } else {
              if(n_read > gd.io_info.last_read) {
                gettimeofday(&tm,&tz);

                bps = (n_read - gd.io_info.last_read) /  elapsed_time(gd.io_info.last_time,tm);
                if( bps < 512) {
                  snprintf(label,256,"Reading %s Data  %.0f%% complete  - %.2g bytes/sec",
                          mr->legend_name,
                          comp,bps);
                } else {
                  snprintf(label,256,"Reading %s Data  %.0f%% complete  - %.2g Kb/sec",
                          mr->legend_name,
                          comp,bps/1000);
                }
                gd.io_info.expire_time++;
                gd.io_info.last_read = n_read;
                gd.io_info.last_time = tm;
              } else {
                snprintf(label,256,"READING %s Data  %.0f%% complete",
                        mr->legend_name,comp);
              }
            }
            if(_params.show_data_messages) gui_label_h_frame(label,1);
          }
          break; // end of case VERT_REQUEST

        case TIMELIST_REQUEST:
          // Check if our data access thread  is done
          if( mr->h_mdvx->getThreadDone() ) { // Yes

            if(mr->h_mdvx->getThreadRetVal()) {  // check the return value

              gd.io_info.busy_status = 0;
              gd.io_info.outstanding_request = 0;
              gd.io_info.request_type = 0;
              gd.io_info.mode = 0;
              mr->time_list_valid = 1;

              if(gd.debug || gd.debug1) {
                fprintf(stderr,"TIMELIST_REQUEST Error %d - %s\n",
                        mr->h_mdvx->getThreadRetVal(),
                        mr->h_mdvx->getErrStr().c_str());
              }
              add_message_to_status_win("TIMELIST_REQUEST error",1);
              add_message_to_status_win((char *) mr->h_mdvx->getErrStr().c_str(),0);
              if(!_params.show_data_messages)set_busy_state(0);

              return;   
            }

            // get a pointer to the time list
            const vector<time_t> &timeList = mr->h_mdvx->getTimeList();

            // allocate enough space
            if(mr->time_list.num_alloc_entries == 0 && timeList.size() > 0 ) {
              mr->time_list.tim = (time_t *) calloc(timeList.size(),sizeof(time_t));
              if(mr->time_list.tim != NULL) mr->time_list.num_alloc_entries  = timeList.size();

            } else if (mr->time_list.num_alloc_entries < timeList.size()) {
              mr->time_list.tim = (time_t *) realloc(mr->time_list.tim,(timeList.size() * sizeof(time_t)));
              if(mr->time_list.tim != NULL) mr->time_list.num_alloc_entries  = timeList.size();
            }

            // copy the time list
            mr->time_list.num_entries = timeList.size();
            for (size_t i = 0; i < mr->time_list.num_entries; i++) {
              mr->time_list.tim[i] = timeList[i];

            }

            if(gd.debug1) 
              fprintf(stderr, "Found %d  Time List entries\n",mr->time_list.num_entries);

            // indicate we're done 
            mr->time_list_valid = 1;
            gd.io_info.outstanding_request = 0;
            gd.io_info.request_type = 0;
            gd.io_info.mode = 0;
            if(_params.show_data_messages) gui_label_h_frame("Done",-1);
            else set_busy_state(0);

          } else {   // Still waiting for the request thread to complete
	    snprintf(label,256,"Waiting for %s Data Index  %ld -  secs before timeout",
                    mr->legend_name,
                    (gd.io_info.expire_time - time(0)));
          }
          if(_params.show_data_messages) gui_label_h_frame(label,1);
	     
          break;  // end of case  TIMELIST_REQUEST
      }
      break;   // End of case DXMDVX_DATA

    case SYMPROD_DATA: // Handle Symbolic product data 

      if(gd.io_info.prod == NULL) break;
 
      // first check for times. Break if the time refs have not been
      // processed, so that io_mode is not reset until both the times
      // and data are in
	      
      DsSpdbThreaded *spdbTimes = gd.io_info.prod->getSpdbTimesObj();
      if(spdbTimes->getThreadDone()) {
        if (gd.io_info.prod->processTimeRefs()) {
          break;
        }
      } else {
        break;
      }

      // Check for data

      DsSpdbThreaded *spdb = gd.io_info.prod->getSpdbObj();

      if( spdb->getThreadDone() ) { // Yes

        // OK - Data is now in.
        if(spdb->getThreadRetVal()) {

          gd.io_info.busy_status = 0;
          gd.io_info.outstanding_request = 0;
          gd.io_info.request_type = 0;
          gd.io_info.mode = 0;
          gd.io_info.prod->_data_valid = 1;

          if(gd.debug || gd.debug1) {
            fprintf(stderr,"Symprod Read Error: %d - %s\n",
                    spdb->getThreadRetVal(),
                    spdb->getErrStr().c_str());
          }
          if(_params.show_data_messages) gui_label_h_frame("Error Reading SYMPROD Data - Aborting",-1);
          add_message_to_status_win("Error Reading SYMPROD Data",0);
          add_message_to_status_win((char *) spdb->getErrStr().c_str(),1);

          return;
        }

        gd.io_info.busy_status = 1;
        // store the Symprod data internally - Indicate the data's valid
        gd.io_info.prod->processChunks();

        // Indicate we're done
        gd.io_info.busy_status = 0;

        gd.io_info.outstanding_request = 0;
        gd.io_info.request_type = 0;
        gd.io_info.mode = 0;
        if(_params.show_data_messages) gui_label_h_frame("Done",-1);
        else set_busy_state(0);

      } else {

        comp = spdb->getPercentComplete();
        n_read = spdb->getNbytesDone();

        if(n_read <= 1) {
          snprintf(label,256,"Waiting for %s service  %ld -  secs before timeout",
                  gd.io_info.prod->_prodInfo.menu_label,
                  (gd.io_info.expire_time - time(0)));
        } else {
          if(n_read > gd.io_info.last_read) {
            gettimeofday(&tm,&tz);

            bps = (n_read - gd.io_info.last_read) /  elapsed_time(gd.io_info.last_time,tm);

            if( bps < 512) {
              snprintf(label,256,"Reading %s Data  %.0f%% complete  - %.2g bytes/sec",
                      gd.io_info.prod->_prodInfo.menu_label,
                      comp,bps);
            } else {
              snprintf(label,256,"Reading %s Data  %.0f%% complete  - %.2g Kb/sec",
                      gd.io_info.prod->_prodInfo.menu_label,
                      comp,bps/1000);
            }

            gd.io_info.last_read = n_read;
            gd.io_info.last_time = tm;
            gd.io_info.expire_time++;
          } else {
            snprintf(label,256,"Reading %s Data  %.0f%% complete",
                    gd.io_info.prod->_prodInfo.menu_label, comp);
          }
        }
        if(_params.show_data_messages) gui_label_h_frame(label,1);

      }
      break;
  }
  return ;
}
