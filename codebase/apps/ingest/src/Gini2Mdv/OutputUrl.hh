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
/*
 *  $Id: OutputUrl.hh,v 1.4 2016/03/07 01:23:01 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header: OutputUrl
// 
// Author: G M Cunning, Modified for Gini2Mdv by Steve Mueller.
// 
// Date: June 2006
// 
// Description:	This class manages the output MDV data.
// 
// 
// 
// 
# ifndef    OUTPUT_URL_H
# define    OUTPUT_URL_H

// C++ include files
#include <string>

// System/RAP include files
#include <Mdv/Mdvx.hh>

// Local include files
#include "GiniPDB.hh"

using namespace std;

// Forward Declarations
class DsMdvx;
class MdvxField;
class Params;
class ProcessGiniFile;
class InputManager;
class GiniCalibrationCurve;

class OutputUrl
   {
   public:
      // Public methods
      // Constructors and more
      OutputUrl(Params *params);
      virtual ~OutputUrl();

      bool isOK() const { return _isOk; }
      string getErrStr() const { return _errStr; }

      // clear out the current information in the output URL
      void clear();
  
      void setMdvMasterHeader();
      void setTimes(const time_t& centroid, const time_t& begin = -1, const time_t& end = -1); 
      
      Mdvx::field_header_t buildMdvFieldHeader(GiniPDB *decodedGiniPDBStruct,
		                               float badDataValue);
      Mdvx::vlevel_header_t buildMdvVlevelHeader();
      MdvxField* buildMdvField(ProcessGiniFile *processGiniFile,
		               InputManager *inputManager,
			       GiniCalibrationCurve *calibrationCurve,
			       Params *params);

      // add string to data set info
      void addToInfo(const string& info_str);
      void addToInfo(const char* info_str);

      // initialize the headers
      void addField(MdvxField* in_field);

      // write out merged volume
      bool writeVol();

      // No public data members

   protected:
      // No protected methods
      // No protected data members

   private:
      // Private data members
      bool _isOk;
      string _errStr;
      static const string _className;

      Mdvx::master_header_t _mdvMasterHeader;
      Params *_params;
      DsMdvx *_mdvx;
      string _url;
      string _path;
      time_t _centroidTime;
      time_t _mergeTime;
      time_t _startTime;
      time_t _endTime;

      // No private methods
   };
# endif     /* OUTPUT_URL_H */
