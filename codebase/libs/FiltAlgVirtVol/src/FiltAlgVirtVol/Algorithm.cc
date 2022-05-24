/**
 * @file Algorithm.cc
 */

#include <FiltAlgVirtVol/Algorithm.hh>
#include <FiltAlgVirtVol/VirtVolVolume.hh>
#include <FiltAlgVirtVol/VirtVolSweep.hh>
#include <rapmath/VolumeData.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaThreadSimple.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/pmu.h>
#include <algorithm>
#include <unistd.h>

//------------------------------------------------------------------
TaThread *Algorithm::AlgThreads::clone(int index)
{
  // it is a simple thread that uses the Algorithm::compute() as method
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadMethod(Algorithm::compute);
  t->setThreadContext(this);
  return (TaThread *)t;
}

//------------------------------------------------------------------
Algorithm::Algorithm(const MathData &data, const VolumeData &vdata)
{
  _ok = false;

  std::vector<FunctionDef> userUops = data.userUnaryOperators();
  LOG(DEBUG) << "Adding " << userUops.size() << " data unary operators";
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }

  userUops = VirtVolSweep::virtVolUserUnaryOperators();
  LOG(DEBUG) << "Adding " << userUops.size() << " virtual sweep unary operators";
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }

  userUops = VirtVolVolume::virtVolUserUnaryOperators();
  LOG(DEBUG) << "Adding " << userUops.size() << " virtual volume unary operators";
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }

  userUops = vdata.userUnaryOperators();
  LOG(DEBUG) << "Adding " << userUops.size() << " volume unary operators";
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }
}

//------------------------------------------------------------------
Algorithm::Algorithm(const FiltAlgParms &p, const MathData &data,
		     const VolumeData &vdata)
{
  _ok = true;

  std::vector<FunctionDef> userUops = data.userUnaryOperators();
  LOG(DEBUG) << "Adding " << userUops.size() << " data unary operators";
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }

  userUops = VirtVolSweep::virtVolUserUnaryOperators();
  LOG(DEBUG) << "Adding " << userUops.size() << " virtual sweep unary operators";
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }

  userUops = VirtVolVolume::virtVolUserUnaryOperators();
  LOG(DEBUG) << "Adding " << userUops.size() << " virtual volume unary operators";
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }

  userUops = vdata.userUnaryOperators();
  LOG(DEBUG) << "Adding " << userUops.size() << " volume unary operators";
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }

  for (size_t i=0; i<p._volumeBeforeFilters.size(); ++i)
  {
    _p.parse(p._volumeBeforeFilters[i], MathParser::VOLUME_BEFORE,
	     p._fixedConstantNames, p._userData);
  }

  for (size_t i=0; i<p._sweepFilters.size(); ++i)
  {
    _p.parse(p._sweepFilters[i], MathParser::LOOP2D_TO_2D, p._fixedConstantNames,
	     p._userData);
  }

  for (size_t i=0; i<p._volumeAfterFilters.size(); ++i)
  {
    _p.parse(p._volumeAfterFilters[i], MathParser::VOLUME_AFTER,
	     p._fixedConstantNames, p._userData);
  }

  _inputs = _p.identifyInputs();
  vector<string>::iterator i;
  for (i=_inputs.begin(); i!=_inputs.end(); )
  {
    if (p.matchesFixedConst(*i))
    {
      i = _inputs.erase(i);
    }
    else
    {
      i++;
    }
  }
  
  _outputs = _p.identifyOutputs();
  
  // make sure inputs are 1 to 1 and onto params, and outputs are
  // a superset of params
  _ok = p.checkConsistency(*this);

  if (!_ok)
  {
    return;
  }
}      

//------------------------------------------------------------------
Algorithm::~Algorithm(void)
{
  _p.cleanup();
}

//------------------------------------------------------------------
bool Algorithm::update(const AlgorithmParms &P, VolumeData *input)
{
  AlgThreads *thread = new AlgThreads();
  thread->init(P.num_threads, false);
  
  PMU_force_register("Process Volume");
  _p.processVolume(input);

  _p.clearOutputDebugAll();
  for (int i=0; i<input->numProcessingNodes(true); ++i)
  {
    PMU_auto_register("New Thread");
    AlgInfo *info = new AlgInfo(i, this, input, thread);
    thread->thread(i, (void *)info);
  }
  thread->waitForThreads();
  delete thread;

  _p.setOutputDebugAll();
  _p.processVolumeAfter(input);
  return true;
}

//------------------------------------------------------------------
void Algorithm::compute(void *ti)
{
  AlgInfo *info = static_cast<AlgInfo *>(ti);
  info->_alg->_p.processOneItem2d(info->_volume, info->_index,
				  info->_thread);
  delete info;
}    

//------------------------------------------------------------------
bool Algorithm::isInput(const std::string &name) const
{
  return (find(_inputs.begin(), _inputs.end(), name) != _inputs.end());
}

//------------------------------------------------------------------
bool Algorithm::isOutput(const std::string &name) const
{
  return (find(_outputs.begin(), _outputs.end(), name) != _outputs.end());
}

//------------------------------------------------------------------
void Algorithm::printOperators(void) const
{
  std::vector<FunctionDef> f = _p.allFunctionDefs();
  for (size_t i=0; i<f.size(); ++i)
  {
    printf("%s:\n%s\n\n", f[i]._name.c_str(), f[i]._description.c_str());
  }
  
  // std::string s = _p.sprintBinaryOperators();
  // printf("Binary operations:\n%s\n", s.c_str());

  // s = _p.sprintUnaryOperators();
  // printf("Unary operations:\n%s\n", s.c_str());
}
