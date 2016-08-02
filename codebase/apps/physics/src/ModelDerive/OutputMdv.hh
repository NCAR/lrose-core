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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* RCS info
 *   $Author: dixon $
 *   $Date: 2016/03/06 23:15:37 $
 *   $Revision: 1.7 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * OutputMdv: OutputMdv program object.
 * Handles the output mdv data and vertical interpolation.
 *
 * Output derived fields will be added to the output url file if it
 * exists.  Thus using the same Input and Output Mdv url will add the 
 * derived fields to the input file. If vertical interpolation is on
 * the entire output files fields (with the same 3d dimensions, even
 * fields not output by this program), will be interpolated to the new 
 * vertical levels.
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 *********************************************************************/

// This header should only be included from ModelDeriveMdv.hh

#ifndef OutputMdv_HH
#define OutputMdv_HH

#include <vector>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

//
// Forward class declarations
//
class MdvxField;
class DsMdvx;
class Params;

//
// Mdv helper functions used in ModelDeriveMdv and here
//
char *stripLevelName(char *fieldName);
bool isForecastFile(const char *mdvFileName);


class OutputMdv {
  
public:

  OutputMdv( Params *params, bool writeForecast );
 
  ~OutputMdv();

  //
  // Writes out the list of fields added to the mdv file.
  // writeVol will call clear().
  int  writeVol(Mdvx::master_header_t masterHdr, const char *dataSetInfo, const char *dataSetSource);

  //
  // Clears all data in memory.
  void clear();

  //
  // Adds a field to the list of fields to write out.
  void addField(MdvxField* inputField);

  // Creates a MdvField from the fieldHeader, vlevelHeader and the data.
  // Adds the rest of the arguments to the fieldHeader.
  // createMdvxField will call addField().
  void createMdvxField(Mdvx::field_header_t fieldHeader, Mdvx::vlevel_header_t vlevelHeader,
		       fl32 *data, const char *shortName, const char *longName, 
		       const char *units, char *level = NULL);

  //
  // Returns a previously added field.
  MdvxField *getField(char *fieldName);

protected:
  
private:

  Params *_params;

  DsMdvx *_mdvObj;
  bool _forecast;

  vector<MdvxField *> fields;

  int _convertGribLevel2MDVLevel(char *GribLevel, char *units);
  void _switchVerticalFieldName(char *outLevelName, Mdvx::field_header_t *field_hdr);

};

#endif

