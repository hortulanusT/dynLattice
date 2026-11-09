\page contactbenchmarks Contact Benchmarks
The contact tests validate the contact formulation and implementation of the current framework.

# Test Cases
## Test 1
Test 1 implements Example 2 from [Zavarise, Wriggers (2000)](https://doi.org/10.1002/1097-0207(20001120)49:8%3C977::AID-NME986%3E3.0.CO;2-C) without friction. In this example, a cantilever beam is pushed against a beam fixed on both ends and the resulting contact forces are recorded. The custom implementation shows good agreement with the results from literature.

![Test 1 Results](contact1_result.png)

## Test 2
Two free elastic rods crossing at a right angle fly towards each other. They touch several times while they vibrate, and must not pass through each other (left), keep their total momentum (middle) and, once separated, their energy (right): the question of Section 7.2 of [Meier, Wall, Popp (2017)](https://doi.org/10.1016/j.cma.2016.11.028), whose example uses two materials and another penalty law, so that only the conservation is compared. The penalty energy of the contact is not part of the potential energy yet, so that the energy dips while the rods touch.

![Test 2a Results](contact2a_result.png)
![Test 2b Results](contact2b_result.png)
![Test 2c Results](contact2c_result.png)
