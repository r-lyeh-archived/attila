#include <iostream>

#include "deps/spot/spot.hpp"
// CImg.h code following
#include "deps/spot/samples/cimg.h"
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
void display( const spot::image &pic, const char *title = "" ) {
    if( pic.size() ) {
        cimg_library::CImg<unsigned char> ctexture( pic.w, pic.h, 1, 4, 0 );
        for( size_t y = 0; y < pic.h; ++y ) {
            for( size_t x = 0; x < pic.w; ++x ) {
                spot::pixel pix = pic.at( x, y ).clamp().to_rgba();
                ctexture( x, y, 0 ) = (unsigned char)(pix.r * 255.f);
                ctexture( x, y, 1 ) = (unsigned char)(pix.g * 255.f);
                ctexture( x, y, 2 ) = (unsigned char)(pix.b * 255.f);
                ctexture( x, y, 3 ) = (unsigned char)(pix.a * 255.f);
            }
        }
        ctexture.display( title );
    }
}

#include "deps/packers/packer.hpp"
#include "deps/packers/MaxRectsBinPack.h"

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <utility>

std::string normalize( std::string filename ) {
    for( auto &ch : filename ) {
        if( ch >= 'A' && ch <= 'Z' ) {
            ch = ch - 'A' + 'a';
        }
        else
        if( ch == '\\' ) {
            ch = '/';
        }
    }
    return filename;
}

struct texture {
    std::string src, dst;
    std::vector<unsigned char> data;

    float w, h, x, y, u0, v0, u1, v1;
    unsigned rotate;

    texture() : w(0), h(0), x(0), y(0), u0(0), v0(0), u1(1), v1(1), rotate(0)
    {}

    bool load( const std::string &pathfile ) {
        //reset
        *this = texture();

        spot::image img( pathfile );
        if( !img.size() )
            return false;

        this->src = pathfile;
        this->w = img.w;
        this->h = img.h;
        this->data = img.rgba_data();

        return true;
    }

    std::string json( const std::string &tab = std::string() ) const {
        std::stringstream ss;
        ss << tab << "\"" << normalize(src) << "\": {" << std::endl;
        ss << tab << tab << "\"src\": \"" << normalize(src) << "\", \"dst\": \"" << normalize(dst) << "\"," << std::endl;
        ss << tab << tab << "\"x\": " << x << ", \"y\": " << y << ", \"w\": " << w << ", \"h\": " << h << "," << std::endl;
        ss << tab << tab << "\"u0\": " << u0 << ", \"v0\": " << v0 << ", \"u1\": " << u1 << ", \"v1\": " << v1 << "," << std::endl;
        ss << tab << tab << "\"rotate\": " << rotate << ", \"hash\": " << std::hash<std::string>()(normalize(src)) << std::endl;
        ss << tab << "}";
        return ss.str();        
    }
};

void warn( std::ostream &out = std::cout, const std::string &a = std::string(), const std::string &b = std::string() ) {
    if( a.size() ) out << a << ' ';
    if( b.size() ) out << b << ' ';
    out << std::endl;
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
        std::cerr << std::string() + argv[0] + " v1.0.0 - lightweight atlas texture/image packer - https://github.com/r-lyeh/attila\n\n";
        std::cerr << std::string() + "Usage:\n\t" + argv[0] + " output.img input.img [input2.img [...]]\n\n";
        std::cerr << "Notes:\n\tA JSON table of contents (toc) is written to stdout. Redirect it to a file if needed.\n\n";

        std::string sep1 = "\tInput image formats: ";
        for( auto &fmt : spot::list_supported_inputs() ) {
            std::cerr << sep1 << fmt;
            sep1 = ", ";
        }
        std::cerr << std::endl;

        std::string sep2 = "\tOutput image formats: ";
        for( auto &fmt : spot::list_supported_outputs() ) {
            std::cerr << sep2 << fmt;
            sep2 = ", ";
        }
        std::cerr << std::endl;

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

        warn(std::cerr, "cannot find", filename);
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

    spot::image mega( width, height );
    std::string ss, sep = "";

    for( int i = 0; i < textures.size(); ++i ) {
        //rects.push_back( mr.Insert( textures[i].w, textures[i].h, MaxRectsBinPack::RectBestShortSideFit ) );

        texture &t = textures[i];

        int x, y, w, h;
        t.rotate = tp->getTextureLocation(i,x,y,w,h) ? 90 : 0;
        t.x = x; t.y = y;
        t.u0 = float(  x) / width, t.u1 = float(  y) / height;
        t.v0 = float(w+x) / width, t.v1 = float(h+y) / height;
        t.dst = output;
        
        ss += sep + textures[i].json("\t");
        sep = ",\n";
        
        spot::image img( t.src );
        mega = mega.paste( x, y, t.rotate ? img.rotate_left() : img );
    }

    std::cout << "{\n" << ss << "\n}\n" << std::endl; 

    //std::cout << "mr = " << mr.Occupancy() << std::endl;

    auto check_ext = []( std::string file, std::string ext ) -> bool {
        file = normalize( file );
        ext = normalize( ext );
        return file.size() < ext.size() ? false : file.substr( file.size() - ext.size() ) == ext;
    };

    /**/ if( check_ext( output, ".bmp" ) ) {
        mega.save_as_bmp( output );
    }
    else if( check_ext( output, ".dds" ) ) {
        mega.save_as_dds( output );
    }
    else if( check_ext( output, ".jpg" ) ) {
        mega.save_as_jpg( output, 70 );
    }
    else if( check_ext( output, ".png" ) ) {
        mega.save_as_png( output );
    }
    else if( check_ext( output, ".tga" ) ) {
        mega.save_as_tga( output );
    }
    else if( check_ext( output, ".webp" ) ) {
        mega.save_as_webp( output, 70 );
    }
    else {
        mega.save_as_png( output );
    }

    TEXTURE_PACKER::releaseTexturePacker( tp );

    std::cerr << "[ OK ] Attila - Texture (" << width << 'x' << height << ") area: " << (100.0*unused_area/(width*height)) << "% free" << std::endl;
    return 0;
}
