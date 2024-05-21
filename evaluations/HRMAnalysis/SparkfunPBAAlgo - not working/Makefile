# Makefile

CXX = g++
CXXFLAGS = -std=c++17 -lstdc++fs
TARGET = SparkfunPBACLI
LIB_ROOT = ../../../components
SRC = $(TARGET).cpp heartRate.cpp

all: $(TARGET)

$(TARGET): $(SRC) $(wildcard *.h)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) -I $(LIB_ROOT)/SignalProcessing/Filters -I $(LIB_ROOT)/Jewelry/HeartEarring -I ../HRMAnalysisCPPCLI

clean:
	rm -f $(TARGET)
