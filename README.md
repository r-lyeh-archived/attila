attila :fire: <a href="https://travis-ci.org/r-lyeh/attila"><img src="https://api.travis-ci.org/r-lyeh/attila.svg?branch=master" align="right" /></a>
======

- Attila is a tiny atlas texture-packer.
- Attila features a wide image/texture file support (see table below).
- Attila generates table of contents in JSON format.
- Attila is zlib/libpng licensed.

## File format support

| File format  | Read | Write |
| :-------------|:-------------:| :-----:|
| BMP files | yes | yes |
| CRN files | yes | no |
| DDS DXT1/2/3/4/5 files | yes | yes |
| GIF files | yes | no |
| HDR files | yes | no |
| JPG files (progressive) | yes | no |
| JPG files | yes | yes |
| KTX (ETC1) files | yes* | yes* |
| KTX (PVRTC) files | yes* | no |
| PIC files | yes | no |
| PKM (ETC1) files | yes | yes |
| PNG files | yes | yes |
| PNM (PPM/PGM) files | yes | no |
| PSD files | yes | no |
| PUG files | yes | yes |
| PVR2 (PVRTC) files | yes* | no |
| PVR3 (ETC1) files | yes* | no |
| PVR3 (PVRTC) files | yes* | yes* |
| SVG files (rasterized) | yes | no |
| TGA files | yes | yes |
| WEBP files | yes | yes |

(*) partial support

## Usage
```c++
attila.exe [options] output.img input.img [input2.img [...]] > atlas.json
attila.exe [options] output.img @imagelist.txt [@imagelist2.txt [...]] > atlas.json
```

## Options
```c++
--help                 prints help
--enable-bleeding      enables output alpha bleeding (default: disabled)
--enable-cropping      enables input alpha cropping (default: disabled)
--enable-edge          enables output blank pixel separator (default: disabled)
--enable-pot           enables output power-of-two texture (default: disabled)
--enable-mipmaps       enables mipmaps (default: disabled)
--enable-width WIDTH   enables minimum fixed width (in pixels) (default: 0)
```

