#this makefile instructs the build of the octave interface for xb_progs

#-------------------------------------------------------------------------------------
#define some variables that can be handy here:
#environment
SRC = ./src
OCT = ./oct

#targets, flags and compiler
OBJECTS = xb_load_data.o xb_write_data.o xb_load_track_info.o xb_write_track_info.o xb_load_clusterZ.o xb_load_klz_field.o xb_write_clusterZ.o xb_ball_neigh.o xb_ball_at.o xb_ball_some_crystals.o
OCTFILES = $(OCT)/xb_load_data.oct $(OCT)/xb_write_data.oct $(OCT)/xb_load_track_info.oct $(OCT)/xb_write_track_info.oct $(OCT)/xb_load_clusterZ.oct $(OCT)/xb_write_clusterZ.oct $(OCT)/xb_ball_neigh.oct $(OCT)/xb_ball_at.oct

OCTFLAGS = -lxb_core -I"../include/" -I"./include/"

C_OCT = mkoctfile

#-------------------------------------------------------------------------------------
#recipes:

#-------------------------------------------------------------------------------------
#utils
xb_ball_some_crystals.o :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_ball_some_crystals.cc -c

#-------------------------------------------------------------------------------------
#data
xb_load_data.o :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_load_data.cc -o $(OCT)/xb_load_data.oct

xb_write_data.o :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_write_data.cc -o $(OCT)/xb_write_data.oct

#-------------------------------------------------------------------------------------
#track info
xb_load_track_info.o :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_load_track_info.cc -o $(OCT)/xb_load_track_info.oct

xb_write_track_info.o :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_write_track_info.cc -o $(OCT)/xb_write_track_info.oct

#-------------------------------------------------------------------------------------
#clusters
xb_load_clusterZ.o :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_load_clusterZ.cc -o $(OCT)/xb_load_clusterZ.oct

xb_load_klz_field.o :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_load_klz_field.cc -o $(OCT)/xb_load_klz_field.oct

xb_write_clusterZ.o :
	$(C_OCT) $(OCTFLAGS) $(SRC)/xb_write_clusterZ.cc -o $(OCT)/xb_write_clusterZ.oct

#-------------------------------------------------------------------------------------
#ball access
xb_ball_neigh.o : xb_ball_some_crystals.o
	$(C_OCT) $(OCTFLAGS) xb_ball_some_crystals.o $(SRC)/xb_ball_neigh.cc -o $(OCT)/xb_ball_neigh.oct

xb_ball_at.o : xb_ball_some_crystals.o
	$(C_OCT) $(OCTFLAGS) xb_ball_some_crystals.o $(SRC)/xb_ball_at.cc -o $(OCT)/xb_ball_at.oct

#-------------------------------------------------------------------------------------
#phony targets

.PHONY: all
all : $(OBJECTS)
	rm $(OBJECTS)

.PHONY : clean
clean :
	rm -f $(OCTFILES) $(OBJECTS)


