CC = g++
CFLAGS = -Wall
PROG = playcarrom

SRCS = carrom.cpp 

ifeq ($(shell uname),Darwin)
	LIBS = -framework OpenGL -framework GLUT
else
	LIBS = -lglut -lGL -lGLU -lm
endif

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)
