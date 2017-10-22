//pluginfo and plugin
#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <climits>
#include <unistd.h>
#include <ctime>
#include <tuple>
#include <iterator>
#include <vector>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "matrix.h"
typedef Matrix<std::tuple<uint, uint, uint>> Image;

using std::tuple;
using std::make_tuple;
using std::tie;
using std::vector;
using std::string;
using std::stringstream;
using std::cout;
using std::cerr;
using std::cin;
using std::endl;
using std::to_string;
using std::numeric_limits;

struct Args;

class Plugin
{
public:
    Plugin();
    
    //do filter in library .so
    virtual Image processing(Image, Args args);
    virtual ~Plugin(){}
    
};

struct PlugInfo
{
	void* ptr[100];
	string name[100];
	int loaded;
	string fname; //filter name
	void* handle; //choosen filters to apply

	PlugInfo(): ptr(),name(),loaded(0),fname("noname"),handle(NULL) {}
	PlugInfo(PlugInfo& v): ptr(),name(),loaded(0),fname(v.fname),handle(NULL) {}
	PlugInfo& operator=(PlugInfo& v){ return *this; }
};
