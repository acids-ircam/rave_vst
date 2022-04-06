# Rave-vst
AU / VST / Standalone plugins for real-time RAVE manipulation

### Building the C++ project
Use cmake to build.  
Tested with cmake 3.21.3, clang 11.0.3, Xcode 11.7 on MacOS 10.15.7  
Tested with cmake 3.19.7, gcc 10.3.1 on Fedora 33 with Kernel 5.14.18  
This will download the PyTorch libraries (And MKL if you're using UNIX)  

- `git submodule update --init --recursive` to get juce
- `mkdir build; cd build`
- `cmake .. -DCMAKE_BUILD_TYPE=Release`
- `cmake --build . --config Release -j 4` or `cmake -G Xcode -B build`
