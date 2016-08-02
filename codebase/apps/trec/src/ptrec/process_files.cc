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
/***************************************************************************
 * process_files.c
 *
 * adapted from both $RAP_DIR/apps/src/ctrec/process_files.c (Ames)
 *              and  $RAP_DIR/apps/src/ptrec/process_beams.c (Dixon)
 *
 * Terri L. Betancourt
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1997
 *
 ****************************************************************************/

#include "trec.h"
#include <sys/times.h>

//
// forward declarations
//

//
// file scope globals
//
trec_field_t Results[NFIELDS_OUT];             // output fields from trec
dimension_data_t Dim_data;                     // geometry of output fields

MDV_handle_t        Mdv_out;
MDV_handle_t        Mdv_in0, Mdv_in1;                // mdv dataset pointers 

ui08 ***Dbz0, ***Dbz1;                         // original reflectivity data
ui08 ***Vel0, ***Vel1;                         // original velocity data

int   nrepeat_beams;                           // number of wrapped-around beams
int   Num_gate,   Num_azim,   Num_elev;        // geometry of
float Delta_gate, Delta_azim, Delta_elev;      // input data
float *Range,     *Azim,      *Elev;

float Dbz_scale0, Dbz_bias0;                   // scale and bias of
float Vel_scale0, Vel_bias0;                   // input data

float Dbz_bad0, Dbz_missing0;                  // bad and missing values of
float Dbz_bad1, Dbz_missing1;                  // input data

float Delta_time;                              // time between volumes (sec)
float Max_distance;                            // search radius (km)

int Max_vec, Num_vec;                          // max and actual number of vectors
int iaz1[32], iaz2[32];                        // vector indicies per elevation
                                               // max # elevations into
                                               // trec = 32
  
float *U, *V, *X, *Y, *Z, *Dop;                // tkcomp results
float ***Ucart, ***Vcart, ***Wcart;            // convenience pointers to 
float ***Conv, ***Dcart;                       // output fields in results.data

double before, after;                          // for timing debug messages

static int get_mdv_data( char *file0, char *file1 );

int process_files( char* file0, char* file1 )
{
   int status;

   MDV_init_handle( &Mdv_in0 );
   MDV_init_handle( &Mdv_in1 );
   MDV_init_handle( &Mdv_out );

   if (Glob->params.debug) {
      printf( "%s: Begin process_files with:\n"
              "%s:    '%s' & \n"
              "%s:    '%s'\n", 
               Glob->prog_name, 
               Glob->prog_name, file0, 
               Glob->prog_name, file1 );
   }

   //
   // Load up the original data
   //
   status = get_mdv_data( file0, file1 );
   if ( status == FAILURE )
     return( FAILURE );
  
   //
   // Replace the original data with synthetic data if
   // we're just doing some testing.  Otherwise, handle
   // missing data.
   //
   if ( Glob->params.use_synthetic_data )
     set_synthetic_data();
   else
      set_missing_data();

   //
   // perform x-correlation in polar space
   //
   if (Glob->params.debug) {
      before =  clock() / 1000000.0;
   }

   tkcomp( Dbz0, Dbz1, Vel1, 
           Dbz_scale0, Dbz_bias0, Vel_scale0, Vel_bias0,
           Max_distance, Delta_time, Delta_azim, Delta_gate,
           Azim, Num_azim, Elev, Num_elev, Range, Num_gate, 
           Glob->radar_altitude, &Dim_data, Max_vec, 
           iaz1, iaz2,
           U, V, X, Y, Z, Dop, &Num_vec );

   if (Glob->params.debug) {
      after =  clock() / 1000000.0;
      printf( "%s:    tkcomp took %g CPU secs.\n", 
              Glob->prog_name, after - before);
      before = after;
   }

   //
   // Resample polar data onto cart grid using least-squares
   //
   if ( Glob->params.debug ) {
      printf( "%s: Beginning least squares fit to output grid\n",
               Glob->prog_name );
   }
   lstsq( &Dim_data, U, X, Y, Ucart, iaz1, iaz2, Elev );
   lstsq( &Dim_data, V, X, Y, Vcart, iaz1, iaz2, Elev );
   lstsq( &Dim_data, Dop, X, Y, Dcart,iaz1, iaz2, Elev );

   if (Glob->params.debug) {
      after =  clock() / 1000000.0;
      printf( "%s:    lstsq took %g CPU secs.\n", 
              Glob->prog_name, after - before);
      before = after;
   }

   //
   // Compute w - vertical velocity - from u and v
   // Also compute convergence
   //
   wair( &Dim_data, Ucart, Vcart, Wcart, Conv );

   if (Glob->params.debug) {
      after =  clock() / 1000000.0;
      printf( "%s:    wair took %g CPU secs.\n", 
              Glob->prog_name, after - before);
   }
		
   //
   // Write trec results to mdv file
   //
   status = write_mdv_file( &Mdv_out, &Dim_data, file0, file1, Results );

   //
   // Clean up
   //

   MDV_free_volume3d((void ***) Dbz0, nrepeat_beams );
   MDV_free_volume3d((void ***) Dbz1, nrepeat_beams );

   MDV_free_volume3d((void ***) Vel0, nrepeat_beams );
   MDV_free_volume3d((void ***) Vel1, nrepeat_beams );

   MDV_free_handle ( &Mdv_in0 );
   MDV_free_handle ( &Mdv_in1 );
   MDV_free_handle ( &Mdv_out );

   return status;
}

