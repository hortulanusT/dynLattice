# Plastic Benchmarks {#plastic_benchmarks}
## Overview
The plastic tests validate the correct implementation of the inelastic material behavior of the beam, as well as the explicit return mapping scheme.

## Test Cases
### Test 1
Test 1 implements Example 5.1 from [Smriti, Kumar, Steinmann (2011)](https://doi.org/10.1002/nme.6566). This example shows the plastic behavior of a cantilever beam under a constant twist, being subjected to axial tension and compression. The hardening observed is a result of the multi-axial loading condition and can be viewed as *geometric hardening*, as no hardening is present in the considered material parameters. The custom implementation shows an excellent agreement with the results from literature

![Test 1 Results](plastic1_result.png)

### Test 2
Test 2 implements Example 5.2 from [Smriti, Kumar, Steinmann (2011)](https://doi.org/10.1002/nme.6566). This example shows the plastic behavior of a cantilever beam subjected to axial tension and compression with the effects of either isotropic or kinematic hardening present in the material model. Both, case a) *isotropic hardening* and case b) *kinematic hardening* show an excellent agreement with the results from literature.

![Test 2a Results](plastic2a_result.png)
![Test 2b Results](plastic2b_result.png)

### Test 3
Test 3 implements Example 5.4 from [Herrnböck, Kumar, Steinmann (2023)](https://doi.org/10.1007/s00466-022-02204-8). This example shows the complex elasto-plastic deformation of a beam under three-dimensional loading conditions. The custom implementation shows good agreement with the reported results from an \f$FE^2\f$ approach. The differences are similar to the ones reported in the referenced publication.

![Test 3 Results](plastic3_result.png)