# ToDo 
## code
- Cleanup Import/Using (use a single import header file; use forward.h from JIVE)
- Setup proper PBC's
## environment
- Create repositrory structure (cookiecutter?)
## test-cases
- single element (cantilever):
  - get point force load to work
- test cases [Simo/Vu-Quoc]:
  - get TEST 7.2 to work with bigger step sizes
  - get TEST 7.5 to work
 
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