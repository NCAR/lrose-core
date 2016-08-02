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
////////////////////////////////////////////////////////////////////////////////
//
//  $Id: DrawQueue.cc,v 1.11 2016/03/03 18:08:49 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include <dataport/bigend.h>
#include <Fmq/DrawQueue.hh>
using namespace std;


// Constructor
DrawQueue::DrawQueue()
             :DsFmq()
{
   processId        = getpid();

   Human_Product.lat_points = NULL;
   Human_Product.lon_points = NULL;
}

int
DrawQueue::sendProduct( time_t issueTime, time_t data_time,
                            si32  id_no, si32  valid_seconds, ui32  num_points,
			    si32 vlevel_num, fl32 vlevel_ht_min,
			    fl32 vlevel_ht_cent, fl32 vlevel_ht_max,
			    const char *id_label, const char *prod_label_text,
			    const char *sender_name, 
			    double *lat_points, 
			    double *lon_points)
{
   unsigned int i;
   int message_len;
   int status;

   Human_Drawn_Msg_t *h_prod; // The fixed length part of the message

   // compute message length
   message_len = sizeof(Human_Drawn_Msg_t) + (num_points -1) * sizeof(latlon_pt_t);

   // Allocate space for a complete message
   char *buf = new char[message_len];

   // cast the struct onto the buffer
   h_prod = (Human_Drawn_Msg_t *) buf;

   //
   // Setup the Human_Drawn_Msg_t message
   // Do the necessary byte swapping
   //
   h_prod->issue_time = BE_from_si32((si32)issueTime );
   h_prod->data_time = BE_from_si32( (si32)data_time);
   h_prod->id_no  = BE_from_si32(id_no);
   h_prod->valid_seconds = BE_from_si32( valid_seconds );
   h_prod->num_points = BE_from_ui32(num_points);
   h_prod->vlevel_num = BE_from_si32(vlevel_num);
   BE_from_fl32(&(h_prod->vlevel_ht_min), &vlevel_ht_min);
   BE_from_fl32(&(h_prod->vlevel_ht_cent), &vlevel_ht_cent);
   BE_from_fl32(&(h_prod->vlevel_ht_max), &vlevel_ht_max);
   
   // Add all the data points
   for(i=0; i < num_points; i++) {
	BE_from_fl64(&h_prod->point[i].lat,&lat_points[i]);
	BE_from_fl64(&h_prod->point[i].lon,&lon_points[i]);
   }

   // Copy the text data 
   strncpy(h_prod->id_label, id_label, ID_label_Len);
   strncpy(h_prod->prod_label_text, prod_label_text, Prod_label_Len);
   strncpy(h_prod->saysWho, sender_name,Name_Len);

   // put it in the FMQ
   if ( writeMsg( HUMAN_DRAWN_PRODUCT, 0, (void*)(h_prod),message_len)) {
      status = -1;
   } else {
      status = 0;
   }

   // Free the buffer
   delete []buf;

   return status;
}

//////////////////////////////////////////////////////
// Returns a const reference to the latest message.
// Status < 0 indicates no new message was retrieved.

const Human_Drawn_Data_t&
DrawQueue::nextProduct( int &status )
{
   int i;
   int        msg_status;                                            
   Human_Drawn_Msg_t  *h_prod;
   bool       gotMsg;
   
   msg_status = readMsg( &gotMsg, HUMAN_DRAWN_PRODUCT );
   if ( msg_status == -1  ||  !gotMsg  ||  getMsgLen() == 0 ) {
      status = -1;
   } else {
      
      // Do the necessary byte swapping and fill the data bundle
      //
      h_prod    = (Human_Drawn_Msg_t*)getMsg();
      Human_Product.issueTime =  (time_t)BE_to_si32( h_prod->issue_time );
      Human_Product.data_time =  (time_t)BE_to_si32( h_prod->data_time );
      Human_Product.id_no =  BE_to_ui32( h_prod->id_no );
      Human_Product.valid_seconds =  BE_to_si32( h_prod->valid_seconds );
      Human_Product.num_points =  BE_to_si32( h_prod->num_points );
      Human_Product.vlevel_num = BE_to_si32( h_prod->vlevel_num );
      BE_to_fl32(&(h_prod->vlevel_ht_min), &(Human_Product.vlevel_ht_min));
      BE_to_fl32(&(h_prod->vlevel_ht_cent), &(Human_Product.vlevel_ht_cent));
      BE_to_fl32(&(h_prod->vlevel_ht_max), &(Human_Product.vlevel_ht_max));
      
      // Strings
      Human_Product.id_label = h_prod->id_label;
      Human_Product.prod_label = h_prod->prod_label_text;
      Human_Product.sender = h_prod->saysWho;

      // Data Points - Free old and allocate space for new
      if(Human_Product.lat_points != NULL) delete []Human_Product.lat_points;
      if(Human_Product.lon_points != NULL) delete []Human_Product.lon_points;

      Human_Product.lat_points = new double[Human_Product.num_points];
      Human_Product.lon_points = new double[Human_Product.num_points];

      for(i=0; i < Human_Product.num_points; i++) {
	  BE_to_fl64(&h_prod->point[i].lat,&Human_Product.lat_points[i]);
	  BE_to_fl64(&h_prod->point[i].lon,&Human_Product.lon_points[i]);
      }

      status = 0;
   }

   return((Human_Product));  
}
