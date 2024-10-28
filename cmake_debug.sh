mkdir build
pushd "$PWD"
cd build
cmake .. -DCMAKE_BUILD_TYPE=DEBUG \
    -DCMAKE_VERBOSE_MAKEFILE=TRUE \
    -DVITA=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DSFV_FOLDER=/mnt/d/downloads/psvita/unity/sfv \
    -DMCS_PATH="'/mnt/c/Program Files (x86)/Mono-2.11.4/lib/mono/2.0'" \
    -DMONO_PATH=/mnt/d/downloads/psvita/unity/sfv/Tools/MonoPSP2
popd