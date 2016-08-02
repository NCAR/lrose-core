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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 18:17:27 $
//   $Id: SweepFile.cc,v 1.4 2016/03/07 18:17:27 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SweepFile: Class for controlling access to the netCDF sweep files.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <string.h>
#include <cerrno>
#include <math.h>

#include <toolsa/os_config.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>

#include "SweepFile.hh"

using namespace std;


// Define globals


/*********************************************************************
 * Constructors
 */

SweepFile::SweepFile(const string &sweep_file_path,
		     const map< string, NegateVar > &negate_var_list,
		     const int num_begin_delete_gates,
		     const string &gates_dim_name,
		     const bool debug_flag) :
  _debug(debug_flag),
  _filePath(sweep_file_path),
  _negateVarList(negate_var_list),
  _numBeginDeleteGates(num_begin_delete_gates),
  _gatesDimName(gates_dim_name),
  _objectInitialized(false)
{
}

  
/*********************************************************************
 * Destructor
 */

SweepFile::~SweepFile()
{
  _sweepFile->close();
  
  delete _sweepFile;
}


/*********************************************************************
 * initialize() - Initialize the SweepFile.
 *
 * Returns true on success, false on failure
 */

bool SweepFile::initialize()
{
  static const string method_name = "SweepFile::initialize()";
  
  if (_debug)
    cerr << "Initializing SweepFile object..." << endl;

  _objectInitialized = false;
  
  // Initialize the input netCDF file

  _sweepFile = new NcFile(_filePath.c_str());
  
  if (!_sweepFile->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Sweep file isn't valid: " << _filePath << endl;

    _sweepFile->close();
    return false;
  }

  _objectInitialized = true;
  
  if (_debug)
  {
    cerr << "... SweepFile object successfully initialized" << endl;
    cerr << "       negate field list:" << endl;
    map< string, NegateVar >::const_iterator negate_var;
    for (negate_var = _negateVarList.begin();
	 negate_var != _negateVarList.end(); ++negate_var)
    {
      negate_var->second.print();
    }
  }
  
  return true;
}


/*********************************************************************
 * writeCleanFile() - Write out a new sweep file that is cleaned up.
 *
 * Returns true on success, false on failure
 */

bool SweepFile::writeCleanFile(const string &output_dir,
			       const string &file_name)
{
  static const string method_name = "SweepFile::writeCleanFile()";
  
  if (_debug)
  {
    cerr << "   Processing file: " << file_name << endl;
    cerr << "   Output dir: " << output_dir << endl;
  }
  
  // Make sure the object was initialized

  if (!_objectInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Object not initialized" << endl;
    
    return false;
  }
  
  // Open the output file

  string file_path = output_dir + "/" + file_name;
  
  NcFile output_file(file_path.c_str(), NcFile::Replace);
  if (!output_file.is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output file: " << file_path << endl;
    return false;
  }
  
  // Save all of the dimensions to the output file

  _addDimensions(output_file);

  // Save all of the variables to the output file

  _addVariables(output_file);
  
  // Save all of the global attributes to the output file

  _addGlobalAttributes(output_file);
  
  // Close the output file

  if (!output_file.sync())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error syncing output file to disk" << endl;
    output_file.close();
    return false;
  }
  
  if (!output_file.close())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error closing output file" << endl;
    return false;
  }
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _addDimensions() - Add the dimensions to the given output file.
 *
 * Returns true on success, false on failure.
 */

