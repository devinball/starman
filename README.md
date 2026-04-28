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