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
#include "calcNewField.hh"
#include "Params.hh"
#include <rapmath/math_macros.h>
using namespace std;


calcNewField::calcNewField()
{
}
double calcNewField::processData(const double first_value, 
				 const double sec_value,
				 const class Params * _params, 
				 const double first_missing_data_value,
				 const double first_bad_data_value,
				 const double sec_missing_data_value,
				 const double sec_bad_data_value,
				 const int MISSING_DATA_VALUE) const
{
    double new_missing_data_value;
    
    if (_params->fill_missing) 
    {
	new_missing_data_value = _params->fill_value;
    }
    else 
    {
	new_missing_data_value = MISSING_DATA_VALUE;
    }
    
    
    switch (_params->processing_type)
    {
      case Params::ADD:

	if((first_value == first_missing_data_value ||
	   first_value == first_bad_data_value) && 
	   (sec_value  == sec_missing_data_value ||
	    sec_value == sec_bad_data_value)) 
	    return new_missing_data_value;

	if((first_value == first_missing_data_value ||
	   first_value == first_bad_data_value) && 
	   sec_value  != sec_missing_data_value &&
	   sec_value != sec_bad_data_value)
	    return sec_value;

	if((sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value) && 
	   first_value  != first_missing_data_value &&
	   first_value != first_bad_data_value)
	    return first_value;

	if((first_value != first_missing_data_value &&
	   first_value != first_bad_data_value) && 
	   (sec_value  != sec_missing_data_value &&
	    sec_value != sec_bad_data_value)) 
	    return first_value + sec_value;

      case Params::ADDNOMISSING:
	if(first_value == first_missing_data_value ||
	   first_value == first_bad_data_value)
	    return new_missing_data_value;

	if(sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value)
	    return new_missing_data_value;
	else
	    return first_value + sec_value;

      case Params::SUBTRACT:
	
	if((first_value == first_missing_data_value ||
	   first_value == first_bad_data_value) && 
	   (sec_value  == sec_missing_data_value ||
	    sec_value == sec_bad_data_value)) 
	    return new_missing_data_value;

	if((first_value == first_missing_data_value ||
	   first_value == first_bad_data_value) && 
	   sec_value  != sec_missing_data_value &&
	   sec_value != sec_bad_data_value)
	    return -sec_value;

	if((sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value) && 
	   first_value  != first_missing_data_value &&
	   first_value != first_bad_data_value)
	    return first_value;

	if((first_value != first_missing_data_value &&
	   first_value != first_bad_data_value) && 
	   (sec_value  != sec_missing_data_value &&
	    sec_value != sec_bad_data_value)) 
	    return first_value - sec_value;

      case Params::MULTIPLY:
	if(first_value == first_missing_data_value ||
	   first_value == first_bad_data_value)
	    return new_missing_data_value;

	if(sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value)
	    return new_missing_data_value;
	else
	    return first_value * sec_value;

      case Params::DIVIDE:
	if(first_value == first_missing_data_value ||
	   first_value == first_bad_data_value)
	    return new_missing_data_value;

	if(sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value)
	    return new_missing_data_value;

	if(sec_value == 0)
	    return new_missing_data_value;
	else
	    return first_value / sec_value;

      case Params::DIVIDE2:
	if(first_value == first_missing_data_value ||
	   first_value == first_bad_data_value)
	    return new_missing_data_value;

	if(sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value)
	    return new_missing_data_value;

	if(first_value == 0)
	    return new_missing_data_value;
	else
	    return sec_value / first_value;

      case Params::THRESHOLD_GREATER:
	if(first_value == first_missing_data_value ||
	   first_value == first_bad_data_value)
	    return new_missing_data_value;

	if(sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value)
	    return new_missing_data_value;
	
	if(sec_value > _params->apply_constant)
	{
	    return new_missing_data_value;
	}
	else 
	{
	    return first_value;
	}

      case Params::THRESHOLD_LESS:
	if(first_value == first_missing_data_value ||
	   first_value == first_bad_data_value)
	    return new_missing_data_value;

	if(sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value)
	    return new_missing_data_value;

	if(sec_value < _params->apply_constant)
	{
	    return new_missing_data_value;
	}
	else
	{
	    return first_value;
	}

      case Params::VECTOR_MAG:
	if(first_value == first_missing_data_value ||
	   first_value == first_bad_data_value)
	    return new_missing_data_value;

	if(sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value)
	    return new_missing_data_value;

	return sqrt(first_value * first_value + sec_value * sec_value);

      case Params::VECTOR_DIR:
    //first_value = U;
    //sec_value = V;
	if(first_value == first_missing_data_value ||
	   first_value == first_bad_data_value)
	    return new_missing_data_value;

	if(sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value)
	    return new_missing_data_value;

	if ((first_value > 0 && sec_value > 0) ||
	    (first_value > 0 && sec_value < 0))
	{
	    return 270 - (RAD_TO_DEG * atan(sec_value/first_value));
	}
	else if ((first_value < 0 && sec_value < 0) ||
		 (first_value < 0 && sec_value > 0))
	{
	    return 90 - (RAD_TO_DEG * atan(sec_value/first_value));
	}
	else if ((first_value == 0 && sec_value == 0) || 
		 (first_value == 0 && sec_value < 0))
	{
	    return 0;
	}
	else if (first_value < 0 && sec_value == 0)
	{
	    return 90;
	}
	else if (first_value == 0 && sec_value > 0)
	{
	    return 180;
	}
	else if (first_value > 0 && sec_value == 0)
	{
	    return 270;
	}
	
      case Params::DELTA_WDIR:
	if(first_value == first_missing_data_value ||
	   first_value == first_bad_data_value)
	    return new_missing_data_value;

	if(sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value)
	    return new_missing_data_value;

	if((first_value - sec_value) > 180)
	{
	    return (first_value - sec_value) - 360;
	}
	else if((first_value - sec_value) < -180)
	{
	    return -((first_value - sec_value) + 360);
	}
	else 
	{
	    return first_value - sec_value;
	}

      case Params::MAX:
	
	if((first_value == first_missing_data_value ||
	   first_value == first_bad_data_value) && 
	   (sec_value  == sec_missing_data_value ||
	    sec_value == sec_bad_data_value)) 
	    return new_missing_data_value;

	if((first_value == first_missing_data_value ||
	   first_value == first_bad_data_value) && 
	   sec_value  != sec_missing_data_value &&
	   sec_value != sec_bad_data_value)
	    return sec_value;

	if((sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value) && 
	   first_value  != first_missing_data_value &&
	   first_value != first_bad_data_value)
	    return first_value;

	if((first_value != first_missing_data_value &&
	   first_value != first_bad_data_value) && 
	   (sec_value  != sec_missing_data_value &&
	    sec_value != sec_bad_data_value)) 
	{
	    if (first_value > sec_value)
	    {
		return first_value;
	    }
	    else 
	    {
		return sec_value;
	    }
	}
	

      case Params::MIN:

	if((first_value == first_missing_data_value ||
	   first_value == first_bad_data_value) && 
	   (sec_value  == sec_missing_data_value ||
	    sec_value == sec_bad_data_value)) 
	    return new_missing_data_value;

	if((first_value == first_missing_data_value ||
	   first_value == first_bad_data_value) && 
	   sec_value  != sec_missing_data_value &&
	   sec_value != sec_bad_data_value)
	    return sec_value;

	if((sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value) && 
	   first_value  != first_missing_data_value &&
	   first_value != first_bad_data_value)
	    return first_value;

	if((first_value != first_missing_data_value &&
	   first_value != first_bad_data_value) && 
	   (sec_value  != sec_missing_data_value &&
	    sec_value != sec_bad_data_value)) 
	{
	    if (first_value > sec_value)
	    {
		return first_value;
	    }
	    else 
	    {
		return sec_value;
	    }
	}

	if(first_value == first_missing_data_value && sec_value == sec_missing_data_value ||
	   first_value == first_bad_data_value && sec_value == sec_bad_data_value)
	    return new_missing_data_value;

	if (first_value < sec_value)
	{
	    return first_value;
	}
	else 
	{
	    return sec_value;
	}

      case Params::AVG:
	if(first_value == first_missing_data_value ||
	   first_value == first_bad_data_value)
	    return new_missing_data_value;

	if(sec_value == sec_missing_data_value ||
	   sec_value == sec_bad_data_value)
	    return new_missing_data_value;

	return (first_value + sec_value) * 0.5;
	
    case Params::MODEL_PRECIP:
    case Params::GRADIENT:
    case Params::GRAD_WDIR:
    case Params::LINE_DET_WE:
    case Params::LINE_DET_SN:
    case Params::SCALE_DATA:
    case Params::DIGITAL_VIL:
    case Params::SQUARE_ROOT:
      // do nothing
      break;
    }
    return new_missing_data_value;
}
    
