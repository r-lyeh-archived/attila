// 	std::transform(filename.begin(), filename.end(), filename.begin(), std::ptr_fun(std::tolower));


// to do : remove all initial characters which are duplicated in *all* entries
// to do : change \ -> / in filenames
// to do : png writing support

// C Headers

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>

// C++ Headers

#include <string>
#include <vector>
#include <fstream>		//fout
#include <sstream>
#include <locale>		//tolower
#include <algorithm>	//transform, replace, replace_if, replace_copy, ...

// Custom headers

#include <dot/dot.hpp>			// custom image library
#include "TexturePacker.h"		// custom texture atlas alg

// Config

// PS3 version need atlas image to be split in 4096x4096 .dds texture files
//#define PS3_VERSION

#define VERSION_INFO \
		"Texture Atlas v1.0 by rlyeh (c) 2010, MIT license.\n" \
		"Using TexturePacker library by John W. Ratcliff.\n" \
		"Using stb_image 1.18 library by Sean Barrett.\n" \
		"Using stb_image DDS extension library by Jonathan Leigh Dummer.\n"

namespace options
{
	bool
		bSubdirs = false,
		bQuiet = false,
		bShowHelp = false;
}

// Program

#define printf( ... ) do if( !options::bQuiet ) printf( __VA_ARGS__ ); while( 0 )

template <typename Elem, typename Traits, typename Ax>
std::basic_string<Elem, Traits, Ax> &tolower(std::basic_string<Elem, Traits, Ax> *str)
{
	std::use_facet<std::ctype<Elem> >(std::locale()).tolower(&*str->begin(), &*str->end());
	return *str;
}

// Copy conversions
template <typename Elem, typename Traits, typename Ax>
std::basic_string<Elem, Traits, Ax> tolower(std::basic_string<Elem, Traits, Ax> str)
{
	std::use_facet<std::ctype<Elem> >(std::locale()).tolower(&*str.begin(), &*str.end());
	return str;
}

//! std::string s;
//! tolower(s.begin(), s.end());
template < typename ForwardIterator >
ForwardIterator makelower(ForwardIterator first, ForwardIterator last,
const std::locale& locale_ref = std::locale())
{
for(; first!=last; ++first)
*first = std::tolower(*first, locale_ref);
return first;
}

//wstring
struct string : public std::string //std::basic_string<char>
{
	//http://msdn.microsoft.com/en-us/library/56e442dc(VS.71).aspx

	string tolower()
	{
		string s = (*this);

		makelower( s.begin(), s.end() );

		return s;
	}

	/*
	string &tolower()
	{
		std::transform( begin(), end(), begin(), (int(*)(int))std::tolower);

		return (*this);
	}
	*/

	int find( const string &target ) const
	{
		int amount = 0, pos = 0;

		while( (pos = (int)( std::string::find( target, pos ) + 1)) != (int)(std::string::npos + 1))
			amount++;

		return amount;
	}

	string replace( const string &target, const string &replacement ) const
	{
		string s = *this;

		size_t found;

		while( ( found = s.std::string::find( target ) ) != string::npos )
		{
			s.std::string::replace( found, target.length(), replacement );
		}

		return s; //.substr( 0, s.length() - 3 );
	}

	//--- Path !

	//std::vector<std::pair<std::string, std::string>> path()
	//{} return vect tokenize("\\/").[] -> file ext

	string shortname() const
	{
		// branch according to OS

		size_t begin, end;

		if( true )
			begin = find_last_of("\\"), end = find_last_of(".");
		else
			begin = find_last_of("//"), end = find_last_of(".");

		if( begin == std::string::npos )
			begin = 0;

		return substr( begin, end );
	}

	string extension() const
	{
		return substr( find_last_of(".") + 1 );
	}

	string remove_extension() const
	{
		return substr( 0, find_last_of(".") );
	}

	string sanitize() const
	{
		// branch according to OS

		if( true )
			return replace( "\\", "/" );
		else
			return replace( "/", "\\" );
	}

	//

	string() : std::string()
	{}

	string( const string &s )
	{
		operator=( s );
	}

	string( string *s )
	{
		operator=( *s );
	}

	string( const std::string &s )
	{
		std::string::operator=( s );
	}

