Cinder-Boost
============

Cinder's Boost Dependency
version 1.55 for use against dev


### Building for Apple Silicon

Download boost [1.66](https://www.boost.org/users/history/)


```
./bootstrap.sh --with-toolset=clang --prefix=./build --with-libraries=system,filesystem,date_time,regex,thread 
./b2 architecture=combined cxxflags="-std=c++11 -arch x86_64 -arch arm64 -mmacosx-version-min=10.9"
```
