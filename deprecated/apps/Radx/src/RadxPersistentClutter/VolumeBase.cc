/**
 * @file VolumeBase.cc
 */
#include "VolumeBase.hh"
#include "RayData1.hh"
#include <rapmath/MathDataSimple.hh>
#include <rapmath/UnaryNode.hh>
#include <rapmath/StatusUserData.hh>
#include <rapmath/FunctionDef.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
VolumeBase::VolumeBase(void) :
  RadxAppVolume(), _parms(NULL), _state(FIRST), _converged(false),
  _doWrite(false)
{
}

//------------------------------------------------------------------
VolumeBase::VolumeBase(const Parms *parms, int argc, char **argv):
  RadxAppVolume(parms, argc, argv), _parms(parms), _state(FIRST),
  _converged(false), _doWrite(false)
{
}

//------------------------------------------------------------------
void VolumeBase::setV(const VolumeBase &r)
{
  *((RadxAppVolume *)this) = r;
  _parms = r._parms;
  _state = r._state;
  _converged = r._converged;
  _doWrite = r._doWrite;
}

//------------------------------------------------------------------
VolumeBase::~VolumeBase(void)
{
}

//------------------------------------------------------------------
bool VolumeBase::needToSynch(const std::string &userKey) const
{
  if (userKey == Parms::_volInitStr)
  {
    // no args
    return false;
  }
  else if (userKey == Parms::_volFinishStr)
  {
    // no args
    return false;
  }
  return false;
}

//------------------------------------------------------------------
bool VolumeBase::hasData(const std::string &userKey,
			   const std::string &name, bool suppressWarn)
{
  LOG(ERROR) << "should not be here";
  return false;
}    


//------------------------------------------------------------------
std::vector<FunctionDef> VolumeBase::userUnaryOperators(void) const
{
  std::vector<FunctionDef> ret;
  ret.push_back(FunctionDef(Parms::_volInitStr, "v", "", 
  			    "Change internal state using the input volume, when appropriate"));
  ret.push_back(FunctionDef(Parms::_volFinishStr, "v", "", 
  			    "Update state after all rays for a volume have been processed"));
  return ret;
}

//------------------------------------------------------------------
void VolumeBase::initialize(void)
{
  _special = SpecialUserData();
}

//------------------------------------------------------------------
// virtual
MathData *VolumeBase::initializeProcessingNode(int index, bool twoD) const
{
  MathData *ret=NULL;
  if (twoD)
  {
    MathDataSimple *rd = new MathDataSimple();
    ret = (MathData *)rd;
  }
  else
  {
    RayData1 *rd = new RayData1(*this, index);
    ret = (MathData *)rd;
  }
  return ret;
}

//------------------------------------------------------------------
// virtual
MathUserData *VolumeBase::processUserVolumeFunction(const UnaryNode &p)
{
  // pull out the keyword
  string keyword;
  if (!p.getUserUnaryKeyword(keyword))
  {
    return NULL;
  }
  vector<string> args = p.getUnaryNodeArgStrings();
  
  if (keyword == Parms::_volInitStr)
  {
    // expect no args
    if (!args.empty())
    {
      LOG(ERROR) << "Wrong number of args want 0 got " << args.size();
      return NULL;
    }
    return volumeInit();
  }

  if (keyword == Parms::_volFinishStr)
  {
    // expect no args
    if (!args.empty())
    {
      LOG(ERROR) << "Wrong number of args want 0 got " << args.size();
      return NULL;
    }
    return volumeFinish();
  }
  LOG(ERROR) << "Unknown keyword " << keyword;
  return NULL;
}

//------------------------------------------------------------------
void VolumeBase::addNew(int zIndex, const MathData *s)
{
}

//------------------------------------------------------------------
// virtual
bool VolumeBase::storeMathUserData(const std::string &name, MathUserData *s)
{
  return _special.store(name, s);
}

