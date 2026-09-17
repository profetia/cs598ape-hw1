FUNC := icpx
copt := -c 
OBJ_DIR := ./bin/
FLAGS := -O3 -march=native -flto -lm -g -Werror -qopenmp -std=c++20 -mprefer-vector-width=512

CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix $(OBJ_DIR),$(notdir $(CPP_FILES:.cpp=.obj)))

TEXTURE_CPP_FILES := $(wildcard src/Textures/*.cpp)
TEXTURE_OBJ_FILES := $(addprefix $(OBJ_DIR)Textures/,$(notdir $(TEXTURE_CPP_FILES:.cpp=.obj)))

all:
	cd ./src && make
	$(FUNC) ./main.cpp -o ./main.exe ./src/*.obj ./src/Textures/*.obj $(FLAGS)

clean:
	cd ./src && make clean
	rm -f ./*.exe
	rm -f ./*.obj

PERF := perf record -F 999 --call-graph dwarf -o

perf-pianoroom: all
	$(PERF) perf-pianoroom.data ./main.exe -i inputs/pianoroom.ray --ppm -o output/perf-pianoroom.ppm -H 500 -W 500

perf-globe: all
	$(PERF) perf-globe.data ./main.exe -i inputs/globe.ray --ppm --no-movie -a inputs/globe.animate -F 24 -o output/perf-globe

perf-sphere: all
	$(PERF) perf-sphere.data ./main.exe -i inputs/elephant.ray --ppm --no-movie -a inputs/elephant.animate -F 24 -W 100 -H 100 -o output/perf-sphere

perf-elephant: all
	sed 's|data/x.txt 1586 data/f.txt 3168|data/elepx.txt 62779 data/elepf.txt 111748|' inputs/elephant.ray > inputs/bench-elephant.ray
	$(PERF) perf-elephant.data ./main.exe -i inputs/bench-elephant.ray --ppm --no-movie -a inputs/elephant.animate -F 24 -W 100 -H 100 -o output/perf-elephant
