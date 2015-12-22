import os


# Set this path to specify with python libraries are.
os.environ["CORAL_PYTHON_PATH"] = "E:/Programs/Python27"
# Set this parth to specify where include files for python sdk are
os.environ["CORAL_PYTHON_INCLUDES_PATH"] = "E:/Programs/Python27/include"
# Set this to specify where the python lib to link
os.environ["CORAL_PYTHON_LIB"] = "python27"
# Set this parth to specify where lib files for python sdk are
os.environ["CORAL_PYTHON_LIBS_PATH"] = "E:/Programs/Python27/libs"


# Set this path specify where boost is installed
os.environ["CORAL_BOOST_INCLUDES_PATH"] = "E:/SDKs/boost_1_55_0"
# Set this path to specify where boost has outputted it's libraries
os.environ["CORAL_BOOST_LIBS_PATH"] = "E:/SDKs/boost_1_55_0/stage/lib/"
# Set this path to specify what boost library to link
os.environ["CORAL_BOOST_PYTHON_LIB"] = "libboost_python-vc100-mt-1_55.lib"
 

# Set this to specify what threading model to user (intel threading blocks)
# os.environ["CORAL_PARALLEL"] = "CORAL_PARALLEL_TBB" 
# os.environ["CORAL_TBB_INCLUDES_PATH"] = "/usr/include/tbb"
# os.environ["CORAL_TBB_LIBS_PATH"] = "/usr/lib"
# os.environ["CORAL_TBB_LIB"] = "tbb"