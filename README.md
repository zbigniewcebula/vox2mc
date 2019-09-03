# vox2mc
Conversion tool for converting VOX models into OBJ generated by Marching Cubes

### Tested compatibility with g++ version
Linux: gcc version 7.4.0 (Ubuntu 7.4.0-1ubuntu1~18.04.1)

Windows: gcc version 5.3.0 (GCC)

### TODO

* Fill README.md with usefull info ( ͡° ͜ʖ ͡°)
* Optimize output mesh
* Valgrind checks
* Basic (MagicaVoxel-like) MTL generation
* ~~Compatibility with Windows~~
* Parametrize:
	1. ~~upscale factor __-u__/__--upscale__~~
	2. ~~scale __-s__/__--scale__~~
	3. offset
		* __-ox__/__--offset-x__
		* __-oy__/__--offset-y__
		* __-oz__/__--offset-z__
	4. ~~flips~~
		* ~~__-fx__/__--flip-x__~~
		* ~~__-fy__/__--flip-y__~~
		* ~~__-fz__/__--flip-z__~~
* Add support for batch conversion
	1. ~~__-id__/__--input-dir__~~
	2. ~~__-od__/__--output-dir__~~
* ~~Add flag for time consumption of conversion __-t__/__--time__~~
* Use Makefile instead of bash injection in *main.cpp*(?)
