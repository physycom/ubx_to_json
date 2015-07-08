EXE = ubx2json.exe

all:
	g++ -std=c++11 -o $(EXE) -I. ubx_to_json.cpp -fpermissive
