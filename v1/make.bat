pushd .
cd ..\dot\redist
call make
popd
cl *.cpp -I .. ..\dot\dot*.c* /Ox /Oy /DNDEBUG /MT /link setargv.obj 
del kk.dds
atlas -r -o kk.dds images
kk.dds
