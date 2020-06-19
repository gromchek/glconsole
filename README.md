## glconsole

Quake-like console with commands and [SDF text rendering]( https://github.com/libgdx/libgdx/wiki/Distance-field-fonts )  

#### How to create SDF texture

You need a tool for generating text atlas ([Windows]( https://www.angelcode.com/products/bmfont/ ))  
Create texture and then run:
```
magick convert intexture.png -filter Jinc -resize 400% -threshold 30% ( +clone -negate -morphology Distance Euclidean -level 50%,-50% ) -morphology Distance Euclidean -compose Plus -composite -level 45%,55% -resize 25% outtexture.png
```

For converting [bmfont config](https://www.angelcode.com/products/bmfont/doc/file_format.html) to JSON you need [gofnt]( https://github.com/KeKsBoTer/gofnt )

### Build

```
mkdir build & cd build
cmake ..
make
```

[Assets dir]( https://www.dropbox.com/s/s6h3srtxqqsf9e1/base.zip?dl=1 )  
Just unarchive files to `build/base` directory  


### Dependencies
sdl2 2.0.10  
glm 0.9.9.8  
glew 2.1.0  


### License
MIT
