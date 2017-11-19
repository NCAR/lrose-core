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
/////////////////////////////////////////////////////////////
// Bdry.cc
//
// C++ wrapper for generic polyline/polygon data.
//
// Nancy Rehak, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// March 2005
//////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <rapformats/Bdry.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/mem.h>

using namespace std;

/*
 * Define values used for parsing the input file.
 */

#define PRODUCT_KEY         "PRODUCT "
#define GENERATE_TIME_KEY   "GENERATE_TIME"
#define PRODUCT_TIME_KEY    "PRODUCT_TIME"
#define PRODUCT_DESC_KEY    "PRODUCT_DESCRIPTION"
#define PRODUCT_MOTION_KEY  "PRODUCT_MOTION"
#define DETECTION_ATTR_KEY  "DETECTION_ATTR"
#define POLYLINE_KEY        "POLYLINE"

/*
 * Define values used for parsing data type
 */

#define MASK1               255
#define MASK2               65280
#define MASK3               16711935
#define SUBTYPE_BIAS        99

/*
 * Define tables for converting input line values to structure
 * values.
 */

const Bdry::parse_table_t Bdry::Type_table[] =
{ { "BDT",  BDRY_TYPE_BDRY_TRUTH },
  { "BDM",  BDRY_TYPE_BDRY_MIGFA },
  { "BDC",  BDRY_TYPE_BDRY_COLIDE },
  { "ANI",  BDRY_TYPE_COMB_NC_ISSUE },
  { "ANV",  BDRY_TYPE_COMB_NC_VALID },
  { "AXI",  BDRY_TYPE_EXTRAP_ISSUE_ANW },
  { "AXV",  BDRY_TYPE_EXTRAP_VALID_ANW },
  { "FGI",  BDRY_TYPE_FIRST_GUESS_ISSUE },
  { "FGV",  BDRY_TYPE_FIRST_GUESS_VALID },
  { "MMI",  BDRY_TYPE_MINMAX_NC_ISSUE },
  { "MMV",  BDRY_TYPE_MINMAX_NC_VALID } };

const int Bdry::Type_table_size =
  sizeof(Bdry::Type_table) / sizeof(Bdry::parse_table_t);

const Bdry::parse_table_t Bdry::Subtype_table[] =
{ { "ALL",  BDRY_SUBTYPE_ALL } };

const int Bdry::Subtype_table_size =
  sizeof(Bdry::Subtype_table) / sizeof(Bdry::parse_table_t);

const Bdry::parse_table_t Bdry::Line_type_table[] =
{ { "COMBINED_LINE",      BDRY_LINE_TYPE_COMBINED },
  { "EXTRAPOLATED_LINE",  BDRY_LINE_TYPE_EXTRAPOLATED },
  { "SHEAR_LINE",         BDRY_LINE_TYPE_SHEAR },
  { "THIN_LINE",          BDRY_LINE_TYPE_THIN },
  { "EMPTY_LINE",         BDRY_LINE_TYPE_EMPTY },
  { "GENERIC_LINE",       BDRY_LINE_TYPE_GENERIC} };

const int Bdry::Line_type_table_size =
  sizeof(Bdry::Line_type_table) / sizeof(Bdry::parse_table_t);


const string Bdry::UNKNOWN_VALUE_STRING = "unknown value";


/************************************************************************
 * Constructors
 */

Bdry::Bdry()
{
  clear();
}

Bdry::Bdry(const time_t &data_time, const int extrap_seconds,
	   const time_t &expire_time,
	   const std::string &typeString, const std::string subtypeString,
	   const int sequence, const int linetype, const int id,
	   const std::string &description,
	   const double motion_direction, const double motion_speed,
	   const double quality, const double qual_thresh)
{
  clear();
  _type                 = _typeString2Type(typeString.c_str());
  _subtype              = _subtypeString2Subtype(subtypeString.c_str());
  // _subtype_sring        = subtypeString;
  // _line_type_string     = typeString;
  _sequenceNum          = sequence;
  _groupId              = 0;
  _generateTime         = time(NULL);
  _dataTime             = data_time;
  _forecastTime         = data_time + extrap_seconds;
  _expireTime           = expire_time;
  _lineType             = linetype;
  _bdryId               = id;
  _motionDirection      = motion_direction;
  _motionSpeed          = motion_speed;
  _lineQualityValue     = quality;
  _lineQualityThreshold = qual_thresh;
  _description          = description;
}


