\page transientbenchmarks Transient Benchmarks
The transient tests validate the capabilities of the implementation to perform dynamic simulations. These tests verify the assembly of individual elements into complex structural systems and validate the behavior of interconnected rod networks.

# Test Cases
\subsection transient1 Test 1
Test 1 implements Example 5.1 from [Simo, Vu-Quoc (1988)](https://doi.org/10.1016/0045-7825(88)90073-4). This example shows the spin-up of a flexible beam and the subsequent elongation under centrifugal load as well as the fluttering of the tip. The results obtained by the custom implementation agree well with the results reported in literature. 

![Test 1 results A](transient1_resultA.png)
![Test 1 results B](transient1_resultB.png)
![Test 1 results C](transient1_resultC.png)

## Test 2
Test 2 implements Example 1 from [Lang, Linn, Arnold (2011)](https://doi.org/10.1007/s11044-010-9223-x). This example shows the free dropping of a flexible beam. The results obtained with the current implementation agree well with the results from literature.

![Test 2 results](transient2_result.png)

## Test 3
Test 3 implements Example 5.2 from [Simo, Vu-Quoc (1988)](https://doi.org/10.1016/0045-7825(88)90073-4). This example shows the dynamic behavior of a right-angle cantilever beam subjected to out-of-plane loading at its elbow. The results obtained with the current implementation agree well with the results from literature.

![Test 3 results](transient3_result.png)
