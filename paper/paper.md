---
title: 'dynLattice: A finite element environment for dynamic simulation of beam networks and lattice metamaterials'
tags:
  - finite elements
  - geometrically exact beams
  - inelastic materials
  - explicit time marching
  - metamaterials
authors:
  - name: Til Gärtner
    orcid: 0000-0002-5088-855X
    affiliation: "1, 2"
  - given-names: Frans P.
    dropping-particle: van der
    surname: Meer
    orcid: 0000-0002-6691-1259
affiliations:
  - name: Delft University of Technology, The Netherlands
    index: 1
    ror: 02e2c7k09
  - name: Netherlands Organisation for Applied Scientific Research, The Netherlands
    index: 2
    ror: 01bnjb948
date: 14 August 2025
bibliography: paper.bib
---

# Summary
Beams and beam structures undergoing fast deformation often experience large strains and inelastic material behavior.
Understanding this behavior enables engineers and designers to design beam networks resulting in so-called mechanical metamaterials.
Metamaterials offer unique properties not found in naturally occurring materials.
Harnessing these properties presents new possibilities for a wide range of fields [@jiao23; @bryn25].
In order to enable engineers to design mechanical metamaterials, the relation between the geometry of the beam network and the resulting properties needs to be understood. 
For large deformations at high rates, this usually requires destructive testing, limiting the speed of developments.
This limitation can be overcome using efficient numerical tools, allowing for both the accurate and fast description of the inelastic deformation at high rates [@bonfanti24].

# Statement of need
In order to design metamaterials undergoing large deformations at high rates, [dynLattice]{.sc} provides both researchers and engineers in the field a toolkit for numerical experiments. 
[dynLattice]{.sc} is built on top of the openly available C++ `JIVE` finite element toolkit [@jive21; @nguyenthanh20]. 
In this toolkit, the implementation of custom _modules_, for parts of the program flow in the simulation framework, and of custom _models_, for anything related to the numerical representation of the system of partial differential equations.
In the present addition to the `JIVE` framework, the _models_ are split into a part representing the geometry and boundary conditions of the problem, and the _materials_, representing the materials considered in the problem.
This separation allows both the easy and user-friendly extension of the frameworks by simple inheritance of the provided classes as well as usability by structuring the inputs needed for the program flow and the problem description.
[dynLattice]{.sc} additionally uses `GMSH` [@geuzaine09] to mesh the imported geometries efficiently.  

[dynLattice]{.sc} has already been used in a series of publications [@gärtner24; @gärtner25; @gärtner_TNO] and laid the foundation for several student theses [@niessen22; @ijzendoorn24; @smit24].
During these efforts it was used both on big computational clusters [@DHPC2024] and on various local machines using the provided `apptainer`. 

# Acknowledgements
The research of Til Gärtner is financed by TNO through the PhD program of the Dutch Ministry of Defence.

# References
