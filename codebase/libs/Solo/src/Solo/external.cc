



extern "C" {
  int hello2(int x) {return x*1000;}
  int get_boundary_mask(int *x, int *y, int npts, int *result) {
    //int *result = new int(npts);
    for (int i=0; i<npts; i++) {
      result[i] = x[i] + y[i];
    }
    return 1;
  }
}


/*

build library like this ...

make clean_all ; make install_include ; make ; make install
cd Solo
g++ -dynamiclib -undefined suppress -flat_namespace *.o -o libSolo.dylib
cp libSolo.dylib /tmp
nm -gU /tmp/libSolo.dylib


 call like this in python ...

[eol-albireo-2:~/test_area/python_wrapper/try2] brenda% python
Python 2.7.14 |Anaconda, Inc.| (default, Oct  5 2017, 02:28:52) 
[GCC 4.2.1 Compatible Clang 4.0.1 (tags/RELEASE_401/final)] on darwin
Type "help", "copyright", "credits" or "license" for more information.
>>> from ctypes import *
>>> cdll.LoadLibrary("/tmp/libSolo.dylib")
<CDLL '/tmp/libSolo.dylib', handle 7fb5d4505040 at 10a088810>
>>> Solo = CDLL("/tmp/libSolo.dylib")
>>> TenIntegers = c_int * 3
>>> xpts = TenIntegers(3,4,5)
>>> ypts = TenIntegers(5,4,3)
>>> mask = TenIntegers(0,0,0)
>>> result = Solo.get_boundary_mask(byref(xpts), byref(ypts), 3, byref(mask))
>>> for i in mask:
...     print(i)
... 
8
8
8

*/
