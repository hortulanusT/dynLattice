# Contributing to dynLattice

Thank you for your interest in contributing to dynLattice! This guide will help you get started with development.

## Development Setup

1. **Fork and clone** the repository from GitHub
  - set up LFS to download the `.sif` file with the repository
  - otherwise simply download the `.sif` file directly from the [original repository](https://github.com/hortulanusT/dynLattice/blob/main/jive.sif)
  - alternatively build the `.sif` file yourself using the `.def` file and the additional dependencies listed in the [documentation](doc/install.md)
2. **Set up the container alias**
   ```bash
   alias jive="apptainer exec -e --env DISPLAY=$DISPLAY $(pwd)/jive.sif"
   ```
3. **Create a feature branch:**
   ```bash
   git checkout -b feature/your-feature-name
   ```
4. **Build and test:**
   ```bash
   jive make tests
   ```

## Development Workflow

- Create feature branches
- Make atomic commits with clear messages
- Update documentation as needed
- Submit pull requests

## Coding Standards

### Style Guidelines

- Use `#pragma once` for headers
- Follow existing naming conventions:
  - `PascalCase` for classes and types
  - `camelCase` for functions and variables
  - `camelCase_` for private/protected functions and variables
  - `UPPER_CASE` for constants
- Document public interfaces with [Doxygen](https://doxygen.nl/) style comments

### Documentation Example

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

### Materials (Rod Constitutive Models)

1. Create `src/materials/YourMaterial.h` and `src/materials/YourMaterial.cpp`
2. Inherit from the `Material` base class (`src/materials/Material.h`)
3. Implement required virtual methods for stress-strain relationships
4. Register in `_declareMaterials.cpp`

### Models (Boundary Conditions, Constraints, etc.)

1. Create `src/models/YourModel.h` and `src/models/YourModel.cpp`
2. Inherit from `jive::model::Model`
3. Implement `takeAction()` for relevant solver actions
4. Register in `_declareModels.cpp`

### Modules (Time Integration, I/O, etc.)

1. Create `src/modules/YourModule.h` and `src/modules/YourModule.cpp`
2. Inherit from `jive::app::Module`
3. Implement required module interface methods
4. Register in `_declareModules.cpp`

### Validation Tests

1. Create a new directory in `tests/` (e.g., `tests/your_test/`)
2. Add required files:
   - `.geo` file for geometry definition (GMSH format)
   - `.pro` file for problem configuration
   - Input/model/output configuration files
   - `.py` for the post-processing
3. Update `tests/testing.mk` to include your test
4. Document expected behavior and reference solutions

## Build Commands

### Development Builds

```bash
# Debug build with symbols
jive make debug

# Standard build  
jive make

# Optimized build
jive make opt

# Clean build artifacts
jive make clean
```

### Testing

```bash
# Run all validation tests
jive make tests

# Run specific test category
jive make beam-tests
jive make contact-tests
jive make plastic-tests
jive make transient-tests
```

### Documentation

```bash
# Generate API documentation
jive make doc

# Open documentation in browser
firefox doc/html/index.html
```

### Debugging

```bash
# Debug with GDB
jive gdb bin/dynLattice-debug

# Within GDB, print JEM/JIVE variables
(gdb) call dprint(&variable_name)
```

## Submitting Changes

1. Push your feature branch to your fork
2. Create a pull request
3. Provide a clear description of changes
4. Include relevant publications

## Getting Help

- **Issues**: Check [existing issues](https://github.com/hortulanusT/dynLattice/issues) first
- **Bugs**: Provide minimal test case and environment details
- **Questions**: Open a discussion or issue with your question
- **Documentation**: See `doc/` directory for technical details
