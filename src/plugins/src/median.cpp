#include "../../include/align.h"
#include "../../include/mvc.h"
#include "../../include/plugin.hpp"


using namespace std;

const string plugin_name = "median";

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
    int radius = args.radius;
    const auto start_i = radius;
    const auto end_i = src_image.n_rows - radius;
    const auto start_j = radius;
    const auto end_j = src_image.n_cols - radius;
    const auto size = radius * 2 + 1;
    
    Image res_image(src_image.n_rows,src_image.n_cols);
    for (uint i = start_i; i < end_i; ++i) {
        for (uint j = start_j; j < end_j; ++j) {
            auto neighbourhood = src_image.submatrix(i - radius,j - radius,size,size);
            //build histogram
            uint r = 0, g = 0, b = 0;
            Histogram Hist;
            for(int k = 0; k < 256; k++) 
                Hist.r[k] = Hist.g[k] = Hist.b[k] = 0;
            for (int hor = 0; hor < size; hor++)
                for (int ver = 0; ver < size; ver++){  
                    std::tie(r,g,b) = neighbourhood(hor,ver);
                    Hist.r[r]++;
                    Hist.g[g]++;
                    Hist.b[b]++;
                }
            int sb = 0,sg = 0, sr = 0;
            r = 0, g = 0, b = 0;
            int k = 0, m = 255; 
            for(k = 0, m = 255; k <= m; )
                sr = sr > 0 ? sr - Hist.r[k++] : sr + Hist.r[m--];
            r = k;
            for(k = 0, m = 255; k <= m; )
                sg = sg > 0 ? sg - Hist.g[k++] : sg + Hist.g[m--];
            g = k;
            for(k = 0, m = 255; k <= m; )
                sb = sb > 0 ? sb - Hist.b[k++] : sb + Hist.b[m--];
            b = k;
    
            res_image(i,j) = make_tuple(r,g,b);
        }
    }
    return res_image;
}