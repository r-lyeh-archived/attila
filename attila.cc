#include <iostream>

#include "dot/dot.hpp"
// CImg.h code following
#include "dot/sample.hpp"
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
void display( const dot::image &pic, const char *title = "" ) {
    if( pic.size() ) {
        cimg_library::CImg<unsigned char> ctexture( pic.w, pic.h, 1, 4, 0 );
        for( size_t y = 0; y < pic.h; ++y ) {
            for( size_t x = 0; x < pic.w; ++x ) {
                dot::pixel pix = pic.at( x, y ).clamp().to_rgba();
                ctexture( x, y, 0 ) = (unsigned char)(pix.r * 255.f);
                ctexture( x, y, 1 ) = (unsigned char)(pix.g * 255.f);
                ctexture( x, y, 2 ) = (unsigned char)(pix.b * 255.f);
                ctexture( x, y, 3 ) = (unsigned char)(pix.a * 255.f);
            }
        }
        ctexture.display( title );
    }
}

#include "packers/packer.hpp"
#include "packers/MaxRectsBinPack.h"

#include <string>
#include <vector>
#include <sstream>
#include <fstream>

struct texture {
    std::string name;
    std::vector<unsigned char> data;

    float w, h, x, y, u0, v0, u1, v1;
    unsigned id, unit, is_rotated;

    texture() : w(0), h(0), x(0), y(0), u0(0), v0(0), u1(1), v1(1), id(0), unit(0), is_rotated(0)
    {}

    bool load( const std::string &pathfile ) {
        //reset
        *this = texture();

        dot::image img( pathfile );
        if( !img.size() )
            return false;

        this->name = pathfile;
        this->w = img.w;
        this->h = img.h;
        this->data = img.rgba_data();

        return true;
    }

    std::string debug() const {
        std::stringstream ss;
        ss << this << std::endl;
        ss << "\t.name: " << name << std::endl;
        ss << "\t.data[" << data.size() << "]..." << std::endl;
        ss << "\t.w: " << w << std::endl;
        ss << "\t.h: " << h << std::endl;
        ss << "\t.x: " << x << std::endl;
        ss << "\t.y: " << y << std::endl;
        ss << "\t.u0: " << u0 << std::endl;
        ss << "\t.v0: " << v0 << std::endl;
        ss << "\t.u1: " << u1 << std::endl;
        ss << "\t.v1: " << v1 << std::endl;
        ss << "\t.id: " << id << std::endl;
        ss << "\t.unit: " << unit << std::endl;
        ss << "\t.is_rotated: " << is_rotated << std::endl;
        return ss.str();
    }
};

void warn( const std::string &a = std::string(), const std::string &b = std::string() ) {
    if( a.size() ) std::cerr << a << ' ';
    if( b.size() ) std::cerr << b << ' ';
    std::cerr << std::endl;
}

std::vector<std::string> split( std::istream &is, char delim ) {
    std::string item;
    std::vector<std::string> items;

    while( std::getline( is, item, delim ) ) {
        items.push_back(item);
    }

    return items;
}

std::vector<std::string> list;
std::vector<texture> textures;

int main( int argc, const char **argv ) {

    if( argc < 3 ) {
        warn( argv[0], "atlas_output_image.png atlas_input_image [atlas_input_image2] [...]");
        return -1;
    }

    std::string output = argv[1];

    while( --argc > 1 ) {
        list.push_back(argv[argc]);
    }

    for( int i = 0; i < list.size(); ++i ) {
        std::string filename = list[i];

        texture t;
        if( filename[0] == '@' ) {
            std::ifstream ifs( filename.c_str() );
            if( ifs.is_open() ) {
                std::vector<std::string> lines = split(ifs,'\n');
                for( int i = 0; i < lines.size(); ++i ) {
                    list.push_back( lines[i] );
                }
                continue;
            }
        }
        else
        if( t.load(filename) ) {
            textures.push_back(t);
            continue;
        }

        warn("cannot find", filename);
    }

    if( textures.empty() ) {
        return -1;
    }

    //using namespace rbp;
    //MaxRectsBinPack mr;
    //std::vector<Rect> rects;

    TEXTURE_PACKER::TexturePacker *tp = TEXTURE_PACKER::createTexturePacker();
    tp->setTextureCount(textures.size());
    for( int i = 0; i < textures.size(); ++i ) {
        tp->addTexture(textures[i].w, textures[i].h);
    }

    int width;
    int height;
    bool is_power_of_two = true;
    bool has_one_pixel_border = true;
    int unused_area = tp->packTextures(width,height,is_power_of_two,has_one_pixel_border);

    //mr.Init( width, height );

    std::cout << "texture (" << width << 'x' << height << ") area: " << (100.0*unused_area/(width*height)) << "% free" << std::endl;

    dot::image mega( width, height );

    for( int i = 0; i < textures.size(); ++i ) {
        //rects.push_back( mr.Insert( textures[i].w, textures[i].h, MaxRectsBinPack::RectBestShortSideFit ) );

        int x, y, w, h;
        bool is_rotated = tp->getTextureLocation(i,x,y,w,h);
        texture &t = textures[i];
        t.x = x; t.y = y;
        t.u0 = float(  x) / width, t.u1 = float(  y) / height;
        t.v0 = float(w+x) / width, t.v1 = float(h+y) / height;
        t.is_rotated = is_rotated;
        // std::cout << textures[i].debug();
        dot::image img( t.name );
        mega = mega.paste( x, y, is_rotated ? img.rotate_left() : img );
    }

    //std::cout << "mr = " << mr.Occupancy() << std::endl;

    mega.save_as_png( output );

    TEXTURE_PACKER::releaseTexturePacker( tp );
    return 0;
}