static int get_mdv_data( char *file0, char *file1 )
{
   static bool     first = true;
   int             dbz_field0, dbz_field1;
   int             vel_field0, vel_field1;
   int             i, status;

   PMU_auto_register( "Reading original images." );

   //
   // Get mdv dataset pointers
   //
   status = MDV_read_all( &Mdv_in0, file0, MDV_INT8 );
   if ( status == MDV_FAILURE ) {
      fprintf( stderr,"%s: Error.  Cannot read mdv dataset '%s'\n",
               Glob->prog_name, file0 );
      return(FAILURE);
   }
   status = MDV_read_all( &Mdv_in1, file1, MDV_INT8 );
   if ( status == MDV_FAILURE ) {
      fprintf( stderr,"%s: Error.  Cannot read mdv dataset '%s'\n",
               Glob->prog_name, file1 );
      return(FAILURE);
   }

   //
   // Find dbz and velocity field indicies in mdv datatsets
   //
   dbz_field0 = find_field( DBZ_FIELD, &Mdv_in0 );
   dbz_field1 = find_field( DBZ_FIELD, &Mdv_in1 );
   vel_field0 = find_field( VEL_FIELD, &Mdv_in0 );
   vel_field1 = find_field( VEL_FIELD, &Mdv_in1 );

   if ( dbz_field0 < 0 || dbz_field1 < 0  ||
        vel_field0 < 0 || vel_field1 < 0 ) {
      return (FAILURE);
   }

   //
   // Set values of some variables for ease of reading
   //
   Dbz_scale0 = Mdv_in0.fld_hdrs[dbz_field0].scale;
   Dbz_bias0  = Mdv_in0.fld_hdrs[dbz_field0].bias;

   Vel_scale0 = Mdv_in1.fld_hdrs[vel_field0].scale;
   Vel_bias0  = Mdv_in1.fld_hdrs[vel_field0].bias;

   Dbz_bad0     = Mdv_in0.fld_hdrs[dbz_field0].bad_data_value;
   Dbz_missing0 = Mdv_in0.fld_hdrs[dbz_field0].missing_data_value;
   Dbz_bad1     = Mdv_in1.fld_hdrs[dbz_field1].bad_data_value;
   Dbz_missing1 = Mdv_in1.fld_hdrs[dbz_field1].missing_data_value;

   Glob->radar_altitude = Mdv_in0.master_hdr.sensor_alt;
   Delta_time = (float)(Mdv_in1.master_hdr.time_begin - 
                        Mdv_in0.master_hdr.time_begin);
   Max_distance = Glob->params.max_vel * 0.001 * Delta_time;

   //
   // Make prototype copies of the mdv headers for the output dataset
   //
   
   MDV_alloc_handle_arrays(&Mdv_out, NFIELDS_OUT,
			   Mdv_in1.master_hdr.max_nz, 0);

   Mdv_out.master_hdr = Mdv_in1.master_hdr;
   for( i=0; i<NFIELDS_OUT; i++ ) {
     Mdv_out.fld_hdrs[i] = Mdv_in1.fld_hdrs[dbz_field1];
     Mdv_out.vlv_hdrs[i] = Mdv_in1.vlv_hdrs[dbz_field1];
   }
   
   //
   // if this is our first time in, save the data geometry
   // (using Dbz0 as a prototype)
   // and allocate a bunch o' data structures
   //
   if ( first ) {
      first = false;
      //
      // Calculate the number of beams to repeat.  This was added to address 
      // the issue of tkcomp not correlating across the 360deg azimuth.  
      // To correct the problem we extend the mdv data arrays, repeat the initial
      // beams of radar data, and tell tkcomp that there are more than 360 azimuths.  
      // Although Delta_time will vary between pairs of radar volumes, we use the 
      // first pair as a bit of a hueristic.  (tlb 9/25/97)
      //
      nrepeat_beams = (int)(2 * Delta_time * Glob->params.max_vel / 350);

      Num_gate = Mdv_in0.fld_hdrs[dbz_field0].nx;
      Num_azim = Mdv_in0.fld_hdrs[dbz_field0].ny + nrepeat_beams;
      Num_elev = Mdv_in0.fld_hdrs[dbz_field0].nz;
      Delta_gate  = Mdv_in0.fld_hdrs[dbz_field0].grid_dx;
      Delta_azim  = Mdv_in0.fld_hdrs[dbz_field0].grid_dy;
      Delta_elev  = Mdv_in0.fld_hdrs[dbz_field0].grid_dz;
      status  = check_geometry( &Mdv_in1.fld_hdrs[dbz_field1] );
      if ( status == FAILURE )
         return FAILURE;
      else {
         //
         // use volume 0 as a prototype for allocating data structures
         //
         setup_data( &Mdv_in0, dbz_field0 );
      }
   }
   else {
      //
      // otherwise, make sure the data geometry 
      // is consistent with previous data
      //
      status = check_geometry( &Mdv_in0.fld_hdrs[dbz_field0] );
      status += check_geometry( &Mdv_in0.fld_hdrs[vel_field0] );
      status += check_geometry( &Mdv_in1.fld_hdrs[dbz_field1] );
      status += check_geometry( &Mdv_in1.fld_hdrs[vel_field1] );
      if ( status != SUCCESS )
         return FAILURE;
   }

   //
   // Set the first dbz and velocity volumes as extended 3D-arrays for tkcomp
   //
   Dbz0 = (ui08 ***) MDV_set_volume3d(&Mdv_in0, dbz_field0, MDV_INT8, nrepeat_beams );
   Vel0 = (ui08 ***) MDV_set_volume3d(&Mdv_in0, vel_field0, MDV_INT8, nrepeat_beams );

   //
   // Set the second dbz and velocity volumes as extended 3D-arrays for tkcomp
   //
   Dbz1 = (ui08 ***) MDV_set_volume3d(&Mdv_in1, dbz_field1, MDV_INT8, nrepeat_beams );
   Vel1 = (ui08 ***) MDV_set_volume3d(&Mdv_in1, vel_field1, MDV_INT8, nrepeat_beams );

   return(SUCCESS);
}

