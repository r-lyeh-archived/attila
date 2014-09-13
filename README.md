attila
======

- Attila is a simple and lightweight atlas texture/image packer
- Attila is MIT licensed.

### supported input file formats
- WebP files
- JPG files
- progressive JPG files
- PNG files
- TGA files
- DDS DXT1/2/3/4/5 files
- BMP files
- PSD files
- GIF files
- PKM (ETC1) files
- PVR (PVRTC) files
- vector SVG files
- HDR files
- PIC files

### supported output file formats
- PNG (for now)

### build
```
D:\prj\attila> cl attila.cc dot\dot*.c* packers\*.cpp /Ox /Oy /MT /DNDEBUG /link setargv.obj
```

### usage
```
D:\prj\attila>attila output.png dot\images\*.*
texture (4096x4096) area: 46.1598% free
D:\prj\attila>start output.png
```

![image](https://raw.github.com/r-lyeh/depot/master/attila.jpg)
