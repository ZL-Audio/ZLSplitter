git submodule update --init --recursive

rm -rf Builds
cmake -B Builds -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DKFR_ENABLE_MULTIARCH=OFF \
    -DKFR_ENABLE_DFT=ON \
    -DZL_JUCE_FORMATS="VST3;Standalone" .
cmake --build Builds --config Release -j 24