void setup_data( MDV_handle_t *mdv, int field_num )
{
   static bool first = true;
   int i;
   int nx_vec, ny_vec;
   float min_value;

   //
   // if this is our first time, 
   // allocate and initialize the data structures
   //
   if ( first ) {

      //
      // set up dimension structure, i.e. processing grid
      //
      Dim_data.min_x = (short)Glob->params.grid_min_x;
      Dim_data.min_y = (short)Glob->params.grid_min_y;
      Dim_data.min_z = mdv->fld_hdrs[field_num].grid_minz;
  
      Dim_data.del_x = (short)Glob->params.grid_del_x;
      Dim_data.del_y = (short)Glob->params.grid_del_y;
      Dim_data.del_z = mdv->fld_hdrs[field_num].grid_dz;
      
      Dim_data.nx = Glob->params.grid_nx;
      Dim_data.ny = Glob->params.grid_ny;
      Dim_data.nz = mdv->fld_hdrs[field_num].nz;
  
      Dim_data.max_x = Dim_data.min_x + Dim_data.del_x * (Dim_data.nx - 1);
      Dim_data.max_y = Dim_data.min_y + Dim_data.del_y * (Dim_data.ny - 1);
      Dim_data.max_z = Dim_data.min_z + Dim_data.del_z * (Dim_data.nz - 1);
  
      if (Glob->params.debug) {
        printf( "%s: Analysis grid - x1, x2, delx = %g %g %g\n"
                "%s: Analysis grid - y1, y2, dely = %g %g %g\n"
                "%s: Analysis grid - z1, z2, delz = %.1f %.1f %.1f\n",
                 Glob->prog_name, Dim_data.min_x, 
                                  Dim_data.max_x, Dim_data.del_x,
                 Glob->prog_name, Dim_data.min_y, 
                                  Dim_data.max_y, Dim_data.del_y,
                 Glob->prog_name, Dim_data.min_z, 
                                  Dim_data.max_z, Dim_data.del_z );
        clock();
      }

      /*
       * compute max number of vectors possible, allowing an extra 25%
       * in x and y dimensions for safety
       */
      nx_vec = (int)(((Dim_data.max_x - Dim_data.min_x) / 
                       Glob->params.box_spacing) * 1.25);
      ny_vec = (int)(((Dim_data.max_y - Dim_data.min_y) / 
                       Glob->params.box_spacing) * 1.25);
      Max_vec = nx_vec * ny_vec * Dim_data.nz;
  
      /*
       * allocate arrays for trec results
       */
      U   = (float *) umalloc(Max_vec * sizeof(float));
      V   = (float *) umalloc(Max_vec * sizeof(float));
      X   = (float *) umalloc(Max_vec * sizeof(float));
      Y   = (float *) umalloc(Max_vec * sizeof(float));
      Z   = (float *) umalloc(Max_vec * sizeof(float));
      Dop = (float *) umalloc(Max_vec * sizeof(float));

      Ucart = (float ***) umalloc3(Dim_data.nx, Dim_data.ny,
			       Dim_data.nz, sizeof(float));
      Vcart = (float ***) umalloc3(Dim_data.nx, Dim_data.ny,
			       Dim_data.nz, sizeof(float));
      Wcart = (float ***) umalloc3(Dim_data.nx, Dim_data.ny,
			       Dim_data.nz, sizeof(float));
      Conv = (float ***) umalloc3(Dim_data.nx, Dim_data.ny,
			      Dim_data.nz, sizeof(float));
      Dcart = (float ***) umalloc3(Dim_data.nx, Dim_data.ny,
			       Dim_data.nz, sizeof(float));
  
      Results[0].data  = Ucart;
      strcpy( Results[0].name,  "u-motion" );
      strcpy( Results[0].units, "m/s" );

      Results[1].data  = Vcart;
      strcpy( Results[1].name,  "v-motion" );
      strcpy( Results[1].units, "m/s" );

      Results[2].data  = Wcart;
      strcpy( Results[2].name,  "w-motion" );
      strcpy( Results[2].units, "m/s" );

      Results[3].data  = Conv;
      strcpy( Results[3].name,  "convergence" );
      strcpy( Results[3].units, "m/s" );

      Results[4].data  = Dcart;
      strcpy( Results[4].name,  "avg dopler" );
      strcpy( Results[4].units, "m/s" );

      //
      // Setup arrays with values of elevation, azimuth, and range
      //
      Elev  = (float *) umalloc( Num_elev * sizeof(float) );
      Azim  = (float *) umalloc( Num_azim * sizeof(float) );
      Range = (float *) umalloc( Num_gate * sizeof(float) );

      if ( Mdv_out.master_hdr.vlevel_included ) {
         for( i=0; i < Num_elev; i++ )
            Elev[i] = mdv->vlv_hdrs[field_num].vlevel_params[i];
      }
      else {
         min_value = mdv->fld_hdrs[field_num].grid_minz;
         for( i=0; i < Num_elev; i++ )
            Elev[i] = min_value + (i * Delta_elev);
      }

      min_value = mdv->fld_hdrs[field_num].grid_miny;
      for( i=0; i < Num_azim; i++ )
         Azim[i] = min_value + (i * Delta_azim);

      min_value = mdv->fld_hdrs[field_num].grid_minx;
      for( i=0; i < Num_gate; i++ )
         Range[i] = min_value + (i * Delta_gate);

      first = false;
   }
}

