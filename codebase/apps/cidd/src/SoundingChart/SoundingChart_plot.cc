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
///////////////////////////////////////////////////////////////
// SoundingChart_plot.cc
//
// SoundingChart object
//
// Niles Oien  RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2004
//
///////////////////////////////////////////////////////////////
#define SoundingChart_plot_CC

using namespace std;

#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>


#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/ushmem.h>
#include <toolsa/pjg.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include <physics/physics.h>
#include <physics/thermo.h>
#include <rapmath/math_macros.h>

#include "SoundingChart.hh"

//////////////////////////////////////////////////
// DOPLOT
void SoundingChart::doPlot(void)
{

  if(P->backing_xid == 0 || P->canvas_xid == 0) return;

  xv_set(P->SoundingChart_GUI->window1,FRAME_BUSY,TRUE,NULL);
  // Create A Time plot
  TimePlot tp;

  // Set Up X GC
  XSetFont(P->dpy,P->def_gc, P->fontst->fid);
  XSetBackground(P->dpy,P->def_gc, P->bg_cell);
  XSetForeground(P->dpy,P->def_gc, P->bg_cell);

  // Clear image
  XFillRectangle(P->dpy,P->backing_xid,P->def_gc,0,0,P->can_width,P->can_height);

  if(P->params.debug) tp.set_debug_level(1);

  tp.Init(P->dpy,P->backing_xid,P->def_gc,P->fg_cell,P->bg_cell,P->fontst,
          P->params.left_margin,
          P->params.top_margin,
          P->can_width - P->params.left_margin - P->params.right_margin,
          P->can_height - P->params.top_margin - P->params.bottom_margin,
          P->can_height - P->params.bottom_margin,
          P->time_start,P->time_end);

  // Draw Reference axes on each side of the plot.
  XSetForeground(P->dpy,P->def_gc, P->fg_cell);
  tp.draw_y_scale(P->params.left_margin-1, P->params.min_height_km, P->params.max_height_km, (char *) "km",0);
  tp.draw_y_scale(P->can_width - P->params.right_margin, P->params.min_height_km, P->params.max_height_km, (char *) "km",1);


  // Draw Grid Psuedo Soundings.
  if (P->params.doMdv && P->cur_grid_field < P->params.GridSrc_n && P->M != NULL){
	  doGridPlot(tp);
  }

  // Draw Soundings if active
  if (P->cur_sound_field < P->params.soundingSrc_n && P->SG != NULL){
	  doSoundingPlot(tp);
  }


  XSetForeground(P->dpy,P->def_gc, P->ref_cell);
  char buf[16];
  if(P->reference_points_on) {
  	sprintf(buf,"%d",P->num_points);
  } else {
  	strncpy(buf,"Data",16);
  }
  tp.draw_ref_line(P->interest_time,P->params.top_margin,buf,P->def_gc);

  
  // Copy the finished graphic onto the screen.
  XCopyArea(P->dpy,P->backing_xid,P->canvas_xid,
			   P->def_gc, 0,0,
			   P->can_width,P->can_height,
			   0,0);

  xv_set(P->SoundingChart_GUI->window1,FRAME_BUSY,FALSE,NULL);

  return;
}

