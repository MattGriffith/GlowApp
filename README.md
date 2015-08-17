# TroidGlow by Matthew Griffith

## Description:

This is a simple particle simulation. The particles bounce and fly around to
fill the window, and when you hold the space key they form themselves into an
image. The look and feel of the program can be controlled through a config file
(glow.ini).

The original project was made early in my high school career, written on Dec
25, 2007. The image spelled out "MERRY X-MAS". I did not look at the code again
until 2012, when I decided to fix up some bugs and expand on the features. I am
still in the process of fixing old/bad code and adding new features.

## Compiling:

TroidGlow uses Corona (corona.sourceforge.net, zlib license) to load images,
the win32 library to create a window (making this Windows-only), and OpenGL to
render graphics into the window. A Code::Blocks project file is included, but
to compile in another IDE (or using a Makefile), be sure to include all the .h
and .cpp files in this directory, and link it with Corona, OpenGL (and GLU),
as well as -lwinmm and -lgdi32.

## License:

This program is released under the GPL. See the file "COPYING" for more info.

## Controls:

**Space bar** - Hold it to form an image with the particles.  
**Escape** - Close the window

## Config file settings:

The following are all the available options for editing the program without
recompiling.

    [display]
      width: Window width. Min is 64, default is 640.
      height: Window height. Min is 64, default is 480.
      caption: Window caption. Default is "TroidGlow by Matthew Griffith".
      bgRed: The background color (red value). Ranges from 0 to 1.
      bgGreen: The background color (green value). Ranges from 0 to 1.
      bgBlue: The background color (blue value). Ranges from 0 to 1.

    [particle]
      texture: Path to the particle's image. Loaded as an alpha map.
      addBlend: Whether or not to use additive blending. Default is false.
      width: Particle's width. Default is 64.
      height: Particle's height. Default is 64.
      maxStartSpeed: Random start speed is capped by this value. Default is 5.
      maxSpeed: The fastest a particle can go. Default is 0 (no limit).
      speedJitter: Range by which speed randomly jitters. Default is 0.125.
      wrap: true for looping across edges, false (default) for bouncing off them.
      uniformColor: true (default) for all particles to have the same color.

    [map]  
      image: Path to the black-and-white image that the particles will form.
      numParticles: Number of particles in window. Default is 500.

## Contact information:

**Email:** ThatMattGriffith@gmail.com  
**Github:** MattGriffith