	string( const char *fmt, ... )
	{
		va_list args;
		char buffer[ 2048 ];

		va_start( args, fmt );
		std::vsprintf( buffer, fmt, args );
		va_end( args );

		std::string::operator=( buffer );
	}

	string( const char *fmt, va_list args )
	{
		char buffer[ 2048 ];

		vsprintf( buffer, fmt, args );

		std::string::operator=( buffer );
	}

	template <class T>
	string( T t )
	{
	  std::ostringstream oss;
	  oss << std::dec << t;
	  operator=( oss.str() );
	}

	~string()
	{}


	void operator =( const string &s )
	{
		std::string::operator=( s.str() );
	}
/*
	void operator =( const std::string &s )
	{
		std::string::operator=( s );
	}
*/
	std::string str() const
	{
		return std::string( c_str() );
	}
};

void progress( const char *str )
{
	static string last;
	static int counter = 0;

	if( str && last != str )
	{
		printf("- %s\n", str );
		last = str;
	}

	return;

	// Print

	char progress_indicator[] = "\\|/-";

	string current = string("%c %s...%s", progress_indicator[ (counter++) % 4 ], str ? str : last.c_str(), str ? "" : " done!" );

	printf( "%s%*s%s", current.c_str(), last.size() > current.size() ? last.size() - current.size() : 0, str ? "\r" : "\r-\n");

	// Changed?

	last = current;
}

int gRenderSet = 0;
int gCommonCharacters = 0x4000;
string gLastFile;

struct image
{
	enum eSaveType
	{
		PPM,
		BMP,
		TGA,
		DDS
	};

	string name;

	int x, y, w, h, ow, oh, rotation;

	dot::image RGBA;

	image()	: x(0),y(0),w(0),h(0),ow(0),oh(0),rotation(0)//,RGBA( NULL )
	{}

	image( const image &i ) : x(0),y(0),w(0),h(0),ow(0),oh(0),rotation(0)
	{
		operator=( i );
	}

	image( image *pi ) : x(0),y(0),w(0),h(0),ow(0),oh(0),rotation(0)
	{
		operator=( *pi );
	}

	~image()
	{}

	image & operator=( const image &i )
	{
		//assert( false );

		name = i.name;
		x = i.x;
		y = i.y;
		w = i.w;
		h = i.h;
		ow = i.ow;
		oh = i.oh;
		rotation = i.rotation;
		RGBA = i.RGBA;

		//(const_cast<image &>(i)).RGBA = NULL;

		return *this;
	}

	bool write( const char *filePath, eSaveType type )
	{
		printf("- Writing texture atlas '%s' (%d x %d) ... ", filePath, w, h);

		assert( RGBA.size() );

#ifdef PS3_VERSION

		// force 4096 height

		if( h > 4096 )
			h = 4096;

		for( ; h < 4096 ; h++ )
			for( int x = 0; x < w; x++)
				RGBA.push_back( 0x80808080 );

#endif
		RGBA.w = w;
		RGBA.h = h;

		if( 1 )
		{
			if( type == PPM )
			{
				FILE *f = fopen(filePath, "w");

				if( f )
				{
					auto &rgba = RGBA.rgba_data();
					unsigned char *color = rgba.data();

					fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);

					for (size_t i=0; i< w*h; i++, color += 4)
						fprintf(f,"%d %d %d ", (color[0]>>0) & 255, (color[1]>>0) & 255, (color[2]>>0) & 255);

					fclose( f );

					printf("done!\n");
					return true;
				}
				else
				{
					printf("failed!\n");
					return false;
				}
			}

			if( type == TGA && RGBA.save_as_tga( filePath ) )
			{
				printf("done!\n");
				return true;
			}

/*
			if( type == BMP && RGBA.save_as_bmp( filePath ) )
			{
				printf("done!\n");
				return true;
			}
*/

			if( type == DDS && RGBA.save_as_dds( filePath ) )
			{
				printf("done!\n");
				return true;
			}
		}

		printf("failed! Error: cant write '%s' file\n", filePath);
		return false;
	}
};

class atlas
{
	private:

		int m_Width, m_Height;
		size_t m_ProcessedFiles;
		float m_FreeSpace;

		std::vector< std::string > files, inputs, outputs;
		std::vector< image > images;

		TEXTURE_PACKER::TexturePacker *tp;

