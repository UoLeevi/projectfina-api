rm -r build
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/projectfina-api -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles"
cmake --build . --target install
