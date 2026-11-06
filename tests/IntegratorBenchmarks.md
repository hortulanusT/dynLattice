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

## Test 3
First two bending frequencies of a slender (L / r = 50) and a stubby (L / r = 10) cantilever after a small tip velocity, from the spectrum of the tip displacement over ten periods, against the clamped-free frequency equation of the Timoshenko beam (Table 9 and Eqs. 109, 110 of [Han, Benaroya, Wei (1999)](https://doi.org/10.1006/jsvi.1999.2257), checked against their example of Sect. 3.5) with the shear coefficient of [Cowper (1966)](https://doi.org/10.1115/1.3625046). All integrators on 16 elements (left, middle); LeapFrog on 16, 32 and 64 elements, where the frequencies converge with the mesh at second order (right).

![Test 3a Results](integrator3a_result.png)
![Test 3b Results](integrator3b_result.png)
![Test 3c Results](integrator3c_result.png)

## Test 4
Freely spinning rigid body of [Krysl, Endres (2005)](https://doi.org/10.1002/nme.1272), Sect. 4.1, close to its intermediate axis (Dzhanibekov effect), as a stiff rod of one element: the angular velocity in the body frame against their Fig. 3 (read from the vector graphics of the figure, `ref_data/test4_ref_omega.csv`), left, and the convergence of LeapFrog, right. All integrators are first order on rigid rotation at present, and `EmbeddedRK` in 3D is affected by its frozen mass matrix: both are run through, not checked, until they are fixed.

![Test 4a Results](integrator4a_result.png)
![Test 4b Results](integrator4b_result.png)

## Test 5
Pure rotation of the free rigid body of [Holzinger, Gerstmayr (2021)](https://doi.org/10.1007/s11044-021-09778-w), Sect. 7.1.2 (with epsilon = 0), about its axis 2 over 100 s, as a stiff rod of one element: the rotation vector against their analytical solution, Eq. (73), while it passes |theta| = 2 pi, 4 pi, ... (left), and the deviation from it (right).

![Test 5a Results](integrator5a_result.png)
![Test 5b Results](integrator5b_result.png)
