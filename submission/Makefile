build: run_file

run: run_file
	./run_file $(file)
run_file: main.o lodepng.o rasterizer.o
	g++ main.o lodepng.o rasterizer.o -o run_file
rasterizer.o:	rasterizer.cpp
	g++ -c -std=c++11 rasterizer.cpp -o rasterizer.o
lodepng.o: lodepng.cpp
	g++ -c lodepng.cpp -I./ -ansi -pedantic -Wall -Wextra -O3 -o lodepng.o
main.o: main.cpp
	g++ -c main.cpp -o main.o
clean:
	rm main.o;
	rm lodepng.o;
	rm rasterizer.o;
	rm run_file;
run_require: run_file
	./run_file hw1indexing.txt
run_require_1: run_file
	./run_file hw1req1.txt
run_require_2: run_file
	./run_file hw1req2.txt
run_nodepth: run_file
	./run_file hw1nodepth.txt
run_depth: run_file
	./run_file hw1depth.txt