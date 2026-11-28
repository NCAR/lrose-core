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

/************************************************************************
 * MdvErodeEdge: MdvErodeEdge program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2007
 *
 * Dan Megenhardt
 *
 ************************************************************************/

#ifndef MdvErodeEdge_HH
#define MdvErodeEdge_HH

#include <map>
#include <string>
#include <stdint.h>
#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <euclid/EllipticalTemplate.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class MdvErodeEdge
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~MdvErodeEdge(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static MdvErodeEdge *Inst(int argc, char **argv);
  static MdvErodeEdge *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static MdvErodeEdge *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // Template used for finding surrounding grid squares within the defined
  // radius of influence.  The actual grid points will change if the field
  // projections change.

  mutable EllipticalTemplate _template;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  MdvErodeEdge(int argc, char **argv);
  

  /*********************************************************************
   * _doTheWork() - Fill grid points with missing data values when 
   *                the fill_percent is below threshold. This is
   *                done for the given field. Input int8_t data.
   *
   * Returns true on success, false on failure.
   */

  void _doTheWork(int8_t *data, Mdvx::field_header_t &field_hdr) const;

  /*********************************************************************
   * _doTheWork() - Fill grid points with missing data values when 
   *                the fill_percent is below threshold. This is
   *                done for the given field. Input int16_t data.
   *
   * Returns true on success, false on failure.
   */

  void _doTheWork(int16_t *data, Mdvx::field_header_t &field_hdr) const;

  /*********************************************************************
   * _doTheWork() - Fill grid points with missing data values when 
   *                the fill_percent is below threshold. This is
   *                done for the given field. Input fl32 data.
   *
   * Returns true on success, false on failure.
   */

  void _doTheWork(fl32 *data, Mdvx::field_header_t &field_hdr) const;

  /*********************************************************************
   * _fillMissingFields() - Fill grid points with missing data values when 
   *                        the fill_percent is below threshold. This is
   *                        done for the given field.
   *
   * Returns true on success, false on failure.
   */

  void _fillMissingField(MdvxField &field) const;
  

  /*********************************************************************
   * _fillMissingFields() - Fill grid points with missing data values when 
   *                        the fill_percent is below threshold. This is
   *                        done for all fields in the file. 
   *
   * Returns true on success, false on failure.
   */

  void _fillMissingFields(Mdvx &mdvx) const;
  

  /*********************************************************************
   * _initTrigger() - Initialize the data trigger.
   *
   * Returns true on success, false on failure.
   */

  bool _initTrigger(void);
  

  /*********************************************************************
   * _processData() - Process data for the given trigger time.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const DateTime &trigger_time);
  

  /*********************************************************************
   * _readInputFile() - Read the indicated input file.
   *
   * Returns true on success, false on failure.
   */

  bool _readInputFile(Mdvx &input_file,
		      const DateTime &trigger_time) const;
  

  /*********************************************************************
   * _updateTemplate() - Update the radius of influence template based on
   *                     the current grid projection.
   */

  void _updateTemplate(const MdvxPjg &proj) const;
  

};


#endif
