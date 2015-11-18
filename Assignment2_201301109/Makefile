CC = g++
PROG = game

SRCS = lattu.cpp imageloader.cpp vec3f.cpp

LIBS = -lglut -lGL -lGLEW -lm

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)
