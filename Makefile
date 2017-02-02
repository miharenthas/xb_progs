#this file is used to make the various programs in XB_PROGS

#define some environment variables that can be handy here
PROGS_HOME = $(PWD)
SRC = $(PROGS_HOME)/src
INCLUDE = $(PROGS_HOME)/include
BIN = $(PROGS_HOME)/bin
LIB = $(PROGS_HOME)/lib
TEST = $(PROGS_HOME)/test
GNUPLOT_I_HOME = /usr/local/gnuplot_i

#define the targets
PROGRAMS = xb_data_translator xb_run_cluster xb_make_spc xb_doppc xb_do_cut
TESTS = xb_check_nn  xb_view_ball xb_view_cluster xb_energy_list xb_try_nn_cluster xb_try_nn_clusterZ xb_try_kmeans_cluster test_xb_cuts xb_draw_cutZ xb_try_parse xb_try_sim_reader
OBJECTS = xb_error xb_data xb_io xb_ball xb_cluster xb_doppler_corr xb_cut_typedefs xb_cut xb_apply_cut xb_parse_cnf_file
GNUPLOT_OBJS = xb_draw_cluster_ball xb_draw_cut xb_draw_gsl_histogram
OBJ_W_ROOT = xb_reader
LIBRARIES = libxb_core libxb_viz libxb_root
BINARIES = $(BIN)/xb_cluster.o $(BIN)/xb_error.o $(BIN)/xb_data.o $(BIN)/xb_io.o $(BIN)/xb_ball.o $(BIN)/xb_doppler_corr.o $(BIN)/xb_cut_typedefs.o $(BIN)/xb_cut.o $(BIN)/xb_apply_cut.o $(BIN)/xb_parse_cnf_file.o
GNUPLOT_BINS = $(BIN)/xb_draw_cluster_ball.o $(BIN)/xb_draw_cut.o $(BIN)/xb_draw_gsl_histogram.o
ROOT_BINS = $(BIN)/xb_reader.o
GNUPLOT_I = $(GNUPLOT_I_HOME)/gnuplot_i.o

#compiler and flags
CXX = g++
CXXFLAGS = -I$(INCLUDE) -L$(LIB) -fopenmp -Wno-write-strings -lgsl -lgslcblas -lm -lCGAL -lCGAL_Core -lgmp
ROOT_CXXFLAGS = `root-config --cflags`
ROOT_CXXFLAGS += -I $(FAIRROOTPATH)/include
FAIR_LIBS = -lBaseMQ -lBase -lEventDisplay -lfairmq_logger -lFairMQ -lFairMQTest -lFairRutherford -lFairTestDetector -lFairTools -lGeane -lGen -lGeoBase -lLmdMQSampler -lMbsAPI -lMCStack -lParBase -lParMQ -lPassive -lPixel -lTrkBase
R3B_LIBS = -lELILuMon -lField -lR3Bbase -lR3BCalo -lR3BData -lR3BDch -lR3BdTof -lR3BEvtVis -lR3BFi4 -lR3BGen -lR3BGfi -lR3BLand -lR3BLos -lR3BMfi -lR3BmTof -lR3BNeuland -lR3BPassive -lR3BPlist -lR3BPsp -lR3BSTaRTra -lR3BTCal -lR3BTof -lR3BTra -lR3BXBall -ltimestitcher
ROOT_LDFLAGS = `root-config --glibs` -L $(FAIRROOTPATH)/lib
ROOT_FLAGS = $(ROOT_LDFLAGS) $(ROOT_CXXFLAGS) $(FAIR_LIBS) $(R3B_LIBS)
GNUPLOT_FLAGS = -I$(GNUPLOT_I_HOME)/src

#recipes
#----------------------------------------------------------------------
#objects
xb_error :
	$(CXX) $(SRC)/xb_error.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_error.o

xb_data : 
	$(CXX) $(SRC)/xb_data.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_data.o

