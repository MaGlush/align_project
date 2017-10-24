//controler
#pragma once

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


template<typename ValueType>
ValueType Control::read_value(string s)
{
    stringstream ss(s);
    ValueType res;
    ss >> res;
    if (ss.fail() or not ss.eof())
        throw string("bad argument: ") + s;
    return res;
}

template<typename ValueType>
bool Control::check_value(string s)
{
    stringstream ss(s);
    ValueType res;
    ss >> res;
    if (ss.fail() or not ss.eof())
        return false;
    return true;
}

template<typename ValueT>
void Control::check_number(string val_name, ValueT val, ValueT from, ValueT to)
{
    if (val < from)
        throw val_name + string(" is too small");
    if (val > to)
        throw val_name + string(" is too big");
}

void Control::check_argc(int argc, int from, int to)
{
    if (argc < from)
        throw string("too few arguments for operation");

    if (argc > to)
        throw string("too many arguments for operation");
}

Matrix<double> Control::parse_kernel(string kernel)
{
    // Kernel parsing implementation here
    return Matrix<double>(0, 0);
}

void Control::parse_args(char **argv, int argc, bool *isPostprocessing, string *postprocessingType, double *fraction, bool *isMirror, 
            bool *isInterp, bool *isSubpixel, double *subScale)
{
    for (int i = 4; i < argc; i++) {
        string param(argv[i]);
        
        if (param == "--gray-world" || param == "--unsharp" || 
        param == "--white-balance" || param == "--autocontrast") {
            *isPostprocessing = true;
            *postprocessingType = param;
            if ((param == "--autocontrast") && ((i+1) < argc) && check_value<double>(argv[i+1])) {
                *fraction = read_value<double>(argv[++i]);
            }
        } else if (param == "--subpixel") {
            *isSubpixel = true;
            if (((i+1) < argc) && check_value<double>(argv[i+1])) {
                *subScale = read_value<double>(argv[++i]);
            }
        } else if (param == "--bicubic-interp") {
            *isInterp = true;
        } else if (param == "--mirror") {
            *isMirror = true;
        }else
            throw string("unknown option for --align ") + param;
    }
}

void Control::filter_args(int argc, char** argv, string action)
{
    if (action == "--align") return;

    if (action == "--sobel-x"){
        check_argc(argc, 4, 4);
        model->args.kernel = {{-1, 0, 1},
                              {-2, 0, 2},
                              {-1, 0, 1}};
    } else if( action == "--sobel-y" || action == "--unsharp" || action == "--gray-world") {
        check_argc(argc, 4, 4);
        model->args.kernel = {{ 1,  2,  1},
                              { 0,  0,  0},
                              {-1, -2, -1}};
    } else if (action == "--resize") {
        check_argc(argc, 5, 5);
        model->args.scale = read_value<double>(argv[4]);
    }  else if (action == "--custom") {
        check_argc(argc, 5, 5);
        model->args.kernel = parse_kernel(argv[4]);
    } else if (action == "--autocontrast") {
        check_argc(argc, 4, 5);
        model->args.fraction = 0.0;
        if (argc == 5) {
            model->args.fraction = read_value<double>(argv[4]);
            check_number("fraction", model->args.fraction, 0.0, 0.4);
        }
    } else if (action == "--gaussian" || action == "--gaussian-separable") {
        check_argc(argc, 5, 6);
        model->args.sigma = read_value<double>(argv[4]);
        check_number("sigma", model->args.sigma, 0.1, 100.0);
        model->args.radius = 3 *  model->args.sigma;
        if (argc == 6) {
            model->args.radius = read_value<int>(argv[5]);
            check_number("radius", model->args.radius, 1, numeric_limits<int>::max());
        }
    } else if (action == "--canny") {
        check_argc(6, 6, numeric_limits<int>::max());
        model->args.threshold1 = read_value<int>(argv[4]);
        check_number("threshold1", model->args.threshold1, 0, 360);
        model->args.threshold2 = read_value<int>(argv[5]);
        check_number("threshold2", model->args.threshold2, 0, 360);
        if (model->args.threshold1 >= model->args.threshold2)
            throw string("threshold1 must be less than threshold2");
    } else if (action == "--median" || action == "--median-linear" ||
                action == "--median-const") {
        check_argc(argc, 4, 5);
        model->args.radius = 1;
        if (argc == 5) {
            model->args.radius = read_value<int>(argv[4]);
            check_number("radius", model->args.radius, 1, numeric_limits<int>::max());
        }
    } else {
        	throw string("unknown action ") + action;
    }
    model->send_msg(action.erase(0,2) + " starts");
    
}

int Control::search_plugins()
{
    cview->print_msg ("searching plugins in " PLUG_PATH);
    
    DIR *dp;
    struct dirent *dirp;

    if((dp  = opendir(PLUG_PATH)) == NULL) {
    	cview->print_msg ("Error: can't open " PLUG_PATH);
        return -1;
    }
    int found = 0;
    while ((dirp = readdir(dp)) != NULL) {
        char *dot = strrchr(dirp->d_name, '.'); 
        if (dot && (strcmp(dot, ".so") == 0)) //search for all .so libs
        {
            pl_name[found] = string(dirp->d_name);
            cview->print_msg( pl_name[found] );
            found++;
        }
    }
    cview->print_msg( to_string(found) + " libraries founded\n" );    
    if(closedir(dp) < 0){
        cview->print_msg ("Error: can't close opened dir " PLUG_PATH);
        return -1;
    }
    return found;
}