bool SweepFile::_addDimensions(NcFile &output_file) const
{
  static const string method_name = "SweepFile::_addDimensions()";
  
  int num_dims = _sweepFile->num_dims();
  if (_debug)
    cerr << "Input file has " << num_dims << " dimensions" << endl;
  
  for (int i = 0; i < num_dims; ++i)
  {
    if (_debug)
      cerr << "   Processing dimension " << i << endl;
    
    NcDim *input_dim = _sweepFile->get_dim(i);
    NcDim *output_dim = 0;
    
    if (_debug)
    {
      cerr << "        Adding dimension:" << endl;
      cerr << "             name = " << input_dim->name() << endl;
    }

    if (input_dim->is_unlimited())
    {
      if (_debug)
	cerr << "             size = unlimited" << endl;
      output_dim = output_file.add_dim(input_dim->name());
    }
    else
    {
      int dim_size = input_dim->size();
      string dim_name = input_dim->name();
      
      if (dim_name == _gatesDimName)
      {
	dim_size -= _numBeginDeleteGates;
	if (_debug)
	  cerr << "Reducing size of " << dim_name
	       << " dimension to " << dim_size << endl;
      }
      
      if (_debug)
	cerr << "             size = " << dim_size << endl;
      output_dim = output_file.add_dim(input_dim->name(), dim_size);
    }

    if (!output_dim->sync())
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error syncing dimension " << i << " to output file" << endl;
      return false;
    }
  
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * _addGlobalAttributes() - Add the global attributes to the given
 *                          output file.
 *
 * Returns true on success, false on failure.
 */

bool SweepFile::_addGlobalAttributes(NcFile &output_file) const
{
  static const string method_name = "SweepFile::_addGlobalAttributes()";
  
  int num_atts = _sweepFile->num_atts();
  if (_debug)
    cerr << "Input file has " << num_atts << " global attributes" << endl;
  
  for (int i = 0; i < num_atts; ++i)
  {
    if (_debug)
      cerr << "   Processing attribute " << i << endl;
    
    NcAtt *input_att = _sweepFile->get_att(i);
    int num_vals = input_att->num_vals();
    NcValues *values = input_att->values();
    
    if (_debug)
    {
      cerr << "       name: " << input_att->name() << endl;
      cerr << "       num_vals: " << num_vals << endl;
    }
    
    switch (input_att->type())
    {
    case ncNoType :
      break;
      
    case ncByte :
    {
      ncbyte *ncbyte_values = new ncbyte[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	ncbyte_values[val] = values->as_ncbyte(val);
      
      if (!output_file.add_att(input_att->name(), num_vals, ncbyte_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output file" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
    
    case ncChar :
    {
      char *char_values = new char[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	char_values[val] = values->as_char(val);
      
      if (!output_file.add_att(input_att->name(), num_vals, char_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output file" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
      
    case ncShort :
    {
      short *short_values = new short[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	short_values[val] = values->as_short(val);
      
      if (!output_file.add_att(input_att->name(), num_vals, short_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output file" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
      
    case ncInt :
    {
      int *int_values = new int[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	int_values[val] = values->as_int(val);
      
      if (!output_file.add_att(input_att->name(), num_vals, int_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output file" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
      
    case ncFloat :
    {
      float *float_values = new float[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	float_values[val] = values->as_float(val);
      
      if (!output_file.add_att(input_att->name(), num_vals, float_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output file" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
      
    case ncDouble :
    {
      double *double_values = new double[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	double_values[val] = values->as_double(val);
      
      if (!output_file.add_att(input_att->name(), num_vals, double_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output file" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
    } /* endswitch - input_att->type() */
    
    delete input_att;
    
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * _addVariableAttributes() - Add the attributes of the input variable 
 *                            to the output variable.
 *
 * Returns true on success, false on failure.
 */

bool SweepFile::_addVariableAttributes(const NcVar &input_var,
				       NcVar &output_var) const
{
  static const string method_name = "SweepFile::_addVariableAttributes()";
  
  int num_atts = input_var.num_atts();
  if (_debug)
    cerr << "Input variable has " << num_atts << " attributes" << endl;
  
  for (int i = 0; i < num_atts; ++i)
  {
    if (_debug)
      cerr << "   Processing attribute " << i << endl;
    
    NcAtt *input_att = input_var.get_att(i);
    int num_vals = input_att->num_vals();
    NcValues *values = input_att->values();
    
    if (_debug)
    {
      cerr << "       name: " << input_att->name() << endl;
      cerr << "       num_vals: " << num_vals << endl;
    }
    
    switch (input_att->type())
    {
    case ncNoType :
      break;
      
    case ncByte :
    {
      ncbyte *ncbyte_values = new ncbyte[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	ncbyte_values[val] = values->as_ncbyte(val);
      
      if (!output_var.add_att(input_att->name(), num_vals, ncbyte_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output variable" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
    
    case ncChar :
    {
      char *char_values = new char[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	char_values[val] = values->as_char(val);
      
      if (!output_var.add_att(input_att->name(), num_vals, char_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output variable" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
      
    case ncShort :
    {
      short *short_values = new short[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	short_values[val] = values->as_short(val);
      
      if (!output_var.add_att(input_att->name(), num_vals, short_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output variable" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
      
    case ncInt :
    {
      int *int_values = new int[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	int_values[val] = values->as_int(val);
      
      if (!output_var.add_att(input_att->name(), num_vals, int_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output variable" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
      
    case ncFloat :
    {
      float *float_values = new float[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	float_values[val] = values->as_float(val);
      
      if (!output_var.add_att(input_att->name(), num_vals, float_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output variable" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
      
    case ncDouble :
    {
      double *double_values = new double[num_vals];
      
      for (int val = 0; val < num_vals; ++val)
	double_values[val] = values->as_double(val);
      
      if (!output_var.add_att(input_att->name(), num_vals, double_values))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error adding attribute to output variable" << endl;
	cerr << "Attribute name: " << input_att->name() << endl;
	return false;
      }
      break;
    }
    } /* endswitch - input_att->type() */
    
    delete input_att;
    
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * _addVariables() - Add the variables to the given output file.
 *
 * Returns true on success, false on failure.
 */

bool SweepFile::_addVariables(NcFile &output_file) const
{
  static const string method_name = "SweepFile::_addVariables()";
  
  int num_vars = _sweepFile->num_vars();
  if (_debug)
    cerr << "Input file has " << num_vars << " variables" << endl;
  
  for (int i = 0; i < num_vars; ++i)
  {
    if (_debug)
      cerr << endl << "*** Processing variable " << i << endl;
    
    NcVar *input_var = _sweepFile->get_var(i);

    // Set the variable dimensions

    int num_dims = input_var->num_dims();
    const NcDim **dim_list = new const NcDim*[num_dims];

    if (_debug)
    {
      cerr << "  Adding variable:" << endl;
      cerr << "       name = " << input_var->name() << endl;
      cerr << "       num_dims = " << num_dims << endl;
    }

    if (_debug)
      cerr << "Orig dimensions:" << endl;
    
    for (int dim = 0; dim < num_dims; ++dim)
    {
      NcDim *input_dim = input_var->get_dim(dim);
      if (_debug)
	cerr << "   " << input_dim->name() << ": "
	     << input_dim->size() << endl;

      NcDim *output_dim;
      if ((output_dim = output_file.get_dim(input_dim->name())) == 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error retrieving dimension from output file: "
	     << input_dim->name() << endl;
	return false;
      }
      
      dim_list[dim] = output_dim;
    }
    
    NcVar *output_var = output_file.add_var(input_var->name(),
					    input_var->type(),
					    num_dims,
					    dim_list);
    
    if (_debug)
    {
      cerr << "New dimensions:" << endl;
      for (int dim = 0; dim < output_var->num_dims(); ++dim)
      {
	NcDim *ncdim = output_var->get_dim(dim);
	cerr << "   " << ncdim->name() << ": " << ncdim->size() << endl;
      }
    }
    
    // Add the attributes to the variable

    if (!_addVariableAttributes(*input_var, *output_var))
      return false;

    // Add the data to the variable

    if (!_copyData(*input_var, *output_var, num_dims))
      return false;
    
    // Negate the data, if requested

    bool negate_var_flag = false;
    string negate_fill_attr_name = "";
    map< string, NegateVar >::const_iterator negate_iter;
    
    if ((negate_iter = _negateVarList.find(output_var->name()))
	!= _negateVarList.end())
    {
      negate_var_flag = true;
      negate_fill_attr_name = negate_iter->second.fillAttrName;
    }
    
    if (_debug)
    {
      cerr << "       negate_var_flag = " << negate_var_flag << endl;
      cerr << "       negate_fill_attr_name = " << negate_fill_attr_name << endl;
    }
    
    if (negate_var_flag &&
	!_negateVariable(*output_var, negate_fill_attr_name))
      return false;
    
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * _copyData() - Copies data from the input variable into the output
 *               variable.
 *
 * Returns true on success, false on failure.
 */

bool SweepFile::_copyData(NcVar &input_var,
			  NcVar &output_var,
			  const int num_dims) const
{
  static const string method_name = "SweepFile::_copyData()";
  
  // Set up the dimensions

  int data_size = output_var.num_vals();
  long *counts = new long[num_dims];
  long *cur = new long[num_dims];
  
  for (int dim = 0; dim < num_dims; ++dim)
  {
    NcDim *output_dim = output_var.get_dim(dim);
    
    if (output_dim->is_unlimited())
    {
      NcDim *input_dim = input_var.get_dim(dim);
      if (input_dim == 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error getting dimension from input var: " << dim << endl;

	delete [] counts;
	delete [] cur;
	
	return false;
      }
      
      if (_debug)
	cerr << "*** Getting input dim size" << endl;
      
      counts[dim] = input_dim->size();

      if (_debug)
	cerr << "*** Got input dim size" << endl;
    }
    else
    {
      counts[dim] = output_dim->size();
    }
    
    string dim_name = output_dim->name();
    if (dim_name == _gatesDimName)
      cur[dim] = _numBeginDeleteGates;
    else
      cur[dim] = 0;

    if (_debug)
      cerr << "   count = " << counts[dim]
	   << ", cur = " << cur[dim] << endl;
  }
  
  switch (input_var.type())
  {
  case ncNoType :
    break;
      
  case ncByte :
  {
    ncbyte *vals = new ncbyte[data_size];
      
    if (!_copyDataT(input_var, output_var, vals, counts, cur,
		    method_name))
    {
      delete [] counts;
      delete [] cur;
      delete [] vals;
      return false;
    }
    delete [] vals;
    break;
  }
    
  case ncChar :
  {
    char *vals = new char[data_size];
      
    if (!_copyDataT(input_var, output_var, vals, counts, cur,
		    method_name))
    {
      delete [] counts;
      delete [] cur;
      delete [] vals;
      return false;
    }
    delete [] vals;
    break;
  }

  case ncShort :
  {
    short *vals = new short[data_size];
      
    if (!_copyDataT(input_var, output_var, vals, counts, cur,
		    method_name))
    {
      delete [] counts;
      delete [] cur;
      delete [] vals;
      return false;
    }
    delete [] vals;
    break;
  }

  case ncInt :
  {
    int *vals = new int[data_size];
      
    if (!_copyDataT(input_var, output_var, vals, counts, cur,
		    method_name))
    {
      delete [] counts;
      delete [] cur;
      delete [] vals;
      return false;
    }
    delete [] vals;
    break;
  }

  case ncFloat :
  {
    float *vals = new float[data_size];
      
    if (!_copyDataT(input_var, output_var, vals, counts, cur,
		    method_name))
    {
      delete [] counts;
      delete [] cur;
      delete [] vals;
      return false;
    }
    delete [] vals;
    break;
  }

  case ncDouble :
  {
    double *vals = new double[data_size];
      
    if (!_copyDataT(input_var, output_var, vals, counts, cur,
		    method_name))
    {
      delete [] counts;
      delete [] cur;
      delete [] vals;
      return false;
    }
    delete [] vals;
    break;
  }
      
  } /* endswitch - input_var.type() */

  delete [] counts;
  delete [] cur;

  return true;
}


/*********************************************************************
 * _negateVariable() - Negate the data values for this variable.
 */

bool SweepFile::_negateVariable(NcVar &output_var,
				const string &negate_fill_attr_name) const
{
  static const string method_name = "SweepFile::_negateVariable()";
  
  // Get the fill value used for this variable

  NcAtt *fill_attr;
  if ((fill_attr = output_var.get_att(negate_fill_attr_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving fill attribute from variable" << endl;
    cerr << "   Variable: " << output_var.name() << endl;
    cerr << "   Fill attr name: " << negate_fill_attr_name << endl;
    
    return false;
  }
  
  // Fill in the counts for the data

  int num_dims = output_var.num_dims();
  long *counts = new long[num_dims];
  for (int dim = 0; dim < num_dims; ++ dim)
    counts[dim] = output_var.get_dim(dim)->size();
  
  int num_vals = output_var.num_vals();
  
  // Process the data based on the data type

  switch (output_var.type())
  {
  case ncNoType :
    break;
    
  case ncByte :
  {
    ncbyte fill_value = fill_attr->as_ncbyte(0);
    _negateDataT(output_var, counts, fill_value, num_vals);
    break;
  }
  
  case ncChar :
  {
    char fill_value = fill_attr->as_char(0);
    _negateDataT(output_var, counts, fill_value, num_vals);
    break;
  }
  
  case ncShort :
  {
    short fill_value = fill_attr->as_short(0);
    _negateDataT(output_var, counts, fill_value, num_vals);
    break;
  }
  
  case ncInt :
  {
    int fill_value = fill_attr->as_int(0);
    _negateDataT(output_var, counts, fill_value, num_vals);
    break;
  }
  
  case ncFloat :
  {
    float fill_value = fill_attr->as_float(0);
    _negateDataT(output_var, counts, fill_value, num_vals);
    break;
  }
  
  case ncDouble :
  {
    double fill_value = fill_attr->as_double(0);
    _negateDataT(output_var, counts, fill_value, num_vals);
    break;
  }
  
  } /* endswitch - output_var.type() */

  return true;
}
