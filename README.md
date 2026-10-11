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

# Immediate:
- The easy thing would be a skybox
- Right now the biggest thing i could do would probably be either bloom or future trajectory predicction
      - trajectories may require an integration that can handle variable dt

# TODO:

## Rendering
- Actual Lighting
- Normal Mapping
- Skybox
- Atmospheres
- Frustrum culling
- Bloom

## Physics
- __THERE IS COORDINATE HANDEDNESS MISALIGNMENT!!__ this is not good. i need to adjust the renderer to use standard right handed coordinates
- Quad, Fixed, or Arbitrary precision positions
- Interpolation, including for planets
- Variable delta time (with energy conservation)
- Collisions (simple proximity based)
- Trajectory based motion system
      - go back to this idea
      - we already have to calculate a trajectory in a reasonable amount of precision
        to show the player, may as well just re-use this work
      - when the engine has extra time, it will further calculate the trajectories of
        objects, up to a limit (probably declared by the player). It then, using polynomial
        approximations interpolates along that trajectory as long as it needs.
      - if something reaches the end of it's trajectory, it would be allowed to either
        slow down or pause the game so that it could catch up.
      - trajectories could be computed on seperate threads from the main game, or even
        from each other, a spacecraft's gravity is so small we ignore it, planet positions
        are determinsitic.
      - we would only have to throw away work when the spacecraft manuvers, this sounds bad,
        but this would already be the case with any (non-patched-conic) system that also shows
        the spacecraft's trajectory. And besides, the engine won't calculate trajectories further
        than it needs to (unless the player says otherwise, in which case the player could choose
        to calculate the trajectory for a specific craft even)
      - same deal when a spacecraft fails to complete a manuever, either because an engine failed
        or it was shot or whatever. It would also need to recalculate in accordance with momentum
        transfer from getting hit.
      - it would only calculate just as much as it needs, this would be especially true when executing
        a manuever (where something goes wrong), it would basically just revert to realtime
      - that's the advantage, the worst case is the standard case of what i am currently doing.
      - there would be a calculation priority, ships that are expected to be shot or hit with debris
        would be a smaller priority, because their orbits are about the change.
      - the active craft, or a craft whose trajectory is being planned would be higher priority.
      - Maybe a spacecraft could even hold onto multiple trajectories, one for status quo, then a
        branch for each manuver. But i think it would be better, for example in the case of an engine
        failing to light, we just discard the manuver trajectories altogether, and just recalculate
        from current position.
      - of course an objects trajectory would just be read into the icon_system for display, if the
        player reduced the section of orbit being viewed, it would not delete that data, just hide it.
        if the player extended the view further than it has been calculated, it would just calculate.
      - Ideally, for this to all work well, the whole system needs to be deterministic.

## UI
- Trajectories (forward in time)

## Gameplay
- Manuver planning
- Programs
      - (Seris of manuvers, primitives, and conditionals)
      - for example, "if close to transfer window to {destination}" -> "plot hohemann transfer to {destination}"
      - or "if incoming projective" -> "dodge with RCS"
      - or "if incoming missile" -> "defend with PDC"
- Enemy AI
      - could use genetic algorithms to find good heuristics for programs
- 

# when resource loading fails (shader, texture, material), substitute with a default

# Done:
- IMGUI integration
- Gravity
      - Fixed orbits (Keplerian)
      - N-body orbits (Newtonian with Symplectic integrator)
- Single Planet
      - Large scale object rendering
- Fix floating point errors for large relative positions (kindof, there is still some rendering jankiness)


## Bugs



## Log
### 250 onscreen simple_frog.stl, default lit material
5/5/26 - Nvidia: 0.5 ms Intel: 5.3 ms


# Notes
maybe for rendering i can habve multipl ernder targets
0 - screen output
1 - normal
2 - depth
3...N - for anything yiu want 


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
