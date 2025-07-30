# Element Benchmarks {#element_benchmarks}
## Overview
The beam tests validate multi-element beam networks and lattice structures under various loading conditions. These tests verify the assembly of individual elements into beams and structural systems.

## Test Cases
### Test 1
Test 1 implements Example 7.1 from [Simo, Vu-Quoc (1986)](https://doi.org/10.1016/0045-7825(86)90079-4). This example shows the convergence properties of the implemented method, by considering the roll-up of a cantilever under a concentrated moment load at the free end. Just as in literature after two iteration steps, the current implementation converges to near machine precision.

![Test 1 Results](beam1_result.png)

### Test 2
Test 2 implements Example 7.2 from [Simo, Vu-Quoc (1986)](https://doi.org/10.1016/0045-7825(86)90079-4). This example shows the displacement of a cantilever beam under follower end load. The obtained results show the same convergence behavior as literature.

![Test 2 Results](beam2_result.png)

\note The FollowerLoadModel is a very crude implementation, resulting in the need to load at smaller steps.

### Test 4
Test 4 implements Example 7.4 from [Simo, Vu-Quoc (1986)](https://doi.org/10.1016/0045-7825(86)90079-4). This example shows the buckling of a right-angle frame under fixed load. The obtained results show the same capability of dealing with buckling as in literature.

![Test 4 Results](beam4_result.png)

### Test 5
Test 5 implements Example 7.5 from [Simo, Vu-Quoc (1986)](https://doi.org/10.1016/0045-7825(86)90079-4). This example shows out of plane deformation of a bent beam under a fixed end load. The obtained results agree well with the results reported in literature, showing the capability of the implementation to handle three dimensional scenarios.

![Test 5 Results](beam5_result.png)
