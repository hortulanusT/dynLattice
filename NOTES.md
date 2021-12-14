# ToDo 
## code
- Create Custom Sample Module for advanced options
  - append without extra header
  - non-float output
- write log to central file (dont write log-files, but capture output in python)
- write \lambda (strain factor) instead of step to the files
- GMSHInputModule with gmsh C++ API??
- investigate options for parallel computing
- PBC Output extent other side ?? (du/dX = (u2-u1)/deltaX)
## environment
- think of smarter ways to handle data

# Ideas
## RUC investigations
1. Notes
    - higher aspect ratios differ from the analytical solution ==> nonlinear effects stronger
    - higher strains seem to amplify the trends from low strains
    - smaller step size not always leads to better convergence
    - Global Buckling due to contacts/due to dynamic effects?
2. Questions
    - extension simulation as well as compression simulation
    - XY-shear differs from YX-shear (small strain case too?)
## test-cases
- single element (cantilever):
  - point force fails for large magnitudes
- test cases [Simo/Vu-Quoc]:
  - TEST 7.2 small step sized due to error in the way the follower load is applied `THIS IS FINE`
  - TEST 7.5 sufers the same problem as the single elements
## homogenization ideas
- for 2.5D structure use 2D homogenization with prism shear modulus/Youngs modulus in third direction scaled up to density?
 
# Information
## debugging
- call (dbg) call dprint(&...) to get any matrix/vector/... printed live
- when compiling OPT sets the flag `NDEBUG`
## .pro files
- `$(CASE_NAME)` also works with environment variables (and other settings)
- `include "xyz.pro";` works to include other .pro files
## others
- shape function node numbering in `$JIVEDIR/packages/geom/doc/images`
- `StdShape` for local coordinates and `Shape` for global coordinates
- `./jive xyz.pro |c++filt` for nicer stack traces
- `JEM_PRECHECK` gets executed always, `JEM_ASSERT` only in non-optimized mode