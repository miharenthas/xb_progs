#this file is used to make the various programs in XB_PROGS

#define some environment variables that can be handy here
PROGS_HOME = $(PWD)
SRC = $(PROGS_HOME)/src
INCLUDE = $(PROGS_HOME)/include
BIN = $(PROGS_HOME)/bin
LIB = $(PROGS_HOME)/lib
TEST = $(PROGS_HOME)/test
GNUPLOT_I_HOME = /home/ahorvat/R3B/gnuplot_i
CGAL_HOME = /home/ahorvat/ext/cgal

#define the targets
PROGRAMS = xb_data_translator xb_run_cluster xb_make_spc xb_doppc xb_do_cut xb_trigger_siv xb_getarb xb_match xb_converter
TESTS = xb_check_nn  xb_view_ball xb_view_cluster xb_energy_list xb_try_nn_cluster xb_try_nn_clusterZ xb_try_kmeans_cluster test_xb_cuts xb_draw_cutZ xb_try_parse xb_try_sim_reader xb_test_adata
OBJECTS = xb_error xb_data xb_io xb_ball xb_cluster xb_doppler_corr xb_cut_typedefs xb_cut xb_apply_cut xb_parse_cnf_file xb_tpat xb_adata
GNUPLOT_OBJS = xb_draw_cluster_ball xb_draw_cut xb_draw_gsl_histogram xb_make_spc__cmd_line
OBJ_W_ROOT = xb_reader xb_root_writer
LIBRARIES = libxb_core libxb_viz libxb_root
BINARIES = $(BIN)/xb_cluster.o $(BIN)/xb_error.o $(BIN)/xb_data.o $(BIN)/xb_io.o $(BIN)/xb_ball.o $(BIN)/xb_doppler_corr.o $(BIN)/xb_cut_typedefs.o $(BIN)/xb_cut.o $(BIN)/xb_apply_cut.o $(BIN)/xb_parse_cnf_file.o $(BIN)/xb_tpat.o $(BIN)/xb_adata.o
GNUPLOT_BINS = $(BIN)/xb_draw_cluster_ball.o $(BIN)/xb_draw_cut.o $(BIN)/xb_draw_gsl_histogram.o $(BIN)/xb_make_spc__cmd_line.o
ROOT_BINS = $(BIN)/xb_reader.o $(BIN)/xb_root_writer.o
GNUPLOT_I = $(GNUPLOT_I_HOME)/gnuplot_i.o

