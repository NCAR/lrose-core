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
 *   $Date: 2016/03/06 23:28:57 $
 *   $Id: VariationalEchoTracker.hh,v 1.2 2016/03/06 23:28:57 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * VariationalEchoTracker: VariationalEchoTracker program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef VariationalEchoTracker_HH
#define VariationalEchoTracker_HH


#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

class VariationalEchoTracker
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

  ~VariationalEchoTracker(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static VariationalEchoTracker *Inst(int argc, char **argv);
  static VariationalEchoTracker *Inst();
  

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

  ///////////////////////
  // Private constants //
  ///////////////////////

  // These are constant values that we send into the FORTRAN minimization
  // routine.

  static const int MC;
  static const int MSAVE;
  static const bool DIAGCO;
  static const int IPRINT[2];
  static const float EPSILON;
  

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static VariationalEchoTracker *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  DsTrigger *_dataTrigger;
  
  // Base field information

  MdvxField *_prevBaseField;
  Mdvx::master_header_t _currMasterHdr;
  
  // Pointers to diagnostic arrays

  double *_gnu;
  double *_gnv;
  double *_ffun1;
  double *_gm;
  
  // Pointers to arrays used by the minimization routine

  int _nunk;
  
  double *_conservationMatrix;
  
  float *_xx;
  float *_gg;
  float *_diag;
  float *_s;
  float *_y;
  float *_ww;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  VariationalEchoTracker(int argc, char **argv);
  

  /*********************************************************************
   * _calcCostFuncGradient() - Calculate the current iteration cost function
   *                           and gradients.
   */

  void _calcCostFuncGradient(const int iteration_num,
			     const MdvxField &prev_base_field,
			     const MdvxField &curr_base_field,
			     MdvxField &u_field,
			     MdvxField &v_field,
			     double &cost_function,
			     MdvxField &u_grad_field,
			     MdvxField &v_grad_field,
			     double *conservation_matrix) const;
  

  /*********************************************************************
   * _calcCostFunction() - Calculate the current iteration cost function.
   */
  
  void _calcCostFunction(const MdvxField &prev_base_field,
			 const MdvxField &curr_base_field,
			 MdvxField &u_field,
			 MdvxField &v_field,
			 double &cost_function,
			 double *conservation_matrix) const;
  

  /*********************************************************************
   * _calcGradients() - Calculate the current iteration gradients.
   */
  
  void _calcGradients(const MdvxField &prev_base_field,
		      MdvxField &u_field,
		      MdvxField &v_field,
		      MdvxField &u_grad_field,
		      MdvxField &v_grad_field,
		      double *conservation_matrix) const;
  

  /*********************************************************************
   * _generateMotionVectors() - Generate the U and V fields based on the
   *                            given previous and current base fields.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  bool _generateMotionVectors(const MdvxField &prev_base_field,
			      const MdvxField &curr_base_field);
  

  /*********************************************************************
   * _getFirstGuessUV() - Specify the first guess U and V fields.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  bool _getFirstGuessUV(MdvxField &u_field,
			MdvxField &v_field) const;
  

  /*********************************************************************
   * _initializeWorkArrays() - Initialize the arrays used in the minimization
   * process.
   */

  void _initializeWorkArrays(const Mdvx::field_header_t &u_field_hdr,
			     const Mdvx::field_header_t &v_field_hdr,
			     const MdvxPjg &base_projection);
  

  /*********************************************************************
   * _performMinimization() - Perform the minimization
   *
   * Returns the minimization flag.
   */

  int _performMinimization(MdvxField &u_field,
			   MdvxField &v_field,
			   const MdvxField &u_grad_field,
			   const MdvxField &v_grad_field,
			   const float cost_function);
  

  /*********************************************************************
   * _processData() - Process the data for the given time.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  bool _processData(const DateTime &trigger_time);
  

  /*********************************************************************
   * _readBaseField() - Read the current base field data.
   *
   * Returns a pointer to the read field on success, 0 on failure.
   */

  MdvxField *_readBaseField(const DateTime &data_time);
  

  /*********************************************************************
   * _writeVectors() - Write the calculated vectors out to the
   *                   appropriate URL.
   *
   * Returns true on success, false on failure
   */

  bool _writeVectors(const MdvxField &u_field,
		     const MdvxField &v_field,
		     const MdvxField &u_grad_field,
		     const MdvxField &v_grad_field) const;
  

};


#endif