xb_io : 
	$(CXX) $(SRC)/xb_io.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_io.o

xb_ball :
	$(CXX) $(SRC)/xb_ball.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_ball.o

xb_reader: 
	$(CXX) $(SRC)/xb_reader.cc $(CXXFLAGS) $(ROOT_CXXFLAGS) -fPIC -c -o $(BIN)/xb_reader.o

xb_cluster:
	$(CXX) $(SRC)/xb_cluster.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_cluster.o

xb_doppler_corr:
	$(CXX) $(SRC)/xb_doppler_corr.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_doppler_corr.o

xb_draw_cluster_ball:
	$(CXX) $(SRC)/xb_draw_cluster_ball.cc $(CXXFLAGS) $(GNUPLOT_FLAGS) -fPIC -c -o $(BIN)/xb_draw_cluster_ball.o

xb_cut_typedefs:
	$(CXX) $(SRC)/xb_cut_typedefs.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_cut_typedefs.o

xb_cut :
	$(CXX) $(SRC)/xb_cut.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_cut.o

xb_draw_cut :
	$(CXX) $(SRC)/xb_draw_cut.cc $(CXXFLAGS) $(GNUPLOT_FLAGS) -fPIC -c -o $(BIN)/xb_draw_cut.o

xb_apply_cut :
	$(CXX) $(SRC)/xb_apply_cut.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_apply_cut.o

xb_parse_cnf_file :
	$(CXX) $(SRC)/xb_parse_cnf_file.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_parse_cnf_file.o

xb_draw_gsl_histogram :
	$(CXX) $(SRC)/xb_draw_gsl_histogram.cc $(CXXFLAGS) $(GNUPLOT_FLAGS) -fPIC -c -o $(BIN)/xb_draw_gsl_histogram.o

#----------------------------------------------------------------------
#libraries
libxb_core : $(OBJECTS)
	$(CXX) $(BINARIES) $(CXXFLAGS) -shared -o $(LIB)/libxb_core.so

libxb_viz : $(GNUPLOT_OBJS)
	$(CXX) $(GNUPLOT_BINS) $(GNUPLOT_I) -shared -o $(LIB)/libxb_viz.so

libxb_root : $(OBJ_W_ROOT)
	$(CXX) $(ROOT_BINS) -shared -o $(LIB)/libxb_root.so

#----------------------------------------------------------------------
#programs
xb_data_translator : libxb_core libxb_root
	$(CXX) -lxb_core -lxb_root $(SRC)/xb_data_translator.cpp $(CXXFLAGS) $(ROOT_FLAGS) -o xb_data_translator

xb_run_cluster : libxb_core libxb_viz
	$(CXX) -lxb_core -lxb_viz $(SRC)/xb_run_cluster.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o xb_run_cluster

xb_make_spc : libxb_core libxb_viz
	$(CXX) -lxb_core -lxb_viz $(SRC)/xb_make_spc.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o xb_make_spc

xb_doppc : libxb_core
	$(CXX) $(SRC)/xb_doppc.cc $(CXXFLAGS) -lxb_core -c -o $(BIN)/xb_doppc.o
	$(CXX) -lxb_core $(BIN)/xb_doppc.o $(SRC)/xb_doppc.cpp $(CXXFLAGS) -o xb_doppc

xb_do_cut : libxb_core libxb_viz
	$(CXX) -lxb_core -lxb_viz $(SRC)/xb_do_cut.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o xb_do_cut

#----------------------------------------------------------------------
#test programs
xb_check_nn: xb_ball xb_error
	$(CXX) $(BIN)/xb_ball.o $(BIN)/xb_error.o $(TEST)/xb_check_nn.cpp $(CXXFLAGS) -o $(TEST)/xb_check_nn

xb_view_ball: xb_ball xb_error $(GNUPLOT_I)
	$(CXX) $(BIN)/xb_ball.o $(BIN)/xb_error.o $(TEST)/xb_view_ball.cpp $(GNUPLOT_I) $(CXXFLAGS) $(GNUPLOT_FLAGS) -o $(TEST)/xb_view_ball

