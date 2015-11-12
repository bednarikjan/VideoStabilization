# Authors: Jan Bedarik, Jakub Kvita
# Date: 14.4.2014

CC=g++
CFLAGS=-Wall -I. -I/usr/local/include -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_video
OBJFILES=main.o xgetopt.o Parameters.o StabilizerGCBPM.o StabilizerSVD.o

all: dis

dis: $(OBJFILES)
	$(CC) $(CFLAGS) $^ -o $@
	
main.o: main.cpp Stabilizer.h StabilizerGCBPM.h Parameters.h Exceptions.h
	$(CC) $(CFLAGS) -c $< -o $@
	
xgetopt.o: xgetopt.c xgetopt.h
	$(CC) $(CFLAGS) -c $< -o $@
	
Parameters.o: Parameters.cpp Parameters.h Stabilizer.h StabilizerGCBPM.h Exceptions.h xgetopt.h
	$(CC) $(CFLAGS) -c $< -o $@

StabilizerGCBPM.o: StabilizerGCBPM.cpp StabilizerGCBPM.h StabilizerGCBPM.h Exceptions.h
	$(CC) $(CFLAGS) -c $< -o $@

StabilizerSVD.o: StabilizerSVD.cpp StabilizerSVD.h Stabilizer.h Exceptions.h
	$(CC) $(CFLAGS) -c $< -o $@	

.PHONY: clean	
    
clean:
	rm -rf *o dis
