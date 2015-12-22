Coral, open-source visual programming environment
===========================================================

Coral is an open-source visual programming environment to help artists and coders with rapid prototyping of CGI algorithms and workflows.
The project is meant to be a community effort to maintain a free core technology, the aim is to have a powerful foundation for many exciting consumer applications or in-house tools. 

## Technology Overview

c++ multi-threaded core library to handle hierarchical dependency graphs
python bindings
Py Qt user interface
maya integration plugin
c++ and python SDK
compatible with windows, linux and mac 

## External Links

Watch the videos of coral in action on the vimeo channel: https://vimeo.com/channels/coralapp
Please join the mailing list for any question: http://groups.google.com/group/coral-app
Visit our technology partner for the executable binaries on windows, linux and mac: http://proceduralinsight.com/
Constantine Tarasenkov is hosting windows builds (32/64 bits) and how-to-compile instructions on this blog: http://coral-app.tumblr.com 

## Dependecies

- Boost
- Scons
- PyQT

## Getting Started (Windows)

### Setting up boost

Download one of the boost versions and unzip it in a folder, preferably without spaces, for example E:\SDKs\boost_155. Run the command bootstrap (bootstrap.bat/bootstrap.sh), and after that run bjam as per boost instructions to compile boost. One important thing you have to consider it to build boost for the right target platform, otherwise you'll get linker errors.

```
@echo off
:: build
call bootstrap.bat
:: static
bjam link=static threading=multi address-model=64 --with-filesystem
--with-thread --with-regex --with-system --with-date_time --with-python
:: dynamic
bjam link=shared threading=multi address-model=64 --with-filesystem
--with-thread --with-regex --with-system --with-date_time --with-python
```

After boost is installed you can now run scons to build the library. From the commandline you can specify which target you want to compile for x86 for 32-bit and x64/AMD64 for 64-bit, and the msvc target version. By default if you don't provide these options scons will use your operating systems platform and the latest installed msvc.

```
scons -c -f buildStandalone.py target=x86 mode=release msvc=10.0
scons -f buildStandalone.py target=x86 mode=release msvc=10.0
scons -f buildStandalone.py target=x86 mode=release msvc=10.0 build-sdk=1
scons -f buildStandalone.py target=x86 mode=release msvc=10.0 build-sdk=1 sdk-install-dir=install
```

## Contributors

Andrea Interguglielmi
Paolo Fazio
Nicholas Yue
Dorian Fevrier
Alan Stanzione
Daniele Niero
Constantine Tarasenkov 
