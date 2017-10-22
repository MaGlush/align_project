#include "../../include/align.h"
#include "../../include/mvc.h"
#include "../../include/plugin.hpp"


using namespace std;

const string plugin_name = "gray-world";

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
    
        double red_sum = 0, green_sum = 0, blue_sum = 0, main_sum = 0;
        double size_pic = src_image.n_cols * src_image.n_rows;
    
        for (uint i = 0; i < src_image.n_rows; i++)
            for (uint j = 0; j < src_image.n_cols; j++) {
                red_sum += get<0>(src_image(i,j))/size_pic;
                green_sum += get<1>(src_image(i,j))/size_pic;
                blue_sum += get<2>(src_image(i,j))/size_pic;
            }
        main_sum = (red_sum + green_sum + blue_sum)/3;
    
        for (uint i = 0; i < src_image.n_rows; i++)
            for (uint j = 0; j < src_image.n_cols; j++) {
                auto r = get<0>(src_image(i,j))*(main_sum/red_sum);
                auto g = get<1>(src_image(i,j))*(main_sum/green_sum);
                auto b = get<2>(src_image(i,j))*(main_sum/blue_sum);
    
                r = r < 0 ? 0 : ( r > 255 ? 255 : r);
                g = g < 0 ? 0 : ( g > 255 ? 255 : g); 
                b = b < 0 ? 0 : ( b > 255 ? 255 : b); 
    
                src_image(i,j) = make_tuple(r, g, b);
        }
        return src_image;
}