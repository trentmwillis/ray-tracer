CC=gcc

main: main.c
	$(CC) -framework GLUT -framework OpenGL main.c -o main
