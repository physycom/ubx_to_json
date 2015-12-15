EXE = ubx2json.exe

all:
	$(CXX) -std=c++11 -o $(EXE) -I. ubx_to_json.cpp -fpermissive
