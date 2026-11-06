\page integratorbenchmarks Integrator Benchmarks
The integrator tests check the explicit time integrators at fixed time steps against exact solutions and published results. `tests/testing.mk` lists the runs; the scripts evaluate them.

# Test Cases
## Test 1
Convergence order of the motion (left) and of the energy error (right) of a vibrating cantilever with an initial tip velocity and a dead tip force, over about three periods of the first mode, for each integrator at five step sizes. There is no closed-form solution of this large-deflection motion: the motion is compared with the run at half the step, and the energy with its conservation. `MilneDevice` at a fixed step is the first-order Adams-Bashforth-Moulton scheme; it damps fast modes and therefore runs to t = 0.02 only. In 3D, `EmbeddedRK` is first order and the energy error of every integrator stops at about 2e-4; these are run through, not checked, until they are fixed.

![Test 1a Results](integrator1a_result.png)
![Test 1b Results](integrator1b_result.png)

## Test 2
Axial wave in a bar (100 elements) under a constant end force, from rest, over three wave cycles, against the exact d'Alembert solution: displacement of the loaded end (left) and reaction at the fixed end (right), where every front has to arrive within three elements. The oscillations ahead of the wave fronts are the dispersion of the mesh, not of the integrators: with the consistent mass, short waves travel faster than c, see [Gabriel et al. (2010)](https://doi.org/10.1016/j.cam.2009.08.043). `MilneDevice` damps them.

![Test 2a Results](integrator2a_result.png)
![Test 2b Results](integrator2b_result.png)
