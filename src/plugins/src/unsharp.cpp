#include "../../include/align.h"
#include "../../include/mvc.h"
#include "../../include/plugin.hpp"


using namespace std;

const string plugin_name = "unsharp";

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
    double one = 1;
    Matrix<double> kernel = {{-one/6, -2*one/3, -one/6},
                             {-2*one/3,  13*one/3, -2*one/3},
                             {-one/6, -2*one/3, -one/6}};
    
    uint radius = (kernel.n_cols - 1) / 2;
    Image Res_image(src_image.n_rows,src_image.n_cols);
    auto size = 2*radius+1;
    const auto start_i = radius;
    const auto end_i = src_image.n_rows - radius;
    const auto start_j = radius;
    const auto end_j = src_image.n_cols - radius;

    for (uint i = start_i; i < end_i; ++i) {
        for (uint j = start_j; j < end_j; ++j) {
            auto neighbourhood = src_image.submatrix(i-radius,j-radius,size,size);
            double r = 0, g = 0, b = 0;
            double sum_r = 0, sum_g = 0, sum_b = 0;
            for (uint hor = 0; hor < size; hor++)
                for (uint ver = 0; ver < size; ver++) {
                        std::tie(r,g,b) = neighbourhood(hor,ver);
                        sum_r += r * kernel(hor,ver);
                        sum_g += g * kernel(hor,ver);
                        sum_b += b * kernel(hor,ver);
                }
            sum_r = sum_r < 0 ? 0 : ( sum_r > 255 ? 255 : sum_r);
            sum_g = sum_g < 0 ? 0 : ( sum_g > 255 ? 255 : sum_g); 
            sum_b = sum_b < 0 ? 0 : ( sum_b > 255 ? 255 : sum_b); 

            Res_image(i,j) = make_tuple(sum_r,sum_g,sum_b);
        }
    }
    return Res_image;
}