#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <deque>

#define ATTILA_VERSION "1.0.8" /* (2016/02/08): pump up libspot
#define ATTILA_VERSION "1.0.7" // (2015/09/28): faster image pasting
#define ATTILA_VERSION "1.0.6" // (2015/07/30): minimum width option
#define ATTILA_VERSION "1.0.5" // (2015/05/11): pump up libspot
#define ATTILA_VERSION "1.0.4" // (2015/04/09): enable mipmap generation
#define ATTILA_VERSION "1.0.3" // (2015/02/12): bugfixed error while handling @filelists
#define ATTILA_VERSION "1.0.2" // (2015/02/11): upgraded to latest spot lib
#define ATTILA_VERSION "1.0.1" // (2015/02/09): options, including image cropping and padding
#define ATTILA_VERSION "1.0.0" // (2015/02/06): initial version */


#include "deps/spot/spot.hpp"
#include "deps/packers/packer.hpp"
#include "deps/packers/MaxRectsBinPack.h"

namespace options { 
    bool        ENABLE_CROPPING      = false;
    bool        ENABLE_POT           = false;
    bool        ENABLE_EDGE          = false;
    bool        ENABLE_BLEEDING      = false;
    bool        ENABLE_MIPMAPS       = false;
    unsigned    ENABLE_MINIMUM_WIDTH = 0;
};

spot::image crop( const spot::image &src, unsigned *left = 0, unsigned *right = 0, unsigned *top = 0, unsigned *bottom = 0 ) {
    unsigned width = src.w;
    unsigned height = src.h;
    unsigned x0 = ~0, y0 = ~0, x1 = 0, y1 = 0;
    for( unsigned y = 0; y < height; y++ ) {
        for( unsigned x = 0; x < width; x++ ) {
            if( src.at(x,y).a != 0) {
                if( x < x0 ) x0 = x;
                if( x > x1 ) x1 = x;
                if( y < y0 ) y0 = y;
                if( y > y1 ) y1 = y;
            }
        }
    }
    if( left   ) *left = x0;
    if( top    ) *top = y0;
    if( right  ) *right = width - 1 - x1;
    if( bottom ) *bottom = height - 1 - y1;
    return src.copy(x0, y0, x1-x0, y1-y0);
}

spot::image &halve( spot::image &img, unsigned factor = 2 ) {
    if( factor >= 2 ) {
        for( unsigned y = 0, h = img.h / factor; y < h; ++y ) {
            for( unsigned x = 0, w = img.w / factor; x < w; ++x ) {
                img[x + y * w] = img[x * factor + y * factor * img.w];
            }
        }
        img.w /= factor;
        img.h /= factor;
        img.resize( img.w * img.h );
    }
    return img;
}

spot::image build_mipmaps( spot::image img, unsigned maxlevels = 0 ) {
    auto edge = img.w, bottom = img.h, left = edge - edge, top = bottom - bottom;
    spot::image cpy( edge + edge / 2, bottom );
    cpy = cpy.paste( left, top, img );
    do {
        img = halve( img );
        cpy = cpy.paste( edge, top, img );
        top += img.h;
    } while( img.w > 1 && img.h > 1 );
    return cpy;
}

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

    float w, h, x, y, u0, v0, u1, v1;
    float left, right, top, bottom;
    unsigned rotate;
    spot::image img;

    texture() :
    w(0), h(0), x(0), y(0), 
    u0(0), v0(0), u1(1), v1(1),
    left(0), right(0), top(0), bottom(0),
    rotate(0)
    {}

    bool load( const std::string &pathfile ) {
        //reset
        *this = texture();

        img.load(pathfile);
        if( img.empty() )
            return false;

        src = pathfile;
        w = img.w;
        h = img.h;

        if( options::ENABLE_CROPPING ) {
            // padding enabled. crop input image: shrink to fit blank pixels
            unsigned l, r, t, b;
            spot::image cropped = crop( img, &l, &r, &t, &b );
            if( !cropped.empty() ) {
                left = l;
                right = r;
                top = t;
                bottom = b;
                img = cropped;
                w = img.w;
                h = img.h;
            }
        }

        return true;
    }

    std::string json( const std::string &tab = std::string() ) const {
        std::stringstream ss;
        ss << tab << "\"" << normalize(src) << "\": {" << std::endl;
        ss << tab << tab << "\"src\": \"" << normalize(src) << "\", \"dst\": \"" << normalize(dst) << "\"," << std::endl;
        ss << tab << tab << "\"x\": " << x << ", \"y\": " << y << ", \"w\": " << w << ", \"h\": " << h << "," << std::endl;
        ss << tab << tab << "\"u0\": " << u0 << ", \"v0\": " << v0 << ", \"u1\": " << u1 << ", \"v1\": " << v1 << "," << std::endl;
        if( options::ENABLE_CROPPING ) {
        ss << tab << tab << "\"crop-left\": " << left << ", \"crop-right\": " << right << ", \"crop-top\": " << top << ", \"crop-bottom\": " << bottom << "," << std::endl;            
        }
        ss << tab << tab << "\"rotate\": " << rotate << ", \"hash\": " << std::hash<std::string>()(normalize(src)) << std::endl;
        ss << tab << "}";
        return ss.str();        
    }
};