	public:

		atlas() //: m_ProcessedFiles( 0 )
		{
		}

		~atlas()
		{
		}

		private:

		void scan_file( std::string pathfile, bool subdirs )
		{
			printf("- Parsing directory...");

			{
				char command[2048];
				sprintf( command, "dir \"%s\" %s /b /o:n > $$temp$$ 2> nul", pathfile.c_str(), subdirs ? "/s" : "" );
				system( command );
			}

			printf(" done!\n");

			FILE *in = fopen("$$temp$$", "rb");

			if( in )
			{
				char buffer[ 2048 ];

				while( fgets( buffer, 2048, in ) )
				{
					char *pos = &buffer[ strlen(buffer) - 1 ];

					while( *pos == '\n' || *pos == '\r' )
						*pos-- = '\0';

					if( string(buffer) != "$$temp$$" )
					{
						files.push_back( string( buffer ).tolower() );

						if( files.size() == 1 )
						{
							gLastFile = files.back();
						}

						int lCommonCharacters = strspn( files.back().c_str(), gLastFile.c_str() );
						gCommonCharacters = ( lCommonCharacters < gCommonCharacters ? lCommonCharacters : gCommonCharacters );
						gLastFile = files.back();
					}
				}

				fclose( in );
				remove("$$temp$$");
			}

			for( int i = 0; i < files.size(); i++ )
			{
				const char *buffer = files[i].c_str();

				static int c = 0;
				progress( string("Reading file %d... %s", ++c, buffer).c_str() );

				int w, h, bpp;

				//optimize me
				dot::image img;

				if( img.load(buffer) )
				{
					image i;

					i.name = buffer;
					i.ow = img.w;
					i.oh = img.h;

					images.push_back( i );
				}

				m_ProcessedFiles++;
			}

			files.clear();

			progress( NULL );
		}

		bool save_subset( string filename, image &im, std::vector< image > &images )
		{
			if( !images.size() )
				return false;

			string extension = string( filename ).extension().tolower();

			if( gRenderSet )
			{
				filename = string( "%s_%d.%s", filename.remove_extension().c_str(), gRenderSet, extension.c_str() );
			}

			string shortname = string( filename ).shortname().tolower();

			bool isDDS = ( extension == "dds" );
			bool isPPM = ( extension == "ppm" );
			bool isBMP = ( extension == "bmp" );
			bool isTGA = ( extension == "tga" );
			bool isXML = ( extension == "xml" );
			bool isTXT = ( extension == "txt" );
			bool isHPP = ( extension == "hpp" || extension == "h" );

			if( isDDS || isPPM || isBMP || isTGA )
			{
				image::eSaveType type[] = { image::DDS, image::DDS, image::PPM, image::BMP, image::TGA };

				return im.write( filename.c_str(), type[ isDDS * 1 | isPPM * 2 | isBMP * 3 | isTGA * 4 ] );
			}

			if( isXML );

			if( isTXT )
			{
				printf("- Writing texture info '%s' ... ", filename.c_str() );

				std::ofstream fout;

				fout.open( filename.c_str(), std::ios::app );    // open file for appending

				if( fout.fail() )
				{
					printf("failed\n");
					return false;
				}

				fout << string(	"; Generated by TextureAtlas. Data per line: \"id\" set x1 y1 x2 y2 w h rotation\n"
								"; \n"
								"; \n"
								"; Atlas size %d x %d, %03.2f%% unused space\n"
								"; \n", m_Width, m_Height, m_FreeSpace);

				for( int i = 0; i < images.size(); i++ )
				{
					string name;

					if( gCommonCharacters < images[i].name.size() )
					{
						name = images[i].name.substr( gCommonCharacters, images[i].name.size() ).c_str();
					}
					else
					{
						name = images[i].name;
					}

					fout << string("\"%s\" %d %d %d %d %d %d %d %d ; %d \n", name.sanitize().c_str(), gRenderSet, images[i].x, images[i].y, images[i].x+images[i].w, images[i].y+images[i].h, images[i].w, images[i].h, images[i].rotation, i);
				}

				fout.close();

				if( fout.fail() )
				{
					printf("failed\n");
					return false;
				}

				printf("done!\n");
			}

			if( isHPP )
			{
				printf("- Writing texture info '%s' ... ", filename.c_str() );

				std::ofstream fout;

				fout.open( filename.c_str(), std::ios::app );    // open file for appending

				if( fout.fail() )
				{
					printf("failed\n");
					return false;
				}

				fout << string( "#ifndef __TEXTURE_ATLAS_COMMON_H__\n"
								"#define __TEXTURE_ATLAS_COMMON_H__\n"
								"\n"
								"struct TextureAtlasEntry { const char *id; size_t set,x1,y1,x2,y2,w,h,rotation; float u0,v0,u1,v1; };\n"
								"\n"
								"#endif /*__TEXTURE_ATLAS_COMMON_H__*/\n"
								"\n"
								"TextureAtlasEntry %s_h[] = {\n"
								"/* Atlas size %d x %d, %03.2f%% unused space */\n", shortname.c_str(), m_Width, m_Height, m_FreeSpace );

				for( int i = 0; i < images.size(); i++ )
				{
					string name;

					if( gCommonCharacters < images[i].name.size() )
					{
						name = images[i].name.substr( gCommonCharacters, images[i].name.size() ).c_str();
					}
					else
					{
						name = images[i].name;
					}

					fout << string("/* %12d */ {\"%s\",%d,%d,%d,%d,%d,%d,%d,%d,%d.f/%d.f,%d.f/%d.f,%d.f/%d.f,%d.f/%d.f}%s",
						i, name.sanitize().c_str(),
						gRenderSet, images[i].x, images[i].y, images[i].x+images[i].w, images[i].y+images[i].h, images[i].w, images[i].h, images[i].rotation,
						images[i].x, m_Width, images[i].y, m_Height, images[i].x+images[i].w, m_Width, images[i].y+images[i].h, m_Height,
						i == images.size() - 1 ? "\n};\n\n" : ",\n" );
				}

				fout.close();

				if( fout.fail() )
				{
					printf("failed\n");
					return false;
				}

				printf("done!\n");
			}

			return false;
		}