//////////////////////////////////////////////////
// DOSOUNDINGPLOT
void SoundingChart::doSoundingPlot(TimePlot &tp)
{
	int direct,ascent,descent;
	XCharStruct overall;
	char buf[128];

	int numFound = 0;
	unsigned int level = 0;

	if(P->SG != NULL) {
        numFound = P->SG->getNumSoundings();
	}
    
    if (P->params.debug){
      cerr << numFound << " Soundings to Plot" << endl;
    }

    // Add A colorscale on the right 
	P->sound_cs[P->cur_sound_field]->draw_colorbar(P->dpy,P->backing_xid,P->def_gc,P->fontst,
									   (P->can_width - P->params.right_margin / 2),
									   P->params.top_margin,
									   P->params.right_margin / 2,
									   (P->can_height - (P->params.top_margin + P->params.bottom_margin))); 
										


	const char   *site = "";
    vector <double> ht;// Heights
    vector <double> u; // E-W Winds
    vector <double> v; // N-S Winds
    vector <double> d; // Data Values
    vector <time_t> t; // Times

	time_t t1,t2;
	double ht1,ht2;
	double min_ht,max_ht;

    XSetForeground(P->dpy,P->def_gc, P->fg_cell);

	// For each Sounding - Plot a column of data 
    for (int i=0; i < numFound; i++){
      
	  min_ht = 99999.99;
	  max_ht = 0.0;
	  ht1 = 0.0;
	  ht2 = 0.0;
      d.clear(); // Averaged data values
      ht.clear();
      u.clear();
      v.clear();
      t.clear();

	  Sndg *S = P->SG->getSoundingData(i);
	  Sndg::header_t head = S->getHeader();
	  vector <Sndg::point_t> pt = S->getPoints();

      time_t dataTime = head.launchTime;
	  site = head.siteName;
      
	  t1 = dataTime - (time_t) (P->sounding_width_secs / 2);
	  t2 = dataTime + (time_t) (P->sounding_width_secs / 2);

      if (P->params.debug){
        cerr << endl << "Observed sounding " << i+1;
        cerr << " time " << utimstr(dataTime) << endl;
      }

	  level = 0;
	  int upoints;
	  int vpoints;
	  int hpoints;
	  int fpoints;
	  double u_sum;
	  double v_sum;
	  double ht_sum;
	  double val_sum;
	  double field_value;
	  double mb_thresh = 1100.0;
	double ht_thresh = 0.0;
	double level_ht;
	
	ht1 = P->params.min_height_km;

	switch(P->params.vscale_type) {
		case Params::VSCALE_MB:
			ht_thresh = PHYmb2meters(mb_thresh)/1000.0;
		 break;

		case Params::VSCALE_KM:
			ht_thresh = P->params.min_height_km + P->params.sounding_interval_km;
		break;
	}
	ht2 = ht_thresh;
      // Loop through each level - Averaging into sounding_interval_mbar or sounding_interval_km chunks
      while( level < pt.size() ){

		 upoints = 0; // Reset for each averaging level.
		 vpoints = 0; 
		 hpoints = 0;
		 fpoints = 0;
		 u_sum = 0.0;
		 v_sum = 0.0;
		 ht_sum = 0.0;
		 val_sum = 0.0;

                 if (pt[level].altitude != Sndg::VALUE_UNKNOWN ) {
			level_ht = pt[level].altitude;
		 } else if (pt[level].pressure != Sndg::VALUE_UNKNOWN ) {
			level_ht = PHYmb2meters(pt[level].pressure)/1000.0;
		 } else { /// No height info - Skip
			level_ht = 0.0;  // - Punt -  place in current level
                 }

		 while (level < pt.size() && level_ht <= ht_thresh) {
			if (pt[level].u != Sndg::VALUE_UNKNOWN ) {
			   u_sum += pt[level].u;  // Accumulate sums for averaging.
			   upoints++;
		   	}
			if (pt[level].v != Sndg::VALUE_UNKNOWN ) {
			   v_sum += pt[level].v;
			   vpoints++;
			}
			if (pt[level].altitude != Sndg::VALUE_UNKNOWN ) {
			   ht_sum += pt[level].altitude;
			   hpoints++;
			} else if (pt[level].pressure != Sndg::VALUE_UNKNOWN ) {
			   ht_sum += PHYmb2meters(pt[level].pressure)/1000.0;
			   hpoints++;
		   	}

			switch(P->params._soundingSrc[P->cur_sound_field].field) {
				   case Params::TEMP: field_value =  pt[level].temp; break;

				   case Params::RH: field_value =  pt[level].rh; break;

				   case Params::PRESS: field_value =  pt[level].pressure; break;

				   case Params::DEWPT: field_value =  pt[level].dewpt; break;

				   case Params::W_SPD: field_value =  pt[level].windSpeed; break;

				   case Params::W_DIR: field_value =  pt[level].windDir; break;

				   case Params::W_WIND: field_value =  pt[level].w; break;

				   case Params::ASCEN_R: field_value =  pt[level].ascensionRate; break;

				   case Params::SPARE1: field_value =  pt[level].spareFloats[0]; break;

				   case Params::SPARE2: field_value =  pt[level].spareFloats[1]; break;

				   case Params::SPARE3: field_value =  pt[level].spareFloats[2]; break;

			}

			if(field_value != Sndg::VALUE_UNKNOWN) {
			   val_sum += field_value;
			   fpoints++;
			}
                        level++;  // Move a level up in the sounding.
                        if (level < pt.size()) {
                          if (pt[level].altitude != Sndg::VALUE_UNKNOWN ) {
                            level_ht =pt[level].altitude;
                          } else if (pt[level].pressure != Sndg::VALUE_UNKNOWN ) {
                            level_ht = PHYmb2meters(pt[level].pressure)/1000.0;
                          } // else  punt - uses last level_ht again
                        }
                          
		} // while (level < pt.size() && level_ht <= ht_thresh)
			 
		 // Set the next threshold
		switch(P->params.vscale_type) {
			case Params::VSCALE_MB:
				mb_thresh -= P->params.sounding_interval_mbar;
				ht_thresh = PHYmb2meters(mb_thresh)/1000.0;
			break;

			case Params::VSCALE_KM:
				ht_thresh += P->params.sounding_interval_km;
			break;
		}
		 

		 // Compute the averages and store in vectors for Line Y-X plot.
		 t.push_back(dataTime);
		 if(upoints > 0) {
			 u.push_back(u_sum/upoints);
		 } else {
			 u.push_back(Sndg::VALUE_UNKNOWN);
		 }
		 if(vpoints > 0) {
			 v.push_back(v_sum/vpoints);
		 } else {
			 v.push_back(Sndg::VALUE_UNKNOWN);
		 }
		 if(hpoints > 0) {
			 ht.push_back(ht_sum/hpoints);
		 } else {
			 ht.push_back(Sndg::VALUE_UNKNOWN);
		 }

		 if(fpoints > 0) {
			 field_value = val_sum / fpoints;
			 d.push_back(field_value);
		 } else {
			 d.push_back(Sndg::VALUE_UNKNOWN);
			 field_value = Sndg::VALUE_UNKNOWN;
		 }


		 // Top of the current cell
	     if(level != 0) {
                if (pt[level -1].altitude != Sndg::VALUE_UNKNOWN ) {
				ht2 = pt[level-1].altitude;
		 	} else if (pt[level-1].pressure != Sndg::VALUE_UNKNOWN ) {
				ht2 = PHYmb2meters(pt[level-1].pressure)/1000.0;
		 	}  else {
			   ht2 = ht1;
			}
		 }

		 if(ht1 < min_ht) min_ht = ht1;
		 if(ht2 > max_ht) max_ht = ht2;

		 //fprintf(stderr,"H1,H2: %.2f,%.2f  HT: %.2f Field_value_ave: %g, Max Ht %.3f P: %.3f  ALT: %.3f\n",
		  //      ht1,ht2,ht[ht.size()-1],field_value,max_ht,
		//	pt[level-1].pressure,pt[level-1].altitude);

	     if(P->sound_color_on) {
		   if(field_value != Sndg::VALUE_UNKNOWN && ht1 != ht2) {
	         int cval = P->sound_cs[P->cur_sound_field]->val2color(field_value);
             XSetForeground(P->dpy,P->def_gc,cval);
	         tp.Draw_cell_3d(t1,t2,ht1,ht2,P->def_gc);
		   }
	     }
		 ht1 = ht2; // Base of next cell is top of this one.

	 }  // Looping through levels.


	 // Draw a boundry around the column
     if(P->params.plot_3d_border) {
	XSetForeground(P->dpy,P->def_gc, P->fg_cell);
	 tp.Draw_cell_border_3d(t1,t2,min_ht, max_ht,P->def_gc);
      }

      if(P->sound_winds_on) {
          XSetForeground(P->dpy,P->def_gc, P->sounding_winds_cell);
	      tp.Draw_barbs(u,v,ht,t,P->def_gc,Sndg::VALUE_UNKNOWN);
      }

      // IF Enabled - Draw Y-X Plot.
      if(P->sound_lines_on) {
	      int x1 = tp.get_pix_x(t1);
	      int x2 = tp.get_pix_x(t2 - ((t2 - t1)/4));
	      double sc_min = P->sound_cs[P->cur_sound_field]->get_min_value();
	      double sc_max = P->sound_cs[P->cur_sound_field]->get_max_value();
          XSetForeground(P->dpy,P->def_gc, P->fg_cell);
	      tp.Draw_yx_plot(x1,x2,sc_min,sc_max,d,ht,P->def_gc,Sndg::VALUE_UNKNOWN);

		  tp.Draw_yx_legend(x1,x2,sc_min,sc_max,
				  ht[ht.size()-1] + (ht[ht.size()-1] - ht[ht.size()-2]),
				  P->def_gc, (char *)NULL,1);
		 
      }
    
   }  // Loop through each sounding.

	// Add a label on the Right
    XSetForeground(P->dpy,P->def_gc, P->fg_cell);
	sprintf(buf,"%s Sounding %s %s",site, P->params._soundingSrc[P->cur_sound_field].label,P->params._soundingSrc[P->cur_sound_field].units);
	XTextExtents(P->fontst,buf,strlen(buf), &direct,&ascent,&descent,&overall);
	XDrawImageString(P->dpy,P->backing_xid,P->def_gc, (P->can_width - overall.width - 2),ascent +1, buf ,strlen(buf));

}


