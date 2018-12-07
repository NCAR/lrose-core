#include "RayData1.hh"
#include "RayData.hh"
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
RayData1::RayData1(void) : MathData()
{
}

//------------------------------------------------------------------
RayData1::RayData1(const RayData &r, int index) :  MathData()
{
}  

//------------------------------------------------------------------
RayData1::~RayData1(void)
{
}

//------------------------------------------------------------------
std::vector<std::pair<std::string, std::string> >
RayData1::userUnaryOperators(void) const
{
  std::vector<std::pair<std::string, std::string> > ret;
  return ret;
}

//------------------------------------------------------------------
// virtual
int RayData1::numData(void) const
{
  return 0;
}

//------------------------------------------------------------------
// virtual
void RayData1::finishProcessingNode(int index, VolumeData *vol)
{
}

//------------------------------------------------------------------
// virtual
bool RayData1::synchInputsAndOutputs(const std::string &output,
				    const std::vector<std::string> &inputs)
{
  return true;
}
    
//------------------------------------------------------------------
// virtual
MathLoopData * RayData1::dataPtr(const std::string &name)
{
  return NULL;
}

//------------------------------------------------------------------
// virtual
const MathLoopData * RayData1::dataPtrConst(const std::string &name) const
{
  return NULL;
}

//------------------------------------------------------------------
// virtual 
bool RayData1::processUserLoopFunction(ProcessingNode &p)
{
  LOG(ERROR) << "Not implemented for this class";
  return false;
}

//------------------------------------------------------------------
MathUserData *RayData1::processUserLoop2dFunction(const UnaryNode &p)
{
  LOG(ERROR) << "Not implemented for this class";
  return NULL;
}

//------------------------------------------------------------------
// virtual
bool RayData1::synchUserDefinedInputs(const std::string &userKey,
				      const std::vector<std::string> &names)
{
  return true;
}

//------------------------------------------------------------------
// virtual
bool RayData1::storeMathUserData(const std::string &name, MathUserData *s)
{
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::smooth(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}
//------------------------------------------------------------------
// virtual
bool RayData1::smoothDBZ(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::stddev(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}
//------------------------------------------------------------------
// virtual
bool RayData1::fuzzy(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::average(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::weighted_average(MathLoopData *l,
				std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::weighted_angle_average(MathLoopData *l,
				      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::median(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::max(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::max_expand(MathLoopData *l,
			 std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData1::expand_angles_laterally(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData1::clump(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::mask(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData1::mask_missing_to_missing(MathLoopData *out,
				       std::vector<ProcessingNode *> &args) const
{
  return false;
}

bool RayData1::trapezoid(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const
{
  return false;
}

bool RayData1::s_remap(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const
{
  return false;
}

//------------------------------------------------------------------
const MathUserData *RayData1::userDataPtrConst(const std::string &name) const
{
  return NULL;
}

//------------------------------------------------------------------
MathUserData *RayData1::userDataPtr(const std::string &name)
{
  return NULL;
}