/************************************************************************
 * Destructor
 */

Bdry::~Bdry()
{
}


/************************************************************************
 * clear() - Clear out all of the information in this boundary.
 */

void Bdry::clear()
{
  _numSpareFloat = 0;
}


/************************************************************************
 * disassemble() - Disassembles a buffer, sets the object values. Handles
 *                 byte swapping.
 *
 * Returns true on success, false on failure.
 */

bool Bdry::disassemble(const void *buf, int len)
{
  static const string method_name = "Bdry::disassemble()";
  
  char *spdb_ptr = (char *)buf;
  BDRY_spdb_product_t *spdb_product = (BDRY_spdb_product_t *)spdb_ptr;
  
  int product_size =
    sizeof(BDRY_spdb_product_t) - sizeof(BDRY_spdb_polyline_t);
  if (product_size > len)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Incoming buffer too small for product" << endl;
    
    return false;
  }

  _spdbProductFromBE(*spdb_product);
  
  /*
   * Make sure to clear out the current information.
   */

  clear();
  
  /*
   * Update the high level boundary information.
   */

  _type                 = spdb_product->type;
  _subtype              = spdb_product->subtype;
  _sequenceNum          = spdb_product->sequence_num;
  _groupId              = spdb_product->group_id;
  _generateTime         = spdb_product->generate_time;
  _dataTime             = spdb_product->data_time;
  _forecastTime         = spdb_product->forecast_time;
  _expireTime           = spdb_product->expire_time;
  _lineType             = spdb_product->line_type;
  _bdryId               = spdb_product->bdry_id;
  _motionDirection      = spdb_product->motion_direction;
  _motionSpeed          = spdb_product->motion_speed;
  _lineQualityValue     = spdb_product->line_quality_value;
  _lineQualityThreshold = spdb_product->line_quality_thresh;
  _description          = spdb_product->desc;

  for (int i=0; i<BDRY_SPARE_FLOAT_LEN; ++i)
  {
    _spare_float[i] = spdb_product->spare_float[i];
    if (_spare_float[i] != 0.0)
    {
      _numSpareFloat = i+1;
    }
  }
  
  /*
   * Now create each polyline.
   */

  spdb_ptr += product_size;
  int spdb_len = len - product_size;
  
  for (int i = 0; i < spdb_product->num_polylines; i++)
  {
    BdryPolyline polyline;
    if (!polyline.disassemble(spdb_ptr, spdb_len))
      return false;
    _polylines.push_back(polyline);
  } /* endfor - i */

  return true;
}


/************************************************************************
 * assemble() - Load up the buffer from the object. Handles byte swapping.
 *
 * Returns true on success, false on failure.
 */

