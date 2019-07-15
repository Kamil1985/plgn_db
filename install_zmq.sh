apt-get install cmake
git clone https://github.com/zeromq/libzmq
cd libzmq
mkdir cmake-build && cd cmake-build
cmake .. && make -j 4
make test && make install && sudo ldconfig
cd ..
apt-get install libtool
apt-get install pkg-config
git clone git://github.com/zeromq/czmq.git
cd czmq
./autogen.sh && ./configure && make 
sudo make install
sudo ldconfig
cd ..