## Showcase
```c++
C:\attila>attila atlas.png deps\spot\images\*.*g --enable-edge > atlas.json
[ OK ] Attila - Texture (2004x2640) area: 30.5367% free

C:\attila>type atlas.json
{
        "deps/spot/images/test_rect.png": {
                "src": "deps/spot/images/test_rect.png", "dst": "atlas.png",
                "x": 1003, "y": 1, "w": 800, "h": 600,
                "u0": 0.500499, "v0": 0.899701, "u1": 0.000378788, "v1": 0.227652,
                "rotate": 0, "hash": 2884469502
        },
        "deps/spot/images/panda.pug": {
                "src": "deps/spot/images/panda.pug", "dst": "atlas.png",
                "x": 1805, "y": 1, "w": 200, "h": 150,
                "u0": 0.900699, "v0": 1.0005, "u1": 0.000378788, "v1": 0.057197,
                "rotate": 0, "hash": 3331997094
        },
        "deps/spot/images/panda.png": {
                "src": "deps/spot/images/panda.png", "dst": "atlas.png",
                "x": 1805, "y": 153, "w": 200, "h": 150,
                "u0": 0.900699, "v0": 1.0005, "u1": 0.0579545, "v1": 0.114773,
                "rotate": 0, "hash": 3872279401
        },
        "deps/spot/images/octocat.svg": {
                "src": "deps/spot/images/octocat.svg", "dst": "atlas.png",
                "x": 1805, "y": 305, "w": 64, "h": 64,
                "u0": 0.900699, "v0": 0.932635, "u1": 0.11553, "v1": 0.139773,
                "rotate": 0, "hash": 1863637957
        },
        "deps/spot/images/nano.svg": {
                "src": "deps/spot/images/nano.svg", "dst": "atlas.png",
                "x": 803, "y": 1003, "w": 640, "h": 480,
                "u0": 0.400699, "v0": 0.72006, "u1": 0.379924, "v1": 0.561742,
                "rotate": 0, "hash": 1382584450
        },
        "deps/spot/images/lenna3.jpg": {
                "src": "deps/spot/images/lenna3.jpg", "dst": "atlas.png",
                "x": 1335, "y": 1514, "w": 540, "h": 589,
                "u0": 0.666168, "v0": 0.96008, "u1": 0.573485, "v1": 0.77803,
                "rotate": 90, "hash": 1193998582
        },
        "deps/spot/images/lenna2.jpg": {
                "src": "deps/spot/images/lenna2.jpg", "dst": "atlas.png",
                "x": 1, "y": 1514, "w": 730, "h": 612,
                "u0": 0.000499002, "v0": 0.36477, "u1": 0.573485, "v1": 0.805303,
                "rotate": 0, "hash": 1461652005
        },
        "deps/spot/images/lenna1.jpg": {
                "src": "deps/spot/images/lenna1.jpg", "dst": "atlas.png",
                "x": 733, "y": 1514, "w": 540, "h": 600,
                "u0": 0.365768, "v0": 0.66517, "u1": 0.573485, "v1": 0.77803,
                "rotate": 90, "hash": 2326321968
        },
        "deps/spot/images/img_test.png": {
                "src": "deps/spot/images/img_test.png", "dst": "atlas.png",
                "x": 1003, "y": 603, "w": 256, "h": 256,
                "u0": 0.500499, "v0": 0.628244, "u1": 0.228409, "v1": 0.325379,
                "rotate": 0, "hash": 3208285877
        },
        "deps/spot/images/img_mars.jpg": {
                "src": "deps/spot/images/img_mars.jpg", "dst": "atlas.png",
                "x": 1, "y": 2128, "w": 512, "h": 512,
                "u0": 0.000499002, "v0": 0.255988, "u1": 0.806061, "v1": 1,
                "rotate": 0, "hash": 1621952076
        },
        "deps/spot/images/drawing.svg": {
                "src": "deps/spot/images/drawing.svg", "dst": "atlas.png",
                "x": 1, "y": 1, "w": 1000, "h": 1000,
                "u0": 0.000499002, "v0": 0.499501, "u1": 0.000378788, "v1": 0.379167,
                "rotate": 0, "hash": 2285228186
        },
        "deps/spot/images/23.svg": {
                "src": "deps/spot/images/23.svg", "dst": "atlas.png",
                "x": 1, "y": 1003, "w": 509, "h": 800,
                "u0": 0.000499002, "v0": 0.399701, "u1": 0.379924, "v1": 0.572727,
                "rotate": 90, "hash": 1458245219
        }
}

C:\attila>start atlas.png
```
![image](https://raw.github.com/r-lyeh/depot/master/attila.jpg)

## Build and redistribution
```bash
REM windows
cl attila.cc deps\spot\spot*.c* deps\packers\*.cpp /EHsc /Ox /Oy /MT /DNDEBUG /link setargv.obj

REM others
$CC -c deps/spot/spotc.c -I deps/spot -O2 -DNDEBUG
$CXX -o attila -std=c++11 attila.cc deps/spot/spot.cpp spotc.o -I deps/spot deps/packers/*.cpp -lrt -O2 -DNDEBUG
```

## Changelog
- v1.0.7 (2015/09/28): faster image pasting
- v1.0.6 (2015/07/30): minimum width option
- v1.0.5 (2015/05/11): pump up libspot
- v1.0.4 (2015/04/09): enable mipmap generation
- v1.0.3 (2015/02/12): bugfixed error while handling @filelists
- v1.0.2 (2015/02/11): upgraded to latest spot lib
- v1.0.1 (2015/02/09): options, including image cropping and padding
- v1.0.0 (2015/02/06): initial version

## Licenses
- [Attila](https://github.com/r-lyeh/attila), zlib/libpng licensed.
- [Texture Packer](https://github.com/r-lyeh/attila/blob/master/deps/packers/packer.hpp) by John W. Ratcliff, MIT licensed.
