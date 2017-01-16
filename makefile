EXE = ubx_to_json.exe

all: dirtree
	$(CXX) -std=c++11 -o bin/$(EXE) -Isrc/jsoncons/src src/ubx_to_json.cpp

dirtree:
	@mkdir -p bin