int check_geometry( MDV_field_header_t *hdr )
{
   if ((hdr->nx != Num_gate) || 
       (hdr->ny != Num_azim - nrepeat_beams) ||
       (hdr->nz != Num_elev) ||
       (Delta_gate  != hdr->grid_dx) || 
       (Delta_azim  != hdr->grid_dy) ||
       (Delta_elev  != hdr->grid_dz)) {
      fprintf( stderr,"%s: Error. "
              "Cannot analyze datasets of varying geometry!\n",
              Glob->prog_name );
      return FAILURE;
   }
   else
      return SUCCESS;
}

void set_synthetic_data()
{
   static bool first = true;
   int i, j, k;
   float cosel, cosaz, sinaz, x1, y1, r1;

   //
   // Just do this announcement once
   //
   if ( first ) {
      first = false;
      fprintf( stderr, "%s: WARNING - synthetic data being analyzed!\n",
               Glob->prog_name );
   }

   for( i=0; i < Num_elev; i++ ) {
      cosel = cos( DTR*Elev[i] );
      for( j=0; j < Num_azim; j++ ) {
         sinaz = sin( DTR*Azim[j] );
         cosaz = cos( DTR*Azim[j] );
         for( k=0; k < Num_gate; k++ ) {
            r1 = Range[k]*cosel;
            x1 = r1*sinaz;
            y1 = r1*cosaz;
            Dbz0[i][j][k] = 50+(ui08)(fabs(50.*sin(2.*PI*(x1)/15.)*
                                               sin(2.*PI*y1/15.)));
            Dbz1[i][j][k] = 50+(ui08)(fabs(50.*sin(2.*PI*(x1+2.)/15.)*
                                               sin(2.*PI*y1/15.)));
         }
      }
   }
}

void set_missing_data()
{
   int i, j, k;

   //
   // Replace missing or thesholded data (=0) with random data
   // This prevents the data from creating artifacts
   //
   for( i=0; i < Num_elev; i++ ) {
      for( j=0; j < Num_azim; j++ ) {
         for( k=0; k < Num_gate; k++ ) {
            if( Dbz0[i][j][k] == Dbz_bad0  ||  Dbz0[i][j][k] == Dbz_missing0 )
               Dbz0[i][j][k] = 50 + ((ui08)(random()))/20;
            if( Dbz1[i][j][k] == Dbz_bad1  ||  Dbz1[i][j][k] == Dbz_missing1 )
               Dbz1[i][j][k] = 50 + ((ui08)(random()))/20;
         }
      }
   }
}
