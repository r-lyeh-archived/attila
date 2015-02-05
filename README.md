attila
======

- Attila is a tiny atlas texture/image packer.
- Attila supports many image inputs and a few image outputs (see table below). 
- Attila generates table of contents in JSON format.
- Attila is BOOST licensed.

## Usage
```bash
attila.exe output.img input.img [input2.img [...]] > atlas.json
```

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

## Showcase
```c++
D:\prj\attila>attila atlas.png deps\spot\images\*.png deps\spot\images\*.jpg > atlas.json
[ OK ] Attila - Texture (2048x2048) area: 54.0687% free

C:\prj\WIP\wip\attila>type atlas.json
{
        "deps/spot/images/lenna3.jpg": {
                "src": "deps/spot/images/lenna3.jpg", "dst": "atlas.png",
                "x": 1345, "y": 1, "w": 540, "h": 589,
                "u0": 0.656738, "v0": 0.944336, "u1": 0.000488281, "v1": 0.26416,
                "rotate": 90, "hash": 1193998582
        },
        "deps/spot/images/lenna2.jpg": {
                "src": "deps/spot/images/lenna2.jpg", "dst": "atlas.png",
                "x": 1, "y": 603, "w": 730, "h": 612,
                "u0": 0.000488281, "v0": 0.356934, "u1": 0.294434, "v1": 0.593262,
                "rotate": 0, "hash": 1461652005
        },
        "deps/spot/images/lenna1.jpg": {
                "src": "deps/spot/images/lenna1.jpg", "dst": "atlas.png",
                "x": 803, "y": 1, "w": 540, "h": 600,
                "u0": 0.39209, "v0": 0.655762, "u1": 0.000488281, "v1": 0.293457,
                "rotate": 0, "hash": 2326321968
        },
        "deps/spot/images/img_mars.jpg": {
                "src": "deps/spot/images/img_mars.jpg", "dst": "atlas.png",
                "x": 733, "y": 603, "w": 512, "h": 512,
                "u0": 0.35791, "v0": 0.60791, "u1": 0.294434, "v1": 0.544434,
                "rotate": 0, "hash": 1621952076
        },
        "deps/spot/images/test_rect.png": {
                "src": "deps/spot/images/test_rect.png", "dst": "atlas.png",
                "x": 1, "y": 1, "w": 800, "h": 600,
                "u0": 0.000488281, "v0": 0.391113, "u1": 0.000488281, "v1": 0.293457,
                "rotate": 0, "hash": 2884469502
        },
        "deps/spot/images/panda.png": {
                "src": "deps/spot/images/panda.png", "dst": "atlas.png",
                "x": 1505, "y": 603, "w": 200, "h": 150,
                "u0": 0.734863, "v0": 0.83252, "u1": 0.294434, "v1": 0.367676,
                "rotate": 0, "hash": 3872279401
        },
        "deps/spot/images/img_test.png": {
                "src": "deps/spot/images/img_test.png", "dst": "atlas.png",
                "x": 1247, "y": 603, "w": 256, "h": 256,
                "u0": 0.608887, "v0": 0.733887, "u1": 0.294434, "v1": 0.419434,
                "rotate": 0, "hash": 3208285877
        }
}

D:\prj\attila>start atlas.png
```
![image](https://raw.github.com/r-lyeh/depot/master/attila.jpg)

## Build and redistribution
```bash
REM windows
cl attila.cc deps\spot\spot*.c* deps\packers\*.cpp /Ox /Oy /MT /DNDEBUG /link setargv.obj

REM others
g++ -o attila --std=c++11 attila.cc deps/spot/spot*.c* deps/packers/*.cpp -O2 -DNDEBUG
```

