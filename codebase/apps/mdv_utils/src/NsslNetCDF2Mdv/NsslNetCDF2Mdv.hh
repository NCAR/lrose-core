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
/**
 * @file NsslNetCDF2Mdv.hh
 * @brief Main algorithm driver class
 * @class NsslNetCDF2Mdv
 * @brief Main algorithm driver class
 */

#ifndef NsslNetCDF2Mdv_HH
#define NsslNetCDF2Mdv_HH

#include "FieldSpec.hh"
#include <Mdv/DsMdvx.hh>
#include <string>
#include <map>
#include <vector>

class Args;
class Params;
class DsTrigger;
class VolumeTrigger;
class DateTime;
class NsslData;
class TimeElev;
class SweepFile;

class NsslNetCDF2Mdv
{
 public:

  /**
   * Flag indicating whether the program status is currently okay.
   */
  bool okay;


  /**
   * Destructor
   */
  virtual ~NsslNetCDF2Mdv(void);
  

  /**
   * Retrieve the singleton instance of this class.
   * @param[in] argc
   * @param[in] argv
   */
  static NsslNetCDF2Mdv *Inst(int argc, char **argv);

  /**
   * Retrieve the singleton instance of this class.
   */
  static NsslNetCDF2Mdv *Inst(void);
  

  /**
   * Initialize the local data.
   * @return true if the initialization was successful, false otherwise.
   */
  bool init(void);
  

  /**
   * run the program.
   */
  void run(void);
  

 private:

  /**
   * Singleton instance pointer
   */
  static NsslNetCDF2Mdv *_instance;
  
  std:: string _progName;             /**< Program name */
  Args *_args;                        /**< Command args */
  Params *_params;                    /**< Program params */
  DsTrigger *_dataTrigger;            /**< Triggering base class ptr */
  VolumeTrigger *_volumeTrigger;      /**< Volume change base class ptr */
  std::string _outputUrl;            /**< Output URL */
  
  /**
   * Spec for each output URL, first = output field directory, second = vector
   * of specifications for each input field that goes to this output location
   */
  std::map<std::string,std::vector<FieldSpec> > _fields;

  /**
   * Iterator over _fields
   */
  typedef std::map<std::string,std::vector<FieldSpec> >::iterator field_it;

  DsMdvx _mdvFile;                   /**< Output object */

  /**
   * Constructor, private because its a singleton 
   */
  NsslNetCDF2Mdv(int argc, char **argv);
  
  void _formInputs(std::vector<NsslData> &nsslData,
		   std::vector<TimeElev> &timeElev) const;
  void _processFiles(const std::string &outPath, 
		     const std::vector<FieldSpec> &field, 
		     const std::vector<NsslData> &data,
		     const std::vector<TimeElev> &timeElev);
  bool _processTimeElev(const TimeElev timeElev,
			const std::string &outPath,
			const std::vector<FieldSpec> &field, 
			const std::vector<NsslData> &data,
			bool firstForThisOutput, bool lastTimeElev);
  bool 
  _processFilesAtTime(const std::string &outPath,
		      std::vector<std::pair<NsslData,FieldSpec> > &data,
		      bool firstForThisOutput, bool lastTimeElev);
  bool _processFileAtTime(NsslData &data, const FieldSpec &spec,
			  const std::string &outPath, bool first, 
			  bool firstField, bool lastTimeElev, bool lastField);
  bool _finishVolume(const SweepFile &sweep_file, const std::string &outPath);
  bool _writeMdvFile(DsMdvx &mdv_file, const DateTime &volume_end_time,
		     const std::string &radarName,
		     const std::string &outPath);
};


#endif
