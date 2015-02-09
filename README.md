attila
======

- Attila is a tiny atlas texture-packer.
- Attila features a wide image/texture file support (see table below). 
- Attila generates table of contents in JSON format.
- Attila is BOOST licensed.

## File format support

| File format  | Input | Output |
| :-------------|:-------------:| :-----:|
| WebP files | yes | yes |
| JPG files | yes | yes |
| JPG files (progressive) | yes | no |
| PNG files | yes | yes |
| TGA files | yes | yes |
| DDS DXT1/2/3/4/5 files | yes | yes |
| BMP files | yes | yes |
| PSD files | yes | no |
| GIF files | yes | no |
| PKM (ETC1) files | yes | no |
| PVR (PVRTC) files | yes | no |
| SVG files (rasterized) | yes | no |
| HDR files | yes | no |
| PIC files | yes | no |
| PNM (PPM/PGM) files | yes | no |

## Usage
```c++
attila.exe [options] output.img input.img [input2.img [...]] > atlas.json
attila.exe [options] output.img @imagelist.txt [@imagelist2.txt [...]] > atlas.json
```

## Options
```c++
--help:              prints help
--enable-bleeding:   enables output alpha bleeding (default: disabled)
--enable-cropping:   enables input alpha cropping (default: disabled)
--enable-edge:       enables output blank pixel separator (default: disabled)
--enable-pot:        enables output power-of-two texture (default: disabled)
```

## Showcase
```c++
C:\attila>attila atlas.png deps\spot\images\*.*g --enable-edge > atlas.json
[ OK ] Attila - Texture (2004x2640) area: 31.1038% free

C:\attila>type atlas.json
{
        "deps/spot/images/test_rect.png": {
                "src": "deps/spot/images/test_rect.png", "dst": "atlas.png",
                "x": 1003, "y": 1, "w": 800, "h": 600,
                "u0": 0.500499, "v0": 0.899701, "u1": 0.000378788, "v1": 0.227652,
                "rotate": 0, "hash": 2884469502
        },
        "deps/spot/images/panda.png": {
                "src": "deps/spot/images/panda.png", "dst": "atlas.png",
                "x": 1805, "y": 1, "w": 200, "h": 150,
                "u0": 0.900699, "v0": 1.0005, "u1": 0.000378788, "v1": 0.057197,
                "rotate": 0, "hash": 3872279401
        },
        "deps/spot/images/octocat.svg": {
                "src": "deps/spot/images/octocat.svg", "dst": "atlas.png",
                "x": 1805, "y": 153, "w": 64, "h": 64,
                "u0": 0.900699, "v0": 0.932635, "u1": 0.0579545, "v1": 0.082197,
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
g++ -o attila --std=c++11 attila.cc deps/spot/spot*.c* deps/packers/*.cpp -O2 -DNDEBUG
```

## Licenses
- [Attila](https://github.com/r-lyeh/attila), BOOST licensed.
- [Texture Packer](https://github.com/r-lyeh/attila/blob/master/deps/packers/packer.hpp) by John W. Ratcliff, MIT licensed.
