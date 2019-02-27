/**
 * @file Algorithm.cc
 */

#include <FiltAlgVirtVol/Algorithm.hh>
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
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }
  userUops = vdata.userUnaryOperators();
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }
}

//------------------------------------------------------------------
Algorithm::Algorithm(const AlgorithmParms &p, const MathData &data,
		     const VolumeData &vdata)
{
  _ok = true;

  std::vector<FunctionDef> userUops = data.userUnaryOperators();
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }
  userUops = vdata.userUnaryOperators();
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }

  for (size_t i=0; i<p._volumeBeforeFilters.size(); ++i)
  {
    _p.parse(p._volumeBeforeFilters[i], MathParser::VOLUME_BEFORE,
	     p._fixedConstants, p._userData);
  }

  for (size_t i=0; i<p._sweepFilters.size(); ++i)
  {
    _p.parse(p._sweepFilters[i], MathParser::LOOP2D_TO_2D, p._fixedConstants,
	     p._userData);
  }

  for (size_t i=0; i<p._volumeAfterFilters.size(); ++i)
  {
    _p.parse(p._volumeAfterFilters[i], MathParser::VOLUME_AFTER,
	     p._fixedConstants, p._userData);
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
  
  // check for internal consistency, completeness, and uniqueness
  vector<string> names;

  for (size_t i=0; i<_inputs.size(); ++i)
  {
    string name = _inputs[i];
    if (find(names.begin(), names.end(), name) == names.end())
    {
      names.push_back(name);
    }
  }

  vector<string> o;
  for (int i=0; i<_p.numLoop2DFilters(); ++i)
  {
    string s = _p.loopFilter2dRef(i)._output;
    if (!s.empty())
    {
      if (find(o.begin(), o.end(), s) == o.end())
      {
	o.push_back(s);
      }
    }
  }

  // see if there are outputs that are not in the list of names, 
  // filter down outputs to those that are actually output
  // note that an input can also be an output. Just lift the outputs

  for (int i=0; i<p.output_n; ++i)
  {
    string name = p._output[i];
    if (find(_outputs.begin(), _outputs.end(), name) == _outputs.end())
    {
      LOG(ERROR) << "Named output " << name << " is not a filter output";
      _ok = false;
    }
  }
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
  std::string s = _p.sprintBinaryOperators();
  printf("Binary operations:\n%s\n", s.c_str());

  s = _p.sprintUnaryOperators();
  printf("Unary operations:\n%s\n", s.c_str());
}
