#include "gl/gl.hpp"

#include <cassert>

#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

//#pragma comment( lib, "user32.lib" )
//#pragma comment( lib, "gdi32.lib" )
//#pragma comment( lib, "shell32.lib" )

#include <cimg/cimg.h>
#pragma comment(lib,"shell32.lib")

#include <dot/dot.hpp>

#include "texture.hpp"


namespace moon9
{

texture::texture() : std::vector<pixel>(), w(0), h(0), iw(0), ih(0), u0(0), v0(0), u1(1), v1(1), delay(0.f), id(0), delegated(false)
{
    create();
}

texture::texture( size_t _w, size_t _h, const pixel &filler ) : std::vector<pixel>(_w*_h,filler),  w(_w), h(_h), iw(_w), ih(_h), u0(0), v0(0), u1(1), v1(1), delay(0.f), id(0), delegated(false)
{
    create();
}

texture::texture( const image &i, bool mirror_w, bool mirror_h ) : std::vector<pixel>(), w(0), h(0), iw(0), ih(0), u0(0), v0(0), u1(1), v1(1), delay(0.f), id(0), delegated(false)
{
    //create();

    if( load( i, mirror_w, mirror_h ) )
    {
        bind();
        filtering();
        submit();
    }
}

texture::texture( size_t _id ) : std::vector<pixel>(), w(0), h(0), iw(0), ih(0), u0(0), v0(0), u1(1), v1(1), delay(0.f), id(_id), delegated(true)
{}

texture::~texture()
{
    destroy();
}

void texture::create()
{
    if( id == 0 )
    {
        GLuint uid;
        glGenTextures( 1, &uid );
        id = uid;
    }
}

void texture::destroy()
{
    // if texture is valid, try to delete it
    if( id )
    {
        GLuint uid = id;
        // if we own texture, delete it
        if( !delegated )
            glDeleteTextures( 1, &uid );
        id = 0;
    }
}

void texture::bind( int unit ) const
{
    if( id )
    {
        GLuint uid = id;
        glEnable( GL_TEXTURE_2D );
        switch( unit )
        {
            case 0: glActiveTexture( GL_TEXTURE0 ); break;
            case 1: glActiveTexture( GL_TEXTURE1 ); break;
            case 2: glActiveTexture( GL_TEXTURE2 ); break;
            case 3: glActiveTexture( GL_TEXTURE3 ); break;
            case 4: glActiveTexture( GL_TEXTURE4 ); break;
            case 5: glActiveTexture( GL_TEXTURE5 ); break;
            case 6: glActiveTexture( GL_TEXTURE6 ); break;
            case 7: glActiveTexture( GL_TEXTURE7 ); break;
            default: break;
        }
        glBindTexture( GL_TEXTURE_2D, uid );
    }
}

void texture::unbind() const
{
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable( GL_TEXTURE_2D );
}

bool texture::load( const std::string &pathFile, bool mirror_w, bool mirror_h )
{
#if 0
    destroy();
    create();

    std::string error = image_load( pathFile, mirror_w, mirror_h, false, true, w, h, delay, *this );

    if( error.size() )
    {
        std::cerr << error << std::endl;
        return false;
    }

    return true;
#else
    return load( moon9::image( pathFile ), mirror_w, mirror_h );
#endif
}

void texture::clone( const moon9::texture &that )
{
    if( &that == this )
        return;

    destroy();

#if 0
    *this = that;
#else
    this->resize( that.size() );
    std::copy( that.begin(), that.end(), this->begin() );
    this->w = that.w;
    this->h = that.h;
    this->iw = that.iw;
    this->ih = that.ih;
    this->id = that.id;
    this->delay = that.delay;
    this->u0 = that.u0;
    this->v0 = that.v0;
    this->u1 = that.u1;
    this->v1 = that.v1;
#endif
    this->delegated = true;
}

void texture::move( const moon9::texture &that )
{
    if( &that == this )
        return;

    clone( that );

    this->delegated = false;
    that. delegated = true;
}

void texture::copy( const moon9::texture &that )
{
    if( &that == this )
        return;

#if 0
    *this = that;
    this->delay = that.delay;
    this->u0 = that.u0;
    this->v0 = that.v0;
    this->u1 = that.u1;
    this->v1 = that.v1;
    this->delegated = true;
#else
    clone( that );

    this->id = 0;
    this->delegated = false;
    if( that.id > 0 )
    {
        create();
        bind();
        filtering(); // @todo: what about 'that' filtering settings? lost forever? :s
        submit();
    }
#endif
}

bool texture::load( const image &_pic, bool mirror_w, bool mirror_h )
{
    image pic = _pic.to_rgba();

    if( mirror_w ) pic = pic.flip_w();
    if( mirror_h ) pic = pic.flip_h();

    // reset
#if 1
	*this = texture( pic.w, pic.h );
    this->delay = pic.delay;
#else
    copy( texture( pic.w, pic.h ) );
#endif

    if( !this->size() )
        return true;

    bool is_power_of_two_w = w && !(w & (w - 1));
    bool is_power_of_two_h = h && !(h & (h - 1));

    if( is_power_of_two_w && is_power_of_two_h )
    {
        this->resize( 0 );

        for( image::const_iterator it = pic.begin(), end = pic.end(); it != end; ++it )
            this->push_back( *it );
    }
    else
    {
        size_t nw = 1, nh = 1, atx, aty;
        while( nw < w ) nw <<= 1;
        while( nh < h ) nh <<= 1;

        // squared as well, cos we want them pixel perfect
        if( nh > nw ) nw = nh; else nh = nw;

        atx = (nw - w) / 2;
        aty = (nh - h) / 2;

        size_t pw = w, ph = h;

#if 1
        *this = texture( nw, nh, pixel::rgba() );
        this->delay = pic.delay;
#else
        copy( texture( nw, nh, pixel::rgba() ) );
#endif
        this->u0 = float(atx) / nw;
        this->v0 = float(aty) / nh;
        this->u1 = float(atx+pw) / nw;
        this->v1 = float(aty+ph) / nh;
        this->iw = pw;
        this->ih = ph;

        for( size_t y = 0; y < ph; ++y )
            for( size_t x = 0; x < pw; ++x )
                this->at( atx + x, aty + y ) = pic.at( x, y );
    }

    return true;
}

void texture::capture()
{
    capture( 0, 0 );
}

void texture::capture( int left, int bottom )
{
    assert( id > 0 );
    assert( w > 0 );
    assert( h > 0 );

/*
    if( size != same || !created )
        destroy();
        create();
        this->resize( w * h );
*/

    bind();

//  esto copia la vram a la textura que indiquemos, pero no se leer los pixels de ella aun (no se escriben en mem.data()!)

    std::vector<unsigned char> mem( w * h * 3 );

//  if(capture_depth) {
//      glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, xSize, ySize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0),
//      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
//  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 ); //mem.data() );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );    // pixel perfect
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );    // pixel perfect

    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, w, h, 0);
