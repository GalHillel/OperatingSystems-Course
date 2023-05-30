CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
TARGET = st_pipeline
OBJS = main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

main.o: main.cpp StPipeline.hpp ActiveObject.hpp PrimeCheck.hpp Queue.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

clean:
	rm -f $(OBJS) $(TARGET)
