#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <initializer_list>
#include <limits>

using std::string;
using std::stringstream;
using std::cout;
using std::cerr;
using std::endl;
using std::numeric_limits;

#include "mvc.h"
#include "controler.hpp"
#include "viewer.hpp"

int main(int argc, char **argv)
{
    Model mod;
    CView view(&mod);
    Control a(&mod, &view);

    auto rvalue = a.alignment(argc, argv);
    
    return rvalue;
}