bool Bdry::assemble()
{
  // Initialize the buffer

  _memBuf.reset();

  // load the product information

  /*
   * Put the main product information into the buffer.
   */

  BDRY_spdb_product_t spdb_product;
  memset(&spdb_product, 0, sizeof(spdb_product));
  
  spdb_product.type                 = _type;
  spdb_product.subtype              = _subtype;
  spdb_product.sequence_num         = _sequenceNum;
  spdb_product.group_id             = _groupId;
  spdb_product.generate_time        = _generateTime.utime();
  spdb_product.data_time            = _dataTime.utime();
  spdb_product.forecast_time        = _forecastTime.utime();
  spdb_product.expire_time          = _expireTime.utime();
  spdb_product.line_type            = _lineType;
  spdb_product.bdry_id              = _bdryId;
  spdb_product.num_polylines        = _polylines.size();
  spdb_product.motion_direction     = _motionDirection;
  spdb_product.motion_speed         = _motionSpeed;
  spdb_product.line_quality_value   = _lineQualityValue;
  spdb_product.line_quality_thresh  = _lineQualityThreshold;
  
  STRcopy(spdb_product.type_string, _type2String(_type).c_str(),
	  BDRY_TYPE_LEN);
  STRcopy(spdb_product.subtype_string, _subtype2String(_subtype).c_str(),
	  BDRY_TYPE_LEN);
  STRcopy(spdb_product.line_type_string, _lineType2String(_lineType).c_str(),
	  BDRY_LINE_TYPE_LEN);
  STRcopy(spdb_product.desc, _description.c_str(), BDRY_DESC_LEN);

  for (int i=0; i<_numSpareFloat; ++i)
  {
    spdb_product.spare_float[i] = _spare_float[i];
  }
  
  _spdbProductToBE(spdb_product);
  
  _memBuf.add((void *)&spdb_product,
	      sizeof(BDRY_spdb_product_t) - sizeof(BDRY_spdb_polyline_t));
  
  /*
   * Now add each of the polylines to the buffer.
   */

  vector< BdryPolyline >::const_iterator polyline;
  
  for (polyline = _polylines.begin(); polyline != _polylines.end(); ++polyline)
    polyline->assemble(_memBuf);
  
  return true;
}

void Bdry::addSpareFloat(const fl32 v)
{
  if (_numSpareFloat >= BDRY_SPARE_FLOAT_LEN)
  {
    // give a warning here
    return;
  }
  _spare_float[_numSpareFloat++] = v;
}


/************************************************************************
 * init(): Initializes the Boundary basd on the SIO shape information read
 *         in from an ASCII file.
 *
 * Returns true on success, false on failure.
 */

void Bdry::init(const SIO_shape_data_t &shape)
{
  /*
   * Update the main product information
   */

  _type = _typeString2Type(shape.type);
  _subtype = _subtypeString2Subtype(shape.sub_type);
  _sequenceNum = shape.sequence_number;
  _groupId = shape.group_id;
  _generateTime = shape.gen_time;
  _dataTime = shape.data_time;
  _forecastTime = shape.valid_time;
  _expireTime = shape.expire_time;
  _lineType = _lineTypeString2LineType(shape.line_type);
  _motionDirection = shape.motion_dir;
  _motionSpeed = shape.motion_speed;
  _lineQualityValue = shape.qual_value;
  _lineQualityThreshold = shape.qual_thresh;
  _description = shape.description;

  _bdryId = atoi(_description.c_str());
  
  /*
   * Update the data for each of the polylines.
   */

  for (int obj = 0; obj < shape.num_objects; obj++)
  {
    BdryPolyline polyline(shape.P[obj].nseconds,
			  shape.P[obj].object_label);
    
    for (int pt = 0; pt < shape.P[obj].npoints; pt++)
    {
      BdryPoint point(shape.P[obj].lat[pt],
		      shape.P[obj].lon[pt]);
      
      if (shape.P[obj].u_comp == NULL)
	point.setUComp(BDRY_VALUE_UNKNOWN);
      else if (shape.P[obj].u_comp[pt] == SIO_MISSING_UV)
	point.setUComp(BDRY_VALUE_UNKNOWN);
      else
	point.setUComp(shape.P[obj].u_comp[pt]);

      if (shape.P[obj].v_comp == NULL)
	point.setVComp(BDRY_VALUE_UNKNOWN);
      else if (shape.P[obj].v_comp[pt] == SIO_MISSING_UV)
	point.setVComp(BDRY_VALUE_UNKNOWN);
      else
	point.setVComp(shape.P[obj].v_comp[pt]);

      if (shape.P[obj].value == NULL)
	point.setSteeringFlow(0);
      else
	point.setSteeringFlow(shape.P[obj].value[pt]);

      polyline.addPoint(point);
      
    } /* endfor - pt */
    
  } /* endfor - obj */
}


/************************************************************************
 * print(): Print the boundary to the given stream.
 */

