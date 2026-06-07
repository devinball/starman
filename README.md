layers are for engine things, like scene mangment
layers are engine operations

systems are for managing things that exist in the world/scene, like physics
systems are world operations

rendering is weird, it has a system that acts on the world, that passes needed
data to the render layer.

layer needs an event system, i could use EnTT, or make my own
events are sent to the disbatcher, which then propagates the event
to all layers

maybe one layer could just handle timing and loops, it would call a special on update / draw / init event?

# TODO:

## Cameras
### multiple cameras support
### physics -> rendering space transform
### camera movement

## Star System
### Single Planet
### texture loading

## Moving ship
### texture loading
### input
### movable camera
### gravity

## gravity
### kepler orbits
### newton orbits

# when resource loading fails (shader, texture, material), substitute with a default

## Bugs


## Log
### 250 onscreen simple_frog.stl, default lit material
5/5/26 - Nvidia: 0.5 ms Intel: 5.3 ms







# Notes

      // for each body we need to compute it's trajectory
      // to do this we need to find all conservative forces
      // acting on the body. my first thought is to just 
      // integrate these forces over time, which may work
      // though i do wonder, given that gravity is consv
      // if i could use an energy-based approach. not sure
      // though, that would not work for atmospheric drag
      // (for orbital decay), or i think time-varying mag
      // fields (maybe??). the other issue would be engine
      // burns. i could also precompute engine burns.

      // the objective of doing this instead of just integrating
      // in realtime is two fold, first is then i can reuse
      // trajectory data for showing the player where an
      // object will be, and the other is it allows me to
      // computer an object's positions on another thread
      
      // of course the issue quickly becomes, how do i know
      // what forces to apply. for gravity i need to apply
      // newton's gravity using the planet's and moon's
      // positions at some future time. but then how do i do
      // that? if they follow kepler's equations then maybe
      // that's an option.

      // i may also want to allow for relativisic dynamics
      // i think the 4d trajectory might work for this?
      // not so sure though, haven't gotten to modern
      // physics yet, my understanding of relativity
      // is youtube videos and wikipedia.

      // TODO: in the future when integrating forces I can use this equation:
      // γ^3 * m * a, where γ is the lorenz factor, and m is rest mass
      // or: a=\frac{F}{m}\left(1-\frac{v^{2}}{c^{2}}\right)^{\frac{3}{2}}

      // for now though i am going to ignore relativity