/*
 * histogram.C
 * 
 */
 
#include "histogram.h"
#include <string.h>
#include <math.h>

#ifndef sgi
#define fsqrt sqrt
#endif

histogramClass::histogramClass(int Size)
{
	histogram = 0;
	histeqtable = 0;
	total = 0;
	size = 0;
	if (Size)
	{
		size = Size;
		histogram = new int[Size];
	}
}

histogramClass::~histogramClass()
{
	if (histogram)
		delete[] histogram;
	if (histeqtable)
		delete[] histeqtable;
}

void histogramClass::clear()
{
    total = 0;
    if (histogram)
	memset(histogram, 0, sizeof(int) * size);
}

void histogramClass::getHistogram(unsigned char *buff, int buffsize)
{
unsigned char *puchar;

	if (!buff)
		return;
	if (histogram && (size != 256))
	{
		delete[] histogram;
		size = 0;
		histogram = 0;
		if (histeqtable)
		{
			delete[] histeqtable;
			histeqtable = 0;
		}
	}
	if (!histogram)		// this is unsigned char version, max 256 levels
	{
		size = 256;
		histogram = new int[size];
	}
	memset(histogram, 0, sizeof(int) * size);
	puchar = buff;
	int count = total = buffsize;
	while (count--)
	{
		histogram[*puchar]++;
		puchar++;
	}
	total = buffsize;
	calcProducts();
}

void histogramClass::addHistogram(unsigned char *buff, int buffsize)
{
unsigned char *puchar;

	if (!buff)
		return;
	if (!histogram)		// this is unsigned char version, max 256 levels
	{
		size = 256;
		histogram = new int[size];
	}
	puchar = buff;
	int count = buffsize;
	while (count--)
	{
		histogram[*puchar]++;
		puchar++;
	}
	total += buffsize;
}

/*
 * ext_histeqtable MUST be same size as Histogram
 */

void histogramClass::calcEqTable(int *ext_histeqtable)
{
int		count = 0, accum_total = 0;
int *localhisteqtable = histeqtable;

	if (!size || !histogram || (total == 0))
		return;
		
	if (ext_histeqtable)
		localhisteqtable = ext_histeqtable;
	else if (!histeqtable)
		localhisteqtable = histeqtable = new int[size];
	memset(localhisteqtable, 0, sizeof(int) * size);
	count = 0;
	while (count < size) 
	{
		accum_total += histogram[count];
		localhisteqtable[count] = ((size-1) * accum_total) / total;
		count++;
	}		
		
}

void histogramClass::calcProducts()
{
	int sum = 0, 
		max = 0, 
		min = total, 
		thisval, 
		diff, 
		valcount = 0;

	int x;

	for (x = 0; x < size; x++)
	{
		thisval = histogram[x];
		sum += thisval * x;
		if (thisval > max)
		{
			max = thisval;
			mode = x;		// mode is value with highest count
		}
		if (thisval < min)
			min = thisval;
		if (thisval) valcount++;
	}
	
//	mean = sum / size;
	mean = (sum * size) / total;

	valcount /= 2;		//	to find median, ie middle defined value
	
	x = 0;
	while (valcount && (x < size))
	{
		if (histogram[x]) valcount--;	// ony dec on defined value
		if (!valcount)		// median value
		    median = x;
		x++;
	}

	variance = 0.0;
	for (x = 0; x < size; x++)
	{
		diff = int(histogram[x] - float(mean));
		variance += (diff * diff);
	}
	variance /= size-1;
	sdev = fsqrt(variance);		
}
