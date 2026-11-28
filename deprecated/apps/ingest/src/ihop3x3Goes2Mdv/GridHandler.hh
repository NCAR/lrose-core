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
 *   $Locker:  $
 *   $Date: 2016/03/07 01:23:08 $
 *   $Id: GridHandler.hh,v 1.5 2016/03/07 01:23:08 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * GridHandler: GridHandler program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak/Kay Levesque
 *
 ************************************************************************/

#ifndef GridHandler_HH
#define GridHandler_HH

#include <string>

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

using namespace std;


class GridHandler
{
 public:

  typedef struct
  {
    DateTime date_time;
    double latitude;
    double longitude;
    double temp;
    double dewpoint;
    double pressure;
    int height;
    int retrieval_type;
  }sounder_data_t;

  static const double MISSING_DATA_VALUE;


  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  GridHandler(const bool debug = false);
  

  /*********************************************************************
   * Destructor
   */

  ~GridHandler(void);
  
  /*********************************************************************
   * Access methods
   */

  void setDebug(const bool debug)
  {
    _debug = debug;
  }

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init(const MdvxPjg& projection, const vector<double> pressureLevels);

  /*********************************************************************
   * updateGrid() - update the corresponding grid values for the given sounding
   *
   * Returns true if successful, false otherwise.
   */

  bool updateGrid(const sounder_data_t &sounder_data);

  /*********************************************************************
   * writeGrid() - write grid to given URL.
   *
   * Returns true if successful, false otherwise.
   */
  
  bool writeGrid(const string &url, const string &dataSource);
  
private:

  //add other _create Other Fields methods here.

  MdvxField* _createTemperatureField(const MdvxPjg& projection,
				     const vector<double> pressureLevels);
  MdvxField* _createTemperatureCField(const MdvxPjg& projection,
				      const vector<double> pressureLevels);
  MdvxField* _createDewpointField(const MdvxPjg& projection,
				     const vector<double> pressureLevels);
  MdvxField* _createDewpointCField(const MdvxPjg& projection,
				   const vector<double> pressureLevels);
  MdvxField* _createPressureField(const MdvxPjg& projection,
				     const vector<double> pressureLevels);
  MdvxField* _createHeightField(const MdvxPjg& projection,
				     const vector<double> pressureLevels);

  MdvxField* _temperatureField;
  MdvxField* _temperatureCField;
  MdvxField* _dewpointField;
  MdvxField* _dewpointCField;
  MdvxField* _pressureField;
  MdvxField* _heightField;
  MdvxPjg _projection;
  vector<double> _pressureLevels;
  DateTime _startTime, _endTime;

  bool _debug;
  bool _dataFound;

};


#endif

