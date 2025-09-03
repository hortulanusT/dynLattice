# Contributing to dynLattice

## Quick Start

1. **Setup:**
   ```bash
   git clone https://github.com/hortulanusT/dynLattice.git
   cd dynLattice
   alias jive="apptainer exec $(pwd)/jive.sif"
   ```

2. **Build and test:**
   ```bash
   jive make tests
   ```

3. **Create feature branch and submit PR when ready**

## Code Organization

- **Materials:** `src/materials/` - Rod material models
- **Models:** `src/models/` - Boundary conditions, contacts, etc.
- **Modules:** `src/modules/` - Time integration, I/O, etc.
- **Tests:** `tests/` - Validation cases with `.geo`, `.pro` files

## Coding Style

- Use `#pragma once` for headers
- Follow existing naming: `PascalCase` classes, `camelCase` functions
- Document with [Doxygen](https://doxygen.nl/) style comments 
- Example from codebase:
  ```cpp
  /// @brief Handle model actions for impact boundary conditions
  /// @param action Action name to execute
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction
      (const String &action,
       const Properties &params,
       const Properties &globdat) override;
  ```

## Adding New Components

**New Material:**
1. Create `src/materials/YourMaterial.h/.cpp`
2. Inherit from `Material` (in `src/materials/Material.h`)
3. Register in material factory (`_declareMaterials.cpp`)

**New Model (boundary conditions, constraints, etc.):**
1. Create `src/models/YourModel.h/.cpp`
2. Inherit from `jive::model::Model`
3. Register in model factory (`_declareModels.cpp`)

**New Module (time integration, I/O, etc.):**
1. Create `src/modules/YourModule.h/.cpp`
2. Inherit from `jive::app::Module`
3. Register in module factory (`_declareModules.cpp`)

**New Test:**
1. Create `tests/your_test/` directory
2. Add `.geo` (geometry), `.pro` (program), input/model/output configs
3. Update `tests/testing.mk`

## Common Commands

```bash
# Development build
jive make debug                   # Debug with symbols
jive gdb bin/dynLattice-debug     # Debug with GDB
(gdb) call dprint(&...)           # print JEM/JIVE variable types

# "normal" build
jive make

# optimized build
jive make opt

# Documentation
jive make docs
```

## Issues/Questions

- Check existing issues first
- Provide minimal test case for bugs
- Include error messages and environment info
