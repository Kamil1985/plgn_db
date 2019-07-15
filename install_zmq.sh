apt-get install cmake
git clone https://github.com/zeromq/libzmq
cd libzmq
mkdir cmake-build && cd cmake-build
cmake .. && make -j 4
make test && make install && sudo ldconfig
make test
cd ..
apt-get install libtools
apt-get install pkg-config
git clone git://github.com/zeromq/czmq.git
cd czmq
./autogen.sh && ./configure && make check
sudo make install
sudo ldconfig
cd ..
