//
//
//
//

#include <string.h>

#include "NcRadarFile.h"
#include <RayConst.h>

#ifndef NCRADARFILE_INLINE
#define NCRADARFILE_INLINE

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline bool NcRadarFile::test_for_variable(const std::string &name)  throw(ForayUtility::Fault){

    int status;
    int variableId;

    status = nc_inq_varid(ncFileId_,name.c_str(),&variableId);

    if(status == NC_NOERR){
	return true;
    }

    // NC_ENOTVAR is error code for variable not found.  
    if(status != NC_ENOTVAR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_variable_id(%s): nc_inq_varid returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return false;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline int NcRadarFile::get_nc_variable_id(const std::string &name)  throw(ForayUtility::Fault){

    int status;
    int variableId;

    status = nc_inq_varid(ncFileId_,name.c_str(),&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_variable_id(%s): nc_inq_varid returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return variableId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline int NcRadarFile::get_nc_dimension_id(const std::string &name)  throw(ForayUtility::Fault){

    int status;
    int dimensionId;

    status = nc_inq_dimid(ncFileId_,name.c_str(),&dimensionId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_dimension_id(%s): nc_inq_demid returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return dimensionId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline int NcRadarFile::get_nc_integer(const std::string &name)  throw(ForayUtility::Fault){

    int    status;
    int    value;
    int    variableId;
    size_t index[] = {0};

    variableId = get_nc_variable_id(name);

    status = nc_get_var1_int(ncFileId_,variableId,index,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_int(%s): nc_get_var1_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return value;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline double NcRadarFile::get_nc_double(const std::string &name)  throw(ForayUtility::Fault){

    int    status;
    double value;
    int    variableId;
    size_t index[] = {0};

    variableId = get_nc_variable_id(name);

    status = nc_get_var1_double(ncFileId_,variableId,index,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_double(%s): nc_get_var1_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline double NcRadarFile::get_nc_double(const std::string &name,const int index)  throw(ForayUtility::Fault){

    int    status;
    double value;
    int    variableId;
    size_t indexArray[1];

    variableId = get_nc_variable_id(name);

    indexArray[0] = index;

    status = nc_get_var1_double(ncFileId_,variableId,indexArray,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_double(%s,%d): nc_get_var1_double returned error: %s.\n",
		name.c_str(),
		index,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline std::string NcRadarFile::get_nc_short_string(const std::string &name,const int index)  throw(ForayUtility::Fault){

    int     status;
    char   *value;
    size_t  startArray[2];
    size_t  countArray[2];

    int shortStringSize = get_nc_dimension_length("short_string");
    int variableId      = get_nc_variable_id(name);

    startArray[0] = index;
    startArray[1] = 0;

    countArray[0] = 1;
    countArray[1] = shortStringSize;

    value = new char[shortStringSize + 1];
    memset(value,0,shortStringSize + 1);

    status = nc_get_vara_text(ncFileId_,variableId,startArray,countArray,value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_short_string(%s,%d): nc_get_vara_text returned error: %s.\n",
		name.c_str(),
		index,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    std::string originalValue(value);
    std::string returnValue;
    int    space = originalValue.find(" ");
    if(space == std::string::npos ){
	returnValue = originalValue;
    }else{
	returnValue = originalValue.substr(0,space);
    }
		
    delete [] value;

    return returnValue;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline int NcRadarFile::get_nc_dimension_length(const std::string &name)  throw(ForayUtility::Fault){

    int    status;
    size_t dimensionLength;
    int    dimensionId;

    dimensionId = get_nc_dimension_id(name);

    status = nc_inq_dimlen(ncFileId_,dimensionId,&dimensionLength);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_dimension_length(%s): nc_get_dimlen returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return (int)dimensionLength;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline std::string NcRadarFile::get_nc_string_attribute(const std::string &name)  throw(ForayUtility::Fault){
    
    return get_nc_string_attribute(NC_GLOBAL,name);
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline std::string NcRadarFile::get_nc_string_attribute(const int variableId, const std::string &name)  throw(ForayUtility::Fault){

    int    status;
    char   *value;
    size_t  length;
    
    status = nc_inq_attlen(ncFileId_,variableId,name.c_str(),&length);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_string_attribute(%s): nc_inq_attlen returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    value = new char[length + 1];
    memset(value,0,length + 1);

    status = nc_get_att_text(ncFileId_,variableId,name.c_str(),value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_string_attribute(%s): nc_get_att_text returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    std::string returnString(value);

    delete [] value;

    return returnString;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline bool NcRadarFile::test_nc_double_attribute(const int variableId, const std::string &name)  throw(ForayUtility::Fault){

    int     status;
    double  value;

    status = nc_get_att_double(ncFileId_,variableId,name.c_str(),&value);

    if(status == NC_NOERR){
	return true;
    }

    // NC_ENOTVAR is error code for variable not found.  
    if(status != NC_ENOTATT){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::test_nc_double_attribute(%s): nc_get_att_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return false;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline double NcRadarFile::get_nc_double_attribute(const int variableId, const std::string &name)  throw(ForayUtility::Fault){

    int     status;
    double  value;

    status = nc_get_att_double(ncFileId_,variableId,name.c_str(),&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_double_attribute(%s): nc_get_att_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline int NcRadarFile::get_nc_integer_attribute(const int variableId, const std::string &name)  throw(ForayUtility::Fault){

    int  status;
    int  value;

    status = nc_get_att_int(ncFileId_,variableId,name.c_str(),&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_integer_attribute(%s): nc_get_att_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_dimension(const std::string &name,const int length, int &dimensionId) throw(ForayUtility::Fault){

    int     status;

    status = nc_def_dim(ncFileId_,name.c_str(),(size_t)length,&dimensionId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_dimension(%s): nc_def_dim returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline int NcRadarFile::create_nc_scalar(const std::string &name,const nc_type type) throw(ForayUtility::Fault){

    int     status;
    int     variableId;  

    status = nc_def_var(ncFileId_,name.c_str(),type,0,NULL,&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::create_nc_scalar(%s): nc_def_var returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return variableId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline int NcRadarFile::create_nc_system_variable(const std::string &name,const nc_type type) throw(ForayUtility::Fault){

    int     status;
    int     variableId;  
    int     dimensionIds[1];

    dimensionIds[0] = numSystemsDimensionId_;

    status = nc_def_var(ncFileId_,name.c_str(),type,1,dimensionIds,&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::create_nc_system_variable(%s): nc_def_var returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return variableId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline int NcRadarFile::create_nc_time_variable(const std::string &name,const nc_type type) throw(ForayUtility::Fault){

    int     status;
    int     variableId;  
    int     dimensionIds[1];

    dimensionIds[0] = timeDimensionId_;

    status = nc_def_var(ncFileId_,name.c_str(),type,1,dimensionIds,&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::create_nc_time_variable(%s): nc_def_var returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return variableId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline int NcRadarFile::create_nc_data_variable(const std::string &name,const nc_type type) throw(ForayUtility::Fault){

    int     status;
    int     variableId;  
    int     dimensionIds[2];

    dimensionIds[0] = timeDimensionId_;
    dimensionIds[1] = maxCellsDimensionId_;

    status = nc_def_var(ncFileId_,name.c_str(),type,2,dimensionIds,&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::create_nc_data_variable(%s): nc_def_var returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return variableId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_text_attribute(const int varid,const std::string &name,const std::string &value) throw(ForayUtility::Fault){

    int status;

    const char *cvalue = value.c_str();

    status = nc_put_att_text(ncFileId_,varid,name.c_str(),strlen(cvalue),cvalue);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_text_attribute(%s): nc_put_att_text returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_integer_attribute(const int varid,const std::string &name,const int &value) throw(ForayUtility::Fault){

    int   status;

    status = nc_put_att_int(ncFileId_,varid,name.c_str(),NC_INT,1,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_integer_attribute(%s): nc_put_att_int returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_float_attribute(const int varid,const std::string &name,const double &value) throw(ForayUtility::Fault){

    int   status;
    float fvalue = value;

    status = nc_put_att_float(ncFileId_,varid,name.c_str(),NC_FLOAT,1,&fvalue);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_float_attribute(%s): nc_put_att_float returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_double_attribute(const int varid,const std::string &name,const double &value) throw(ForayUtility::Fault){

    int   status;

    status = nc_put_att_double(ncFileId_,varid,name.c_str(),NC_DOUBLE,1,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_double_attribute(%s): nc_put_att_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_double_attribute(const int varid,const std::string &name,
						 const double &value1,const double &value2)  throw(ForayUtility::Fault){

    int    status;
    double dvalue[2];

    dvalue[0] = value1;
    dvalue[1] = value2;

    status = nc_put_att_double(ncFileId_,varid,name.c_str(),NC_DOUBLE,2,dvalue);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_double_attribute(%s): nc_put_att_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_short_attribute(const int varid,const std::string &name,const short &value) throw(ForayUtility::Fault){

    int   status;

    status = nc_put_att_short(ncFileId_,varid,name.c_str(),NC_SHORT,1,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_short_attribute(%s): nc_put_att_short returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_int_attribute(const int varid,const std::string &name,const int &value) throw(ForayUtility::Fault){

    int   status;

    status = nc_put_att_int(ncFileId_,varid,name.c_str(),NC_INT,1,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_int_attribute(%s): nc_put_att_int returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_missing_and_fill_attributes(const int varid) throw(ForayUtility::Fault){

    nc_type type;

    int status = nc_inq_vartype(ncFileId_,varid,&type);
    if(status != NC_NOERR){
        char varName[1024];
        strcpy(varName, "Unknown");
        nc_inq_varname(ncFileId_,varid,varName);
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_missing_and_fill_attributes: id=%d, name=%s, nc_inq_vartype returned error: %s.\n",
                varid, varName,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    
    try {

	if(type == NC_SHORT){
          set_missing_and_fill_attributes(varid,RayConst::badShort);
	}else{
          set_missing_and_fill_attributes(varid,RayConst::badDouble);
	}

    }catch(ForayUtility::Fault &fault){
	fault.add_msg("NcRadarFile::set_missing_and_fill_attributes: caught fault.\n");
	throw(fault);
    }
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_missing_and_fill_attributes(const int varid,const double value) throw(ForayUtility::Fault){
    
    try {

	nc_type type;

	int status = nc_inq_vartype(ncFileId_,varid,&type);
        char varName[1024];
        strcpy(varName, "Unknown");
        nc_inq_varname(ncFileId_,varid,varName);
	if(status != NC_NOERR){
          char statusMessage[2048];
          sprintf(statusMessage,"NcRadarFile::set_missing_and_fill_attributes: id=%d, name=%s, nc_inq_vartype returned error: %s.\n",
                  varid, varName,
                  nc_strerror(status));
	    throw ForayUtility::Fault(statusMessage);
	}

	if(type == NC_FLOAT){
	    set_nc_float_attribute(varid,"_FillValue"   ,value);
	    set_nc_float_attribute(varid,"missing_value",value);
	}else if(type == NC_DOUBLE){
	    set_nc_double_attribute(varid,"_FillValue"   ,value);
	    set_nc_double_attribute(varid,"missing_value",value);
	}else if(type == NC_SHORT){
	    set_nc_short_attribute(varid,"_FillValue"   ,(short)value);
	    set_nc_short_attribute(varid,"missing_value",(short)value);
	}else if(type == NC_INT){
	    set_nc_int_attribute(varid,"_FillValue"   ,(int)value);
	    set_nc_int_attribute(varid,"missing_value",(int)value);
	}else{
	  char statusMessage[2048];
	  sprintf(statusMessage,"NcRadarFile::set_missing_and_fill_attributes: id=%d, name=%s, type (%d) not supported. Value is %g\n",
                  varid, varName,
                  type,value);
	    throw ForayUtility::Fault(statusMessage);
	}

    }catch(ForayUtility::Fault &fault){
          char faultMessage[2048];
          sprintf(faultMessage,"NcRadarFile::set_missing_and_fill_attributes: caught Fault, id=%d, value=%g\n", varid, value);
	  fault.add_msg(faultMessage);
	throw(fault);
    }

}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::create_fields_variable() throw(ForayUtility::Fault){

    int status;
    int dimensionIds[2];
    int variableId;

    dimensionIds[0] = numFieldsDimensionId_;
    dimensionIds[1] = shortStringDimensionId_;

    status = nc_def_var(ncFileId_,"field_names",NC_CHAR,2,dimensionIds,&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::create_fields_variable: nc_def_var returned error: %s.\n",
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_integer(const std::string &name, const int value) throw(ForayUtility::Fault){

    int    status;
    int    variableId;
    size_t index[1];

    variableId = get_nc_variable_id(name);
    index[0]   = 0;

    status = nc_put_var1_int(ncFileId_,variableId,index,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_integer(%s,%d): nc_put_var1_int returned error: %s.\n",
		name.c_str(),
		value,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_float(const std::string &name, const float value) throw(ForayUtility::Fault){

    set_nc_float(name,value,0);

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_float(const std::string &name, const float value,const int indexValue) throw(ForayUtility::Fault){

    int    status;
    int    variableId;
    size_t index[1];

    variableId = get_nc_variable_id(name);
    index[0]   = indexValue;

    status = nc_put_var1_float(ncFileId_,variableId,index,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_float(%s,%d,%d): nc_put_var1_float returned error: %s.\n",
		name.c_str(),
		value,
		indexValue,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_double(const std::string &name, const double value) throw(ForayUtility::Fault){

    set_nc_double(name,value,0);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_nc_double(const std::string &name, const double value, 
				       const int indexValue) throw(ForayUtility::Fault){

    int    status;
    int    variableId;
    size_t index[1];

    variableId = get_nc_variable_id(name);
    index[0]   = indexValue;

    status = nc_put_var1_double(ncFileId_,variableId,index,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_double(%s,%f,%d): nc_put_var1_double returned error: %s.\n",
		name.c_str(),
		value,
		indexValue,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
inline void NcRadarFile::set_fields_variable() throw(ForayUtility::Fault){

    int    status;
    int    variableId;
    int    numberOfFields;
    size_t start[2];
    size_t count[2];

    try {
	variableId     = get_nc_variable_id("field_names");
	numberOfFields = get_integer("number_of_fields");

	for(int fieldIndex = 0; fieldIndex < numberOfFields; fieldIndex++){

	    std::string fieldName = get_string("field_name",fieldIndex);

	    start[0] = fieldIndex;
	    start[1] = 0;
	    count[0] = 1;
	    count[1] = fieldName.size();

	    status = nc_put_vara_text(ncFileId_,variableId,start,count,fieldName.c_str());

	    if(status != NC_NOERR){
		char statusMessage[2048];
		sprintf(statusMessage,"NcRadarFile::set_fields_variable: nc_put_vara_text returned error: %s.\n",
			nc_strerror(status));
		throw ForayUtility::Fault(statusMessage);
	    }
	}
	
    }catch(ForayUtility::Fault &fault){
	fault.add_msg("NcRadarFile::set_fields_variable: caught fault.\n");
	throw fault;
    }
}


#endif // NCRADARFILE_INLINE
