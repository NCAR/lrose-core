/**
 * @file Volume.cc
 */
#include "Volume.hh"
#include "RayData1.hh"
#include "RadxTimeMedian.hh"
#include <rapmath/MathDataSimple.hh>
#include <rapmath/UnaryNode.hh>
#include <rapmath/StatusUserData.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Volume::Volume(void) :
  RadxAppVolume(), _p(NULL)
{
}

//------------------------------------------------------------------
Volume::Volume(const Parms *parms, int argc, char **argv) :
  RadxAppVolume(parms, argc, argv)
{
  _p = new RadxTimeMedian(*parms);
}

//------------------------------------------------------------------
Volume::~Volume(void)
{
  if (_p != NULL)
  {
    delete _p;
    _p = NULL;
  }
}

//------------------------------------------------------------------
bool Volume::needToSynch(const std::string &userKey) const
{
  return false;
}

//------------------------------------------------------------------
bool Volume::hasData(const std::string &userKey,
		      const std::string &name, bool suppressWarn)
{
  LOG(ERROR) << "Should never be here";
  return false;
}    


//------------------------------------------------------------------
void Volume::initialize(void)
{
  _special = SpecialUserData();
}

//------------------------------------------------------------------
void Volume::processLast(void)
{
  _p->processLast(this);
}

//------------------------------------------------------------------
void Volume::copy(Volume &templateVolume) const
{
  *((RadxAppVolume *)(&templateVolume)) = *((RadxAppVolume *)this);
  templateVolume._special = SpecialUserData();
  templateVolume._p = NULL;
}

//------------------------------------------------------------------
void Volume::setFrom(const Volume &templateVolume)
{
  *((RadxAppVolume *)this) =  *((RadxAppVolume *)(&templateVolume));

}

//------------------------------------------------------------------
std::vector<FunctionDef> Volume::userUnaryOperators(void) const
{
  std::vector<FunctionDef> ret;
  ret.push_back(FunctionDef(Parms::_volInitStr, "v", "", 
			    "initialize volume when it is the first one"));
  ret.push_back(FunctionDef(Parms::_volFinishStr, "v", "", "Finish each volume"));
  return ret;
}

//------------------------------------------------------------------
// virtual
MathData *Volume::initializeProcessingNode(int index, bool twoD) const
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
MathUserData *Volume::processUserVolumeFunction(const UnaryNode &p) //const
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
    return _volumeInit();
  }

  if (keyword == Parms::_volFinishStr)
  {
    // expect no args
    if (!args.empty())
    {
      LOG(ERROR) << "Wrong number of args want 0 got " << args.size();
      return NULL;
    }
    return _volumeFinish();
  }
  LOG(ERROR) << "Unknown keyword " << keyword;
  return NULL;
}

//------------------------------------------------------------------
void Volume::addNew(int zIndex, const MathData *s)
{
}

//------------------------------------------------------------------
// virtual
bool Volume::storeMathUserData(const std::string &name, MathUserData *s)
{
  return _special.store(name, s);
}

//------------------------------------------------------------------
MathUserData *Volume::_volumeInit(void)
{
  _p->initFirstTime(this);
  StatusUserData *s = new StatusUserData(true);
  return (MathUserData *)s;
}

//------------------------------------------------------------------
MathUserData *Volume::_volumeFinish(void)
{
  StatusUserData *s = new StatusUserData(true);
  return (MathUserData *)s;
}

