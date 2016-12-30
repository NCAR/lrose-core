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
//  $Id: DrawQueue.hh,v 1.12 2016/10/17 22:57:30 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DrawQueue_INC
#define _DrawQueue_INC

#include <string>
#include <unistd.h>
#include <Fmq/DsFmq.hh>
using namespace std;

// A nice bundle of data to be returned as a const reference
typedef struct {
       time_t issueTime;
       time_t data_time;
       si32  id_no;
       si32  valid_seconds;
       si32  num_points;
       si32  vlevel_num;
       fl32  vlevel_ht_min;
       fl32  vlevel_ht_cent;
       fl32  vlevel_ht_max;
       string id_label;
       string prod_label;
       string sender;
       double *lat_points; // Points to num_points doubles
       double *lon_points; // Points to num_points doubles
} Human_Drawn_Data_t;

class DrawQueue : public DsFmq
{
public:
   DrawQueue();

   enum msgType {  HUMAN_DRAWN_PRODUCT};

   //
   // FMQ method for receiver application - Returns a const reference to
   //      A nicely bundled data struct
   //
   const Human_Drawn_Data_t  &nextProduct(int &status);

   //
   // FMQ method for source applications - Returns 0 on success
   //
   int  sendProduct( time_t issueTime, 	// When User pressed  OK
		     time_t data_time,  // The time stamp of the data the user
					// was looking at when drawing 
		     si32  id_no,       // A user chosen serial number
		     si32  valid_seconds, // How many seconds the user feels
					  // the product should remain valid
		     ui32  num_points,    // The number of points in the
					  // drawing
		     si32  vlevel_num,      // Index of the current vertical
		                            // level of the data the user was
		                            // displaying
		     fl32  vlevel_ht_min,   // Minimum value of the vertical
		                            // height of the data the user was
		                            // displaying in native units
		     fl32  vlevel_ht_cent,  // Centroid of the vertical height
		                            // of the data the user was
		                            // displaying in native units
		                            // (this would match the MDV
		                            // vlevel height value)
		     fl32 vlevel_ht_max,    // Maximum value of the vertical
		                            // height of the data the user was
		                            // displaying in native units
		     const char *id_label,      // A String to Determine Type
					  // Of the product. The receiver will
					  // ignore messages they don't recognize
		     const char *prod_label_text, // Text to be displayed with the product
					    // Can also be used to store
					    // additional arguments for
					    // specific products
		     const char *sender_name,     // The senders program name
		     double *lat_points,    // The point data arrays
		     double *lon_points
                    );


   static const int ID_label_Len=32;
   static const int Prod_label_Len=128;
   static const int Name_Len=128;


private:
   //
   // The Struct where the latest message data is kept
   //
   Human_Drawn_Data_t Human_Product;

   //
   // Process management
   //
   pid_t            processId;

   typedef struct {
      double lat;
      double lon;
    } latlon_pt_t;

   //
   // Human Drawn Product Message
   //
   typedef struct {
      si32  issue_time;
      si32  data_time;
      ui32  id_no;
      si32  valid_seconds;
      si32  num_points;
      si32  vlevel_num;
      fl32  vlevel_ht_min;
      fl32  vlevel_ht_cent;
      fl32  vlevel_ht_max;
      char  id_label[ID_label_Len];
      char  prod_label_text[Prod_label_Len];
      char  saysWho[Name_Len];
      latlon_pt_t point[1]; // Message actually has num_points of these.
   } Human_Drawn_Msg_t;
};

#endif