#compiler and flags
CXX = g++
CXXFLAGS = -I$(INCLUDE) -I$(CGAL_HOME)/include -I$(VMCWORKDIR)/r3bdata/xballData/ -I$(VMCWORKDIR)/bodgelogger  -L$(LIB) -L$(CGAL_HOME)/lib64 -L$(FAIRLIBDIR) -fopenmp -Wno-write-strings -lgsl -lgslcblas -lm -lCGAL -lgmp -std=c++11 -ggdb -frounding-math
ROOT_CXXFLAGS = `root-config --cflags | sed 's/-stdlib=libc++//g'`
ROOT_CXXFLAGS += -I $(FAIRROOTPATH)/include
FAIR_LIBS = -lFairTools -lParBase -lBase
R3B_LIBS = -lELILuMon -lField -lR3Bbase -lR3BCalifa -lR3BData -lR3BDch -lR3BdTof -lR3BEvtVis -lR3BFi4 -lR3BGen -lR3BGfi -lR3BLand -lR3BLos -lR3BMfi -lR3BmTof -lR3BNeuland -lR3BPassive -lR3BPsp -lR3BStartrack -lR3BTCal -lR3BTof -lR3BTra -lR3BXBall -ltimestitcher
ROOT_LDFLAGS = `root-config --glibs | sed 's/-stdlib=libc++//g'` -L $(FAIRROOTPATH)/lib
ROOT_FLAGS = $(ROOT_LDFLAGS) $(ROOT_CXXFLAGS) $(FAIR_LIBS) $(R3B_LIBS)
GNUPLOT_FLAGS = -I$(GNUPLOT_I_HOME)/src

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
	rm -rf $(BIN)/* $(PROGRAMS) $(TESTS) $(LIB)/*

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

xb_root_writer: 
	$(CXX) $(SRC)/xb_root_writer.cc $(CXXFLAGS) $(ROOT_CXXFLAGS) -fPIC -c -o $(BIN)/xb_root_writer.o

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

xb_make_spc__cmd_line :
	$(CXX) $(SRC)/xb_make_spc/cmd_line.cc $(CXXFLAGS) $(GNUPLOT_FLAGS) -fPIC -c -o $(BIN)/xb_make_spc__cmd_line.o

xb_tpat :
	$(CXX) $(SRC)/xb_tpat.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_tpat.o

xb_adata :
	$(CXX) $(SRC)/xb_arbitrary_data.cc $(CXXFLAGS) -fPIC -c -o $(BIN)/xb_adata.o


#----------------------------------------------------------------------
#libraries
libxb_core : $(OBJECTS)
	$(CXX) $(BINARIES) $(CXXFLAGS) -shared -o $(LIB)/libxb_core.so

libxb_viz : libxb_core $(GNUPLOT_OBJS)
	$(CXX) $(GNUPLOT_BINS) $(CXXFLAGS) $(GNUPLOT_I) $(GNUPLOT_FLAGS) -L$(LIB) -lxb_core  -shared -o $(LIB)/libxb_viz.so

libxb_root : libxb_core $(OBJ_W_ROOT)
	$(CXX) $(ROOT_BINS) $(CXXFLAGS) $(ROOT_FLAGS) -L$(LIB) -lxb_core -shared -o $(LIB)/libxb_root.so

#----------------------------------------------------------------------
#programs
xb_data_translator : libxb_core libxb_root
	$(CXX) -lxb_core -lxb_root $(SRC)/xb_data_translator.cpp $(CXXFLAGS) $(ROOT_FLAGS) -o xb_data_translator

xb_converter : libxb_core libxb_root
	$(CXX) -lxb_core -lxb_root $(SRC)/xb_converter.cpp $(CXXFLAGS) $(ROOT_FLAGS) -o xb_converter
	
xb_getarb : libxb_core libxb_root
	$(CXX) -lxb_core -lxb_root $(SRC)/xb_getarb.cpp $(CXXFLAGS) $(ROOT_FLAGS) -o xb_getarb

xb_run_cluster : libxb_core libxb_viz
	$(CXX) -lxb_core -lxb_viz $(SRC)/xb_run_cluster.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o xb_run_cluster

xb_make_spc : libxb_core libxb_viz
	$(CXX) -lxb_core -lxb_viz $(SRC)/xb_make_spc.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o xb_make_spc

xb_doppc : libxb_core
	$(CXX) $(SRC)/xb_doppc.cc $(CXXFLAGS) -lxb_core -c -o $(BIN)/xb_doppc.o
	$(CXX) -lxb_core $(BIN)/xb_doppc.o $(SRC)/xb_doppc.cpp $(CXXFLAGS) -o xb_doppc

xb_do_cut : libxb_core libxb_viz
	$(CXX) -lxb_core -lxb_viz $(SRC)/xb_do_cut.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o xb_do_cut

xb_trigger_siv : libxb_core
	$(CXX) -lxb_core $(SRC)/xb_trigger_siv.cpp $(CXXFLAGS) -o xb_trigger_siv
xb_match : libxb_core
	$(CXX) -lxb_core $(SRC)/xb_match.cpp $(CXXFLAGS) -o xb_match

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

xb_cml: libxb_core libxb_viz
	$(CXX) -lxb_viz -lxb_core $(TEST)/xb_cml.cpp $(CXXFLAGS) $(GNUPLOT_FLAGS) -o $(TEST)/xb_cml

xb_test_adata : libxb_core libxb_root
	$(CXX) -lxb_core -lxb_root $(TEST)/xb_test_adata.cpp $(CXXFLAGS) $(ROOT_FLAGS) -o $(TEST)/xb_test_adata
