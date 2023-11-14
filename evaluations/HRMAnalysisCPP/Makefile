# Makefile

CXX = g++
CXXFLAGS = -std=c++17 -lstdc++fs
TARGET = HRMAnalysisCPP
SRC = HRMAnalysisCPP.cpp IIR1/Biquad.cpp IIR1/Butterworth.cpp IIR1/Cascade.cpp IIR1/PoleFilter.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)