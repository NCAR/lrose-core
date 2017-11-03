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
 *  @file MdvHowSimilar.hh
 *
 *  @class MdvHowSimilar
 *
 *  Does everything
 *
 *  @author P. Prestopnik, G. McCabe
 * 
 *  @date September 2015
 *
 *  @version $Id: MdvHowSimilar.hh,v 1.9 2017/09/13 19:42:57 mccabe Exp $
 */



#ifndef MDVHOWSIMILAR_H
#define MDVHOWSIMILAR_H

// C++ include files
#include <string>

#include <fstream>

//library includes
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>    

//local includes
#include "Args.hh"
#include "Params.hh"



class MdvHowSimilar {

public:

  /** 
   * singleton instance invocation
   *
   * @param[in] argc number of command line arguments
   * @param[in] argv pointer to arguments
   */
  static MdvHowSimilar* instance(int argc, char **argv);

  /** singleton instance invocation */
  static MdvHowSimilar* instance();

  /** destructor */
  virtual ~MdvHowSimilar();

  /** Execution of program */
  int run();

  bool isOK() { return _isOK; }

  static bool isValid(const Mdvx::field_header_t& fh, float value);
  static bool isEqual(float a, float b);
	
  static const float MISSING_DATA_VALUE;
  static const float SIG_DIFF_DEFAULT;
  static const float RMS_SIG_DIFF_DEFAULT;

	//similar to the Params one, but with a string
  typedef struct {
    string field_name;
    double significant_difference;
    double rms_sig_diff;
  } field_comparison_info_t;

	vector<field_comparison_info_t> fieldComparisonInfo;
	typedef vector<field_comparison_info_t>::iterator fciIx_t;
	typedef vector<field_comparison_info_t>::const_iterator fciCIx_t;


private:
  std::string _progName;
  Args* _args;
  Params* _params;
	bool _isOK;
	std::ostream* rout; //report out
	std::ofstream frout; //helper for rout

	size_t _nx, _ny, _nz;
	
  // Singleton instance pointer

  static MdvHowSimilar *_instance; 
	DsMdvx _mdvx1;
	DsMdvx _mdvx2;
	DsMdvx _mdvxOut;

	//output data
	std::map<std::string, fl32 *> _outVols;

  // template master, field and vlevel headers -- used to generate output headers
  Mdvx::master_header_t _outMasterHdr;
  Mdvx::field_header_t _outFieldHdr;
  Mdvx::vlevel_header_t _outVlevelHdr;

	bool _initialize(int argc, char **argv);


	void _readInputFiles();
	void _compareInputFiles();

	void _printFieldSummary(const string fieldName, const int vDiffs);
	float _doStatisticsForOneLevel(const string header, const Mdvx::field_header_t& fh1, const fl32* data1, 
	                                              const Mdvx::field_header_t& fh2, const fl32* data2,
				       const float sigDiff, fl32* outData, float& totalDiff, float& totalAbsDiff);

	bool _allocateOutputData();

	void _writeOutput();
	void _printFieldSummary(const string fieldName, const int vDiffs, const float avgRMS, const float diffTotal, const float absDiffTotal);

	void _cleanupOutputData();
	void _setFieldHeader(Mdvx::field_header_t& hdr, const string& name);
	void _checkFieldDimensions(const Mdvx::field_header_t& fh1, const Mdvx::field_header_t& fh2);
				
	void _setFieldDimensionsAndHeaders(const MdvxField* mf);
	void _addFieldToOutput(string name); 

	void _setMasterHeader(Mdvx::master_header_t& hdr);

        void _fillVector(string commaSep, vector<string>& vec, bool removeWhite=true);

  /**  Disallow the constructor (singleton)*/
  MdvHowSimilar();
  /**  Disallow the copy constructor (singleton)*/
  MdvHowSimilar(const MdvHowSimilar &);
  /**  Disallow the assignment operator (singleton)*/
  MdvHowSimilar &operator=(const MdvHowSimilar &);

};



#endif