//  }

//  asi que uso esto...

    glReadPixels( left, bottom, w, h, GL_RGB, GL_UNSIGNED_BYTE, mem.data() );

    // invert image vertically
    for( size_t y = 0, it = 0; y < h; ++y )
    for( size_t x = 0; x < w; ++x, ++it )
    {
        size_t offset = ( (x) + (h-1 - y) * w ) * 3;

        float r = mem[ offset + 0 ] / 255.f;
        float g = mem[ offset + 1 ] / 255.f;
        float b = mem[ offset + 2 ] / 255.f;

        this->operator[]( it ) = moon9::pixel::rgba( r, g, b );
    }

    //submit();
}

void texture::filtering( int min, int mag, int wrap_s, int wrap_t, int wrap_r )
{
    if( min )
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min );

    if( mag )
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag );

    if( wrap_s )
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s );

    if( wrap_t )
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t );

    if( wrap_r )
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrap_r );
}

void texture::submit( int format, bool build_mip_maps )
{
    std::vector<unsigned char> pixels = ( format == GL_RGBA ? rgba_data() : rgb_data() );

    // Set unpack alignment to one byte

    GLint   UnpackAlignment;
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &UnpackAlignment );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    // Submit decoded data to texture
    GLint ret;
    if( build_mip_maps )
        ret = gluBuild2DMipmaps(GL_TEXTURE_2D, format, w, h, format, GL_UNSIGNED_BYTE, pixels.data() );
    else
        glTexImage2D(GL_TEXTURE_2D, 0 /*LOD*/, format, w, h, 0 /*border*/, format, GL_UNSIGNED_BYTE, pixels.data() );

    // Restore old unpack alignment
    glPixelStorei( GL_UNPACK_ALIGNMENT, UnpackAlignment );
}

