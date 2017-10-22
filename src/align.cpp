
#include "mvc.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::get;
using std::make_tuple;
using std::vector;
using std::tuple;



Result Metrix(Image &fixed, Image &moving, const int limit, Result Data);        
Image colorization(Image& GreenImage,
    const Image& RedImage,
    const Image& BlueImage,
    Result Data_Green_Red,
    Result Data_Green_Blue);
Pyram pyramid(Pyram structure);
Image mirror(const Image& src_image, const int radius);
Image mirror_crop(const Image& src_image, const int radius);
    

void Model::send_msg(const string msg)
{
    message += msg;
    notify();
}

Image Model::run_plugin(Image src)
{
    void* handle = plug.handle;
    if (!handle){
        send_msg ( "Unexpected error: filter [" + plug.fname + "] not founded" );
        return src;
    }

    Plugin* (*create)();
    void (*destroy)(Plugin*);

    create = reinterpret_cast<Plugin* (*)()>( dlsym(handle, "create_object") );
    destroy = reinterpret_cast<void (*)(Plugin*)>( dlsym(handle, "destroy_object") );
    Plugin* plugin = reinterpret_cast<Plugin*>( create() );
    Image dst = plugin->processing(src, args);
    destroy( plugin );

    return dst;
}

Image Model::big_align(Image srcImage)
{
    Image res_image(srcImage.n_rows,srcImage.n_cols);
    
    const int limit = 10;
    Result GR;
    Result GB;
    Pyram structure(srcImage,GR,GB,limit);
    cerr << "start pyramid" << endl;
    structure = pyramid(structure);
    cerr << "end of pyramid" << endl;
    res_image = structure.res_image.deep_copy();

    return res_image;
}

Image Model::align(Image srcImage)
{
    Image res_image(srcImage.n_rows,srcImage.n_cols);
   
    const int limit = 15;
    auto step = res_image.n_rows/3;
    Image BlueImage = srcImage.submatrix(0,0,step,srcImage.n_cols);
    Image GreenImage = srcImage.submatrix(step,0,step,srcImage.n_cols);
    Image RedImage = srcImage.submatrix(step * 2,0,step,srcImage.n_cols);
    //metrica
    Result Data;
    Result Data_Green_Red = Metrix(GreenImage,RedImage,limit, Data);
    Result Data_Green_Blue = Metrix(GreenImage,BlueImage,limit, Data);
    //colorization
    res_image = colorization(GreenImage,RedImage,BlueImage,Data_Green_Red,Data_Green_Blue);

    return res_image;
}

Image Model::postprocessing(Image src, bool isMirror,bool isInterp, bool isSubpixel)
{
    if(isMirror)
        src = mirror(src,1);

    src = run_plugin(src);    

    if(isMirror)
     src = mirror_crop(src,1);

    return src;
}

Image Model::resize(Image src, double scale)
{
    args.scale = scale;
    return run_plugin(src);
}

Pyram Model::pyramid(Pyram structure) {
    structure.counter++;
    cerr << "level down: " << structure.counter << endl;
    double scale = 2;
    Image prev = structure.res_image.deep_copy(); 
    if (structure.res_image.n_rows > 900) {
        structure.res_image = resize(structure.res_image, 1/scale).deep_copy();
        structure = pyramid(structure);
    }

    // cerr << "level up: " << structure.counter << endl;
    // cerr << "metrix old: " << structure.GR.x << endl;
    // cerr << "limit old: " << structure.limit << endl;
    auto step = prev.n_rows/3;
    Image BlueImage = prev.submatrix(0,0,step,prev.n_cols).deep_copy();
    Image GreenImage = prev.submatrix(step,0,step,prev.n_cols).deep_copy();
    Image RedImage = prev.submatrix(step * 2,0,step,prev.n_cols).deep_copy();

    structure.GR = Metrix(GreenImage,RedImage,structure.limit, structure.GR);
    structure.GB = Metrix(GreenImage,BlueImage,structure.limit, structure.GB);
    if (structure.counter == 1) 
        structure.res_image = colorization(GreenImage,RedImage,BlueImage,structure.GR,structure.GB).deep_copy();

    structure.limit = structure.limit * 1/scale;
    structure.GR = structure.GR * scale;
    structure.GB = structure.GB * scale;
    structure.counter--;
    // cerr << "limit new: " << structure.limit << endl;
    // cerr << "metrix new: " << structure.GR.x << endl;


    return structure;
}

