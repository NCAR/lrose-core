/*
 *  RayFile test Application 01.
 *
 */

#include <iostream>
using namespace std;

#include <stdio.h>

int    netcdfToInt   (double scale, double bias,double value);
double netcdfToDouble(double scale, double bias,int    value);

void   computeScaleAndBias(double min,    double max, 
			   double &scale, double &bias);

void   encode(double value, double scale,double bias);
void   decode(short value, double scale,double bias);

int main(int argc, char *argv[]){

    double scale;
    double bias;

    double maxValue;
    double minValue;

    maxValue =  50.0;
    minValue = -20.0;

    computeScaleAndBias(minValue,maxValue,scale,bias);

    printf("min: %f , max: %f \n",minValue,maxValue);
    printf("scale: %f , bias: %f \n",scale,bias);

    printf("\n");
    for(int index = -20; index <= 50; index += 5){
	double value = (double)index;
	encode(value,scale,bias);
    }

    printf("\n");
    for(double value = -20.0; value < 50.0; value += 5.001){
	encode(value,scale,bias);
    }

    printf("\n");
    for(short index = -32767; index <= -32760; index++){
	decode(index,scale,bias);
    }

    printf("\n");
    for(short index = 4675; index <= 4685; index++){
	decode(index,scale,bias);
    }

    printf("\n");
    decode(-32767,scale,bias);
    decode(0     ,scale,bias);
    decode(32767,scale,bias);

}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int    netcdfToInt   (double scale, double bias,double value){

    double scaled = (value - bias) / scale;

    if(scaled < 0.0){
	return (int)(scaled - 0.5);
    }else{
	return (int)(scaled + 0.5);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double netcdfToDouble(double scale, double bias,int    value){

    return ((double)value * scale) + bias;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void   computeScaleAndBias(double min,    double max, 
			   double &scale, double &bias){


    double minShort = -32767;
    double maxShort =  32767;
    
    scale = (max - min)/ (maxShort - minShort);
    bias  = max - (scale * maxShort);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void   encode(double value, double scale,double bias){

    short   encodedValue;
    double  decodedValue;

    encodedValue = netcdfToInt(scale,bias,value);

    decodedValue = netcdfToDouble(scale,bias,encodedValue);

    printf("%9.5f -> %6d : 0x%04hX -> %9.5f\n",value,encodedValue,encodedValue,decodedValue);

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void   decode(short value, double scale,double bias){

    double  decodedValue;

    decodedValue = netcdfToDouble(scale,bias,value);

    printf("%9s    %6d : 0x%04hX -> %9.5f\n"," ",value,value,decodedValue);

}
