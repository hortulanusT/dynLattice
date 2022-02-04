# :hammer_and_pick: TO DO 
## Code
- Create Custom Sample Module for advanced options
  - append without extra header
  - non-float output
- write `.log` to central file?
  - dont write log-files, but capture output in python?
- write $\lambda$ (strain factor) instead of step to the `.res` files
- GMSHInputModule with gmsh C++ API
- investigate options for parallel computing
- PBC Output extent other side
  - $\frac{du}{dX} \approx \frac{u_2-u_1}{\Delta X}$
- ParaviewOutput write wrapper file for exported files ~ `.pvd`
- export some standard actions to seperate script
  - plotting
  - setting up the enviroment
  - running a simulatio with added parameters
## Environment
- think of smarter ways to handle data
- rethink the folder structure
- structure git repos with "submodules"
## Investigations
- Implement Arc-Length 
- do the remaining test cases from [Simo/Vu-Quoc]
# :scroll: Notes
## test-cases
- single element (cantilever):
  - point force fails for large magnitudes (and small step sizes?!?)
- test cases [Simo/Vu-Quoc]:
  - TEST 7.2 small step sized due to error in the way the follower load is applied `THIS IS FINE`
  - TEST 7.5 sufers the same problem as the single elements
## homogenization ideas
- for 2.5D structure use 2D homogenization with prism shear modulus/Youngs modulus in third direction scaled up to density?
 
# :heavy_check_mark: Information
## debugging
- call (dbg) call dprint(&...) to get any matrix/vector/... printed live
- when compiling OPT, it sets the flag `NDEBUG`
## .pro files
- `$(CASE_NAME)` also works with environment variables (and other settings)
- `include "xyz.pro";` works to include other .pro files
## others
- shape function node numbering in `$JIVEDIR/packages/geom/doc/images`
- `StdShape` for local coordinates and `Shape` for global coordinates
- `./jive xyz.pro |c++filt` for nicer stack traces
- `JEM_PRECHECK` gets executed always, `JEM_ASSERT` only in non-optimized mode
- use the `*.tiny` setting of the solver module to enable convergence for smaller load steps
