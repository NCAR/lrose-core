/*
  readMetarAXF.h - reads AXF format metar file into obj class container
*/

#ifndef __READMETARAXF_H
#define __READMETARAXF_H


class readMetar
{
  readMetar() {};
  virtual ~readMatar() {};
  virtual void readFile(char *fname) {};
};  

class readMetarAXF : public readMetar
{
  readMetarAXF();
  virtual ~readMatarAXF();
  virtual void readFile(char *fname);
};
  



#endif