void Bdry::print(FILE *stream, const bool print_points) const
{
  fprintf(stream, "\nBoundary Product:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   type = %d\n", _type);
  fprintf(stream, "   subtype = %d\n", _subtype);
  fprintf(stream, "   sequence_num = %d\n", _sequenceNum);
  fprintf(stream, "   group_id = %d\n", _groupId);
  fprintf(stream, "   generate_time = %s\n", _generateTime.dtime());
  fprintf(stream, "   data_time = %s\n", _dataTime.dtime());
  fprintf(stream, "   forecast_time = %s\n", _forecastTime.dtime());
  fprintf(stream, "   expire_time = %s\n", _expireTime.dtime());
  fprintf(stream, "   line_type = %d\n", _lineType);
  fprintf(stream, "   bdry_id = %d\n", _bdryId);
  fprintf(stream, "   num_polylines = %d\n", (int) _polylines.size());
  fprintf(stream, "   motion_direction = %f\n", _motionDirection);
  fprintf(stream, "   motion_speed = %f\n", _motionSpeed);
  fprintf(stream, "   line_quality_value = %f\n", _lineQualityValue);
  fprintf(stream, "   line_quality_thresh = %f\n", _lineQualityThreshold);
  fprintf(stream, "   type_string = <%s>\n", _type2String(_type).c_str());
  fprintf(stream, "   subtype_string = <%s>\n",
	  _subtype2String(_subtype).c_str());
  fprintf(stream, "   line_type_string = <%s>\n",
	  _lineType2String(_lineType).c_str());
  fprintf(stream, "   desc = <%s>\n", _description.c_str());

  vector< BdryPolyline >::const_iterator polyline;
  for (polyline = _polylines.begin(); polyline!= _polylines.end(); ++polyline)
    polyline->print(stream, print_points);
}


void Bdry::print(ostream &stream, const bool print_points) const
{
  stream << endl;
  stream << "Boundary Product:" << endl;
  stream << endl;
  stream << "   type = " << _type << endl;
  stream << "   subtype = " << _subtype << endl;
  stream << "   sequence_num = " << _sequenceNum << endl;
  stream << "   group_id = " << _groupId << endl;
  stream << "   generate_time = " << _generateTime << endl;
  stream << "   data_time = " << _dataTime << endl;
  stream << "   forecast_time = " << _forecastTime << endl;
  stream << "   expire_time = " << _expireTime << endl;
  stream << "   line_type = " << _lineType << endl;
  stream << "   bdry_id = " << _bdryId << endl;
  stream << "   num_polylines = " << _polylines.size() << endl;
  stream << "   motion_direction = " << _motionDirection << endl;
  stream << "   motion_speed = " << _motionSpeed << endl;
  stream << "   line_quality_value = " << _lineQualityValue << endl;
  stream << "   line_quality_thresh = " << _lineQualityThreshold << endl;
  stream << "   type_string = <" << _type2String(_type) << ">" << endl;
  stream << "   subtype_string = <"
	 << _subtype2String(_subtype) << ">" << endl;
  stream << "   line_type_string = <" << _lineType2String(_lineType)
	 << ">" << endl;
  stream << "   desc = <" << _description << ">" << endl;

  vector< BdryPolyline >::const_iterator polyline;
  for (polyline = _polylines.begin(); polyline != _polylines.end(); ++polyline)
    polyline->print(stream, print_points);
}


/************************************************************************
 * toSIOShape(): Converts the boundary into the internal SIO shape structure.
 *
 * Returns a pointer to memory allocated by this routine that should be
 * freed by the calling routine.  Returns NULL if there is an error.
 */

