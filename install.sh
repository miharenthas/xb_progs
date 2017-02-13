#tiny shell script that installs the libraries and the programs

#core
if [ -L /usr/local/lib/libxb_core.so ]; then
	echo "Library \"libxb_core\" already installed."
else
	ln -s $PWD/lib/libxb_core.so /usr/local/lib/
	ln -s $PWD/lib/libxb_core.so /usr/lib/
	ln -s $PWD/lib/libxb_core.so /usr/lib64/
fi

#visualizations
if [ -L /usr/local/lib/libxb_viz.so ]; then
	echo "Library \"libxb_viz\" already installed."
else
	ln -s $PWD/lib/libxb_viz.so /usr/local/lib/
	ln -s $PWD/lib/libxb_viz.so /usr/lib/
	ln -s $PWD/lib/libxb_viz.so /usr/lib64/
fi

#root
if [ -L /usr/local/lib/libxb_root.so ]; then
	echo "Library \"libxb_root\" already installed."
else
	ln -s $PWD/lib/libxb_root.so /usr/local/lib/
	ln -s $PWD/lib/libxb_root.so /usr/lib/
	ln -s $PWD/lib/libxb_root.so /usr/lib64/
fi

#programs
if [ -L /usr/local/bin/xb_data_translator ]; then
	echo "Program \"xb_data_translator\" already installed."
else
	ln -s $PWD/xb_data_translator /usr/local/bin/
fi

if [ -L /usr/local/bin/xb_run_cluster ]; then
	echo "Program \"xb_run_cluster\" already installed."
else
	ln -s $PWD/xb_run_cluster /usr/local/bin/
fi

if [ -L /usr/local/bin/xb_doppc ]; then
	echo "Program \"xb_doppc\" already installed."
else
	ln -s $PWD/xb_doppc /usr/local/bin/
fi

if [ -L /usr/local/bin/xb_make_spc ]; then
	echo "Program \"xb_make_spc\" already installed."
else
	ln -s $PWD/xb_make_spc /usr/local/bin/
fi

if [ -L /usr/local/bin/xb_do_cut ]; then
	echo "Program \"xb_do_cut\" already installed."
else
	ln -s $PWD/xb_do_cut /usr/local/bin/
fi
