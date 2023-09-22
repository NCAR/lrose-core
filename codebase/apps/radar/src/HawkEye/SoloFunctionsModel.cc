
#include <vector>
#include <iostream>

#include "SoloFunctionsModel.hh"
#include "RemoveAcMotion.cc"


#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxCfactors.hh>
#include <Radx/RadxGeoref.hh>
#include <toolsa/LogStream.hh>

using namespace std;

/*
SoloFunctionsModel::SoloFunctionsModel() {

}
*/

vector<double> SoloFunctionsModel::RemoveAircraftMotion(string fieldName, RadxVol *vol) { // SpreadSheetModel *context) {

  // TODO: what is being returned? the name of the new field in the model that
  // contains the results.
  // since the std::vector<double> data has to be copied to QVector anyway, 
  // go ahead and format it as a string?
  // maybe return a pointer to std::vector<double> ?? then when presenting the data, we can convert it to string,
  // but maintain the precision in the model (RadxVol)??

  LOG(DEBUG_VERBOSE) << "entry with fieldName ... ";
  LOG(DEBUG_VERBOSE) << fieldName;

  // gather data from context -- most of the data are in a DoradeRadxFile object

  // TODO: convert the context RadxVol to DoradeRadxFile and DoradeData format;
  //RadxVol vol = context->_vol;
  // make sure the radar angles have been calculated.

  
  const RadxField *field;
  field = vol->getFieldFromRay(fieldName);
  if (field == NULL) {
    LOG(DEBUG_VERBOSE) << "no RadxField found in volume";
    throw "No data field with name " + fieldName;;
  }
  

  // TODO: get the ray for this field 
  const vector<RadxRay *>  &rays = vol->getRays();
  if (rays.size() > 1) {
    LOG(DEBUG_VERBOSE) <<  "ERROR - more than one ray; expected only one";
  }
  RadxRay *ray = rays.at(0);
  if (ray == NULL) {
    LOG(DEBUG_VERBOSE) << "ERROR - first ray is NULL";
    throw "Ray is null";
  } 

  const RadxGeoref *georef = ray->getGeoreference();
  if (georef == NULL) {
    LOG(DEBUG_VERBOSE) << "ERROR - georef is NULL";
    throw "Georef is null";
  } 
 
  float vert_velocity = georef->getVertVelocity();  // fl32
  float ew_velocity = georef->getEwVelocity(); // fl32
  float ns_velocity = georef->getNsVelocity(); // fl32;

  float ew_gndspd_corr = 0.0; 
  const RadxCfactors *cfactors = ray->getCfactors();
  if (cfactors != NULL) {
    ew_gndspd_corr = cfactors->getEwVelCorr(); // ?? _gndspd_corr; // fl32;
  }
 
  float tilt = georef->getTilt(); // fl32; 
  // TODO: elevation changes with different rays/fields how to get the current one???
  float elevation = ray->getElevationDeg(); // doradeData.elevation; // fl32;

  // TODO:  look up the dataField and get the associated values
  // look through DoradeRadxFile::_ddParms for a parameter_t type that has parameter_name that matches
  // the dataField.
  // short *data; // data is in and out parameter
  if (field->getDataType() != Radx::SI16) {
    throw  "ERROR - data is not 16-bit signed integer as expected";
  } 

  //Radx::fl32 *rawData;
  //rawData = field->getDataFl32();
  //Radx::fl32 *ptr = rawData;
  cerr << " A few data values ";
  for (int i=0; i< 10; i++) {
    cerr << field->getDoubleValue(i) << ", ";
  }
  cerr << endl;
  //vector<double> data = field->getData  Radx::fl32 *getDataFl32();   
  //  double scale = 1.0 / parm.parameter_scale;
  //  double bias = (-1.0 * parm.parameter_bias) / parm.parameter_scale;

  // related to field->setTypeSi32(parm.bad_data, scale, bias)
  // RadxField::_scale;  RadxField::_offset = bias; RadxField::_missingSi32 = bad_data
  // 
  // TODO: need to find the field. How to do this????
  short bad = field->getMissingSi16(); // doradeData.bad_data;
  float parameter_scale = 1.0 / field->getScale(); // doradeData.parameter_scale; 
  float parameter_bias = -1.0 * field->getOffset() * field->getScale(); // doradeData.parameter_bias; 
  int dgi_clip_gate = field->getNPoints(); // field->num_samples; // or number_cells
  short dds_radd_eff_unamb_vel = ray->getNyquistMps(); // doradeData.eff_unamb_vel;
  int seds_nyquist_velocity = 0; // TODO: what is this value?

  LOG(DEBUG_VERBOSE) << "sizeof(short) = " << sizeof(short);

  LOG(DEBUG_VERBOSE) << "args: ";
  LOG(DEBUG_VERBOSE) << "vert_velocity " << vert_velocity;
  LOG(DEBUG_VERBOSE) <<   "ew_velocity " << ew_velocity;
  LOG(DEBUG_VERBOSE) <<   "ns_velocity " << ns_velocity;
  LOG(DEBUG_VERBOSE) <<   "ew_gndspd_corr " << ew_gndspd_corr;
  LOG(DEBUG_VERBOSE) <<   "tilt " << tilt;
  LOG(DEBUG_VERBOSE) <<   "elevation " << elevation;
  LOG(DEBUG_VERBOSE) <<   "bad " << bad;
  LOG(DEBUG_VERBOSE) <<   "parameter_scale " << parameter_scale;
  LOG(DEBUG_VERBOSE) <<   "dgi_clip_gate " << dgi_clip_gate;
  LOG(DEBUG_VERBOSE) <<   "dds_radd_eff_unamb_vel " << dds_radd_eff_unamb_vel;
  LOG(DEBUG_VERBOSE) <<   "seds_nyquist_velocity " << "??";
  
  //SoloFunctionsApi soloFunctionsApi;
  int result = se_remove_ac_motion(vert_velocity, ew_velocity, ns_velocity,
     ew_gndspd_corr, tilt, elevation,
     field->getDataSi16(), bad, parameter_scale, parameter_bias, dgi_clip_gate,
     dds_radd_eff_unamb_vel, seds_nyquist_velocity);
  
  LOG(DEBUG_VERBOSE) << " result: " << result;
  LOG(DEBUG_VERBOSE) << " A few data values ";
  for (int i=0; i< 10; i++) {
      LOG(DEBUG_VERBOSE) << field->getDoubleValue(i);
  }

  // TODO: We are converting from short to double!!!  <=====
  vector<double> newData; // (data, dgi_clip_gate+1);
  //  for (vector<double>::iterator it = data.begin(); it != data.end(); ++it)
  //  newData.push_back(*it * 2.0);

  LOG(DEBUG_VERBOSE) << "exit ";

  return newData;
}



