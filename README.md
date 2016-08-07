# HBST: Hamming Binary Search Tree Header-only library

Code last updated: 2016-05-16 <br/>
Contributors: https://github.com/schdomin, https://github.com/grisetti <br/>
<br/>

Supported platforms: <br/>
- UNIX x86/x64 <br/>
- Windows x86/x64 (untested) <br/>
<br/>

Requirements: <br/>
- CMake 2.8.3+ (https://cmake.org) <br/>
- C++ 11 STL libraries (http://en.cppreference.com/w/cpp) <br/>
- Eigen3 (http://eigen.tuxfamily.org) for probabilisticly enhanced search access <br/>
<br/>

CMake build sequence for example code (in project root): <br/>
mkdir build <br/>
cd build <br/>
cmake .. <br/>
make <br/>
<br/>

## Build your own Descriptor/Node types!
The 2 base classes: CBSNode and CDescriptorBinary (see types_core) can easily be inherited. <br/>
Users might specify their own, augmented binary descriptor and node classes with specific leaf spawning. <br>
The probabilistic (variance based) search components (see types_custom) are implemented just in this way. <br/>
The CBSTree class itself does not have to be subclassed, so that the client interface may remain identical. <br/>

