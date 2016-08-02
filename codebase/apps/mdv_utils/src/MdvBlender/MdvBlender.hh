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
 *  @file MdvBlender.hh
 *
 *  @class MdvBlender
 *
 *  Controls the MdvBlender processing
 *
 *  @author P. Prestopnik, G. McCabe
 * 
 *  @date July 2014
 *
 *  @version $Id: MdvBlender.hh,v 1.10 2016/06/23 17:15:16 mccabe Exp $
 */



#ifndef MDVBLENDER_H
#define MDVBLENDER_H

// C++ include files
#include <string>
#include <ctime>
#include <vector>
#include <map>

// System/RAP include files
#include "toolsa/os_config.h"
#include "dataport/port_types.h"
#include "Mdv/DsMdvx.hh"

// Local include files
#include "Constants.hh"
#include "Params.hh"

// Forward declarations
class Path;
class Args;
class DsMdvxInput;
class Blender;

class MdvBlender {
  
public:

  /** 
   * singleton instance invocation
   *
   * @param[in] argc number of command line arguments
   * @param[in] argv pointer to arguments
   */
  static MdvBlender* instance(int argc, char **argv);

  /** singleton instance invocation */
  static MdvBlender* instance();

  /** destructor */
  virtual ~MdvBlender();

  /** Execution of program */
  int run();

	bool isOK() { return _isOK; }


	/** parameter structures */
	struct blenderFieldNames_t {
		std::vector<std::string> outputFieldNames;
		std::vector<std::string> outputFieldLongNames;
		std::vector<std::string> outputFieldUnits;
		std::vector<std::string> input1FieldNames;
		std::vector<std::string> input2FieldNames;
		std::vector<Params::round_t> outputRounding;
		std::string input1WeightName;
		std::string input2WeightName;
		double input1ConstantWeight;
		double input2ConstantWeight;
	};

protected:
  
private:

  std::string _progName;

  Args* _args;
  Params* _params;
  static const std::string _className;

  // Singleton instance pointer
  static MdvBlender *_instance; 

	bool _isOK;

	/** Input related data members */
	DsMdvxInput* _inputTrigger;
	time_t _runTime;
	DsMdvx* _mdvx1;
	DsMdvx* _mdvx2;
	
	std::vector<blenderFieldNames_t*> _blenderFieldNames;

	/** output related datemembers */
	DsMdvx* _out;
	unsigned int _numX;
  unsigned int _numY;
  unsigned int _numZ;
  unsigned int _numElem;

	//  vector<int> _useInputs;
  float * _useInputs;

  // template master, field and vlevel headers -- used to generate output headers
  Mdvx::master_header_t _outMasterHdr;
  Mdvx::field_header_t _outFieldHdr;
  Mdvx::vlevel_header_t _outVlevelHdr;

	std::map<std::string, float *> _vols;

	std::map<std::string,float*> _blenderOutputVolumes;  //key is fieldname

	Blender* _blender;

	/**
   * setup the MdvBlender object state 
   * 
   * @param[in] argc number of command line arguments
   * @param[in] argv pointer to arguments
   */
  bool _initialize(int argc, char **argv);

	/** setup headers for output & allocate memory */
	bool _initializeOutputs();

	/** helper function for _getNextData() */
	bool _readInputFiles(int bix);

	/** parse comma separated param strings into vectors*/
	bool _parseParams();

	/** helper for _parseParams() */
	void _fillVector(std::string commaSep, std::vector<std::string>& vec,
                   bool removeWhite=true);

	void printBlenderField(blenderFieldNames_t* bft);
	bool _writeOutput();
	bool _addPassThroughFields();
	bool _doBlend(int idx);
	bool _allocateOutputData(int dix, int fix);
	void _cleanupOutputData();
	void _addFieldToOutput(string name, string longName, string units);

	void _setMasterHeader(Mdvx::master_header_t& hdr);
  void _setFieldHeader(Mdvx::field_header_t& hdr, const string& name,
			  const string& longName, const string& unit, int fCode, int nx,
                                 int ny, int nz);
	bool _verifyDimensions(MdvxField* input1Field, MdvxField* input2Field);

	double _evaluateAverage(double d1, double d2, double w1, double w2, double miss1, double miss2, Params::round_t round);
	int _evaluateDither(double w1, double w2);

  /**  Disallow the constructor (singleton)*/
  MdvBlender();
  /**  Disallow the copy constructor (singleton)*/
  MdvBlender(const MdvBlender &);
  /**  Disallow the assignment operator (singleton)*/
  MdvBlender &operator=(const MdvBlender &);

};

# endif   // MDVBLENDER_H
