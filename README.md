# coccus-world
A game about coccus.

## Compiling
```shell
make
```

## Usage
```shell
./coccus-world
```

## Control
Alive:

key or button|function
:-:|:-:
`Left mouse button`|ejection

Dead:

key or button|function
:-:|:-:
`W` `A` `S` `D`|move
`Right mouse button`|ejection(empty)
`Space`|pause and continue

Common:

key or button|function
:-:|:-:
`LEFT` `RIGHT`|slow(can turn to reverse direction), accelerate
`Left mouse button` with `Z`|increase size
`Right mouse button` with `Z`|decrease size
`Left mouse button` with `X`|add coccus
`Right mouse button` with `X`|remove coccus
`Left mouse button` with `C`|undirected dragging
`Right mouse button` with `C`|directed dragging
`ESC`|quit

## Dependencies
[OpenGL - The Industry's Foundation for High Performance Graphics](https://www.opengl.org/)

[GLFW - An OpenGL library](http://www.glfw.org/)

[GLEW - The OpenGL Extension Wrangler Library](http://glew.sourceforge.net/)

[GLM - OpenGL Mathematics](http://glm.g-truc.net/0.9.6/index.html)

[SOIL - Simple OpenGL Image Library](http://www.lonesock.net/soil.html)

[ALSA - Advanced Linux Sound Architecture](http://www.alsa-project.org/main/index.php/Main_Page)

## Plan
###### Basic
* networking
* text
* border texture
* collision effect, ripple
* absorption effect
* background ripple
* rigion selection
* angle of cue arrow after scaling
* choose charactor

###### Gameplay
* eat small circles in the corner only by decreasing

## Screentshots
![1](http://7xkk5t.com1.z0.glb.clouddn.com/coccus_world_1.png)

![2](http://7xkk5t.com1.z0.glb.clouddn.com/coccus_world_2.png)
