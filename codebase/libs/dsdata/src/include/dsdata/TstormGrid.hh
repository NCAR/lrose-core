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
///////////////////////////////////////////////////////////////////
// TstormGrid class
//   Contains information about the grid used for thunderstorms
//
// $Id: TstormGrid.hh,v 1.10 2017/06/09 16:27:58 prestop Exp $
///////////////////////////////////////////////////////////////////
#ifndef _TSTORM_GRID_HH
#define _TSTORM_GRID_HH

#include <cstdio>
#include <string>

#include <euclid/Pjg.hh>
#include <euclid/PjgTypes.hh>
#include <rapformats/tstorm_spdb.h>
using namespace std;

class TstormGrid : public Pjg
{

 public:

   //////////////////
   // Constructors //
   //////////////////

   TstormGrid();


   ////////////////
   // Destructor //
   ////////////////

   ~TstormGrid(){};


   ////////////////////////////
   // Initialization methods //
   ////////////////////////////

   int set( titan_grid_t* tgrid );


   ////////////////////
   // Access methods //
   ////////////////////

   int getProjType() const { return projection; }

   const string &getXUnits() const { return xUnits; }
   const string &getYUnits() const { return yUnits; }
   const string &getZUnits() const { return zUnits; }
  

   void setGridValues(titan_grid_t &titan_grid) const;
  

   //////////////////////////
   // Input/Output methods //
   //////////////////////////

   void print(ostream &out, const string &leader = "") const;
  
   void print(FILE *out, const string &leader) const;
  
   static const string projType2String(const PjgTypes::proj_type_t proj_type);
  
 private:

   //
   // Grid origin in deg
   //
   float originLat;
   float originLon;

   //
   // Grid dimensions
   //
   bool  dzConstant;

   //
   // Sensor information
   //
   float sensorX;
   float sensorY;
   float sensorZ;
   float sensorLat;
   float sensorLon;

   //
   // Units
   //
   string xUnits;
   string yUnits;
   string zUnits;

   //
   // Projection information
   //
   PjgTypes::proj_type_t projection;
   int      setProjection( titan_grid_t* tgrid );
   void     clearProjInfo();

   //
   // For flat projection
   //
   float rotation;
   
   //
   // For Lambert Conformal projection
   //
   float lat1;
   float lat2;
   float SwLat;
   float SwLon;
   float originX;
   float originY;

   //
   // Set members to defaults
   //
   void clear();

};

#endif
   
   
   