double calcNewField::processData(const double index, 
				 const class Params * _params, 
				 const double missing_data_value,
				 const double bad_data_value,
				 const int MISSING_DATA_VALUE) const
{
    double new_missing_data_value;
    if (_params->fill_missing) 
    {
	new_missing_data_value = _params->fill_value;
    }
    else 
    {
	new_missing_data_value = MISSING_DATA_VALUE;
    }
    
    switch (_params->processing_type)
    {
      case Params::ADD:
	if(index == missing_data_value ||
	   index == bad_data_value)
	    return new_missing_data_value;
	else	
	    return index + _params->apply_constant;

      case Params::SUBTRACT:
	if(index == missing_data_value ||
	   index == bad_data_value)
	    return new_missing_data_value;
	else	
	    return index - _params->apply_constant;

      case Params::MULTIPLY:
	if(index == missing_data_value ||
	   index == bad_data_value)
	    return new_missing_data_value;
	else	
	    return index * _params->apply_constant;

      case Params::DIVIDE:
	if(index == missing_data_value ||
	   index == bad_data_value)
	    return new_missing_data_value;
	else	
	    return index / _params->apply_constant;

      case Params::DIVIDE2:
	if(index == missing_data_value ||
	   index == bad_data_value)
	    return new_missing_data_value;
	else	
	    return _params->apply_constant / index;

      case Params::MODEL_PRECIP:
	if(index == missing_data_value ||
	   index == bad_data_value)
	    return new_missing_data_value;
	else	
	    return (index * _params->apply_constant)/25.4;

      case Params::THRESHOLD_GREATER:
	if(index > _params->apply_constant ||
	   index == missing_data_value ||
	   index == bad_data_value)
	    return new_missing_data_value;
	else 
	    return index;

      case Params::THRESHOLD_LESS:
	if(index < _params->apply_constant || 
	   index == missing_data_value ||
	   index == bad_data_value) 
	    return new_missing_data_value;
	else
	    return index;

      // equation provided by MIT-LL
      // for digitalVIL
      //----------------------------------
      //if (vilval<0.189)
      //       vil8bitval= 90.6591*vilval+1+1
      //else
      //    vil8bitval= 82.9028+38.8763*log(vilval)+1 end
      //---------------------------------- 

     case Params::DIGITAL_VIL:
      if(index == missing_data_value ||
	 index == bad_data_value) 
	return new_missing_data_value;
      else if (index < 0.189)
	return 90.6591 * index + 1 + 1;
      else
	return 82.9028 + 38.8763 * log(index) + 1;
      break;

     case Params::SQUARE_ROOT:
      if(index == missing_data_value ||
	 index == bad_data_value) 
	return new_missing_data_value;
      else 
	return sqrt( index );
      break;

    case Params::ADDNOMISSING:
    case Params::VECTOR_MAG:
    case Params::VECTOR_DIR:
    case Params::DELTA_WDIR:
    case Params::MAX:
    case Params::MIN:
    case Params::AVG:
    case Params::GRADIENT:
    case Params::GRAD_WDIR:
    case Params::LINE_DET_WE:
    case Params::LINE_DET_SN:
    case Params::SCALE_DATA:
      // do nothing
      break;
    }
    return new_missing_data_value;
}

