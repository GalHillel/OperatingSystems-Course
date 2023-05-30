CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
TARGET = st_pipeline
OBJS = Queue.o PrimeCheck.o ActiveObject.o StPipeline.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

Queue.o: Queue.cpp Queue.hpp
	$(CXX) $(CXXFLAGS) -c Queue.cpp -o Queue.o

PrimeCheck.o: PrimeCheck.cpp PrimeCheck.hpp
	$(CXX) $(CXXFLAGS) -c PrimeCheck.cpp -o PrimeCheck.o

ActiveObject.o: ActiveObject.cpp ActiveObject.hpp Queue.hpp
	$(CXX) $(CXXFLAGS) -c ActiveObject.cpp -o ActiveObject.o

StPipeline.o: StPipeline.cpp StPipeline.hpp ActiveObject.hpp PrimeCheck.hpp Queue.hpp
	$(CXX) $(CXXFLAGS) -c StPipeline.cpp -o StPipeline.o

clean:
	rm -f $(OBJS) $(TARGET)
