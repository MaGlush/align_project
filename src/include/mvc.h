//model control view classes
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
#include "EasyBMP.h"

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
using std::ofstream;

typedef Matrix<std::tuple<uint, uint, uint>> Image;

#include "plugin.hpp"

struct Result
{
	int x;
    int y;
    unsigned long long metrica;
	
    Result(): x(0), y(0), metrica(ULLONG_MAX){}
    
    Result& operator= (const Result& s)
    {
		x = s.x;
        y = s.y;
        metrica = s.metrica;
        return *this;
    }
	
    Result operator*(double a)
    {
		x *= a;
        y *= a;
        return *this;
    }
};

struct Histogram
{
	int r[256];
    int g[256];
    int b[256];
};
struct Pyram 
{
	Image res_image;
    Result GR;
    Result GB;
    int limit;    
    int counter;
	
    Pyram(const Image& src, Result r, Result b, int lim): res_image(src), GR(r), GB(b), limit(lim), counter(0) {}
};

struct Args{
	int radius;
	double scale;
	double subscale;
	double fraction;
	double sigma;
	int threshold1;
	int threshold2;
	Matrix<double> kernel;
	
	Args(): radius(1), scale(2), subscale(2), fraction(0.2), sigma(0), threshold1(0), threshold2(0), kernel(0) {}
};


class Control;
class CView;
class Model;



class Observ
{
public:
	virtual void update() = 0;
	virtual ~Observ(){}
};

class Subject
{
    vector < class Observ * > views; 
public:
  	Subject(): views(0){}
	void add_view(Observ *obs){
        views.push_back(obs);
    }
  
    void notify(){
    	for (uint i = 0; i < views.size(); ++i)
          views[i]->update();  
    }
};

class Model : public Subject
{
	string state;
	void set_state(const string& s);
public:
	Args args;
	string message;
	PlugInfo plug;
	Model(): state("Program starts"), args(), message(), plug() { message.clear(); }
	Model(Model& v):  state(v.state), args(), message(), plug() { message.clear(); }
	Model& operator=(Model& v){ return *this; }
	string get_state();

	Image load_image(const char*);
	void save_image(const Image&, const char*);
	//alignment
	Image align(Image srcImage);
	Image big_align(Image srcImage); 
	Pyram pyramid(Pyram structure);
	Image postprocessing(Image,bool,bool,bool);
	Image resize(Image src_image, double scale);
	
				
	Image run_plugin(Image src);
	void send_msg(const string);
	
		
};

class Control
{
	Model* model;
	CView* cview;

	template<typename ValueType>
	ValueType read_value(string s);

	template<typename ValueType>
	bool check_value(string s);

	template<typename ValueT>
	void check_number(string val_name, ValueT val, ValueT from, ValueT to);

	void check_argc(int argc, int from, int to);

	Matrix<double> parse_kernel(string kernel);

	void parse_args(char **argv, int argc, bool *isPostprocessing, string *postprocessingType, double *fraction, bool *isMirror, 
				bool *isInterp, bool *isSubpixel, double *subScale);
	void filter_args(int, char**, string);
	//plugin manager
	int search_plugins();
	int load_plugins(const int);
	void* choose_plugin(const int,string&,const string);
	void* find_n_load(int&,int&,const string);
	void send_plugins_info(string,int,void*);
public:
	void* pl_ptr[100];
	string pl_name[100];

	Control(Model* m, CView* v): model(m), cview(v), pl_ptr(), pl_name() {}
	Control(Control& v): model(v.model), cview(v.cview), pl_ptr(), pl_name() {}
	Control& operator=(Control& v){ return *this; }

	int alignment(int argc, char **argv);
};


class CView : public Observ
{	Model* model;
	ofstream logf; 
public:
	CView(Model* mod);
	CView(CView& v): model(v.model), logf("log.txt") {}
	CView& operator=(CView& v){ return *this; }

	void update();
	void print_msg(string s);
	void catch_error(const string&, char**);
	void print_help(const char *argv0);
};