int Control::load_plugins(const int founded)
{
    cview->print_msg ("loading plugins...");
    void* handle;
    int loaded = 0;
    for (int i=0; i<founded; ++i){
        string name = pl_name[i];        
        string pathname = PLUG_PATH + name;
        handle = dlopen(pathname.c_str(), RTLD_LAZY);
        if (handle){
            pl_ptr[loaded] = handle;
            name = name.substr(4,name.size()-7); //crop lib_*name*.so
            pl_name[loaded] = name;
            cview->print_msg( "["+to_string(loaded+1) +"] "+pl_name[loaded] );
            loaded++;              
        }
    }
    if (loaded < founded)
        cview->print_msg( to_string(founded - loaded)+ " plugins wasn't loaded!" );

    return loaded;
}

void* Control::choose_plugin(const int loaded, string& name, const string flag)
{
    void* handle = NULL;
    if(flag == "--filter"){ //user choice
        name = "noname";
        cview->print_msg( "Choose plugin to run:" );
        int num;
        cin >> num;
        if (num > 0 && num <= loaded){
            handle = pl_ptr[num-1];
            string (*get_plugin_name)();
            get_plugin_name = reinterpret_cast<string (*)()>( dlsym(handle, "get_plugin_name") );
            name = get_plugin_name();
            cview->print_msg ( "\'" + name + "\' apply!" );
        }
        else
            cview->print_msg ( "Error: no such filter [" + to_string(num) + "] founded" );
            
        return handle;
    }
    //auto choosing by flag
    // originaly name = --flag
    name = name.substr(2, name.size()-2); //name = flag
    string plugin_name = "nofilter";
    for(int i = 0; i < loaded; ++i){ //search by filename
        handle = pl_ptr[i];
        string (*get_plugin_name)();
        get_plugin_name = reinterpret_cast<string (*)()>( dlsym(handle, "get_plugin_name") );
        plugin_name = get_plugin_name();
        if(name == plugin_name){
            break;
        }
    }
    if (plugin_name == "nofilter")
        throw string(name + " not loaded. aborted.");
    else 
        cview->print_msg ( "\'" + name + "\' apply!" );
    
    return handle;
}

void Control::send_plugins_info(string name, int loaded, void* handle)
{
    for(int i = 0; i<loaded; ++i){
        model->plug.ptr[i] = pl_ptr[i];
        model->plug.name[i] = pl_name[i];
    }
    model->plug.loaded = loaded;
    model->plug.fname = name;
    model->plug.handle = handle;
    model->send_msg(name + " starts");
    
}
    
void* Control::find_n_load(int& founded, int& loaded, const string flag)
{
    void* handle = NULL;
    founded = search_plugins(), loaded = 0;

    if (founded > 0)
    loaded = load_plugins(founded);
    
    string filter_name = flag;
    if (loaded > 0)
        handle = choose_plugin(loaded,filter_name,flag); 
    
    if (handle)
        send_plugins_info(filter_name, loaded, handle);
    else if(flag == "--filter")
        return handle;
    else
        throw string(filter_name + " not founded. aborted.");
    return handle;
}

int Control::alignment(int argc, char **argv)
{
try	{
        model->notify();
        check_argc(argc, 2, numeric_limits<int>::max());
        if (string(argv[1]) == "--help") {
            cview->print_help(argv[0]);
            return 0;
        }

        check_argc(argc, 4, numeric_limits<int>::max());
        Image src_image = model->load_image(argv[1]), dst_image = src_image;
        model->send_msg("Image loaded");

        string action(argv[3]);
        int founded = 0, loaded = 0;

        if (action == "--filter"){
            check_argc(argc, 4, 4);
            model->send_msg("Plugins loading");
            
            void* handle = find_n_load(founded, loaded, action);
            if (handle){ 
                dst_image = model->run_plugin(src_image);
                action = "no action";
            }else
                action = "--align"; //default action if cant choose specific
        }
        if ( action != "no action"){
        filter_args(argc, argv, action);
        if (action == "--align") {
            bool isPostprocessing = false, isInterp = false,
                isSubpixel = false, isMirror = false;
                
            string postprocessingType;
    
            double fraction = 0.0, subScale = 2.0;
            if (argc >= 5) {
                parse_args(argv, argc, &isPostprocessing, &postprocessingType, &fraction, &isMirror,
                    &isInterp, &isSubpixel, &subScale);                    
            }
            model->args.fraction = fraction;
            model->args.subscale = subScale;
            if ( (src_image.n_rows > 900) && (src_image.n_cols > 900) ){
                model->send_msg("Big resolution detected");
                model->send_msg("Plugin resize loading");
                find_n_load(founded, loaded, "--resize"); //check on resize plugin for pyramid work   
                model->send_msg("Alignment starts");
                dst_image = model->big_align(src_image);  
            } else {
                model->send_msg("Alignment starts");
                dst_image = model->align(src_image);
            }
            if(isPostprocessing){
                model->send_msg("Postprocessing begins");
                model->send_msg("Plugin " + postprocessingType + " loading");
                find_n_load(founded, loaded, postprocessingType);
                model->send_msg("Postprocessing begins::"+postprocessingType);
                dst_image = model->postprocessing(dst_image, isMirror, isInterp, isSubpixel);
            }
        } else {
            model->send_msg("Plugins loading");
            find_n_load(founded, loaded, action);
            dst_image = model->run_plugin(src_image);
        }}
        model->save_image(dst_image, argv[2]);
        model->send_msg("Image saved");
	} catch (const string &s) {
		cview->catch_error(s, argv);
    }  
    return 0;
}