xb_view_cluster: $(OBJECTS) $(GNUPLOT_OBJS) $(GNUPLOT_I)
	$(CXX) $(BINARIES) $(GNUPLOT_BINS) $(GNUPLOT_I) $(TEST)/xb_view_cluster.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o $(TEST)/xb_view_cluster

xb_energy_list: $(OBJECTS)
	$(CXX) $(BINARIES) $(TEST)/xb_energy_list.cpp $(CXXFLAGS) -o $(TEST)/xb_energy_list

xb_try_nn_cluster: $(OBJECTS) $(GNUPLOT_OBJS) $(GNUPLOT_I)
	$(CXX) $(BINARIES) $(GNUPLOT_BINS) $(GNUPLOT_I) $(TEST)/xb_try_nn_cluster.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o $(TEST)/xb_try_nn_cluster

xb_try_nn_clusterZ: $(OBJECTS) $(GNUPLOT_OBJS) $(GNUPLOT_I)
	$(CXX) $(BINARIES) $(GNUPLOT_BINS) $(GNUPLOT_I) $(TEST)/xb_try_nn_clusterZ.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o $(TEST)/xb_try_nn_clusterZ

xb_try_kmeans_cluster: $(OBJECTS) $(GNUPLOT_OBJS) $(GNUPLOT_I)
	$(CXX) $(BINARIES) $(GNUPLOT_BINS) $(GNUPLOT_I) $(TEST)/xb_try_kmeans_cluster.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o $(TEST)/xb_try_kmenans_cluster

test_xb_cuts: $(OBJECTS)
	$(CXX) $(BINARIES) $(TEST)/test_xb_cuts.cpp $(CXXFLAGS) -o $(TEST)/test_xb_cuts

xb_draw_cutZ: $(OBJECTS) $(GNUPLOT_OBJS) $(GNUPLOT_I)
	$(CXX) $(BINARIES) $(GNUPLOT_BINS) $(GNUPLOT_I) $(TEST)/xb_draw_cutZ.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o $(TEST)/xb_draw_cutZ

xb_try_parse: $(OBJECTS) $(GNUPLOT_OBJS) $(GNUPLOT_I)
	$(CXX) $(BINARIES) $(GNUPLOT_BINS) $(GNUPLOT_I) $(TEST)/xb_try_parse.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o $(TEST)/xb_try_parse

xb_try_sim_reader: $(OBJECTS) $(GNUPLOT_OBJS) $(GNUPLOT_I) $(OBJ_W_ROOT)
	$(CXX) $(ROOT_BINS) $(BINARIES) $(GNUPLOT_BINS) $(GNUPLOT_I) $(TEST)/xb_try_sim_reader.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) $(ROOT_FLAGS) -o $(TEST)/xb_try_sim_reader

#-----------------------------------------------------------------------
#collective operations
.PHONY: all
all: $(OBJECTS) $(LIBRARIES) $(PROGRAMS)

.PHONY: libs
libs: $(LIBRARIES)

.PHONY: test
test: $(OBJECTS) $(TESTS)

.PHONY: install
install :
	./install.sh

.PHONY: uninstall
uninstall :
	rm -f /usr/local/lib/libxb_core.so
	rm -f /usr/local/lib/libxb_viz.so
	rm -f /usr/local/lib/libxb_root.so
	rm -f /usr/lib/libxb_core.so
	rm -f /usr/lib/libxb_viz.so
	rm -f /usr/lib/libxb_root.so
	rm -f /usr/lib64/libxb_core.so
	rm -f /usr/lib64/libxb_viz.so
	rm -f /usr/lib64/libxb_root.so

.PHONY: clean
clean:
	rm -f $(BIN)/* $(PROGRAMS) $(TESTS) $(LIB)/*

