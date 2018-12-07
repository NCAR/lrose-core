#include "RayData.hh"
 #include <toolsa/LogStream.hh>

//------------------------------------------------------------------
RayData::RayData(void) :
  RadxAppVolume()
{
}

//------------------------------------------------------------------
RayData::RayData(const RadxAppParms *parms, int argc, char **argv):
  RadxAppVolume(parms, argc, argv)
{
}

//------------------------------------------------------------------
RayData::RayData(const RayData &r) : RadxAppVolume(r)
{
}

void RayData::operator=(const RayData &r)
{
  *((RadxAppVolume *)this) = r;
}

//------------------------------------------------------------------
RayData::~RayData(void)
{
}

//------------------------------------------------------------------
std::vector<std::pair<std::string, std::string> >
RayData::userUnaryOperators(void) const
{
  std::vector<std::pair<std::string, std::string> > ret;

  return ret;
}

//------------------------------------------------------------------
// virtual
MathData *RayData::initializeProcessingNode(int index, bool twoD) const
{
  return NULL;
}

//------------------------------------------------------------------
// virtual
bool RayData::synchUserDefinedInputs(const std::string &userKey,
				     const std::vector<std::string> &names)
{
  return true;
}

//------------------------------------------------------------------
// virtual
MathUserData *RayData::processUserVolumeFunction(const UnaryNode &p) //const
{
  return NULL;
}

//------------------------------------------------------------------
void RayData::addNew(int zIndex, const MathData *s)
{
}

//------------------------------------------------------------------
// virtual
bool RayData::storeMathUserData(const std::string &name, MathUserData *s)
{
  return false;
}

