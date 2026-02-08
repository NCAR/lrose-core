#include "Calib.hh"
#include "Processor.hh"
#include <toolsa/LogStream.hh>

const string Calib::CALIB_AV_I_FIELD_NAME = "av_i";
const string Calib::CALIB_AV_Q_FIELD_NAME = "av_q";
const string Calib::CALIB_PHASE_ER_FIELD_NAME = "phase_er";

Calib::Calib() : _refN(0.0)
{
}

Calib::~Calib()
{
}


////////////////////////////////////////////////////////////
// initialization - return 0 on success, -1 on failure

int Calib::initialize(const std::string &ref_file_name)
{
  _calibFile.setReadPath(ref_file_name);
  if (_calibFile.readVolume() != 0) {
    cerr << "ERROR - cannot read calibration file: " << ref_file_name << endl;
    cerr << _calibFile.getErrStr() << endl;
    return -1;
  }

  MdvxField *calib_av_i_field =
    _calibFile.getField(CALIB_AV_I_FIELD_NAME.c_str());
  MdvxField *calib_av_q_field =
    _calibFile.getField(CALIB_AV_Q_FIELD_NAME.c_str());
  MdvxField *calib_phase_er_field =
    _calibFile.getField(CALIB_PHASE_ER_FIELD_NAME.c_str());
  if (calib_av_i_field == nullptr) {
    cerr << "ERROR - av I field missing in calibration file. "
         << ref_file_name << endl;
    return -1;
  }
  if (calib_av_q_field == 0) {
    cerr << "ERROR - av I field missing in calibration file. "
         << ref_file_name << endl;
    return -1;
  }

  _averageIQ = FieldDataPair(calib_av_i_field, calib_av_q_field);
  _phaseEr = FieldWithData(calib_phase_er_field);
  _refN = _calibFile.getMasterHeader().user_data_fl32[0];

  return 0;

}
