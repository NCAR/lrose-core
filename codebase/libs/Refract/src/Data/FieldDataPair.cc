/**
 * @file FieldDataPair.cc
 */
#include <Refract/FieldDataPair.hh>
#include <Refract/RefractConstants.hh>
#include <Refract/RefractUtil.hh>
#include <Refract/MdvCreate.hh>
#include <Refract/FieldWithData.hh>
#include <Refract/RefractInput.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/str.h>
#include <cstring>
#include <cmath>

//-----------------------------------------------------------------------
FieldDataPair::~FieldDataPair(void)
{
  _free();
}

//-----------------------------------------------------------------------
FieldDataPair::FieldDataPair(const FieldDataPair &iq, const std::string &iName,
			     const std::string &iUnits,
			     const std::string &qName,
			     const std::string &qUnits) : VectorIQ()
{
  _I = iq._createI(iName, iUnits);
  _Q = iq._createQ(qName, qUnits);
  _scanSize = iq._scanSize;
  _numAzimuth = iq._numAzimuth;
  _numGates = iq._numGates;
  Mdvx::field_header_t field_hdr = _I->getFieldHeader();
  _IBad = field_hdr.bad_data_value;
  _IMissing = field_hdr.missing_data_value;
  field_hdr = _Q->getFieldHeader();
  _QBad = field_hdr.bad_data_value;
  _QMissing = field_hdr.missing_data_value;
  _idata = (fl32 *)_I->getVol();
  _qdata = (fl32 *)_Q->getVol();
  VectorIQ::initialize(_idata, _qdata, _scanSize);
}

//-----------------------------------------------------------------------
FieldDataPair::FieldDataPair(const FieldDataPair &iq, const std::string &iName,
			     const std::string &iUnits, double ivalue,
			     const std::string &qName,
			     const std::string &qUnits, double qvalue)
  : VectorIQ()
{
  _I = iq._createI(iName, iUnits, ivalue);
  _Q = iq._createQ(qName, qUnits, qvalue);
  _scanSize = iq._scanSize;
  _numAzimuth = iq._numAzimuth;
  _numGates = iq._numGates;
  Mdvx::field_header_t field_hdr = _I->getFieldHeader();
  _IBad = field_hdr.bad_data_value;
  _IMissing = field_hdr.missing_data_value;
  field_hdr = _Q->getFieldHeader();
  _QBad = field_hdr.bad_data_value;
  _QMissing = field_hdr.missing_data_value;
  _idata = (fl32 *)_I->getVol();
  _qdata = (fl32 *)_Q->getVol();
  VectorIQ::initialize(_idata, _qdata, _scanSize);
}

//-----------------------------------------------------------------------
FieldDataPair::FieldDataPair(const FieldWithData &I, const FieldWithData &Q)
  : VectorIQ()
{
  _I = I.createMatchingFieldWithData();
  _Q = Q.createMatchingFieldWithData();
  _scanSize = I.scanSize();
  _numAzimuth = I.numAzimuth();
  _numGates = I.numGates();
  Mdvx::field_header_t field_hdr = _I->getFieldHeader();
  _IBad = field_hdr.bad_data_value;
  _IMissing = field_hdr.missing_data_value;
  field_hdr = _Q->getFieldHeader();
  _QBad = field_hdr.bad_data_value;
  _QMissing = field_hdr.missing_data_value;
  _idata = (fl32 *)_I->getVol();
  _qdata = (fl32 *)_Q->getVol();
  VectorIQ::initialize(_idata, _qdata, _scanSize);
}

//-----------------------------------------------------------------------
FieldDataPair::FieldDataPair(const MdvxField *I, const MdvxField *Q) :
  VectorIQ()
{
  _I = new MdvxField(*I);
  _Q = new MdvxField(*Q);
  Mdvx::field_header_t field_hdr = _I->getFieldHeader();
  _numAzimuth = field_hdr.ny;
  _numGates = field_hdr.nx;
  _scanSize = _numAzimuth*_numGates;
  _IBad = field_hdr.bad_data_value;
  _IMissing = field_hdr.missing_data_value;
  field_hdr = _Q->getFieldHeader();
  _QBad = field_hdr.bad_data_value;
  _QMissing = field_hdr.missing_data_value;
  _idata = (fl32 *)_I->getVol();
  _qdata = (fl32 *)_Q->getVol();
  VectorIQ::initialize(_idata, _qdata, _scanSize);
}