//////////////////////////////////////////////////
// DOGRIDPLOT
void SoundingChart::doGridPlot(TimePlot &tp)
{ 
	int direct,ascent,descent;
	XCharStruct overall;
	char buf[128];


    // Add A colorscale on the left 
	P->grid_cs[P->cur_grid_field]->draw_colorbar(P->dpy,P->backing_xid,P->def_gc,P->fontst,
									   0,
									   P->params.top_margin,
									   P->params.left_margin / 2,
									   (P->can_height - (P->params.top_margin + P->params.bottom_margin))); 

	// Add a label on the Left
    XSetForeground(P->dpy,P->def_gc, P->fg_cell);
	sprintf(buf,"%s  Model %s",
			P->params._GridSrc[P->cur_grid_field].units ,
			P->params._GridSrc[P->cur_grid_field].label);
	XTextExtents(P->fontst,buf,strlen(buf), &direct,&ascent,&descent,&overall);
	XDrawImageString(P->dpy,P->backing_xid,P->def_gc, 0,ascent +1, buf ,strlen(buf));
					 
    vector <string> fieldNames;
    fieldNames.push_back(P->params._GridSrc[P->cur_grid_field].fieldname);
    fieldNames.push_back(P->params._GridSrc[P->cur_grid_field].u_fname);
    fieldNames.push_back(P->params._GridSrc[P->cur_grid_field].v_fname);
    
    int mdvNumFound = P->M->getNumSoundings();
    
    if (P->params.debug){
      cerr << mdvNumFound << " MDV soundings to Plot." << endl;
    }
    
    int numFields = fieldNames.size();
    
    vector <time_t> t; // Times
    vector <double> ht;// Heights
    vector <double> u; // E-W Winds
    vector <double> v; // N-S Winds
    vector <double> d; // Data values

	time_t t1,t2;
	double ht1,ht2;

    double val0,val1,val2,val3;

    XSetForeground(P->dpy,P->def_gc, P->fg_cell);

    for (int i=0; i < mdvNumFound; i++){
      
      t.clear();
      ht.clear();
      u.clear();
      v.clear();
      d.clear();

      double *sData = P->M->getSoundingData(i);
      int numLevels = P->M->getSoundingNLevels(i);
      time_t dataTime = P->M->getSoundingTime(i);
      
	  if(i== 0) { // First Cell
		 t1 = P->time_start;
		 t2 = P->M->getSoundingTime(0) + ((P->M->getSoundingTime(1) - P->M->getSoundingTime(0)) / 2);

	  } else if (i == mdvNumFound -1) { // Last Cell
		 t1 = P->M->getSoundingTime(i-1) + ((P->M->getSoundingTime(i) - P->M->getSoundingTime(i-1)) / 2);
		 t2 = P->time_end;
	  } else {  // Interior Cells
		 t1 = P->M->getSoundingTime(i-1) + ((P->M->getSoundingTime(i) - P->M->getSoundingTime(i-1)) / 2);
		 t2 = P->M->getSoundingTime(i) + ((P->M->getSoundingTime(i+1) - P->M->getSoundingTime(i)) / 2);
	  }

      if (P->params.debug){
        cerr << endl << "MDV sounding " << i+1;
        cerr << " time " << utimstr(dataTime) << endl;
      }
      

      // Loop through each level
      for (int level = 0; level < numLevels; level++){

	val0 = sData[level*(numFields+1) + 0];
	val1 = sData[level*(numFields+1) + 1];
	val2 = sData[level*(numFields+1) + 2];
	val3 = sData[level*(numFields+1) + 3];

	 if (val2 != mdvSounding::badVal && val3 != mdvSounding::badVal){
	     t.push_back(dataTime);
	     ht.push_back(PHYmb2meters(val0) / 1000.0); // Convert to Km

	     u.push_back(val2);
	     v.push_back(val3);
	 } // If valid wind data 

	 if(val1 != mdvSounding::badVal) {

	   d.push_back(val1); // Store data value

	   if(level == 0 ) {  // First Level
	     ht1 = P->params.min_height_km;
	     ht2 = (sData[level*(numFields+1)] + sData[(level +1)*(numFields+1)]) / 2.0;
	     ht2 = PHYmb2meters(ht2) / 1000.0;
	   } else if (level == numLevels -1) { // Last Level
	     ht1 = (sData[level*(numFields+1)] + sData[(level -1)*(numFields+1)]) / 2.0;
		 ht1 = PHYmb2meters(ht1) / 1000.0;

	     ht2 = sData[level*(numFields+1)] + 
				((sData[level*(numFields+1)] - sData[(level-1)*(numFields+1)]) / 2.0);
		 ht2 = PHYmb2meters(ht2) / 1000.0;
	   } else { // Interior levels
	     ht1 = (sData[level*(numFields+1)] + sData[(level -1)*(numFields+1)]) / 2.0;
		 ht1 = PHYmb2meters(ht1) / 1000.0;

	     ht2 = (sData[level*(numFields+1)] + sData[(level +1)*(numFields+1)]) / 2.0;
	     ht2 = PHYmb2meters(ht2) / 1000.0;
	   }

	   if(P->grid_color_on) {
	       int cval = P->grid_cs[P->cur_grid_field]->val2color(val1);
               XSetForeground(P->dpy,P->def_gc,cval);
	       tp.Draw_cell(t1,t2,ht1,ht2,P->def_gc);
	   }

	 }
       } // Each level of data

      if(P->grid_winds_on) {
          XSetForeground(P->dpy,P->def_gc, P->grid_winds_cell);
	  tp.Draw_barbs(u,v,ht,t,P->def_gc,mdvSounding::badVal);
      }

      // IF Enabled - Draw Y-X Plot.
      if(P->grid_lines_on) {
	      int x1 = tp.get_pix_x(t1);
	      int x2 = tp.get_pix_x(t2);
	      double sc_min = P->grid_cs[P->cur_grid_field]->get_min_value();
	      double sc_max = P->grid_cs[P->cur_grid_field]->get_max_value();
          XSetForeground(P->dpy,P->def_gc, P->fg_cell);
	      tp.Draw_yx_plot(x1,x2,sc_min,sc_max,d,ht,P->def_gc,mdvSounding::badVal);

		  if(ht.size() > 0 &&  (i-1) % 3 == 0 ) { // Every third column
			  tp.Draw_yx_legend(x1,x2,sc_min,sc_max,
				  ht[ht.size()-1] + (ht[ht.size()-1] - ht[ht.size()-2]),
				  P->def_gc, (char *)NULL,1);
		  }
      }

   } // For each time found 

  return;

}
