
import ctypes
TestLib = ctypes.cdll.LoadLibrary('/tmp/LibSolo.so')
print TestLib.SampleAddInt(1,2)
data = [1,2,3,4]
newData = [0,0,0,0]
bad = -3
bnd = [1,1,1,1]
TestLib.se_despeckle(pointer(data(4)), pointer(newData(4)), 4, -3.0, 1, 5, pointer(bnd(4)))
