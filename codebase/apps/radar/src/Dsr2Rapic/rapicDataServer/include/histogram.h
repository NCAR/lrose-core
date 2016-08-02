/*
 *  histogram.h
 */

#ifndef __HISTOGRAM_H
#define __HISTOGRAM_H



class histogramClass
{
public:
	int *histogram;
	int *histeqtable;
	int size;			// number of entries in histogram
	int total;			// size of source array
	float mean, variance, sdev;
	int median, mode;
	histogramClass(int Size);
	~histogramClass();
	void calcProducts();	
	void calcEqTable(int *ext_histeqtable = 0);	// allow calc to external table
	void clear();	// clear the histogram
	void getHistogram(unsigned char *buff, int buffsize); // read buffer to histogram
	void addHistogram(unsigned char *buff, int buffsize); // add the buffer to this histogram
};

#endif // __HISTOGRAM_H
