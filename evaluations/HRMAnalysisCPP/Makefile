# Makefile

CXX = g++
CXXFLAGS = -std=c++17 -lstdc++fs
TARGET = HRMAnalysisCPP
SRC = HRMAnalysisCPP.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)