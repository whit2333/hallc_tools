#!/bin/bash

prefix=$1

mkdir -p scratch && cd scratch

git clone https://github.com/nlohmann/json.git
mkdir -p build_json  
pushd build_json
cmake ../json/. -DCMAKE_INSTALL_PREFIX=${prefix} \
   -DBUILD_TESTING=OFF
make -j4 install
popd 
rm -r build_json

git clone https://github.com/fmtlib/fmt.git
mkdir -p build_fmt
pushd build_fmt
cmake ../fmt/. -DCMAKE_INSTALL_PREFIX=${prefix} \
   -DFMT_TEST=OFF  -DFMT_DOC=OFF -DBUILD_SHARED_LIBS=ON
make -j4 install
popd 
rm -r build_fmt

git clone https://github.com/HowardHinnant/date.git
mkdir -p build_date
pushd build_date
cmake ../date/. -DCMAKE_INSTALL_PREFIX=${prefix} \
  -DENABLE_DATE_TESTING=OFF -DBUILD_SHARED_LIBS=ON -DCURL_LIBRARY=$(curl-config --prefix)/lib -DCURL_INCLUDE_DIR=$(curl-config --prefix)/include
make -j4 install
popd 
rm -r build_date

git clone https://github.com/ericniebler/range-v3.git
mkdir -p build_range
pushd build_range
cmake ../range-v3/. -DCMAKE_INSTALL_PREFIX=${prefix} \
   -DRANGE_V3_PERF=OFF -DRANGE_BUILD_CALENDAR_EXAMPLE=OFF -DBUILD_TESTING=OFF -DRANGE_V3_EXAMPLES=OFF -DRANGE_V3_TESTS=OFF -DRANGE_V3_DOCS=OFF -DRANGES_CXX_STD=17
make -j4 install
popd 
rm -r build_range

wget http://bitbucket.org/eigen/eigen/get/3.3.5.tar.gz -O - | tar -xzf -
mv eigen-eigen* eigen
mkdir -p build_eigen
pushd build_eigen
cmake ../eigen/. -DCMAKE_INSTALL_PREFIX=${prefix} \
  -DBUILD_TESTING=OFF
make -j4 install
pushd ${prefix}/include
ln -s eigen3/Eigen
ls
popd
popd
rm -r build_eigen

# now comes with analzyer
#git clone  https://github.com/gabime/spdlog.git
#pushd spdlog/include
#git checkout tags/v1.3.0
#cp -r spdlog ${prefix}/include/.
#popd 