vector<double> SoloFunctionsModel::RemoveAircraftMotion(vector<double> data, RadxVol *vol) { // SpreadSheetModel *context) {

  // TODO: what is being returned? the name of the new field in the model that
  // contains the results.
  // since the std::vector<double> data has to be copied to QVector anyway, 
  // go ahead and format it as a string?
  // maybe return a pointer to std::vector<double> ?? then when presenting the data, we can convert it to string,
  // but maintain the precision in the model (RadxVol)??

  cerr << "RemoveAircraftMotion ... "  << endl;

  // gather data from context -- most of the data are in a DoradeRadxFile object

  // TODO: convert the context RadxVol to DoradeRadxFile and DoradeData format;
  //RadxVol vol = context->_vol;
  // make sure the radar angles have been calculated.

  /*
  const RadxField *field;
  field = vol.getFieldFromRay(fieldName);
  if (field == NULL) {
    cerr << "no RadxField found " <<  endl;
    throw "No data field with name " + fieldName;;
  }
  */

  // TODO: get the ray for this field 
  const vector<RadxRay *>  &rays = vol->getRays();
  if (rays.size() > 1) {
    cerr << "ERROR - more than one ray; expected only one\n";
  }

  RadxRay *ray = rays.at(0);
  if (ray == NULL) {
    cerr << "ERROR - first ray is NULL" << endl;
  } 
  //const RadxGeoref *georef = ray->getGeoreference();

  // float vert_velocity = georef->getVertVelocity();  // fl32
  // float ew_velocity = georef->getEwVelocity(); // fl32
  // float ns_velocity = georef->getNsVelocity(); // fl32;

  float ew_gndspd_corr = 0.0; 
  const RadxCfactors *cfactors = ray->getCfactors();
  if (cfactors != NULL) {
    ew_gndspd_corr = cfactors->getEwVelCorr(); // ?? _gndspd_corr; // fl32;
  }
  LOG(DEBUG_VERBOSE) << "ew_gndspd_corr: " << ew_gndspd_corr;
 
  // float tilt = georef->getTilt(); // fl32; 
  // TODO: elevation changes with different rays/fields how to get the current one???
  // float elevation = ray->getElevationDeg(); // doradeData.elevation; // fl32;

  // TODO:  look up the dataField and get the associated values
  // look through DoradeRadxFile::_ddParms for a parameter_t type that has parameter_name that matches
  // the dataField.
  //short *data; // data is in and out parameter
  //data = 
  //  double scale = 1.0 / parm.parameter_scale;
  //  double bias = (-1.0 * parm.parameter_bias) / parm.parameter_scale;

  // related to field->setTypeSi32(parm.bad_data, scale, bias)
  // RadxField::_scale;  RadxField::_offset = bias; RadxField::_missingSi32 = bad_data
  // 
  /* TODO: need to find the field. How to do this????
  short bad = field->getMissingSi16(); // doradeData.bad_data;
  float parameter_scale = 1.0 / field->getScale(); // doradeData.parameter_scale; 
  float parameter_bias = -1.0 * field->getOffset() * field->getScale(); // doradeData.parameter_bias; 
  int dgi_clip_gate = 7; // field->num_samples; // or number_cells
  short dds_radd_eff_unamb_vel = ray->getNyquistMps(); // doradeData.eff_unamb_vel;
  int seds_nyquist_velocity; // TODO: what is this value?

  cerr << "args: " << endl <<
    "vert_velocity " << vert_velocity << endl <<
    "ew_velocity " << ew_velocity << endl <<
    "ns_velocity " << ns_velocity << endl <<
    "ew_gndspd_corr " << ew_gndspd_corr << endl <<
    "tilt " << tilt << endl <<
    "elevation " << elevation <<
    endl;
  */
  //SoloFunctionsApi soloFunctionsApi;
  /*  int result = soloFunctionsApi.se_remove_ac_motion(vert_velocity, ew_velocity, ns_velocity,
     ew_gndspd_corr, tilt, elevation,
     data, bad, parameter_scale, parameter_bias, dgi_clip_gate,
     dds_radd_eff_unamb_vel, seds_nyquist_velocity);
  */

  // TODO: We are converting from short to double!!!  <=====
  vector<double> newData; // (data, dgi_clip_gate+1);
  for (vector<double>::iterator it = data.begin(); it != data.end(); ++it)
    newData.push_back(*it * 2.0);
  return newData;
}



