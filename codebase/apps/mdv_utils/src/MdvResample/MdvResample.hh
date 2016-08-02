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
 *
 * @file MdvResample.hh
 *
 * @class MdvResample
 *
 * MdvResample is the top level application class.
 *  
 * @date 8/30/2011
 *
 */

#ifndef MdvResample_HH
#define MdvResample_HH

#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <euclid/EllipticalTemplate.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaThreadDoubleQue.hh>

#include "Args.hh"
#include "Params.hh"
#include "StatCalc.hh"

class ResampleInfo;

using namespace std;


/** 
 * @class MdvResample
 */

class MdvResample
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Flag indicating whether the program status is currently okay.
   */

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Destructor
   */

  ~MdvResample();
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static MdvResample *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static MdvResample *Inst();
  

  /**
   * @brief Initialize the local data.
   *
   * @return Returns true if the initialization was successful, false
   *         otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /**
   * @brief Run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer
   */

  static MdvResample *_instance;
  
  /**
   * @brief Program name.
   */

  char *_progName;

  /**
   * @brief Command line arguments.
   */

  Args *_args;

  /**
   * @brief Parameter file parameters.
   */

  Params *_params;
  
  /**
   * @brief Triggering object
   */

  DsTrigger *_dataTrigger;
  
  /**
   * @brief The desired output projection.
   */

  MdvxPjg _outputProj;
  
  /**
   * @brief Object to use for calculating the statistic used for resampling.
   */

  StatCalc *_statCalc;
  
  /**
   * @brief The current input projection.
   */

  mutable MdvxPjg _inputProj;
  
  /**
   * @brief The template to use for the resampling.  This template is created
   *        based on the input projection and is used to gather the values
   *        used to calculate the statistic at each output grid point.
   */

  mutable GridTemplate *_resampleTemplate;
  
  /**
   * @class ThreadAlg
   * @brief Simple class to instantiate TaThreadDoubleQue by implementing
   *        the clone method
   */
  class ThreadAlg : public TaThreadDoubleQue
  {
  public:
    /**
     * Constructor
     */
    inline ThreadAlg() : TaThreadDoubleQue() {}

    /**
     * Destructor
     */
    inline virtual ~ThreadAlg() {}

    /**
     * Clone a thread and return pointer to base class
     * @param[in] index
     */
    TaThread *clone(const int index);
  };


  /**
   * The threading object
   */
  ThreadAlg _thread;


  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   *
   * @note The constructor is private because this is a singleton object.
   */

  MdvResample(int argc, char **argv);
  

  /**
   * @brief Create a blank output field based on the given input field
   *        information.  The output field data values will be filled with
   *        missing data.
   *
   * @param[in] input_field_hdr     The field header of the input field.
   * @param[in] input_vlevel_hdr    The vlevel header of the input field.
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   */

  MdvxField *_createBlankField(const Mdvx::field_header_t input_field_hdr,
			       const Mdvx::vlevel_header_t input_vlevel_hdr) const;
  

  /**
   * @brief Initialize the output file based on the input file's master
   *        header.
   *
   * @param[in,out] output_file         The output file.
   * @param[in]     input_master_hdr    The master header from the input file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initOutputFile(DsMdvx &output_file,
		       const Mdvx::master_header_t input_master_hdr) const;
  

  /**
   * @brief Initialize the output projection.
   *
   * Returns true on success, false on failure.
   */

  bool _initOutputProj();
  

  /**
   * @brief Initialize the data trigger.
   *
   * Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /**
   * @brief Initialize the statistic calculator.
   *
   * Returns true on success, false on failure.
   */

  bool _initStatCalc();
  

  /**
   * @brief Process data for the given trigger time.
   *
   * @param trigger_time The current trigger time.  Data for this time
   *                     should be processed.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const TriggerInfo &trigger_info);
  

  /**
   * @brief Read the input file.
   *
   * @param[out] mdvx Input file.
   * @param[in] data_time Desired data time.
   *
   * @return Returns true on sucess, false on failure.
   */

  bool _readInputFile(DsMdvx &mdvx,
		      const DateTime &data_time,
		      const DateTime &fcst_time) const;
  

  /**
   * @brief Create a new field that is a resampled version of the given
   *        field.
   *
   * @param[in] input_field      The original field.
   *
   * @return Returns a pointer to the resampled field on success, 0 on false.
   */

  MdvxField *_resampleField(const MdvxField &input_field);
  

  /**
   * @brief Set the input projection being used.
   *
   * @param[in] proj     The input projection being used.
   */

  void _setInputProj(const MdvxPjg &proj) const
  {
    // If the projection hasn't changed, then we don't need to do anything

    if (_inputProj == proj)
      return;
    
    // Save the input projection

    _inputProj = proj;
    
    // Create the new resample template to use.

    delete _resampleTemplate;
    
    _resampleTemplate =
      new EllipticalTemplate(0.0,
			     _inputProj.km2yGrid(_params->resample_radius),
			     _inputProj.km2xGrid(_params->resample_radius));
  }
  
  /**
   * @brief Update the master header in the output file.
   *
   * @param[in,out] mdvx Output MDV file.
   */

  void _updateMasterHeader(DsMdvx &mdvx) const;

  
  /**
   * @brief Write the output file.
   *
   * @param[in] mdvx The output file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _writeFile(DsMdvx &mdvx) const;
  

  /**
   * Static compute method called by tbe thread
   *
   * @param[in] i Information pointer, which is a pointer to ResampleInfo
   */
  static void _compute(void *i);


  /**
   * Method called by the _compute() method, using the context pointer
   * 
   * @param[in] info  The resampling information passed into the thread
   */
  void _resample(const ResampleInfo &info) const;

  /**
   * Resample a single point
   * 
   * @param[in] resampler  The template for resampling
   * @param[in] info  The resampling information passed into the thread
   * @param[in] x     The x point index value
   */
  void _resampleX(GridTemplate *resampler, const ResampleInfo &info,
		  int x) const;
};


#endif
