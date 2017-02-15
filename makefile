all: sample2D

sample2D: Sample_GL3_2D.cpp
	g++ -std=c++11 -g -o sample2D Sample_GL3_2D.cpp -lpthread -lglfw -lGLEW -lGL -ldl -lao -lmpg123

clean:
	rm sample2D
