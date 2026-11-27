/**
 * @file RayData1.cc
 */
#include "RayData1.hh"
#include "VolumeBase.hh"
#include "RayClutterInfo.hh"
#include "RadxPersistentClutter.hh"
#include <radar/RadxAppRayLoopData.hh>
#include <rapmath/ProcessingNode.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
RayData1::RayData1(void) : RadxAppRayData()
{
}

//------------------------------------------------------------------
RayData1::RayData1(const VolumeBase &r, int index) :
  RadxAppRayData(r, r.specialRef(), index)
{
  _p = r.algPtr();
}  

//------------------------------------------------------------------
RayData1::~RayData1(void)
{
}

//------------------------------------------------------------------
void RayData1::radxappAppendUnaryOperators(std::vector<FunctionDef> &ret) const
{
   ret.push_back(FunctionDef(Parms::_histoAccumStr, "v", "field",
   			    "Update histogram internal state for this field on the ray"));
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
  RayClutterInfo *h = _p->initRay(*_ray, r);
  if (h != NULL)
  {
    return _p->processRay(r, h);
  }
  else
  {
    // LOG(ERROR) << "No init for this ray";
    return false;
  }
}