size_t texture::delegate() const
{
    // ensure we wont destroy this texture on destructor
    delegated = true;

    return id;
}

std::vector<unsigned char> texture::rgba_data() const
{
    std::vector<unsigned char> pixels( w * h * 4 );
    pixels.resize(0);
    for( texture::const_iterator it = this->begin(), end = this->end(); it != end; ++it )
    {
        pixel p = it->clamp();

        pixels.push_back( (unsigned char)(p.r * 255.f) );
        pixels.push_back( (unsigned char)(p.g * 255.f) );
        pixels.push_back( (unsigned char)(p.b * 255.f) );
        pixels.push_back( (unsigned char)(p.a * 255.f) );
    }
    return pixels;
}

std::vector<unsigned char> texture::rgb_data() const
{
    std::vector<unsigned char> pixels( w * h * 3 );
    pixels.resize(0);
    for( texture::const_iterator it = this->begin(), end = this->end(); it != end; ++it )
    {
        pixel p = it->clamp();

        pixels.push_back( (unsigned char)(p.r * 255.f) );
        pixels.push_back( (unsigned char)(p.g * 255.f) );
        pixels.push_back( (unsigned char)(p.b * 255.f) );
    }
    return pixels;
}

std::vector<unsigned char> texture::a_data() const
{
    std::vector<unsigned char> pixels( w * h * 1 );
    pixels.resize(0);
    for( texture::const_iterator it = this->begin(), end = this->end(); it != end; ++it )
    {
        pixel p = it->clamp();

        pixels.push_back( (unsigned char)(p.a * 255.f) );
    }
    return pixels;
}

#if 0
void texture::sprite( /*const moon9::vec2 &pos,*/ float scale ) const
{
    moon9::disable::lighting dl;
    moon9::matrix::ortho flat; // 0,0 is top-left

    //moon9::matrix::translate t( pos );
    //glRasterPos2f

    moon9::style::texture tx( id );
    moon9::matrix::scale sc( moon9::vec3( w * scale, h * scale, 1 ) );
    moon9::quad sprite(1.f);
}
#endif

void texture::display( const std::string &title ) const
{
    const texture &pic = *this;

    cimg_library::CImg<unsigned char> ctexture( pic.w, pic.h, 1, 4, 0 );

    for( size_t y = 0; y < pic.h; ++y )
        for( size_t x = 0; x < pic.w; ++x )
        {
            pixel pix = pic.at( x, y ).clamp();

            ctexture( x, y, 0 ) = (unsigned char)(pix.r * 255.f);
            ctexture( x, y, 1 ) = (unsigned char)(pix.g * 255.f);
            ctexture( x, y, 2 ) = (unsigned char)(pix.b * 255.f);
            ctexture( x, y, 3 ) = (unsigned char)(pix.a * 255.f);
        }

#if 1
    ctexture
//        .deriche(0.5)
        .display( title.size() ? title.c_str() : custom("[\1] moon9::texture", this).c_str() );
#else
    cimg_library::CImgDisplay disp;
    disp.resize( pic.w, pic.h );

    ctexture.display( disp );
    while(!disp.is_closed());
#endif
}

} // namespace moon9


