#this makefile instructs the build of the octave interface for xb_progs

#-------------------------------------------------------------------------------------
#define some variables that can be handy here:
#environment
SRC = ./src
OCT = ./oct

#targets, flags and compiler
OBJECTS = xb_load_data xb_write_data xb_load_track_info xb_write_track_info
OCTFILES = $(OCT)/xb_load_data.oct $(OCT)/xb_write_data.oct $(OCT)/xb_load_track_info.oct $(OCT)/xb_write_track_info.oct

OCTFLAGS = -lxb_core -I"../include/"

C_OCT = mkoctfile

#-------------------------------------------------------------------------------------
#recipes
xb_load_data :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_load_data.cc -o $(OCT)/xb_load_data.oct
	rm xb_load_data.o

xb_write_data :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_write_data.cc -o $(OCT)/xb_write_data.oct
	rm xb_write_data.o

xb_load_track_info :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_load_track_info.cc -o $(OCT)/xb_load_track_info.oct
	rm xb_load_track_info.o

xb_write_track_info :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_write_track_info.cc -o $(OCT)/xb_write_track_info.oct
	rm xb_write_track_info.o

#-------------------------------------------------------------------------------------
#phony targets

.PHONY: all
all : $(OBJECTS)

.PHONY : clean
clean :
	rm -f $(OCTFILES)


