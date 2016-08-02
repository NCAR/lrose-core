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

//
// Small routine to perpetrate normalization on a lidar beam
// according to ITT's specifications. Niles Oien December 2006.
//
// Returns 0 if OK, -1 if error.
//

int normBeam(float *oldData,       // Pointer to the input beam.
	     float *newData,       // Pointer to the (already allocated) space for the new beam.
	     double gateSpacing,   // Gate spacing, meters.
	     double startRange,    // Start range, Km
	     double endRange,      // End range, Km
	     int percentToTake,    // Percent to take, percent.
	     double displayOffset, // Display offset
	     int numGatesPerBeam,  // Number of gates in a beam.
	     int debug,            // 0 => silent, 1 => some messages, 2 => lots of messages on stderr
	     double minVal,        // Lower limit on useful data values.
	     double maxVal,        // Upper limit on useful data values.
	     double firstGateRange, // Range to first gate, Km.
	     double minPercentGood); // Min number of good points in normalized range.