#if 0

namespace
{
    void setup_filtering( bool high_quality )
    {
    #if 0

        // Nearest Filtered Texture
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexImage2D(...)

        // Linear Filtered Texture
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexImage2D(...)

        // MipMapped Texture
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
        gluBuild2DMipmaps(...)

    #endif

        /* blurry/blocky

        ref: http://gregs-blog.com/2008/01/17/opengl-texture-filter-parameters-explained/
        MAG_FILTER/MIN_FILTER                 Bilinear Filtering          Mipmapping
                                                Near   Far
        GL_NEAREST / GL_NEAREST_MIPMAP_NEAREST  Off    Off                Standard
        GL_NEAREST / GL_LINEAR_MIPMAP_NEAREST   Off    On                 Standard (* chars)
        GL_NEAREST / GL_NEAREST_MIPMAP_LINEAR   Off    Off                Use trilinear filtering
        GL_NEAREST / GL_LINEAR_MIPMAP_LINEAR    Off    On                 Use trilinear filtering (* chars)
        GL_NEAREST / GL_NEAREST                 Off    Off                None
        GL_NEAREST / GL_LINEAR                  Off    On                 None (* scene)
        GL_LINEAR / GL_NEAREST_MIPMAP_NEAREST   On     Off                Standard
        GL_LINEAR / GL_LINEAR_MIPMAP_NEAREST    On     On                 Standard
        GL_LINEAR / GL_NEAREST_MIPMAP_LINEAR    On     Off                Use trilinear filtering
        GL_LINEAR / GL_LINEAR_MIPMAP_LINEAR     On     On                 Use trilinear filtering
        GL_LINEAR / GL_NEAREST                  On     Off                None
        GL_LINEAR / GL_LINEAR                   On     On                 None

        (*) intended for prjx

        @todo
        enum { near_blurry, near_blocky }
        enum { far_blurry, far_blocky }
        enum { mipmap_none, mipmap_std, mipmap_trilinear };
        mask -> near_blocky | far_blurry | mipmap_std

        */

        if( high_quality )
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
}
        struct texture : render_detail::nocopy
        {
            texture( const std::string &pathFile, const bool high_quality = true ) : id( 0 )
            {
                id = manager( true, pathFile, high_quality );

                glEnable(GL_TEXTURE_2D);
            }

            explicit texture( size_t texture_id ) : id( texture_id )
            {
                glEnable(GL_TEXTURE_2D);

                glBindTexture(GL_TEXTURE_2D, id );
            }

            ~texture()
            {
                #if 0
                unbind();
                #endif

                glDisable(GL_TEXTURE_2D);
            }
/*
            void unbind()
            {
                if( id )
                {
                    id = manager( false, pathFile );
                }
            }
*/
            static void unbind( const std::string &pathFile )
            {
                manager( false, pathFile );
            }



            static size_t create( int num )
            {
                GLuint id;
                glGenTextures( 1, &id );
                return id;
            }

            static void bind( size_t _id )
            {
                GLuint id = _id;
                glBindTexture(GL_TEXTURE_2D, id );
                //std::cout << "texture #" << id << std::endl;
            }

