// class to manage thresholding of data

#ifndef _CTHRESHOLD_H_
#define _CTHRESHOLD_H_

class CThreshold {
public:
	virtual int GetNumThreshs();
	CThreshold(int nDatasize = 8);  // datasize => number of bits in original data
	~CThreshold();
	void SetThresholds(int nNum, int* pLevels);
	void ThresholdData(void *pIPbuffer, unsigned char *pOPbuffer, int length);
protected:
	int m_nNumThreshs;
	unsigned char* m_pTable;
	char	b16BitSrc;
	int		m_Size;
};

#endif