		void render_subset( image &target, std::vector< image > &images )
		{
			if( images.size() )
			{
				target.w = m_Width;
				target.h = m_Height;

				target.RGBA = dot::image( m_Width, m_Height );
				target.RGBA.resize(0);

				for( int i = 0; i < m_Width * m_Height; ++i )
					target.RGBA.push_back( dot::pixel::RGBA(0x80808080) );

				for( int i = 0; i < images.size(); i++)
				{
					progress("Rendering subset");

#ifdef PS3_VERSION

					if( images[i].name == "$$temp$$" )
						continue;

#endif

					int w, h, bpp;
					dot::image img;

					bool loaded = img.load( images[i].name.c_str() );
					assert( loaded );

					int iw = images[i].w, ih = images[i].h, ix = images[i].x, iy = images[i].y, ir = images[i].rotation;

					if( !ir ) {
						target.RGBA = target.RGBA.paste( ix, iy, img );
					}
					else {
						target.RGBA = target.RGBA.paste( ix, iy, img.rotate_left() );
					}
				}
			}

			progress( NULL );
		}

		bool pack_subset( std::vector< image > &images )
		{
			if( images.size() )
			{
				tp = TEXTURE_PACKER::createTexturePacker();

				assert( tp );

#ifdef PS3_VERSION

				// force 4096 width

				images.push_back( image() );
				images.back().ow = 4096;
				images.back().oh = 1;
				images.back().name = "$$temp$$";

#endif

				tp->setTextureCount( images.size() );

				for( int i = 0; i < images.size(); i++ )
				{
					tp->addTexture( images[i].ow, images[i].oh );
				}

				// it seems .DDS file specs do need bForcePowerOfTwo to be true in all cases
				bool bForcePowerOfTwo = true, bAddOnePixelBorder = false;

				m_FreeSpace = tp->packTextures( m_Width, m_Height, bForcePowerOfTwo, bAddOnePixelBorder );
				m_FreeSpace = (100.f * m_FreeSpace ) / (m_Width * m_Height);

				for( int i = 0; i < images.size(); i++ )
				{
					images[i].rotation = 90 * tp->getTextureLocation( i, images[i].x, images[i].y, images[i].w, images[i].h ); // returns true if the texture has been rotated 90 degrees
				}

				releaseTexturePacker(tp);

				printf("%dx%d texture atlas (%d images, %03.2f%% surface free) ... ", m_Width, m_Height, images.size(), m_FreeSpace );

#ifdef PS3_VERSION

				// remove 4096 width hack

				images.erase( --images.end() );

				// and validate

				return m_Width == 4096 && m_Height <= 4096;

#endif
			}

			return true;
		}

