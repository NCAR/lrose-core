/**
 * @file MathData.cc
 */
#include <rapmath/MathData.hh>
#include <rapmath/ProcessingNode.hh>
#include <toolsa/LogStream.hh>
using std::string;
using std::pair;

//-----------------------------------------------------------------------
const MathLoopData *
MathData::loadData(std::vector<ProcessingNode *> &args, int index) const
{
  if ((int)args.size() <= index)
  {
    LOG(ERROR) << "Wrong number of args want at least " << index;
    return NULL;
  }
  string dataName = args[index]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named data for arg " << index;
    return NULL;
  }
  const MathLoopData *field = dataPtrConst(dataName);
  if (field == NULL)
  {
    LOG(ERROR) << "No data to go with " << dataName;
  }
  return field;
}

//-----------------------------------------------------------------------
bool MathData::loadDataValueValue(std::vector<ProcessingNode *> &args,
				  const MathLoopData **field, double &v0,
				  double &v1) const
{
  if (args.size() != 3)
  {
    LOG(ERROR) << "Wrong number of args want 3 got " << args.size();
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named data for arg 0";
    return false;
  }

  if (!args[1]->getValue(v0))
  {
    LOG(ERROR) << "No value in arg 1";
    return false;
  }
  if (!args[2]->getValue(v1))
  {
    LOG(ERROR) << "No value in arg 2";
    return false;
  }

  *field = dataPtrConst(dataName);
  if (*field == NULL)
  {
    LOG(ERROR) << "No data to go with " << dataName;
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::loadDataValue(std::vector<ProcessingNode *> &args,
			     const MathLoopData **field, double &v) const
{
  if (args.size() != 2)
  {
    LOG(ERROR) << "Wrong number of args want 2 got " << args.size();
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named data for arg 0";
    return false;
  }

  if (!args[1]->getValue(v))
  {
    LOG(ERROR) << "No value in arg 1";
    return false;
  }

  *field = dataPtrConst(dataName);
  if (*field == NULL)
  {
    LOG(ERROR) << "No data to go with " << dataName;
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------
bool
MathData::loadDataAndPairs(std::vector<ProcessingNode *> &args,
			   const MathLoopData **field,
			   std::vector<std::pair<double,double> > &pairs) const
{
  // expect an odd # of args  field x,y,x,y,..x,y
  int n = (int)args.size();
  if (n%2 == 0)
  {
    LOG(ERROR) << "Expect odd number of args";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named arg 0";
    return false;
  }

  *field = dataPtrConst(dataName);
  if (*field == NULL)
  {
    LOG(ERROR) << "No data to go with " << dataName;
    return false;
  }

  pairs.clear();
  for (int i=1; i<n; i+=2)
  {
    double x, y;
    if (!args[i]->getValue(x))
    {
      LOG(ERROR) << "No value in arg position" << i;
      return false;
    }
    if (!args[i+1]->getValue(y))
    {
      LOG(ERROR) << "No value in arg position << i+1";
      return false;
    }
    pairs.push_back(pair<double,double>(x,y));
  }
  return true;
}

//-----------------------------------------------------------------------
bool
MathData::loadDataDataAndPairs(std::vector<ProcessingNode *> &args,
			       const MathLoopData **field,
			       const MathLoopData **field2,
			   std::vector<std::pair<double,double> > &pairs) const
{
  // expect an even # of args  field field2 x,y,x,y,..x,y
  int n = (int)args.size();
  if (n%2 != 0)
  {
    LOG(ERROR) << "Expect even number of args";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named arg 0";
    return false;
  }
  *field = dataPtrConst(dataName);
  if (*field == NULL)
  {
    LOG(ERROR) << "No data to go with " << dataName;
    return false;
  }

  dataName = args[1]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named arg 1";
    return false;
  }
  *field2 = dataPtrConst(dataName);
  if (*field2 == NULL)
  {
    LOG(ERROR) << "No data to go with " << dataName;
    return false;
  }

  pairs.clear();
  for (int i=2; i<n; i+=2)
  {
    double x, y;
    if (!args[i]->getValue(x))
    {
      LOG(ERROR) << "No value in arg position" << i;
      return false;
    }
    if (!args[i+1]->getValue(y))
    {
      LOG(ERROR) << "No value in arg position << i+1";
      return false;
    }
    pairs.push_back(pair<double,double>(x,y));
  }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::
loadValueAndMultiData(std::vector<ProcessingNode *> &args,
		      double &value,
		      std::vector<const MathLoopData *> &fields) const
{
  // expect 0th arg to be a number, rest of args are
  // data, so better be at least 3 args
  if (args.size() < 3)
  {
    LOG(ERROR) << "Expect at least 3 args";
    return false;
  }

  if (!args[0]->getValue(value))
  {
    LOG(ERROR) << "No value in position 0";
    return false;
  }

  fields.clear();
  for (size_t i=1; i<args.size(); ++i)
  {
    string dataName = args[i]->leafName();
    if (dataName.empty())
    {
      LOG(ERROR) << " NO name in position " << i;
      return false;
    }
    const MathLoopData *field = dataPtrConst(dataName);
    if (field == NULL)
    {
      LOG(ERROR) << "No data for " << dataName;
      return false;
    }
    fields.push_back(field);
  }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::
loadMultiData(std::vector<ProcessingNode *> &args,
	      std::vector<const MathLoopData *> &fields) const
{
  if (args.size() < 2)
  {
    LOG(ERROR) << "Expect at least 2 args";
    return false;
  }

  fields.clear();
  for (size_t i=0; i<args.size(); ++i)
  {
    string dataName = args[i]->leafName();
    if (dataName.empty())
    {
      LOG(ERROR) << " NO name in position " << i;
      return false;
    }
    const MathLoopData *field = dataPtrConst(dataName);
    if (field == NULL)
    {
      LOG(ERROR) << "No data for " << dataName;
      return false;
    }
    fields.push_back(field);
  }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::
loadDataData(std::vector<ProcessingNode *> &args,
	     const MathLoopData **data0, const MathLoopData **data1) const
{
  if (args.size() != 2)
  {
    LOG(ERROR) << "Need 2 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 0";
    return false;
  }
  *data0 = dataPtrConst(dataName);
  if (data0 == NULL)
  {
    LOG(ERROR) << "No data for " << dataName;
    return false;
  }
  dataName = args[1]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 1";
    return false;
  }
  *data1 = dataPtrConst(dataName);
  if (data1 == NULL)
  {
    LOG(ERROR) << "No data for" << dataName;
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::
loadDataAndThreeNumbers(std::vector<ProcessingNode *> &args,
		       const MathLoopData **data,
			double &n0, double &n1, double &n2) const
{
  if (args.size() != 4)
  {
    LOG(ERROR) << "Need 5 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 0";
    return false;
  }
  *data = dataPtrConst(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data for " << dataName;
    return false;
  }

  if (!args[1]->getValue(n0))
  {
    LOG(ERROR) << "No number in arg 1";
    return false;
  }

  if (!args[2]->getValue(n1))
  {
    LOG(ERROR) << "No number in arg 2";
    return false;
  }

  if (!args[3]->getValue(n2))
  {
    LOG(ERROR) << "No number in arg 3";
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::
loadDataAndFourNumbers(std::vector<ProcessingNode *> &args,
		       const MathLoopData **data,
		       double &n0, double &n1, double &n2,
		       double &n3) const
{
  if (args.size() != 5)
  {
    LOG(ERROR) << "Need 5 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 0";
    return false;
  }
  *data = dataPtrConst(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data for " << dataName;
    return false;
  }

  if (!args[1]->getValue(n0))
  {
    LOG(ERROR) << "No number in arg 1";
    return false;
  }

  if (!args[2]->getValue(n1))
  {
    LOG(ERROR) << "No number in arg 2";
    return false;
  }

  if (!args[3]->getValue(n2))
  {
    LOG(ERROR) << "No number in arg 3";
    return false;
  }

  if (!args[4]->getValue(n3))
  {
    LOG(ERROR) << "No number in arg 4";
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::
loadDataAndFiveNumbers(std::vector<ProcessingNode *> &args,
		       const MathLoopData **data,
		       double &n0, double &n1, double &n2,
		       double &n3, double &n4) const
{
  if (args.size() != 6)
  {
    LOG(ERROR) << "Need 6 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 0";
    return false;
  }
  *data = dataPtrConst(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data for " << dataName;
    return false;
  }

  if (!args[1]->getValue(n0))
  {
    LOG(ERROR) << "No number in arg 1";
    return false;
  }

  if (!args[2]->getValue(n1))
  {
    LOG(ERROR) << "No number in arg 2";
    return false;
  }

  if (!args[3]->getValue(n2))
  {
    LOG(ERROR) << "No number in arg 3";
    return false;
  }

  if (!args[4]->getValue(n3))
  {
    LOG(ERROR) << "No number in arg 4";
    return false;
  }
  if (!args[5]->getValue(n4))
  {
    LOG(ERROR) << "No number in arg 5";
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::
loadNumberAndDataNumberPairs(std::vector<ProcessingNode *> &args,
			     double &n,
			     std::vector<const MathLoopData *> &data,
			     std::vector<double> &numbers) const
{
  if (args.size() < 3)
  {
    LOG(ERROR) << "Expect at least 3 args";
    return false;
  }
  if ((int)(args.size() %2) == 0)
  {
    LOG(ERROR) << "Expect odd # of args";
    return false;
  }

  if (!args[0]->getValue(n))
  {
    LOG(ERROR) << "No value in arg 0";
    return false;
  }

  for (size_t i=1; i<args.size(); i+=2)
  {
    string dataName = args[i]->leafName();
    if (dataName.empty())
    {
      LOG(ERROR) << " NO name in arg " << i;
      return false;
    }
    const MathLoopData *d = dataPtrConst(dataName);
    if (d == NULL)
    {
      LOG(ERROR) << "No data for " << dataName;
      return false;
    }

    double w;
    if (!args[i+1]->getValue(w))
    {
      LOG(ERROR) << "No value in arg " << i+1;
      return false;
    }

    data.push_back(d);
    numbers.push_back(w);
  }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::loadDataAndUserData(std::vector<ProcessingNode *> &args,
				   const MathLoopData **data,
				   MathUserData **udata)
{
  if (args.size() != 2)
  {
    LOG(ERROR) << "Need 2 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 0";
    return false;
  }
  *data = dataPtrConst(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data for " << dataName;
    return false;
  }

  dataName = args[1]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 1";
    return false;
  }
  *udata = userDataPtr(dataName);
  if (udata == NULL)
  {
    LOG(ERROR) << "No data for" << dataName;
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::loadDataAndTwoUserDatas(std::vector<ProcessingNode *> &args,
				       const MathLoopData **data,
				       MathUserData **udata1,
				       MathUserData **udata2)
{
  if (args.size() != 3)
  {
    LOG(ERROR) << "Need 3 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 0";
    return false;
  }
  *data = dataPtrConst(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data for " << dataName;
    return false;
  }

  dataName = args[1]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 1";
    return false;
  }
  *udata1 = userDataPtr(dataName);
  if (udata1 == NULL)
  {
    LOG(ERROR) << "No data for" << dataName;
    return false;
  }

  dataName = args[2]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 2";
    return false;
  }
  *udata2 = userDataPtr(dataName);
  if (udata2 == NULL)
  {
    LOG(ERROR) << "No data for" << dataName;
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::loadDataAndUserDataAndValue(std::vector<ProcessingNode *> &args,
					   const MathLoopData **data,
					   MathUserData **udata,
					   double &value)
{
  if (args.size() != 3)
  {
    LOG(ERROR) << "Need 3 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 0";
    return false;
  }
  *data = dataPtrConst(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data for " << dataName;
    return false;
  }

  dataName = args[1]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 1";
    return false;
  }
  *udata = userDataPtr(dataName);
  if (udata == NULL)
  {
    LOG(ERROR) << "No data for" << dataName;
    return false;
  }
  if (!args[2]->getValue(value))
    {
      LOG(ERROR) << "No value in arg position 2";
      return false;
    }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::loadNameAndUserDataAndValue(std::vector<ProcessingNode *> &args,
					   std::string &name,
					   MathUserData **udata,
					   double &value)
{
  if (args.size() != 3)
  {
    LOG(ERROR) << "Need 3 inputs";
    return false;
  }
  name = args[0]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name in arg 0";
    return false;
  }

  string dataName = args[1]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 1";
    return false;
  }
  *udata = userDataPtr(dataName);
  if (udata == NULL)
  {
    LOG(ERROR) << "No data for" << dataName;
    return false;
  }
  if (!args[2]->getValue(value))
    {
      LOG(ERROR) << "No value in arg position 2";
      return false;
    }
  return true;
}

//-----------------------------------------------------------------------
bool MathData::
loadDataDataValuesAndPairs(std::vector<ProcessingNode *> &args,
			   int numValues,
			   const MathLoopData **field,
			   const MathLoopData **field2,
			   std::vector<double> &values,
			   std::vector<std::pair<double,double> > &pairs) const
{
  // expect 2 + numValues to leave an even number of args
  int n = (int)args.size();
  int npairs = n - 2 - numValues;
  if (npairs%2 != 0)
  {
    LOG(ERROR) << "Expect even number of paired args";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named arg 0";
    return false;
  }
  *field = dataPtrConst(dataName);
  if (*field == NULL)
  {
    LOG(ERROR) << "No data to go with " << dataName;
    return false;
  }

  dataName = args[1]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named arg 1";
    return false;
  }
  *field2 = dataPtrConst(dataName);
  if (*field2 == NULL)
  {
    LOG(ERROR) << "No data to go with " << dataName;
    return false;
  }

  values.clear();
  for (int i=2; i<2+numValues; i++)
  {
    double x;
    if (!args[i]->getValue(x))
    {
      LOG(ERROR) << "No value in arg position" << i;
      return false;
    }
    values.push_back(x);
  }


  pairs.clear();
  for (int i=2+numValues; i<n; i+=2)
  {
    double x, y;
    if (!args[i]->getValue(x))
    {
      LOG(ERROR) << "No value in arg position" << i;
      return false;
    }
    if (!args[i+1]->getValue(y))
    {
      LOG(ERROR) << "No value in arg position << i+1";
      return false;
    }
    pairs.push_back(pair<double,double>(x,y));
  }
  return true;
}

//-----------------------------------------------------------------------
MathUserData *MathData::userData(std::vector<ProcessingNode *> &args)
{
  if (args.size() != 1)
  {
    LOG(ERROR) << "Bad interface";
    return NULL;
  }

  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named data in args 0";
    return NULL;
  }
  MathUserData *u = userDataPtr(dataName);
  if (u == NULL)
  {
    LOG(ERROR) << "No data associated with " << dataName;
    return NULL;
  }
  return u;
}


//-----------------------------------------------------------------------
std::string MathData::getDataName(std::vector<ProcessingNode *> &args,
				  int index) const
{
  if ((int)args.size() <= index)
  {
    LOG(ERROR) << "Bad interface";
    return NULL;
  }
  
  string dataName = args[index]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named data in args " << index;
  }
  return dataName;
}

//-----------------------------------------------------------------------
bool MathData::
loadMultiDataAndMultiValues(std::vector<ProcessingNode *> &args,
			    int numData,
			    std::vector<const MathLoopData *> &data,
			    int numValues,
			    std::vector<double> &values) const
{
  if ((int)args.size() != numData + numValues)
  {
    LOG(ERROR) << numData << " Data + " << numValues
	       << " values  but have nargs=" << args.size();
    return false;
  }
  data.clear();
  for (int i=0; i<numData; ++i)
  {
    string dataName = args[i]->leafName();
    if (dataName.empty())
    {
      LOG(ERROR) << "No named data in args " << i;
      return false;
    }
    const MathLoopData *d = dataPtrConst(dataName);
    if (d == NULL)
    {
      LOG(ERROR) << "No data associated with " << dataName;
      return false;
    }
    data.push_back(d);
  }

  values.clear();
  for (int i=0; i<numValues; ++i)
  {
    double x;
    if (!args[i+numData]->getValue(x))
    {
      LOG(ERROR) << "No value in arg position" << i+numData;
      return false;
    }
    values.push_back(x);
  }
  return true;
}
