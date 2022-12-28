/**
 * @file RayData1.cc
 */
#include "RayData1.hh"
#include "Volume.hh"
#include "Parms.hh"
#include "RayHisto.hh"
#include "RadxTimeMedian.hh"
#include <rapmath/ProcessingNode.hh>
#include <toolsa/LogStream.hh>


//------------------------------------------------------------------
RayData1::RayData1(void) : RadxAppRayData()
{
}

//------------------------------------------------------------------
RayData1::RayData1(const Volume &r, int index) :
  RadxAppRayData(r, r.specialRef(), index)
{
  _p = r.algPtrConst();
}  

//------------------------------------------------------------------
RayData1::~RayData1(void)
{
}

//------------------------------------------------------------------
void RayData1::radxappAppendUnaryOperators(std::vector<FunctionDef> &ret) const
{
  ret.push_back(FunctionDef(Parms::_histoAccumStr, "v", "field", 
			    "update histogram info for this field"));
}

//------------------------------------------------------------------
bool RayData1::radxappUserLoopFunction(const std::string &keyword,
				       ProcessingNode &p)
{
  if (keyword == Parms::_histoAccumStr)
  {
    return _processAccumHisto(*(p.unaryOpArgs()));
  }
  else
  {
    printf("Unknown keyword %s\n", keyword.c_str());
    return false;
  }
}

//------------------------------------------------------------------
MathUserData *RayData1::radxappUserLoopFunctionToUserData(const UnaryNode &p)
{
  LOG(ERROR) << "Not implemented for this class";
  return NULL;
}

//------------------------------------------------------------------
bool RayData1::_processAccumHisto(std::vector<ProcessingNode *> &args) const
{
  RayxData r;
  RayHisto *h = _p->initRay(*_ray, r);
  if (h != NULL)
  {
    return _p->processRay(r, h);
  }
  else
  {
    LOG(ERROR) << "No accumulation for this ray";
    return false;
  }
}