Result Metrix(Image &fixed, Image &moving, const int limit, Result Data) {

    if(limit == 0) return Data;

    int shift_hor = Data.x;
    int shift_ver = Data.y;    
    int r = fixed.n_rows;
    int c = fixed.n_cols;
    int frame_i = 0.2 * c;
    int frame_j = 0.2 * r;

    for (int x = shift_hor-limit; x < shift_hor+limit; x++) { // shift 
        for (int y = shift_ver-limit; y < shift_ver+limit; y++) { //x - horizontal y - vertical
            //find intersection
            // fixed pic borders
            int from_i = y < 0 ? 0 : y;
            int from_j = x < 0 ? 0 : x;
            int to_i = y < 0 ? r-abs(y) : r;
            int to_j = x < 0 ? c-abs(x) : c;

            unsigned long long res = 0;
            for (int i = from_i + frame_i; i < to_i - frame_i; i++){ //square metrix
                for (int j = from_j + frame_j; j < to_j - frame_j; j++) {
                    res += pow(int(get<0>(fixed(i,j))) - int(get<0>(moving(i-y, j-x))),2);
                }
            }
            // res = res/(r * c);
            if (res <= Data.metrica)
            {
                Data.metrica = res;
                Data.x = x;
                Data.y = y;
            }
        }
    }
    return Data;
}

Image colorization(Image& GreenImage,
                  const Image& RedImage,
                  const Image& BlueImage,
                  Result Data_Green_Red,
                  Result Data_Green_Blue)
{
    cerr << "colorization" << endl;
    ssize_t r = GreenImage.n_rows;
    ssize_t c = GreenImage.n_cols;
    ssize_t from_j = Data_Green_Red.x < 0 ? 0 : Data_Green_Red.x;
    ssize_t from_i = Data_Green_Red.y < 0 ? 0 : Data_Green_Red.y;
    ssize_t to_j = Data_Green_Red.x > 0 ? c : c - abs(Data_Green_Red.x);
    ssize_t to_i = Data_Green_Red.y > 0 ? r : r - abs(Data_Green_Red.y);
    
    for (ssize_t i = from_i; i < to_i; i++)
        for (ssize_t j = from_j; j < to_j; j++) {
            auto red = get<0>(RedImage(i - Data_Green_Red.y, j - Data_Green_Red.x));
            GreenImage(i,j) = make_tuple(red,
                                       get<1>(GreenImage(i,j)),
                                       get<2>(GreenImage(i,j)));
        }

    from_j = Data_Green_Blue.x < 0 ? 0 : Data_Green_Blue.x;
    from_i = Data_Green_Blue.y < 0 ? 0 : Data_Green_Blue.y;
    to_j = Data_Green_Blue.x > 0 ? c : c - abs(Data_Green_Blue.x);
    to_i = Data_Green_Blue.y > 0 ? r : r - abs(Data_Green_Blue.y);
    
    
    for (ssize_t i = from_i; i < to_i; i++)
        for (ssize_t j = from_j; j < to_j; j++) {   
            auto blue = get<2>(BlueImage(i - Data_Green_Blue.y, j - Data_Green_Blue.x));
            GreenImage(i,j) = make_tuple(get<0>(GreenImage(i,j)),
                                         get<1>(GreenImage(i,j)),
                                         blue);
        }
    return GreenImage;
}

bool comparePixels(const std::tuple<uint, uint, uint> &a, const std::tuple<uint, uint, uint> &b){
    double BrightnessA = get<0>(a) * 0.2125 + get<1>(a) * 0.7154 + get<2>(a) * 0.0721;
    double BrightnessB = get<0>(b) * 0.2125 + get<1>(b) * 0.7154 + get<2>(b) * 0.0721;
    return (BrightnessA < BrightnessB);
}

Image mirror(const Image& src_image, const int radius)
{
    Image res_image{src_image.n_rows + 2*radius, src_image.n_cols + 2*radius};
    for(ssize_t i = 0; i < res_image.n_rows; ++i)
        for(ssize_t j = 0; j < res_image.n_cols; ++j){
            auto x = i - radius;
            auto y = j - radius;
            if(x >= src_image.n_rows)
                x = 2*(src_image.n_rows-1) - x;
            if(y >= src_image.n_cols)
                y = 2*(src_image.n_cols-1) - y;
            res_image(i,j) = src_image(abs(x),abs(y));
        }
    return res_image;
}

Image mirror_crop(const Image& src_image, const int radius)
{       
    Image res_image{src_image.n_rows-2*radius, src_image.n_cols-2*radius};
    for(ssize_t i = radius; i < src_image.n_rows-radius; ++i)
        for(ssize_t j = radius; j < src_image.n_cols-radius; ++j)
            res_image(i-radius,j-radius) = src_image(i,j);
    return res_image;
}