//-----------------------------------------------------------------------
FieldDataPair::FieldDataPair(const FieldDataPair &p) : VectorIQ(p)

{
  _scanSize = p._scanSize;
  _numAzimuth = p._numAzimuth;
  _numGates = p._numGates;
  if (p._I == NULL || p._Q == NULL)
  {
    _I = NULL;
    _Q = NULL;
    _idata = NULL;
    _qdata = NULL;
  }
  else
  {
    _I = new MdvxField(*p._I);
    _Q = new MdvxField(*p._Q);
    Mdvx::field_header_t field_hdr = _I->getFieldHeader();
    _IBad = field_hdr.bad_data_value;
    _IMissing = field_hdr.missing_data_value;
    field_hdr = _Q->getFieldHeader();
    _QBad = field_hdr.bad_data_value;
    _QMissing = field_hdr.missing_data_value;
    _idata = (fl32 *)_I->getVol();
    _qdata = (fl32 *)_Q->getVol();
  }
}

//-----------------------------------------------------------------------
FieldDataPair::FieldDataPair(const FieldDataPair &inp0, double w0,
			     const FieldDataPair &inp1, double w1) :
  _I(NULL), _Q(NULL), _scanSize(0), _idata(NULL), _qdata(NULL) 
{
  if (w0 <= 0 && w1 > 0)
  {
    *this = inp1;
  }
  else if (w1 <= 0 && w0 > 0)
  {
    *this = inp0;
  }
  else
  {
    *this = inp0;
    for (int i = 0; i < _scanSize; ++i)
    {
      if (inp0.missingIorQ(i))
      {
	if (inp1.missingIorQ(i))
	{
	  _idata[i] = inp0._idata[i];
	  _qdata[i] = inp0._qdata[i];
	}
	else
	{
	  _idata[i] = inp1._idata[i];
	  _qdata[i] = inp1._qdata[i];
	}
      }
      else
      {
	if (inp1.missingIorQ(i))
	{
	  _idata[i] = inp0._idata[i];
	  _qdata[i] = inp0._qdata[i];
	}
	else
	{
	  _idata[i] = inp0._idata[i]*w0 + inp1._idata[i]*w1;
	  _qdata[i] = inp0._qdata[i]*w0 + inp1._qdata[i]*w1;
	}
      }
    }
    // copy all values into vectoriq
    VectorIQ::initialize(_idata, _qdata, _scanSize);
  }
}

//-----------------------------------------------------------------------
FieldDataPair & FieldDataPair::operator=(const FieldDataPair &p)
{
  if (this == &p)
  {
    return *this;
  }
  
  _free();
  _scanSize = p._scanSize;
  _numAzimuth = p._numAzimuth;
  _numGates = p._numGates;
  _IBad = p._IBad;
  _IMissing = p._IMissing;
  _QBad = p._QBad;
  _QMissing = p._QMissing;
  if (p._I == NULL || p._Q == NULL)
  {
    _I = NULL;
    _Q = NULL;
    _idata = NULL;
    _qdata = NULL;
    VectorIQ::initialize(NULL, NULL, 0);
  }
  else
  {
    _I = new MdvxField(*p._I);
    _Q = new MdvxField(*p._Q);
    _idata = (fl32 *)_I->getVol();
    _qdata = (fl32 *)_Q->getVol();
    VectorIQ::initialize(_idata, _qdata, _scanSize);
  }
  return *this;
}

//-----------------------------------------------------------------------
void FieldDataPair::copyLargeR(int rMin, const VectorIQ &inp)
{
  VectorIQ::copyLargeR(rMin, _numGates, _numAzimuth, inp);
}

