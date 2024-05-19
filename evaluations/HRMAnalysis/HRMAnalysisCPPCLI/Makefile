# Makefile

CXX = g++
CXXFLAGS = -std=c++17 -lstdc++fs
TARGET = HRMAnalysisCPPCLI
LIB_ROOT = ../../../components
SRC = HRMAnalysisCPPCLI.cpp

all: $(TARGET)

$(TARGET): $(SRC) $(wildcard *.h)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) -I $(LIB_ROOT)/SignalProcessing/Filters -I $(LIB_ROOT)/Jewelry/HeartEarring

clean:
	rm -f $(TARGET)
