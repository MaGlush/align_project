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

CView::CView(Model* mod): model(mod),  logf("log.txt", std::ios::app)
{
	mod->add_view(this);
}
void CView::update()
{
    string msg = model->message;
    if(!msg.empty()){
        cout << "State: " << model->message << endl;
        logf << "State: " << model->message << endl;
        model->message.clear();
    }else{
        cout << "State: " << model->get_state() << endl;
        logf << "State: " << model->message << endl; 
    }       
}

void CView::catch_error(const string& s, char** argv)
{
	cout << "Error: " << s << endl;
    cout << "For help type: " << endl << argv[0] << " --help" << endl;
	logf << "Error: " << s << endl;
    logf << "For help type: " << endl << argv[0] << " --help" << endl;

}

void CView::print_msg(const string msg)
{
    cout << msg << endl;
}

void CView::print_help(const char *argv0)
{
    const char *usage =
R"(where PARAMS are from list:

--align [ (--gray-world or --unsharp or 
           --autocontrast [<fraction>=0.0] or --white-balance) ||
           
           --subpixel [<k>=2.0] ||
           
           --bicubic-interp ||
           
           --mirror]
    align images with different options: one of postprocessing functions,
    subpixel accuracy, bicubic interpolation 
    for scaling and mirroring for filtering

--gaussian <sigma> [<radius>=1]
    gaussian blur of image, 0.1 < sigma < 100, radius = 1, 2, ...

--gaussian-separable <sigma> [<radius>=1]
    same, but gaussian is separable

--sobel-x
    Sobel x derivative of image

--sobel-y
    Sobel y derivative of image

--unsharp
    sharpen image

--gray-world
    gray world color balancing

--autocontrast [<fraction>=0.0]
    autocontrast image. <fraction> of pixels must be croped for robustness
    
--white-balance
    align white balance

--resize <scale>
    resize image with factor scale. scale is real number > 0

--canny <threshold1> <threshold2>
    apply Canny filter to grayscale image. threshold1 < threshold2,
    both are in 0..360

--median [<radius>=1]
    apply median filter to an image (quadratic time)

--median-linear [<radius>=1]
    apply median filter to an image (linear time)
    
--median-const [<radius>=1]
    apply median filter to an image (const. time)
    
--custom <kernel_string>
    convolve image with custom kernel, which is given by kernel_string, example:
    kernel_string = '1,2,3;4,5,6;7,8,9' defines kernel of size 3

[<param>=default_val] means that parameter is optional.
)";
    cout << "Usage: " << argv0 << " <input_image_path> <output_image_path> "
         << "PARAMS" << endl;
    cout << usage;
}