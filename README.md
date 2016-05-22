# BST: Binary Search Tree Header-only library

Last updated: 2016-05-16 <br/>
Contributors: https://github.com/schdomin, https://github.com/grisetti <br/>
<br/>

Abstract: https://www.google.ch <br/>
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

## Build your own types!
The 2 base classes: CBSNode and CDescriptorBinary (see types_core) can be easily inherited. <br/>
Users can specify their own, augmented binary descriptor and nodes classes with specific leaf spawning. <br>
The probabilistic (variance based) search components (see types_custom) are implemented just in this way. <br/>
The CBSTree class itself does not have to be subclassed, therefore the client interface may remain identical. <br/>

