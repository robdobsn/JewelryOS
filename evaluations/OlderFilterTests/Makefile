# Makefile

CXX = g++
CXXFLAGS = -std=c++17 -lstdc++fs -Wall -g -O0
TARGET = TestFilter4thOrder
SRC = $(TARGET).cpp

all: $(TARGET)

$(TARGET): $(SRC) $(wildcard *.h)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
