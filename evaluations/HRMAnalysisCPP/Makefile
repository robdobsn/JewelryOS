# Makefile

CXX = g++
CXXFLAGS = -std=c++17 -lstdc++fs
TARGET = HRMAnalysisCPP
LIB_ROOT = ../../components
SRC = HRMAnalysisCPP.cpp IIR1/Biquad.cpp IIR1/Butterworth.cpp IIR1/Cascade.cpp IIR1/PoleFilter.cpp

all: $(TARGET)

$(TARGET): $(SRC) $(wildcard *.h)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) -I $(LIB_ROOT)/SignalProcessing/Filters -I $(LIB_ROOT)/Jewelry/HeartEarring
	./$(TARGET)

clean:
	rm -f $(TARGET)