		bool pack_procedurally( std::vector< image > &subset, int idx = 0 )
		{
			printf("- Packing subset from %d to %d files: ", idx, idx + subset.size() );

			if( !pack_subset( subset ) )
			{
				printf("failed\n");

				std::vector< image > subsetL;
				std::vector< image > subsetR;

				for( int i = 0; i < subset.size(); i++ )
				{
					if( i < subset.size() * 0.5f )
						subsetL.push_back( subset[i] );
					else
						subsetR.push_back( subset[i] );
				}

				return pack_procedurally( subsetL, idx ) && pack_procedurally( subsetR, idx + subsetL.size() + 1 );
			}

			printf("done!\n");

			image im;

			render_subset( im, subset );

#ifdef PS3_VERSION

			gRenderSet++;

#endif

			for( int i = 0; i < outputs.size(); i++)
				save_subset( outputs[i], im, subset );

			return true;
		}

		public:

		void input( std::string s )
		{
			inputs.push_back( s );
		}

		void output( std::string s )
		{
			outputs.push_back( s );
		}

		void process( bool bSubdirs )
		{
			if( !inputs.size() )
			{
				printf("Please specify at least one input source\n");
				return;
			}

			for( int i = 0; i < inputs.size(); i++ )
			{
				scan_file( inputs[i], bSubdirs );
			}

			pack_procedurally( images );
		}
};


int main( int argc, char **argv )
{
	atlas a;

	for( int argn = 1; argn < argc; argn++ )
	{
		char *arg = argv[argn], *arg_next = argv[argn + 1];

		if( arg[0] == '-' )
		{
			if(!strcmpi(arg, "-q"))
				options::bQuiet = true;

			if(!strcmpi(arg, "-h"))
				options::bShowHelp = true;

			if(!strcmpi(arg, "-s"))
				options::bSubdirs = true;

			if(!strcmpi(arg, "-r"))
				options::bSubdirs = true;

			if(!strcmpi(arg, "-o"))
				if( arg_next )
					a.output( arg_next );
		}
		else
		{
			a.input( arg );
		}
	}

	if( options::bShowHelp )
	{
		options::bQuiet = false;

		printf("%s", VERSION_INFO );
		printf("\n");
		printf("This tool packs collections of textures into a single larger enclosing image.\n");
		printf("Following input file formats are supported: JPG, PNG, BMP, TGA, PSD, DDS and HDR.\n");
		printf("\n");
		printf("Usage:\n");
		printf("\n");
		printf(" * Specify as many input files/paths as you need. Use -r for recursive search.\n");
		printf(" * Specify as many outputs as you need (dds/bmp/ppm/cpp/xml/txt), or none for texture atlas evaluation.\n");
		printf("\n");
		printf("Syntax:\n");
		printf("\n");
		printf("  %s [-h][-q][-r][-v][-o filePath] filePath1 [filePath2] [...]\n", argv[0] );
		printf("\n");
		printf("Parameters:\n");
		printf("\n");
		printf("  -h : Show this help page\n");
		printf("  -q : Quiet mode\n");
		printf("  -r : Recurse subdirectories\n");
		printf("  -v : Show version\n");
//		printf("\n");
//		printf("Examples:\n");
//		printf("\n");
//		printf("Search for images in current folder and subfolders and do nothing:\n");
//		printf("  %s *.*\n", argv[0]);
//		printf("\n");
//		printf("Search for images in current folder and write atlas texture/info in test.bmp/test.xml:\n");
//		printf("  %s -bmp -test.bmp test.xml\n", argv[0]);
//		printf("\n");
//		printf("Search for images in given folders and write atlas texture/info in test.dds/test.xml\n");
//		printf("  %s -f:*.tga -f:png\\*.png -f:misc\\*.* test.dds test.xml\n", argv[0]);
	}
	else
	{
		a.process( options::bSubdirs );
	}

	return 0;
}
