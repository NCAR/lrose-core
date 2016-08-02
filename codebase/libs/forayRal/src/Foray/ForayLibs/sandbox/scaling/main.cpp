/*
 *  RayFile test Application 01.
 *
 */

#include <iostream>
using namespace std;

#include <stdio.h>

double toDoradeScale(double scale);
double toDoradeBias (double netcdfBias,double netcdfScale);
double toNetcdfScale(double scale);
double toNetcdfBias (double doradeBias,double doradeScale);

int    doradeToInt   (double scale, double bias,double value);
double doradeToDouble(double scale, double bias,int    value);

int    netcdfToInt   (double scale, double bias,double value);
double netcdfToDouble(double scale, double bias,int    value);

int main(int argc, const char **argv){

    double scale;
    double bias;

    double doradeScale,doradeBias,netcdfScale,netcdfBias;

    printf("\n");
    printf("********************************************************************************\n");
    printf("conversion test\n");

    netcdfScale = 0.5;
    netcdfBias  = -20.0;
    printf("%8s, %6s, %6s\n","type","scale","bias");
    printf("%8s, %6.2f, %6.2f\n","netcdf",netcdfScale,netcdfBias);

    doradeScale = toDoradeScale(netcdfScale);
    doradeBias  = toDoradeBias(netcdfBias,netcdfScale);
    printf("%8s, %6.2f, %6.2f\n","dorade",doradeScale,doradeBias);

    netcdfScale = toNetcdfScale(doradeScale);
    netcdfBias  = toNetcdfBias(doradeBias,doradeScale);
    printf("%8s, %6.2f, %6.2f\n","netcdf",netcdfScale,netcdfBias);


    printf("\n");
    printf("********************************************************************************\n");
    printf("-20 to 80 maps to  0 to 200\n");

    netcdfScale = 0.5;
    netcdfBias  = -20.0;

    doradeScale = toDoradeScale(netcdfScale);
    doradeBias  = toDoradeBias(netcdfBias,netcdfScale);

    printf("\n");
    printf("netcdf scale: %8.3f  \n",netcdfScale);
    printf("netcdf bias : %8.3f  \n",netcdfBias);
    printf("%6s, %6s, %6s\n","orig","encode","recall");
    for(double value = -20; value <= 80; value += 7){
	int     encode = netcdfToInt   (netcdfScale,netcdfBias,value);
	double  recall = netcdfToDouble(netcdfScale,netcdfBias,encode);
	printf("%6.2f, %6d, %6.2f\n",value,encode,recall);
    }

    printf("\n");
    printf("dorade scale: %8.3f  \n",doradeScale);
    printf("dorade bias : %8.3f  \n",doradeBias);
    printf("%6s, %6s, %6s\n","orig","encode","recall");
    for(double value = -20; value <= 80; value += 7){
	int     encode = doradeToInt   (doradeScale,doradeBias,value);
	double  recall = doradeToDouble(doradeScale,doradeBias,encode);
	printf("%6.2f, %6d, %6.2f\n",value,encode,recall);
    }

    printf("\n");
    printf("********************************************************************************\n");
    printf("-20 to 80 maps to -250 to 250\n");

    netcdfScale = 0.2;
    netcdfBias = 30.0;

    doradeScale = toDoradeScale(netcdfScale);
    doradeBias  = toDoradeBias(netcdfBias,netcdfScale);
    
    printf("\n");
    printf("netcdf scale: %8.3f  \n",netcdfScale);
    printf("netcdf bias : %8.3f  \n",netcdfBias);
    printf("%6s, %6s, %6s\n","orig","encode","recall");
    for(double value = -20; value <= 80; value += 7){
	int     encode = netcdfToInt(netcdfScale,netcdfBias,value);
	double  recall = netcdfToDouble(netcdfScale,netcdfBias,encode);
	printf("%6.2f, %6d, %6.2f\n",value,encode,recall);
    }

    printf("\n");
    printf("dorade scale: %8.3f  \n",doradeScale);
    printf("dorade bias : %8.3f  \n",doradeBias);
    printf("%6s, %6s, %6s\n","orig","encode","recall");
    for(double value = -20; value <= 80; value += 7){
	int     encode = doradeToInt   (doradeScale,doradeBias,value);
	double  recall = doradeToDouble(doradeScale,doradeBias,encode);
	printf("%6.2f, %6d, %6.2f\n",value,encode,recall);
    }

}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double toDoradeScale(double scale){
    return 1.0/scale;
}

double toDoradeBias(double netcdfBias,double netcdfScale){
    
    return -1.0 * netcdfBias / netcdfScale;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double toNetcdfScale(double scale){
    return 1.0/scale;
}

double toNetcdfBias(double doradeBias,double doradeScale){
    
    return -1.0 * doradeBias / doradeScale;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int    doradeToInt   (double scale, double bias,double value){

    double scaled = (value * scale) + bias;

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
double doradeToDouble(double scale, double bias,int    value){

    return ((double)value - bias) / scale;
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




