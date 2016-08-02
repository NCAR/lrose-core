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
 * @file PhaseFit.hh
 *
 * @class PhaseFit
 *
 * Class for performing the phase fitting algorithm.
 *  
 * @date 3/29/2010
 *
 */

#ifndef PhaseFit_HH
#define PhaseFit_HH

#include <dataport/port_types.h>

using namespace std;


/** 
 * @class PhaseFit
 */

class PhaseFit
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Pointer to the phase data to fit.  The calling method must set
   *        these values after calling init() and before calling
   *        fitPhaseField().  There are num_beams * num_gates values in this
   *        array.
   */

  float *phase;

  /**
   * @brief Pointer to the quality data to fit.  The calling method must set
   *        these values after calling init() and before calling
   *        fitPhaseField().  There are num_beams * num_gates values in this
   *        array.
   */

  float *quality;

  /**
   * @brief Pointer to the I data.  The calling method must set these values
   *        after calling init() and before calling fitPhaseField().  There
   *        are num_beams * num_gates values in this array.
   */

  float *inphase;

  /**
   * @brief Pointer to the Q data.  The calling method must set these values
   *        after calling init() and before calling fitPhaseField().  There
   *        are num_beams * num_gates values in this array.
   */

  float *quadrature;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   */

  PhaseFit();
  
  /**
   * @brief Destructor
   */

  virtual ~PhaseFit(void);
  
  /**
   * @brief Initialize the object.
   *
   * @param[in] num_beams Number of beams in the data.
   * @param[in] num_gates Number of gates in the data.
   * @param[in] gate_spacing Gate spacing in meters.
   * @param[in] wavelength The wavelength for the radar.
   * @param[in] min_consistency Minimum consistency of phase to accept
   *                            the measurement.  Higher means smaller
   *                            coverage of better data.
   * @param[in] r_min Minimum rage get of ground echo.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose flag.
   *
   * @return Returns true on success, false on failure.
   */

  bool init(const int num_beams,
	    const int num_gates,
	    const double gate_spacing,
	    const double wavelength,
	    const double min_consistency,
	    const int r_min,
	    const bool debug_flag = false,
	    const bool verbose_flag = false);
  


  /**
   * @brief Smooth the given phase field and compute the related N-field and
   *        its error.  Before calling this method, the caller must call
   *        the init() method, set the values in the phase, quality, inphase
   *        and quadrature arrays and call the following access methods:
   *        setRefN(), setSmoothSideLen(), setNOutput() and setNError().
   *
   * @param[in] do_relax Flag indicating whether to perform the relaxation
   *                     step.
   *
   * @return Returns true on success, false on failure.
   */

  bool fitPhaseField(const bool do_relax);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Set the reference N value to use in the calculations.
   */

  void setRefN(const double ref_n)
  {
    _refN = ref_n;
  }
  

  /**
   * @brief Set the smoothing side length value to use in the calculations.
   */

  void setSmoothSideLen(const double smooth_sidelen)
  {
    _smoothSideLen = smooth_sidelen;
  }
  

  /**
   * @brief Set the N output pointer.  This is the pointer to the location
   *        where the calculated N values will be written.  This array must
   *        be allocated by the calling method before this call and must
   *        be deleted by the calling method when no longer needed.
   */

  void setNOutput(fl32 *n_output)
  {
    _nOutput = n_output;
  }
  
  /**
   * @brief Set the N error pointer.  This is the pointer to the location
   *        where the calculated N error values will be written.  This array
   *        must be allocated by the calling method before this call and must
   *        be deleted by the calling method when no longer needed.
   */

  void setNError(fl32 *n_error)
  {
    _nError = n_error;
  }
  

 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  /**
   * @brief Very large data value.
   */

  static const double VERY_LARGE;

  /**
   * @brief Invalid data value.
   */

  static const float INVALID;

  /**
   * @brief Number of azimuths to smooth over when calculating the average
   *        slope.
   */

  static const int SMEAR_AZ;

  /**
   * @brief Number of azimuths to smooth over when calculating the initial
   *        slope.
   */

  static const int SMEAR_AZ_INIT;

  /**
   * @brief Number of bins to smooth over when calculating the average slope.
   */

  static const int SMEAR_RA;

  /**
   * @brief Number of meters to use in the initial slope calculation.
   */

  static const int INITIAL_SLOPE_LEN_M;

  /**
   * @brief Minimum number of iterations.
   */

  static const int MIN_ITER;
  
  /**
   * @brief Maximum number of iterations.
   */

  static const int MAX_ITER;
  
  /**
   * @brief Minimum absolute consistency value.
   */

  static const double MIN_ABS_CONSISTENCY;


  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  
  /**
   * @brief Verbose debug flag.
   */

  bool _verbose;
  
  /**
   * @brief Number of beams in the data.
   */

  int _numBeams;
  
  /**
   * @brief Number of gates in the data.
   */

  int _numGates;
  
  /**
   * @brief Gate spacing in meters
   */

  double _gateSpacing;
  
  /**
   * @brief Wavelength of data.
   */

  double _wavelength;
  
  /**
   * @brief Minimum consistency of phase to accept N (DN) measurement.
   *        Higher means smaller coverage of (hopefully) better data.
   */

  double _minConsistency;
  
  /**
   * @brief Minimum range gate of ground echo.
   */

  int _rMin;
  
  /**
   * @brief Azimuth spacing in degrees.
   */

  double _azimSpacing;
  
  /**
   * @brief Reference N value.
   */
  
  double _refN;

  /**
   * @brief Smoothed phase field.
   */

  float *_phaseFit;

  /**
   * @brief Error in the smoothed phase field.
   */

  float *_phaseError;

  /**
   * @brief I component of the smoothed phase field.
   */

  float *_smoothI;

  /**
   * @brief Q component of the smoothed phase field.
   */

  float *_smoothQ;

  /**
   * @brief Calculated N values.  This pointer points to space not owned by
   *        this object and must not be deleted by this object.
   */

  fl32 *_nOutput;

  /**
   * @brief Error in the calculated N value.  This is a function of sigma(N)
   *        and the input data density/quality.  This pointer points to space
   *        not owned by this object and must not be deleted by this object.
   */

  fl32 *_nError;

  /**
   * @brief Expected phase for 1st range.
   */

  double _expectedPhaseRange0;

  /**
   * @brief Average d(Phase)/d(range)
   */

  double _rangeSlope;

  /**
   * @brief Smoothing side length in meters.
   */

  double _smoothSideLen;


  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Calculate the square of the given value.
   *
   * @param[in] Value to square.
   *
   * @return Returns the square of the given value.
   */

  static double SQR(double value)
  {
    return value * value;
  }
  

  /**
   * @brief Smooth the phase measurements of the map information loaded
   *        in this object.
   *
   * @return Returns the initial range slope value for this scan.  Returns
   *         INVALID on error.
   */

  float _doSmoothing();
  

  /**
   * @brief Compute average slope of phase data with range 
   *   using a pulse-pair method.
   *   Output: Average slope in degrees per gatespacing.
   *   Called by: _fitPhases() and _doSmoothing()
   *
   * @param[in] inphase The I values.
   * @param[in] quadrature The Q values.
   * @param[in] num_azim Number of azimuths in data.
   * @param[in] num_range Number of ranges in data.
   * @param[in] range0 First useful range bin.
   * @param[in] max_r
   * @param[in] smear_az
   *
   * @return Returns the average slope of the phase data.  Returns INVALID
   *         on error.
   */

  double _meanPhaseSlope(const int max_r, const int smear_az) const;
  

  /**
   * @brief Compute the average slope of phase data with range using a
   *        pulse-pair method.
   *
   * @return Returns the calculated slope value.  Returns INVALID on
   *         error.
   */

  double _meanPhaseSlopeAvg() const;
  

  /**
   * @brief Compute the initial slope of phase data with range using a
   *        pulse-pair method.
   *
   * @return Returns the calculated slope value.  Returns INVALID on
   *         error.
   */

  double _meanPhaseSlopeInit() const;
  

  /**
   * @brief Evaluate the phase at range RMin.  Since it is at very close
   *        range, azimuthal dependance of phase is unlikely and it provides a
   *        needed starting point to compute N from d(Phi)/dr.
   *
   * @return Returns the calculated phase value.
   */

  double _phaseRange0();
  

  /**
   * @brief Perform a quality-dependent iterative smoothing of the
   *        refractivity field, constrained by the smoothed phase field.
   *   Uses an iterative "diffusion and relaxation" approach to the problem, 
   *   even if it takes an eternity:
   *   First the value of N are nudged to match better the phase data,
   *   then the field is smoothed.  As a result, the regions where data are
   *   available gradually percolate into regions where they are not.
   *   Input: Phases_to_fit structure and initial N field data.
   *   Output: Updated N field data.
   *   Called by: fit_phase_field().
   */

  int _relax(float *ndata);
  

};


#endif
