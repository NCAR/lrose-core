#include "RayData2.hh"
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
RayData2::RayData2(void) :  MathData()
{
}

//------------------------------------------------------------------
RayData2::RayData2(const RayData &r, int index) : MathData()
{
}  

//------------------------------------------------------------------
RayData2::~RayData2(void)
{
}

//------------------------------------------------------------------
std::vector<std::pair<std::string, std::string> >
RayData2::userUnaryOperators(void) const
{
  std::vector<std::pair<std::string, std::string> > ret;
  return ret;
}

//------------------------------------------------------------------
// virtual
int RayData2::numData(void) const
{
  return 0;
}

//------------------------------------------------------------------
// virtual
void RayData2::finishProcessingNode(int index, VolumeData *vol)
{
}

//------------------------------------------------------------------
// virtual
bool RayData2::synchInputsAndOutputs(const std::string &output,
				    const std::vector<std::string> &inputs)
{
  return false;
}
    
//------------------------------------------------------------------
// virtual
MathLoopData * RayData2::dataPtr(const std::string &name)
{
  return NULL;
}

//------------------------------------------------------------------
// virtual
const MathLoopData * RayData2::dataPtrConst(const std::string &name) const
{
  return NULL;
}

//------------------------------------------------------------------
// virtual 
bool RayData2::processUserLoopFunction(ProcessingNode &p)
{
  return false;
}

//------------------------------------------------------------------
MathUserData *RayData2::processUserLoop2dFunction(const UnaryNode &p)
{
  LOG(ERROR) << "Not implemented for this class";
  return NULL;
}

//------------------------------------------------------------------
// virtual
bool RayData2::synchUserDefinedInputs(const std::string &userKey,
				      const std::vector<std::string> &names)
{
  return true;
}

//------------------------------------------------------------------
// virtual
bool RayData2::storeMathUserData(const std::string &name, MathUserData *s){
  LOG(WARNING) << "RayData2 does not support user data yet storing " << name;
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData2::smooth(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

// virtual
//------------------------------------------------------------------
bool RayData2::smoothDBZ(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData2::stddev(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}
//------------------------------------------------------------------
// virtual
bool RayData2::fuzzy(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData2::average(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData2::max_expand(MathLoopData *l,
			 std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData2::weighted_average(MathLoopData *l,
				std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::mask(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::median(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::weighted_angle_average(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::expand_angles_laterally(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::clump(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::mask_missing_to_missing(MathLoopData *out,
				       std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::trapezoid(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::s_remap(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::max(MathLoopData *out,
		   std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
const MathUserData *RayData2::userDataPtrConst(const std::string &name) const
{
  return NULL;
}

//------------------------------------------------------------------
MathUserData *RayData2::userDataPtr(const std::string &name)
{
  return NULL;
}