            static void apply( const std::string &pathFile, bool mirror_h = false, bool mirror_v = false )
            {
                if( !pathFile.size() )
                    return;

                std::cout << "Registering texture: " << pathFile << std::endl;

                // decode
                int imageWidth = 0, imageHeight = 0, imageBpp;
                std::vector<stbi_uc> image;

                {
                    moon9::strings pathFileExt = moon9::string( pathFile ).tokenize("|");
                    int subframe = ( pathFileExt.size() > 1 ? pathFileExt[1].as<int>() : 0 );

                    stbi_gif_subframe_selector = subframe;
                    stbi_uc *imageuc = stbi_load( pathFileExt[0].c_str(), &imageWidth, &imageHeight, &imageBpp, 4 );

                    if( !imageuc )
                    {
                        // failed
                        std::cerr << moon9::string( "Cant find texture '\1'\n", pathFile );
                        // assert( false );
                        // yellow/black texture instead?
                        return;
                    }

                    bool is_power_of_two_w = imageWidth && !(imageWidth & (imageWidth - 1));
                    bool is_power_of_two_h = imageHeight && !(imageHeight & (imageHeight - 1));

                    if( is_power_of_two_w && is_power_of_two_h )
                    {
                        image.resize( imageWidth * imageHeight * 4 );

                        //memcpy( image.data(), imageuc, imageWidth * imageHeight * 4 );
                        memcpy( &image[0], imageuc, imageWidth * imageHeight * 4 );
                    }
                    else
                    {
                        int nw = 1, nh = 1, atx, aty;
                        while( nw < imageWidth ) nw <<= 1;
                        while( nh < imageHeight ) nh <<= 1;

                        // squared as well, cos we want them pixel perfect
                        if( nh > nw ) nw = nh; else nh = nw;

                        image.resize( nw * nh * 4, 0 );
                        atx = (nw - imageWidth) / 2;
                        aty = (nh - imageHeight) / 2;

                        //std::cout << moon9::string( "\1x\2 -> \3x\4 @(\5,\6)\n", imageWidth, imageHeight, nw, nh, atx, aty);

                        for( int y = 0; y < imageHeight; ++y )
//                            memcpy( &((stbi_uc*)image.data())[ ((atx)+(aty+y)*nw)*4 ], &imageuc[ (y*imageWidth) * 4 ], imageWidth * 4 );
                            memcpy( &((stbi_uc*)&image[0])[ ((atx)+(aty+y)*nw)*4 ], &imageuc[ (y*imageWidth) * 4 ], imageWidth * 4 );

                        imageWidth = nw;
                        imageHeight = nh;
                    }

                    stbi_image_free( imageuc );
                }

                if( mirror_h && !mirror_v )
                {
                    std::vector<stbi_uc> mirrored( image.size() );

                    for( int c = 0, y = 0; y < imageHeight; ++y )
                    for( int x = imageWidth - 1; x >= 0; --x )
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 0 ],
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 1 ],
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 2 ],
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 3 ];

                    image = mirrored;
                }
                else
                if( mirror_v && !mirror_h )
                {
                    std::vector<stbi_uc> mirrored( image.size() );

                    for( int c = 0, y = imageHeight - 1; y >= 0; --y )
                    for( int x = 0; x < imageWidth; ++x )
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 0 ],
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 1 ],
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 2 ],
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 3 ];

                    image = mirrored;
                }
                else
                if( mirror_h && mirror_v )
                {
                    std::vector<stbi_uc> mirrored( image.size() );

                    for( int c = 0, y = imageHeight - 1; y >= 0; --y )
                    for( int x = imageWidth - 1; x >= 0; --x )
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 0 ],
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 1 ],
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 2 ],
                        mirrored[ c++ ] = image[ ( x + y * imageWidth ) * 4 + 3 ];

                    image = mirrored;

                    std::swap( imageWidth, imageHeight );
                }

                // submit decoded data to texture

                GLint   UnpackAlignment;

                // Here we bind the texture and set up the filtering.
                //glBindTexture(GL_TEXTURE_2D, id);

                // Set unpack alignment to one byte
                glGetIntegerv( GL_UNPACK_ALIGNMENT, &UnpackAlignment );
                glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

                GLenum type = GL_RGBA; //GL_ALPHA, GL_LUMINANCE, GL_RGBA

                #if 1 //if buildmipmaps

                GLint ret = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, imageWidth, imageHeight,
