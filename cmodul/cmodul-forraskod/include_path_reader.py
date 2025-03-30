from sysconfig import get_paths
import numpy

print("Python include path:", get_paths()["include"])
print("NumPy include path:", numpy.get_include())
