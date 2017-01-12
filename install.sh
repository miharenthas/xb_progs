#tiny shell script that installs the libraries

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
