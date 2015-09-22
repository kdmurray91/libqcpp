libqc++
=======

[![Build Status](http://biojenkins.anu.edu.au/job/libqc++/badge/icon)](http://biojenkins.anu.edu.au/job/libqc++/)

A C++11 library for sequence quality control.

Design features:

- Modularity
- Parallelisation
- Pipelined
- Simple API

Installation
------------

Statically compiled libraries and binaries are available from the
[GitHub releases page](https://github.com/kdmurray91/libqcpp/releases).

These can be uncompressed then copied to, for instance, `/usr/local/` or `~/`.

### From Source:

On Debian-like systems:

    sudo apt-get install libboost-all-dev libyaml-cpp-dev zlib1g-dev cmake \
                         build-essential
    git clone https://github.com/kdmurray91/libqcpp.git
    cd libqcpp
    mkdir build && cd build
    cmake ..
    make
    make install

License
-------

All code is licensed under the GNU Lesser General Public License, version 3 or
at your option any later version.

Catch.hpp, seqan and libssw are licensed under their respective licenses, see
their LICENSE files for more information.
