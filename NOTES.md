# :hammer_and_pick: TO DO 
## Investigations
- Patch-Sets with Arc-Length
- Implement Dynamics
  - Use TimeStepper Factory for Stepping determination
- Find Test-Cases for Dynamic Beams
## Code
- investigate options for parallel computing
- PBC Output extent other side
  - $\frac{du}{dX} \approx \frac{u_2-u_1}{\Delta X}$
### Models
- Mesh Model
  - consisting out of multiple Rod-Modules as child
  - adapt Input to represent this
- Rod Model
  - Mixed collocation?
### Modules
- Touch GroupSettings in PBC Input
- Custom Sample Module
  - append without extra header
  - non-float output
  - HDF5 output `pandas`-format
  - write variables to the `.res` files
- Custom Log Module
  - write `.log` to central file?
- Custom Application
  - add shortcuts for `$(CASE_NAME)` without folder
  - enable multiple `.pro` files in command line
- GMSHInputModule with gmsh C++ API
- ParaviewOutputModule
  - write wrapper file for exported files ~ `.pvd`
### Python
- export some standard actions to seperate script
  - plotting
  - setting up the enviroment
  - running a simulatio with added parameters
## Environment
- smarter ways to handle data
- cleanup folder structure
- structure git repos with "submodules"
# :scroll: Notes
## test-cases
- test cases [Simo/Vu-Quoc]:
  - TEST 7.2 only works properly with shut off geometric stiffness matrix (i.e. only the symmetric part)
  - TEST 7.5 worse convergence steps compared to literature (30 instead of 300)
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
