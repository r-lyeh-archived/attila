if exist atlas.png del atlas.png
dir ..\*.bmp ..\*.png ..\*.gif /s /b > @list
atlas atlas.png @list
del @list