SIO_shape_data_t *Bdry::toSIOShape()
{
  /*
   * Allocate space for the returned structure.
   */

  SIO_shape_data_t *shape =
    (SIO_shape_data_t *)umalloc(sizeof(SIO_shape_data_t));
  
  /*
   * Update the main shape structure.
   */

  shape->type = STRdup(_type2String(_type).c_str());
  shape->sub_type = STRdup(_subtype2String(_subtype).c_str());
  shape->sequence_number = _sequenceNum;
  shape->group_id = _groupId;
  shape->gen_time = _generateTime.utime();
  shape->data_time = _dataTime.utime();
  shape->valid_time = _forecastTime.utime();
  shape->expire_time = _expireTime.utime();
  STRcopy(shape->description, _description.c_str(), SIO_LABEL_LEN);
  shape->motion_dir = _motionDirection;
  shape->motion_speed = _motionSpeed;
  shape->line_type = STRdup(_lineType2String(_lineType).c_str());
  shape->qual_value = _lineQualityValue;
  shape->qual_thresh = _lineQualityThreshold;
  shape->num_objects = _polylines.size();
  
  /*
   * Allocate space for the object structures.
   */

  shape->P =
    (SIO_polyline_object_t *)umalloc(shape->num_objects *
				     sizeof(SIO_polyline_object_t));
  
  /*
   * Update the data for each of the objects.
   */

  for (int obj = 0; obj < shape->num_objects; obj++)
  {
    SIO_polyline_object_t *object = &(shape->P[obj]);
    
    STRcopy(object->object_label, _polylines[obj].getLabel().c_str(),
	    SIO_LABEL_LEN);
    object->nseconds = _polylines[obj].getNumSecsExtrap();

    /*
     * Allocate space for the point arrays
     */

    const vector< BdryPoint > points = _polylines[obj].getPoints();
    
    object->npoints = points.size();

    object->lat = (float *)umalloc(object->npoints * sizeof(float));
    object->lon = (float *)umalloc(object->npoints * sizeof(float));
    object->u_comp = (float *)umalloc(object->npoints * sizeof(float));
    object->v_comp = (float *)umalloc(object->npoints * sizeof(float));
    object->value = (float *)umalloc(object->npoints * sizeof(float));
    
    /*
     * Set the point array values.
     */

    for (int pt = 0; pt < object->npoints; pt++)
    {
      object->lat[pt] = points[pt].getLat();
      object->lon[pt] = points[pt].getLon();
      object->u_comp[pt] = points[pt].getUComp();
      object->v_comp[pt] = points[pt].getVComp();
      object->value[pt] = points[pt].getSteeringFlow();
      
    } /* endfor - pt */
    
  } /* endfor - obj */
  
  return(shape);
}


/************************************************************************
 * spdb2PjgDirection(): Converts the SPDB direction value to the
 *                      value used by PJG routines.
 *
 * Colide stores the direction using cartesian coordinates where
 * 0 degrees is along the X axis and degrees increase counter-
 * clockwise.  PJG uses map coordinates, where 0 degrees is
 * north and degrees increase clockwise.  Both give the angle we are
 * moving TO.
 */

double Bdry::spdb2PjgDirection(const double spdb_direction)
{
  double pjg_direction;
  
  pjg_direction = 90.0 - spdb_direction;
  while (pjg_direction < 0.0)
    pjg_direction += 360.0;
  while (pjg_direction >= 360.0)
    pjg_direction -= 360.0;
    
  return pjg_direction;
}


/************************************************************************
 * getSpdbDataType():  Sets data type so that first 8 bits (from lowest
 *                     order bit to highest order bit) are the 
 *                     boundary type, the next 8 bits are the subtype 
 *                     and the last 8 bits are the forecast period.
 *                     A constant value is subtracted from the subtype
 *                     to ensure the value will fit in the 8 bits given
 *                     to it.
 */

si32 Bdry::getSpdbDataType() const
{
  int forecast_period = _forecastTime.utime() - _dataTime.utime();
  
   int word = 0;
   si32 data_type = 0;
   
   word = forecast_period;
   word <<= 16;
   data_type = ((data_type) | word);
   
   word = _subtype - SUBTYPE_BIAS;
   word <<= 8;
   data_type = ((data_type) | word);
   
   word = _type;
   data_type = ((data_type) | word);

   return data_type;
}

