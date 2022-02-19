#OCVHOME		:= /home/asm/OCV346
OCVHOME		:= /usr/local
OCVVERS		:= 3.4.6
LIBRARIES	:= $(OCVHOME)/lib/libopencv_world.so.$(OCVVERS) -lianpr1.8 -lianprinterface1.8

CC              := g++ -g3 -std=c++11
CFLAGS          := -I$(OCVHOME)/include -L/usr/local/lib
OBJECTS         := 


.PHONY: all clean

all: sample

sample: 
	$(CC)  $(CFLAGS) -c *.cpp
	$(CC)  $(CFLAGS) *.o $(LIBRARIES) -o image_new
        
clean:
	rm -f *.o
	rm -f image_new


