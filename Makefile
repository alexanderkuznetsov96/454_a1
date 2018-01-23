LDFLAGS = -L. -lglfw -lGL -ldl # -lpthread
CXXFLAGS = -g -Wall -Wno-write-strings -Wno-parentheses -DLINUX #-pthread

OBJS = linalg.o ll.o world.o lander.o landscape.o gpuProgram.o strokefont.o fg_stroke.o glad/src/glad.o 
EXEC = ll

all:    $(EXEC)

ll:	$(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS)  $(LDFLAGS) 

clean:
	rm -f  *~ $(EXEC) $(OBJS)

depend:	
	makedepend -Y *.h *.cpp 2> /dev/null

# DO NOT DELETE

gpuProgram.o: headers.h glad/include/glad/glad.h linalg.h
headers.o: glad/include/glad/glad.h linalg.h
lander.o: headers.h glad/include/glad/glad.h linalg.h
landscape.o: headers.h glad/include/glad/glad.h linalg.h
strokefont.o: headers.h glad/include/glad/glad.h linalg.h
world.o: headers.h glad/include/glad/glad.h linalg.h landscape.h lander.h
world.o: ll.h
fg_stroke.o: strokefont.h headers.h glad/include/glad/glad.h linalg.h
gpuProgram.o: gpuProgram.h headers.h glad/include/glad/glad.h linalg.h
lander.o: headers.h glad/include/glad/glad.h linalg.h lander.h world.h
lander.o: landscape.h ll.h gpuProgram.h
landscape.o: headers.h glad/include/glad/glad.h linalg.h landscape.h
landscape.o: gpuProgram.h ll.h
linalg.o: linalg.h
ll.o: headers.h glad/include/glad/glad.h linalg.h gpuProgram.h world.h
ll.o: landscape.h lander.h ll.h
strokefont.o: strokefont.h headers.h glad/include/glad/glad.h linalg.h
strokefont.o: fg_stroke.h
world.o: world.h headers.h glad/include/glad/glad.h linalg.h landscape.h
world.o: lander.h ll.h gpuProgram.h strokefont.h