/************************************************************************
 * parseDataType(): Parses the data type field into type, subtype
 *                  and forecast period, assuming that the data type
 *                  was created such that the first 8 bits (from 
 *                  lowest order bit to highest order bit) are the
 *                  boundary type, the next 8 bits are the subtype
 *                  and the last 8 bits are the forecast period.
 *                  A constant value is added back to the subtype
 *                  since it should have been subtracted off in
 *                  the creation of the data type to ensure that the
 *                  subtype value would fit into the 8 bits assigned
 *                  to it.
 */

void Bdry::parseDataType(const si32 data_type)
{
  _type = (MASK1 & data_type);
  _subtype = ((MASK2 & data_type) >> 8) + SUBTYPE_BIAS;
  int forecast_period = (MASK3 & data_type) >> 16;
  _forecastTime = _dataTime + forecast_period;
}


/************************************************************************
 * STATIC ROUTINES
 ************************************************************************/

/************************************************************************
 * _convertParseValue(): Convert a string value read in from the input
 *                       file into the corresponding integer value using
 *                       the given parse table.
 */

int Bdry::_convertParseValue(const char *string,
			     const parse_table_t *parse_table,
			     const int parse_table_size)
{
  for (int i = 0; i < parse_table_size; i++)
    if (STRequal_exact(string, parse_table[i].parse_string))
      return parse_table[i].parse_value;
    
  return BDRY_VALUE_UNKNOWN;
}


/************************************************************************
 * _convertStringValue(): Convert an integer value into the corresponding
 *                        string using the given parse table.
 */

string Bdry::_convertStringValue(const int parse_value,
				 const parse_table_t *parse_table,
				 const int parse_table_size)
{
  for (int i = 0; i < parse_table_size; i++)
    if (parse_table[i].parse_value == parse_value)
      return parse_table[i].parse_string;
    
  return UNKNOWN_VALUE_STRING;
}


/************************************************************************
 * _getExtrapolatedProdType(): Parse the product type line to see if
 *                             it's an extrapolated product.
 */

int Bdry::_getExtrapolatedProdType(const char *type_string)
{
  int prod_type;

  switch(type_string[0])
  {
  case 'E' :
    prod_type = BDRY_TYPE_EXTRAP_ISSUE_MIGFA;
    break;
    
  case 'V' :
    prod_type = BDRY_TYPE_EXTRAP_VALID;
    break;
    
  case 'X' :
    prod_type = BDRY_TYPE_EXTRAP_ISS_COLIDE;
    break;
    
  default :
    return(BDRY_VALUE_UNKNOWN);
  } /* endswitch */
  
  return prod_type;
}


/************************************************************************
 * _getExtrapolatedProdTypeString(): Convert an extrapolated product type
 *                                   to a string.
 */

string Bdry::_getExtrapolatedProdTypeString(const int prod_type)
{
  switch(prod_type)
  {
  case BDRY_TYPE_EXTRAP_ISSUE_MIGFA :
    return "E";

  case BDRY_TYPE_EXTRAP_VALID :
    return "V";

  case BDRY_TYPE_EXTRAP_ISS_COLIDE :
    return "X";
    
  default :
    return UNKNOWN_VALUE_STRING;
  } /* endswitch */
}


/************************************************************************
 * _lineType2String(): Converts the line type value to a string.
 */

string Bdry::_lineType2String(const int line_type)
{
  return(_convertStringValue(line_type,
			     Line_type_table,
			     Line_type_table_size));
}


/************************************************************************
 * _lineTypeString2LineType(): Returns the integer line type
 *                             for a given line type string.
 */

int Bdry::_lineTypeString2LineType(const char *line_type_string)
{
  return(_convertParseValue(line_type_string,
			    Line_type_table,
			    Line_type_table_size));
}

int Bdry::_lineTypeString2LineType(const string &line_type_string)
{
  return(_lineTypeString2LineType(line_type_string.c_str()));
}


/************************************************************************
 * _spdbProductFromBE(): Convert a boundary product from big-endian
 *                       format to native format.
 */

