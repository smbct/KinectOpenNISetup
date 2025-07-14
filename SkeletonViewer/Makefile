
SkeletonViewer: main.o Scene.o Skeleton.o
	g++ main.o Scene.o Skeleton.o -lOpenNI -lGL -lGLU -lglut -o SkeletonViewer

Scene.o: Scene.cpp
	g++ Scene.cpp -I /usr/include/ -I /usr/include/ni -c -o Scene.o

Skeleton.o: Skeleton.cpp
	g++ Skeleton.cpp -I /usr/include/ -I /usr/include/ni -c -o Skeleton.o

main.o: main.cpp
	g++ main.cpp -I /usr/include/ni -I /usr/include/ -c -o main.o

clean:
	rm -f *.o SkeletonViewer