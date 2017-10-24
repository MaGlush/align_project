#include "../../include/align.h"
#include "../../include/mvc.h"
#include "../../include/plugin.hpp"


using namespace std;

const string plugin_name = "resize";

extern "C" Plugin* create_object()
{
  return new Plugin;
}

extern "C" void destroy_object( Plugin* object )
{
  delete object;
}

extern "C" string get_plugin_name()
{
    return plugin_name;
}

Plugin::Plugin(){}

Image Plugin::processing(Image src_image, Args args) {
    double scale = args.scale;
    double f_r = 0, f_g = 0, f_b = 0;
    double r1 = 0, g1 = 0, b1 = 0;    
    double r2 = 0, g2 = 0, b2 = 0;    
    double r3 = 0, g3 = 0, b3 = 0;    
    double r4 = 0, g4 = 0, b4 = 0;
    double x1,y1,x2,y2;
    Image res_image(int(src_image.n_rows * scale),int(src_image.n_cols * scale));    
    
    for (uint i = 0; i < uint(src_image.n_rows * scale); i++)
        for (uint j = 0; j < uint(src_image.n_cols * scale); j++) {
            if (i >= src_image.n_rows * scale - scale) 
                x1 = src_image.n_rows - 2;
            else 
                x1 = i / scale;
            if (j >= src_image.n_cols * scale - scale)
                y1 = src_image.n_cols - 2;
            else  
                y1 = j / scale;
            
            x2 = x1 + 1;
            y2 = y1 + 1;
            

            double fracX = x1 - int(x1);
            double fracY = y1 - int(y1);
            
            std::tie(r1,g1,b1) = src_image(x1,y1);
            std::tie(r2,g2,b2) = src_image(x2,y1);
            std::tie(r3,g3,b3) = src_image(x1,y2);
            std::tie(r4,g4,b4) = src_image(x2,y2);

            f_r = (r1 * (1 - fracX) + r2 * fracX)*(1 - fracY) + (r3*(1 - fracX) + r4*fracX)*fracY;
            f_g = (g1 * (1 - fracX) + g2 * fracX)*(1 - fracY) + (g3*(1 - fracX) + g4*fracX)*fracY;
            f_b = (b1 * (1 - fracX) + b2 * fracX)*(1 - fracY) + (b3*(1 - fracX) + b4*fracX)*fracY;

            f_r = f_r < 0 ? 0 : ( f_r > 255 ? 255 : f_r);
            f_g = f_g < 0 ? 0 : ( f_g > 255 ? 255 : f_g); 
            f_b = f_b < 0 ? 0 : ( f_b > 255 ? 255 : f_b); 

            res_image(i,j) = make_tuple(f_r,f_g,f_b);

        }
    return res_image;
}