//-----------------------------------------------------------------------
void FieldDataPair::smoothClose(int rMin, int rMax,
				const FieldDataPair &difPrevScan)
{
  VectorIQ::smoothClose(rMin, rMax, _numAzimuth, _numGates, difPrevScan);
}

//-----------------------------------------------------------------------
void FieldDataPair::smoothFar(int rMax, const FieldDataPair &difPrevScan)
{
  VectorIQ::smoothFar(rMax, _numAzimuth, _numGates, difPrevScan);
}

//-----------------------------------------------------------------------
void FieldDataPair::copyIQFilterBadOrMissing(const FieldDataPair &r)
{
  VectorIQ::copyIQFilterBadOrMissing(r, r._IBad, r._IMissing, r._QBad,
				     r._QMissing);
}

//-----------------------------------------------------------------------
void FieldDataPair::addToOutput(DsMdvx &mdvFile) const
{
  VectorIQ::copyToArrays(_idata, _qdata, _scanSize);
  mdvFile.addField(new MdvxField(*_I));
  mdvFile.addField(new MdvxField(*_Q));
}

//-----------------------------------------------------------------------
double FieldDataPair::gateSpacing(void) const
{
  Mdvx::field_header_t fhdr = _I->getFieldHeader();
  return static_cast<double>(fhdr.grid_dx);
}

//-----------------------------------------------------------------------
double FieldDataPair::azimuthalSpacing(void) const
{
  Mdvx::field_header_t fhdr = _I->getFieldHeader();
  return static_cast<double>(fhdr.grid_dy);
}

//-----------------------------------------------------------------------
double FieldDataPair::missingValueI(void) const
{
  Mdvx::field_header_t fhdr = _I->getFieldHeader();
  return static_cast<double>(fhdr.missing_data_value);
}

//-----------------------------------------------------------------------
double FieldDataPair::missingValueQ(void) const
{
  Mdvx::field_header_t fhdr = _Q->getFieldHeader();
  return static_cast<double>(fhdr.missing_data_value);
}

//-----------------------------------------------------------------------
bool FieldDataPair::missingIorQ(int index) const
{
  return _iq[index].isBadOrMissing(_IBad, _IMissing, _QBad, _QMissing);
}
  
//-----------------------------------------------------------------------
bool FieldDataPair::wrongGateSpacing(double gate_spacing) const
{
  double gi = gateSpacing();

  if (gate_spacing != gi)
  {
    LOG(ERROR) << "Change in gate spacing old:" << gate_spacing
	       << " new:" << gi;
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------
FieldWithData FieldDataPair::createFromI(const std::string &name,
					 const std::string &units,
					 double value) const
{
  return FieldWithData(_I, name, units, value);
}

//-----------------------------------------------------------------------
FieldWithData FieldDataPair::createFromI(const std::string &name,
					 const std::string &units) const
{
  return FieldWithData(_I, name, units, missingValueI());
}

//-----------------------------------------------------------------------
MdvxField *FieldDataPair::_createI(const std::string &name,
				   const std::string &units) const
{
  return MdvCreate::createMatchingFieldWithData(_I, name, units);
}

//-----------------------------------------------------------------------
MdvxField *FieldDataPair::_createQ(const std::string &name,
				   const std::string &units) const
{
  return MdvCreate::createMatchingFieldWithData(_Q, name, units);
}

//-----------------------------------------------------------------------
MdvxField *FieldDataPair::_createI(const std::string &name,
				   const std::string &units,
				   double value) const
{
  return MdvCreate::createMatchingFieldWithValue(_I, name, units, value);
}

//-----------------------------------------------------------------------
MdvxField *FieldDataPair::_createQ(const std::string &name,
				   const std::string &units,
				   double value) const
{
  return MdvCreate::createMatchingFieldWithValue(_Q, name, units, value);
}

//-----------------------------------------------------------------------
void FieldDataPair::_free(void)
{
  if (_I != NULL)
  {
    delete _I;
    _I = NULL;
  }
  if (_Q != NULL)
  {
    delete _Q;
    _Q = NULL;
  }

  VectorIQ::initialize(NULL, NULL, 0);
  _scanSize = 0;
}