// taken from https://github.com/r-lyeh/wire
std::deque<std::string> split( std::istream &is, const std::string &delimiters ) {
    std::string map( 256, '\0' );
    for( const unsigned char &ch : delimiters )
        map[ ch ] = '\1';
    std::deque<std::string> tokens(1);
    std::stringstream ss;
    ss << is.rdbuf();
    for( const unsigned char &ch : ss.str() ) {
        /**/ if( !map.at(ch)          ) tokens.back().push_back( char(ch) );
        else if( tokens.back().size() ) tokens.push_back( std::string() );
    }
    while( tokens.size() && !tokens.back().size() ) tokens.pop_back();
    return tokens;
}


int help( const char **argv ) {
    std::cerr << std::string() + argv[0] + " v" ATTILA_VERSION " (libspot v" SPOT_VERSION ") - lightweight atlas texture-packer - https://github.com/r-lyeh/attila\n\n";
    std::cerr << std::string() + "Usage:\n";
    std::cerr << std::string() + "\t" + argv[0] + " [options] output.img input.img [input2.img [...]]\n";
    std::cerr << std::string() + "\t" + argv[0] + " [options] output.img @imagelist.txt [@imagelist2.txt [...]]\n\n";
    std::cerr << std::string() + "Options:\n";
    std::cerr << std::string() + "\t--help                 prints help\n";
    std::cerr << std::string() + "\t--enable-bleeding      enables output alpha bleeding (default: disabled)\n";
    std::cerr << std::string() + "\t--enable-cropping      enables input alpha cropping (default: disabled)\n";
    std::cerr << std::string() + "\t--enable-edge          enables output blank pixel separator (default: disabled)\n";
    std::cerr << std::string() + "\t--enable-pot           enables output power-of-two texture (default: disabled)\n";
    std::cerr << std::string() + "\t--enable-mipmaps       enables mipmaps (default: disabled)\n";
    std::cerr << std::string() + "\t--enable-width WIDTH   enables minimum fixed width (in pixels) (default: 0)\n\n";
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

std::deque<std::string> list;
std::deque<texture> textures;

int attila( int argc, const char **argv ) {

    while( --argc > 0 ) {
        list.push_back(argv[argc]);
    }

    for( unsigned i = 0; i < list.size(); ) {
        const std::string filename = list[i];

        /**/ if( filename == "--enable-cropping") {
            list.erase( list.begin() + i );
            options::ENABLE_CROPPING = true;
        }
        else if( filename == "--enable-edge") {
            list.erase( list.begin() + i );
            options::ENABLE_EDGE = true;
        }
        else if( filename == "--enable-pot") {
            list.erase( list.begin() + i );
            options::ENABLE_POT = true;
        }
        else if( filename == "--enable-bleeding") {
            list.erase( list.begin() + i );
            options::ENABLE_BLEEDING = true;
        }
        else if( filename == "--enable-mipmaps") {
            list.erase( list.begin() + i );
            options::ENABLE_MIPMAPS = true;
        }
        else if( filename == "--enable-width") {
            list.erase( list.begin() + i );
            if( i-1 >= 0 && i-1 < list.size() ) {
                options::ENABLE_MINIMUM_WIDTH = std::strtoul( (list.begin() + --i)->c_str(), 0, 0 );
                list.erase( list.begin() + i );
            }
        }
        else if( filename == "--help" ) {
            return help( argv );
        }
        else if( filename[0] == '@' ) {
            std::ifstream ifs( &filename[1] );
            list.erase( list.begin() + i );
            if( ifs.good() ) {
                std::deque<std::string> lines = split(ifs,"\t\f\v\r\n");
                for( unsigned i = 0; i < lines.size(); ++i ) {
                    list.push_front( lines[i] );
                }
            }
        }
        else {
            ++i;
        }    
    }

    if( list.size() <= 1 ) {
        return help( argv );
    }

    std::string output = list.back();
    list.pop_back();

    for( const auto &filename : list ) {
        textures.push_back( texture() );
        if( !textures.back().load(filename) ) {
            std::cerr << "[FAIL] Attila - cannot load: " << filename << std::endl;
            textures.pop_back();
        }
    }

    if( textures.empty() ) {
        return -1;
    }

    std::sort( textures.begin(), textures.end(), []( texture &a, texture &b ) { return a.src < b.src; } );

    //using namespace rbp;
    //MaxRectsBinPack mr;
    //std::vector<Rect> rects;

    if( options::ENABLE_MINIMUM_WIDTH ) {
        textures.push_back( texture() );
        textures.back().w = options::ENABLE_MINIMUM_WIDTH;
        textures.back().h = 1;
    }

    TEXTURE_PACKER::TexturePacker *tp = TEXTURE_PACKER::createTexturePacker();
    tp->setTextureCount(textures.size());
    for( int i = 0; i < textures.size(); ++i ) {
        tp->addTexture(textures[i].w, textures[i].h);
    }

    if( options::ENABLE_MINIMUM_WIDTH ) {
        textures.pop_back();
    }

    int width;
    int height;
    int unused_area = tp->packTextures(width,height,options::ENABLE_POT != 0,options::ENABLE_EDGE != 0);

    if( options::ENABLE_MINIMUM_WIDTH ) {
        width -= options::ENABLE_MINIMUM_WIDTH;
    }

    //mr.Init( width, height );

    spot::image mega( width, height );
    //std::cerr << "[ OK ] Attila - Built " << width << 'x' << height << " texture...\n";

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

#if 0
        spot::image img;
        if( !img.load(t.src) ) {
            std::cerr << "[FAIL] Attila - cannot load: " << t.src << std::endl;            
            return -1;
        }
        if( options::ENABLE_CROPPING ) {
            img = crop(img);
        }
#else
        spot::image &img = t.img;
#endif
        mega.paste( mega, x, y, t.rotate ? img.rotate_left() : img );
    }

    std::cout << "{\n" << ss << "\n}\n" << std::endl; 

    //std::cout << "mr = " << mr.Occupancy() << std::endl;

    auto check_ext = []( std::string file, std::string ext ) -> bool {
        file = normalize( file );
        ext = normalize( ext );
        return file.size() < ext.size() ? false : file.substr( file.size() - ext.size() ) == ext;
    };

    if( options::ENABLE_MIPMAPS ) {
        mega = build_mipmaps( mega );
    }

    if( options::ENABLE_BLEEDING ) {
        mega = mega.bleed();
    }

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
    else if( check_ext( output, ".pug" ) ) {
        mega.save_as_pug( output, 70 );
    }
    else {
        mega.save_as_png( output );
    }

    TEXTURE_PACKER::releaseTexturePacker( tp );

    std::cerr << "[ OK ] Attila - Texture (" << width << 'x' << height << ") area: " << (100.0*unused_area/(width*height)) << "% free" << std::endl;
    return 0;
}

int main( int argc, const char **argv ) {
    try {
        return attila( argc, argv );
    }
    catch( std::exception &e ) {
        std::cerr << "[FAIL] Attila - " << e.what() << std::endl;
    }
    catch( ... ) {
        std::cerr << "[FAIL] Attila - unknown exception" << std::endl;
    }
    return -1;
}
