#include <rapmath/MathDataSimple.hh>
#include <rapmath/FunctionDef.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
MathDataSimple::MathDataSimple(void) :  MathData()
{
}

//------------------------------------------------------------------
MathDataSimple::~MathDataSimple(void)
{
}

//------------------------------------------------------------------
std::vector<FunctionDef> MathDataSimple::userUnaryOperators(void) const
{
  std::vector<FunctionDef> ret;
  return ret;
}

//------------------------------------------------------------------
// virtual
int MathDataSimple::numData(void) const
{
  return 0;
}

//------------------------------------------------------------------
// virtual
void MathDataSimple::finishProcessingNode(int index, VolumeData *vol)
{
}

//------------------------------------------------------------------
// virtual
bool
MathDataSimple::synchInputsAndOutputs(const std::string &output,
				      const std::vector<std::string> &inputs)
{
  return false;
}
    
//------------------------------------------------------------------
// virtual
MathLoopData * MathDataSimple::dataPtr(const std::string &name)
{
  return NULL;
}

//------------------------------------------------------------------
// virtual
const MathLoopData * MathDataSimple::dataPtrConst(const std::string &name) const
{
  return NULL;
}

//------------------------------------------------------------------
// virtual 
bool MathDataSimple::processUserLoopFunction(ProcessingNode &p)
{
  LOG(ERROR) << "Not implemented for this class";
  return false;
}

//------------------------------------------------------------------
MathUserData *
MathDataSimple::processUserLoopFunctionToUserData(const UnaryNode &p)
{
  LOG(ERROR) << "Not implemented for this class";
  return NULL;
}

//------------------------------------------------------------------
// virtual
bool
MathDataSimple::synchUserDefinedInputs(const std::string &userKey,
				       const std::vector<std::string> &names)
{
  return true;
}

//------------------------------------------------------------------
// virtual
bool MathDataSimple::storeMathUserData(const std::string &name, MathUserData *s)
{
  LOG(WARNING) << "MathDataSimple does not support user data yet storing "
	       << name;
  return false;
}

//------------------------------------------------------------------
// virtual
bool MathDataSimple::smooth(MathLoopData *l,
			    std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

// virtual
//------------------------------------------------------------------
bool MathDataSimple::smoothDBZ(MathLoopData *l,
			       std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool MathDataSimple::stddev(MathLoopData *l,
			    std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}
//------------------------------------------------------------------
// virtual
bool MathDataSimple::fuzzy(MathLoopData *l,
			   std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool MathDataSimple::average(MathLoopData *l,
			     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool MathDataSimple::max_expand(MathLoopData *l,
				std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool MathDataSimple::weighted_average(MathLoopData *l,
				      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
bool MathDataSimple::mask(MathLoopData *l,
			  std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
bool MathDataSimple::median(MathLoopData *l,
			    std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
bool MathDataSimple::
weighted_angle_average(MathLoopData *l,
		       std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
bool MathDataSimple::
expand_angles_laterally(MathLoopData *l,
			std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
bool MathDataSimple::clump(MathLoopData *l,
			   std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
bool MathDataSimple::
mask_missing_to_missing(MathLoopData *out,
			std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
bool MathDataSimple::trapezoid(MathLoopData *out,
			       std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
bool MathDataSimple::s_remap(MathLoopData *out,
			     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
bool MathDataSimple::max(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
const MathUserData *
MathDataSimple::userDataPtrConst(const std::string &name) const
{
  return NULL;
}

//------------------------------------------------------------------
MathUserData *MathDataSimple::userDataPtr(const std::string &name)
{
  return NULL;
}
