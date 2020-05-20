/**
 * @file IndexMgr.hh
 * @brief manages indices for identifying clumps
 * @class IndexMgr
 * @brief manages indices for identifying clumps
 */

#ifndef IndexMgr_h
#define IndexMgr_h

class IndexMgr
{
public:

  inline static int managedIndex(int actualIndex) {return (actualIndex+1)%100; }

  inline static int updateManagedIndex(int previousManagedIndex, int actualIndex)
  {
    if (previousManagedIndex < 100)
    {
      if (previousManagedIndex == managedIndex(actualIndex))
      {
	return previousManagedIndex;
      }
      else
      {
	return previousManagedIndex + managedIndex(actualIndex)*100;
      }
    }
    else if (previousManagedIndex >=100 && previousManagedIndex < 10000)
    {
      // pull out the two numbers already in there
      vector<int> existing = individualManagedIndices(previousManagedIndex);
      int newInd = managedIndex(actualIndex);
      for (size_t i=0; i<existing.size(); ++i)
      {
	if (newInd == existing[i])
	{
	  return previousManagedIndex;
	}
      }
      return previousManagedIndex + managedIndex(actualIndex)*10000;
    }
    else // if (previousManagedIndex >= 10000)
    {
      // ignore any possible new thing
      return previousManagedIndex;
    }
  }
  
  static std::vector<int> individualManagedIndices(int managedIndex)
  {
    std::vector<int> ret;
    if (managedIndex < 100)
    {
      ret.push_back(managedIndex);
    }
    else if (managedIndex >= 100 && managedIndex < 10000)
    {
      // two numbers within
      int nbig = managedIndex/100;
      int nsmall = managedIndex - nbig*100;
      ret.push_back(nbig);
      ret.push_back(nsmall);
    }
    else if (managedIndex >= 10000)
    {
      // three numbers within
      int nbiggest = managedIndex/10000;
      int remainder = managedIndex - nbiggest*10000;
      int nbig = remainder/100;
      int nsmall = remainder - nbig*100;
      ret.push_back(nbiggest);
      ret.push_back(nbig);
      ret.push_back(nsmall);
    }
    return ret;
  }
      
  
protected:
private:


};

#endif
