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

#include "SpdbOut.hh"

#include <ctetwws/smem.h>
#include <physics/physics.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/str.h>
using namespace std;

// constructor. Does nothing.
 
SpdbOut::SpdbOut(){
  __spdb_buffer=NULL;
  __spdb_buffer_alloc=0;       
}
 
// destructor. Does nothing.
 
SpdbOut::~SpdbOut(){
  if (NULL!=__spdb_buffer)
    free(__spdb_buffer);
}
    
// public method.
 
void SpdbOut::WriteOut(Params *P,
		       int num,
		       double *lat,
		       double *lon,
		       time_t dataTime,
		       int leadMinutes,
		       int id,
		       int seqNum,
		       double U,
		       double V){


  id = id + P->ID_offset;

  if (P->debug){
    fprintf(stderr,"For ID %d the points are :\n", id);
    for (int i=0; i < num; i++){
      fprintf(stderr,
	      "%g\t%g\n",lat[i],lon[i]);
    }
  }

  
  // create the object.
  DsSpdb D;
  D.clearPutChunks();
  D.clearUrls();
  D.addUrl( P->OutUrl );


  // load a boundary to a chunk:
  if ((leadMinutes != 0))
    D.setPutMode(Spdb::putModeAdd);
  else
    D.setPutMode(Spdb::putModeOver);

  // set up label values based on whether its an extrap or detect
  if (!_set_label_values(leadMinutes*60))
    return;

  // augment the buffer to write to, based on number of points in bdry.
  _set_buffer_size(num);
      
  __spdb_valid_time = dataTime;
  __spdb_expire_time = dataTime + P->expiry;
  __spdb_id = id;
      
  // possible values are as follows:

  char *name;
  if ((leadMinutes == 0))
    name = "THIN_LINE";
  else 
    name = "EXTRAPOLATED_LINE";
      
  double speed, dir;
  speed=PHYwind_speed(U,V);
  dir=PHYwind_dir(U,V);
  dir = dir + 180.0;
  if (dir >= 360.0) dir = dir - 360.0;

  if (P->debug){
    fprintf(stderr,"U,V   Speed,Dir : %g,%g   %g,%g\n",
	    U,V,speed,dir);
  }

  if (leadMinutes == 0){
    __spdb_data_type = BDRY_TYPE_BDRY_COLIDE;
  } else {
    __spdb_data_type = BDRY_TYPE_EXTRAP_ISS_COLIDE;
  }

  // fill the buffer header.
  __spdb_buffer->type = __spdb_data_type;
  __spdb_buffer->subtype = BDRY_SUBTYPE_ALL;
  __spdb_buffer->sequence_num = seqNum;
  __spdb_buffer->group_id = 0;
  __spdb_buffer->generate_time = time(NULL);
  __spdb_buffer->data_time = __spdb_valid_time;
  __spdb_buffer->forecast_time = __spdb_valid_time;
  __spdb_buffer->expire_time = __spdb_expire_time;
  __spdb_buffer->line_type =
    BDRY_line_type_string_to_line_type(name);
  __spdb_buffer->bdry_id = id;
  __spdb_buffer->num_polylines = 1;
  __spdb_buffer->motion_direction = dir;
  __spdb_buffer->motion_speed = speed;
  __spdb_buffer->line_quality_value = 0.0;
  __spdb_buffer->line_quality_thresh = 0.0;
  STRcopy(__spdb_buffer->type_string, __spdb_product_type, BDRY_TYPE_LEN);
  STRcopy(__spdb_buffer->subtype_string, "ALL", BDRY_TYPE_LEN);
  STRcopy(__spdb_buffer->line_type_string, name, BDRY_LINE_TYPE_LEN);
  sprintf(__spdb_buffer->desc, __spdb_label);

  // fill the points
  BDRY_spdb_point_t *point;
  int pt;
  BDRY_spdb_polyline_t *polyline;
    
  // Update the polyline information (there is only one polyline) 
  polyline = &__spdb_buffer->polylines[0];
  polyline->num_secs = leadMinutes*60;
  STRcopy(polyline->object_label, __spdb_label, BDRY_LABEL_LEN);
	    
  // Update each of the points 
  for (pt = 0; pt < num; pt++){
    point = &polyline->points[pt];
    point->lat = lat[pt];
    point->lon = lon[pt];
    point->u_comp = U;
    point->v_comp = V;
    point->value = 0.0;
  }
  polyline->num_pts = num;
  //
  // finally, store the chunks to output urls.
  //
  if (__spdb_buffer==NULL){
    fprintf(stderr,"I HAVE NULL HERE.\n");
    exit(0);
  }

  BDRY_spdb_product_to_BE(__spdb_buffer);

  D.addPutChunk(__spdb_data_type, __spdb_valid_time,
		__spdb_expire_time, __spdb_buffer_alloc,
		(void *)__spdb_buffer, id); 
  
 
  if (D.put(SPDB_BDRY_ID, SPDB_BDRY_LABEL) != 0)
    {
      fprintf(stderr,	"Error writing chunk.\n");
      return;
    }

  return;
}

//----------------------------------------------------------------
bool SpdbOut::_set_label_values(int extrap_time)
{
    // is this an extrap or a detect?
    if (extrap_time > 0)
    {
	sprintf (__spdb_product_type, "X%d", extrap_time/60);
	sprintf(__spdb_label, "%d+%d", __spdb_id, extrap_time/60);
    }
    else
    {
	sprintf(__spdb_product_type, "BDC");
	sprintf(__spdb_label, "%d", __spdb_id);
    }
    __spdb_data_type = BDRY_type_string_to_type(__spdb_product_type);
    return true;
}


//----------------------------------------------------------------
void SpdbOut::_set_buffer_size(int npt)
{
  int buffer_size = sizeof(BDRY_spdb_product_t) + npt*sizeof(BDRY_spdb_point_t);
  // Allocate space for the outgoing buffer 
  if (__spdb_buffer_alloc < buffer_size)
    {
      unsigned char *c;
      c = (unsigned char *)__spdb_buffer;
      if (c == NULL){
	c = MEM_CALLOC(buffer_size, unsigned char);
	__spdb_buffer_alloc = buffer_size;
      } else {
	c = MEM_GROW(c, buffer_size, unsigned char);
      }
      __spdb_buffer_alloc = buffer_size;
      __spdb_buffer = (BDRY_spdb_product_t *)c;
    }
  memset(__spdb_buffer, 0, __spdb_buffer_alloc);
  return;
}