void Bdry::_spdbProductFromBE(BDRY_spdb_product_t &prod)
{
  prod.type                = BE_to_si32(prod.type);
  prod.subtype             = BE_to_si32(prod.subtype);
  prod.sequence_num        = BE_to_si32(prod.sequence_num);
  prod.group_id            = BE_to_si32(prod.group_id);
  prod.generate_time       = BE_to_si32(prod.generate_time);
  prod.data_time           = BE_to_si32(prod.data_time);
  prod.forecast_time       = BE_to_si32(prod.forecast_time);
  prod.expire_time         = BE_to_si32(prod.expire_time);
  prod.line_type           = BE_to_si32(prod.line_type);
  prod.bdry_id             = BE_to_si32(prod.bdry_id);
  prod.num_polylines       = BE_to_si32(prod.num_polylines);

  BE_to_array_32(prod.spare_int, BDRY_SPARE_INT_LEN * sizeof(si32));
  
  BE_to_array_32(&prod.motion_direction, 4);
  BE_to_array_32(&prod.motion_speed, 4);
  BE_to_array_32(&prod.line_quality_value, 4);
  BE_to_array_32(&prod.line_quality_thresh, 4);

  BE_to_array_32(prod.spare_float, BDRY_SPARE_FLOAT_LEN * sizeof(fl32));
  
  /* type_string is okay */
  /* subtype_string is okay */
  /* line_type_string is okay */
  /* desc is okay */
}


/************************************************************************
 * _spdbProductToBE(): Convert a boundary product from native format
 *                     to big-endian format.
 */

void Bdry::_spdbProductToBE(BDRY_spdb_product_t &prod)
{
  prod.type                = BE_from_si32(prod.type);
  prod.subtype             = BE_from_si32(prod.subtype);
  prod.sequence_num        = BE_from_si32(prod.sequence_num);
  prod.group_id            = BE_from_si32(prod.group_id);
  prod.generate_time       = BE_from_si32(prod.generate_time);
  prod.data_time           = BE_from_si32(prod.data_time);
  prod.forecast_time       = BE_from_si32(prod.forecast_time);
  prod.expire_time         = BE_from_si32(prod.expire_time);
  prod.line_type           = BE_from_si32(prod.line_type);
  prod.bdry_id             = BE_from_si32(prod.bdry_id);
  prod.num_polylines       = BE_from_si32(prod.num_polylines);
  BE_from_array_32(prod.spare_int, BDRY_SPARE_INT_LEN * sizeof(si32));
  
  BE_from_array_32(&prod.motion_direction, 4);
  BE_from_array_32(&prod.motion_speed, 4);
  BE_from_array_32(&prod.line_quality_value, 4);
  BE_from_array_32(&prod.line_quality_thresh, 4);
  BE_from_array_32(prod.spare_float, BDRY_SPARE_FLOAT_LEN * sizeof(fl32));
  
  /* type_string is okay */
  /* subtype_string is okay */
  /* line_type_string is okay */
  /* desc is okay */

  return;
}


/************************************************************************
 * _subtype2String(): Convert the given subtype to a string.
 */

string Bdry::_subtype2String(const int subtype)
{
  return(_convertStringValue(subtype,
			     Subtype_table,
			     Subtype_table_size));
}


/************************************************************************
 * _subtypeString2Subtype(): Returns the integer subtype for a
 *                          given subtype string.
 */

int Bdry::_subtypeString2Subtype(const char *subtype_string)
{
  return(_convertParseValue(subtype_string,
			    Subtype_table,
			    Subtype_table_size));
}


/************************************************************************
 * _type2String(): Returns the string associated with a given boundary
 *                 type.
 */

string Bdry::_type2String(const int type)
{
  string type_string = _convertStringValue(type,
					   Type_table,
					   Type_table_size);
  
  if (type_string == UNKNOWN_VALUE_STRING)
    type_string = _getExtrapolatedProdTypeString(type);
  
  return type_string;
}


/************************************************************************
 * _typeString2Type(): Returns the integer type for a given type
 *                    string.
 */

int Bdry::_typeString2Type(const char *type_string)
{
  int type = _convertParseValue(type_string,
				Type_table,
				Type_table_size);
  
  if (type == -1)
    type = _getExtrapolatedProdType(type_string);
  
  return type;
}
