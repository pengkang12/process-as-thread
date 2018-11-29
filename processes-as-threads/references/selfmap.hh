#if !defined(DOUBLETAKE_SELFMAP_H)
#define DOUBLETAKE_SELFMAP_H

/*
 * @file   selfmap.h
 * @brief  Process the /proc/self/map file.
 * @author Tongping Liu <http://www.cs.umass.edu/~tonyliu>
 * @author Emery Berger <http://www.cs.umass.edu/~emery>

The r-xp entry describes a block of executable memory (x permission flag). That's the code.
The r--p entry describes a block of memory that is only readable (r permission flag). That's static data (constants).
The rw-p entry describes a block of memory that is writable (w permission flag). This is for global variables of the library.
The ---p entry describes a chunk of address space that doesn't have any permissions (or any memory mapped to it).
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <functional>
#include <map>
#include <new>
#include <string>
#include <utility>

#include "interval.hh"
#include "mm.hh"
#include "real.hh"

// From heaplayers
#include "wrappers/stlallocator.h"

using namespace std;

struct regioninfo {
  void* start;
  void* end;
};

/**
 * A single mapping parsed from the /proc/self/maps file
 */
class mapping {
public:
  mapping() : _valid(false) {}

  mapping(uintptr_t base, uintptr_t limit, char* perms, size_t offset, std::string file)
      : _valid(true), _base(base), _limit(limit), _readable(perms[0] == 'r'),
        _writable(perms[1] == 'w'), _executable(perms[2] == 'x'), _copy_on_write(perms[3] == 'p'),
        _offset(offset), _file(file) {}

  bool valid() const { return _valid; }

  bool isText() const { return _readable && !_writable && _executable; }

  bool isStack() const { return _file == "[stack]"; }

  bool isGlobals(std::string mainfile) const {
    // global mappings are RW_P, and either the heap, or the mapping is backed
    // by a file (and all files have absolute paths)
		// the file is the current executable file, with [heap], or with lib*.so
		// Actually, the mainfile can be longer if it has some parameters.
	 return (_readable && _writable && !_executable && _copy_on_write) &&
           (_file.size() > 0 && (_file == mainfile ||  _file == "[heap]" || _file.find(".so") != std::string::npos));
  }

  uintptr_t getBase() const { return _base; }

  uintptr_t getLimit() const { return _limit; }

  const std::string& getFile() const { return _file; }

private:
  bool _valid;
  uintptr_t _base;
  uintptr_t _limit;
  bool _readable;
  bool _writable;
  bool _executable;
  bool _copy_on_write;
  size_t _offset;
  std::string _file;
};

/// Read a mapping from a file input stream
static std::ifstream& operator>>(std::ifstream& f, mapping& m) {
  if(f.good() && !f.eof()) {
    uintptr_t base, limit;
    char perms[5];
    size_t offset;
    size_t dev_major, dev_minor;
    int inode;
    string path;

    // Skip over whitespace
    f >> std::skipws;

    // Read in "<base>-<limit> <perms> <offset> <dev_major>:<dev_minor> <inode>"
    f >> std::hex >> base;
    if(f.get() != '-')
      return f;
    f >> std::hex >> limit;

    if(f.get() != ' ')
      return f;
    f.get(perms, 5);

    f >> std::hex >> offset;
    f >> std::hex >> dev_major;
    if(f.get() != ':')
      return f;
    f >> std::hex >> dev_minor;
    f >> std::dec >> inode;

    // Skip over spaces and tabs
    while(f.peek() == ' ' || f.peek() == '\t') {
      f.ignore(1);
    }

    // Read out the mapped file's path
    getline(f, path);

    m = mapping(base, limit, perms, offset, path);
  }

  return f;
}

class selfmap {
public:
  static selfmap& getInstance() {
    static char buf[sizeof(selfmap)];
    static selfmap* theOneTrueObject = new (buf) selfmap();
    return *theOneTrueObject;
  }

  /// Check whether an address is inside the DoubleTake library itself.
  bool isDoubleTakeLibrary(void* pcaddr) {
    return ((pcaddr >= _doubletakeStart) && (pcaddr <= _doubletakeEnd));
  }

  /// Check whether an address is inside the main application.
  bool isApplication(void* pcaddr) {
    return ((pcaddr >= _appTextStart) && (pcaddr <= _appTextEnd));
  }

  void getStackInformation(void** stackBottom, void** stackTop) {
    for(const auto& entry : _mappings) {
      const mapping& m = entry.second;
      if(m.isStack()) {
        *stackBottom = (void*)m.getBase();
        *stackTop = (void*)m.getLimit();
        return;
      }
    }
    fprintf(stderr, "Couldn't find stack mapping. Giving up.\n");
    abort();
  }

  /// Get information about global regions.
  void getTextRegions() {
    for(const auto& entry : _mappings) {
      const mapping& m = entry.second;
      if(m.isText()) {
        if(m.getFile().find("libdoubletake") != std::string::npos) {
          _doubletakeStart = (void*)m.getBase();
          _doubletakeEnd = (void*)m.getLimit();
        } else if(m.getFile() == _main_exe) {
          _appTextStart = (void*)m.getBase();
          _appTextEnd = (void*)m.getLimit();
        }
      }
    }
  }

  /// Collect all global regions.
  void getGlobalRegions(regioninfo* regions, int* regionNumb) {
    size_t index = 0;

    for(const auto& entry : _mappings) {
      const mapping& m = entry.second;

      // skip libdoubletake
      if(m.isGlobals(_main_exe) && m.getFile().find("libdoubletake") == std::string::npos) {
        regions[index].start = (void*)m.getBase();
        regions[index].end = (void*)m.getLimit();
        index++;
      }
    }
    
    // We only need to return this.
    *regionNumb = index;
  }

private:
  selfmap() {
    // Read the name of the main executable
   // char buffer[PATH_MAX];
    //Real::readlink("/proc/self/exe", buffer, PATH_MAX);
    //_main_exe = std::string(buffer);
		bool gotMainExe = false;
    // Build the mappings data structure
    ifstream maps_file("/proc/self/maps");

    while(maps_file.good() && !maps_file.eof()) {
      mapping m;
      maps_file >> m;
			// It is more clean that that of using readlink. 
			// readlink will have some additional bytes after the executable file 
			// if there are parameters.	
			if(!gotMainExe) {
				_main_exe = std::string(m.getFile());
				gotMainExe = true;
			} 

      if(m.valid()) {
			//	fprintf(stderr, "Base %lx limit %lx\n", m.getBase(), m.getLimit()); 
        _mappings[interval(m.getBase(), m.getLimit())] = m;
      }
    }
  }

  std::map<interval, mapping, std::less<interval>,
           HL::STLAllocator<std::pair<interval, mapping>, InternalHeapAllocator>> _mappings;

  std::string _main_exe;
  void* _appTextStart;
  void* _appTextEnd;
  void* _doubletakeStart;
  void* _doubletakeEnd;
};

#endif
