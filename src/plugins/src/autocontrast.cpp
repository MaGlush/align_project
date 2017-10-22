#include "../../include/align.h"
#include "../../include/mvc.h"
#include "../../include/plugin.hpp"

using namespace std;

const string plugin_name = "autocontrast";

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
    double fraction = args.fraction;
    unsigned long long ymax = 0, ymin = ULLONG_MAX, y = 0;
    double r = 0, g = 0, b = 0, f_r = 0, f_g = 0, f_b = 0;
    Image res_image(src_image.n_rows,src_image.n_cols);

    for (uint i = 0; i < src_image.n_rows; i++)
        for (uint j = 0; j < src_image.n_cols; j++) {
            std::tie(r,g,b) = src_image(i,j);
            y = 0.2125 * r + 0.7154 * g + 0.0721 * b;
            if (ymax <= y)
                ymax = y;
            if (ymin >= y)
                ymin = y;
        }
    ymax = ymax - (ymax - ymin)*fraction;
    ymin = ymin + (ymax - ymin)*fraction;

    for (uint i = 0; i < src_image.n_rows; i++)
        for (uint j = 0; j < src_image.n_cols; j++) {
            std::tie(r,g,b) = src_image(i,j);
            f_r = (r - ymin) * 255 / (ymax - ymin);
            f_g = (g - ymin) * 255 / (ymax - ymin);
            f_b = (b - ymin) * 255 / (ymax - ymin);

            f_r = f_r < 0 ? 0 : ( f_r > 255 ? 255 : f_r);
            f_g = f_g < 0 ? 0 : ( f_g > 255 ? 255 : f_g); 
            f_b = f_b < 0 ? 0 : ( f_b > 255 ? 255 : f_b); 

            res_image(i,j) = make_tuple(f_r,f_g,f_b);
        }
    return res_image;
}