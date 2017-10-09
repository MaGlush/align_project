#include "align.h"
#include <string>
#include <climits>
#include <iterator>
#include <vector>


unsigned long long MAX_var = ULLONG_MAX; //maximum variable


struct Pyram 
{
    int limit;
    Image res_image;
    Result GR;
    Result GB;
    int counter;
};


struct Result
{
    unsigned long long metrica;
    int x;
    int y;
};

struct Histogram
{
    int r[256];
    int g[256];
    int b[256];
};

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::get;
using std::make_tuple;
using std::vector;
using std::tuple;


Result Metrix(Image &fixed, Image &moving, const int limit = 15) {
    Result Data;
    Data.x = 0;
    Data.y = 0;
    Data.metrica = MAX_var;
    int r = fixed.n_rows;
    int c = fixed.n_cols;
    int frame_i = 0.2 * c;
    int frame_j = 0.2 * r;

    for (int x = -limit; x < limit; x++) { // shift 
        for (int y = -limit; y < limit; y++) { //x - horizontal y - vertical
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

Image align(Image srcImage, bool isPostprocessing, std::string postprocessingType, double fraction, bool isMirror, 
            bool isInterp, bool isSubpixel, double subScale)
{
    Image res_image(srcImage.n_rows,srcImage.n_cols);
    const int limit = 15;
    Pyram structure;
    structure.res_image = srcImage;
    structure.limit = 15;
    structure.counter = 0;
    structure.GR.metrica = MAX_var;
    structure.GR.x = 0;
    structure.GR.y = 0;

    structure.GB.metrica = MAX_var;
    structure.GB.x = 0;
    structure.GB.y = 0;
    if (srcImage.n_rows > 800) && (srcImage.n_cols > 800) {
        structure = pyramid(structure);
        return structure.res_image;
    }
    
    auto step = res_image.n_rows/3;
    Image BlueImage = srcImage.submatrix(0,0,step,srcImage.n_cols);
    Image GreenImage = srcImage.submatrix(step,0,step,srcImage.n_cols);
    Image RedImage = srcImage.submatrix(step * 2,0,step,srcImage.n_cols);
    //metrica
    Result Data_Green_Red = Metrix(GreenImage,RedImage,limit);
    Result Data_Green_Blue = Metrix(GreenImage,BlueImage,limit);
    //colorization
    res_image = colorization(GreenImage,RedImage,BlueImage,Data_Green_Red,Data_Green_Blue);
    //postprocessing
    if (isPostprocessing){
        if (postprocessingType == "--gray-world")
            res_image = gray_world(res_image);
        if (postprocessingType == "--unsharp")
            res_image = unsharp(res_image);
        if (postprocessingType == "--autocontrast")
            res_image = autocontrast(res_image,fraction);
    }
    
    return res_image;
}

Image sobel_x(Image src_image) {
    Matrix<double> kernel = {{-1, 0, 1},
                             {-2, 0, 2},
                             {-1, 0, 1}};
    return custom(src_image, kernel);
}

Image sobel_y(Image src_image) {
    Matrix<double> kernel = {{ 1,  2,  1},
                             { 0,  0,  0},
                             {-1, -2, -1}};
    return custom(src_image, kernel);
}

Image unsharp(Image src_image) {
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

Image gray_world(Image src_image) {
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


Image resize(Image src_image, double scale) {
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



Image custom(Image src_image, Matrix<double> kernel) {
    // Function custom is useful for making concrete linear filtrations
    // like gaussian or sobel. So, we assume that you implement custom
    // and then implement other filtrations using this function.
    // sobel_x and sobel_y are given as an example.
    return src_image;
}

Pyram pyramid(Pyram structure) {
    structure.counter++;
    double scale = 0.5;
    if (structure.res_image.n_rows > 800) {
        structure.res_image = resize(structure.res_image,scale);
        structure = pyramid(structure);
    }

    auto step = structure.res_image.n_rows/3;
    Image BlueImage = srcImage.submatrix(0,0,step,srcImage.n_cols);
    Image GreenImage = srcImage.submatrix(step,0,step,srcImage.n_cols);
    Image RedImage = srcImage.submatrix(step * 2,0,step,srcImage.n_cols);

    structure.GR = Metrix(GreenImage,RedImage,structure.limit);
    structure.GB = Metrix(GreenImage,BlueImage,structure.limit);

    structure.limit = structure.limit * scale;

    if (structure.counter == 1) 
        structure.res_image = colorization(GreenImage,RedImage,BlueImage,structure.GR,structure.GB);
    return structure;
}

Image autocontrast(Image src_image, double fraction) {
    unsigned long long ymax = 0, ymin = MAX_var, y = 0;
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

Image gaussian(Image src_image, double sigma, int radius)  {
    return src_image;
}

Image gaussian_separable(Image src_image, double sigma, int radius) {
    return src_image;
}

bool comparePixels(const std::tuple<uint, uint, uint> &a, const std::tuple<uint, uint, uint> &b){
    double BrightnessA = get<0>(a) * 0.2125 + get<1>(a) * 0.7154 + get<2>(a) * 0.0721;
    double BrightnessB = get<0>(b) * 0.2125 + get<1>(b) * 0.7154 + get<2>(b) * 0.0721;
    return (BrightnessA < BrightnessB);
}

Image median(Image src_image, int radius) {
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




Image median_linear(Image src_image, int radius) {
    return src_image;
}

Image median_const(Image src_image, int radius) {
    return src_image;
}

Image canny(Image src_image, int threshold1, int threshold2) {
    return src_image;
}