double calcNewField::processData(const double index, 
				 const class Params * _params, 
				 const double missing_data_value,
				 const double bad_data_value,
				 const double min_value,
				 const double max_value,
				 const int MISSING_DATA_VALUE) const
{
    double new_missing_data_value;
    if (_params->fill_missing) 
    {
	new_missing_data_value = _params->fill_value;
    }
    else 
    {
	new_missing_data_value = MISSING_DATA_VALUE;
    }
    
    if (_params->processing_type == Params::SCALE_DATA)
    {
	if(index == missing_data_value ||
	   index == bad_data_value)
	    return new_missing_data_value;
	else
	{
	    double new_data;
	    double new_max;
	    if(max_value > 60)
		new_max = 60;
	    else 
		new_max = max_value;
	    if (index > 60)
		new_data = 60;
	    else 
		new_data  = index;
	//	    return index * (255/(max_value - min_value)) - (255/(max_value - min_value)) * min_value;
	    return new_data * (250/new_max);
	}
    }

    return new_missing_data_value;
}

    
double calcNewField::processData(const double index1, 
				 const double index2, 
				 const double index3, 
				 const double index4, 
				 const class Params * _params, 
				 const double missing_data_value,
				 const double bad_data_value,
				 const int MISSING_DATA_VALUE) const
{
    double new_missing_data_value;
    if (_params->fill_missing) 
    {
	new_missing_data_value = _params->fill_value;
    }
    else 
    {
	new_missing_data_value = MISSING_DATA_VALUE;
    }
    
    if (_params->processing_type == Params::GRADIENT)
    {
	if(index1 == missing_data_value ||
	   index1 == bad_data_value ||
	   index2 == missing_data_value ||
	   index2 == bad_data_value ||
	   index3 == missing_data_value ||
	   index3 == bad_data_value ||
	   index4 == missing_data_value ||
	   index4 == bad_data_value)
	    return new_missing_data_value;
	else
	    return MAX(fabs(index1 - index2),
		       fabs(index3 - index4));
    }
    else if (_params->processing_type == Params::GRAD_WDIR)
    {
	if(index1 == missing_data_value ||
	   index1 == bad_data_value ||
	   index2 == missing_data_value ||
	   index2 == bad_data_value ||
	   index3 == missing_data_value ||
	   index3 == bad_data_value ||
	   index4 == missing_data_value ||
	   index4 == bad_data_value)
	    return new_missing_data_value;
	else 
	{
	    double diff1, diff2;

	    if ( fabs(index1 - index2) > 180)
		diff1 = 360 - fabs(index1 - index2);
	    else
		diff1 = fabs(index1 - index2);

	    if ( fabs(index3 - index4) > 180)
		diff2 = 360 - fabs(index3 - index4);
	    else
		diff2 = fabs(index3 - index4);
	    
	    return MAX(diff1, diff2);
	}
    }
    return new_missing_data_value;
}