//                                                type, GL_UNSIGNED_BYTE, image.data());
                                                type, GL_UNSIGNED_BYTE, &image[0]);

                #else // else...

                glTexImage2D(GL_TEXTURE_2D, 0 /*LOD*/, GL_RGBA, imageWidth, imageHeight,
                                0 /*border*/, type, GL_UNSIGNED_BYTE, image.data());

                #endif

                // Restore old unpack alignment
                glPixelStorei( GL_UNPACK_ALIGNMENT, UnpackAlignment );
            }

            static void config_filtering( bool high_quality )
            {
#if 0

                // Nearest Filtered Texture
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
                glTexImage2D(...)

                // Linear Filtered Texture
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
                glTexImage2D(...)

                // MipMapped Texture
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
                gluBuild2DMipmaps(...)

#endif

                /* blurry/blocky

                ref: http://gregs-blog.com/2008/01/17/opengl-texture-filter-parameters-explained/
                MAG_FILTER/MIN_FILTER                 Bilinear Filtering          Mipmapping
                                                        Near   Far
                GL_NEAREST / GL_NEAREST_MIPMAP_NEAREST  Off    Off                Standard
                GL_NEAREST / GL_LINEAR_MIPMAP_NEAREST   Off    On                 Standard (* chars)
                GL_NEAREST / GL_NEAREST_MIPMAP_LINEAR   Off    Off                Use trilinear filtering
                GL_NEAREST / GL_LINEAR_MIPMAP_LINEAR    Off    On                 Use trilinear filtering (* chars)
                GL_NEAREST / GL_NEAREST                 Off    Off                None
                GL_NEAREST / GL_LINEAR                  Off    On                 None (* scene)
                GL_LINEAR / GL_NEAREST_MIPMAP_NEAREST   On     Off                Standard
                GL_LINEAR / GL_LINEAR_MIPMAP_NEAREST    On     On                 Standard
                GL_LINEAR / GL_NEAREST_MIPMAP_LINEAR    On     Off                Use trilinear filtering
                GL_LINEAR / GL_LINEAR_MIPMAP_LINEAR     On     On                 Use trilinear filtering
                GL_LINEAR / GL_NEAREST                  On     Off                None
                GL_LINEAR / GL_LINEAR                   On     On                 None

                (*) intended for prjx

                @todo
                enum { near_blurry, near_blocky }
                enum { far_blurry, far_blocky }
                enum { mipmap_none, mipmap_std, mipmap_trilinear };
                mask -> near_blocky | far_blurry | mipmap_std

                */

                if( high_quality ) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
                }
                else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }

            size_t id;

            private:

            static size_t manager( bool acquire, const std::string &pathFile, const bool high_quality = true )
            {
                static std::map< std::string, GLuint > map;
                static moon9::mutex mutex;
                moon9::blocker blocker( &mutex );

                std::map< std::string, GLuint >::iterator id_it = map.find( pathFile );
                bool found = ( id_it != map.end() );

                // delete ?
                if( !acquire )
                {
                    assert( found );
                    glDeleteTextures( 1, &id_it->second );
                    map.erase( id_it );
                    return 0;
                }

                // if acquire...

                // previously acquired?
                if( found )
                {
                    //std::cout << "Caching texture" << std::endl;

                    assert( id_it->second != 0 );
                    bind( id_it->second );
                    return id_it->second;
                }

                GLuint id = create(1);
                bind( id );
                apply( pathFile );
                config_filtering( high_quality );

                // cache insertion
                map[ pathFile ] = id;

                return id;
            }


            /*
                Contrast
                Crop
                Extract alpha channel
                Convert to grayscale
                Intensity
                Lightness
                Color quantization (conversion of true-color images to 256 colors)
                Bilinear resize
                Gaussian blur
                Threshold

                blend(texture &t, float01)
                or(texture &t, float01) //(saturated)
                and(texture &t, float01)

                conversion policies:
                onSize: clip/stretch, scale/bilinear/trilinear/anisotropic
                onColor: uniform/palettized/etc
            */
        };



#endif
