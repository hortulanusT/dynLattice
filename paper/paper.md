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
    affiliation: 1
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

There is a wide plethora of existing finite element toolkits available in the market, such as MOOSE [@moose], FEniCS [@fenics], or deal.II [@dealii], to name just the most prominent ones.
The usage of these libraries for explicit dynamics with inelastic beams however, would require substantial alteration of their code, as for example in FEniCS explicit dynamics is only available using lumped mass matrices, which would be nonphysical for the rotational inertia contributions of the beams, in deal.II neither beam elements nor explicit solves are available and in MOOSE only a limited Timoshenko-Ehrenfest beam is implemented with only elastic behavior.
Other finite element toolkits having built in beam-elements and explicit dynamics include Kratos [@kratos; @Dadvand2010] and Akantu [@Richart2024].
For Kratos similar problems as for MOOSE can be seen in only a limited set of beam elements being available and none supporting inelastic behavior in the cross-sectional strain measures.
Akantu, focusing on fracture mechanics and providing a solid representation of contact, on the other hand only provides Euler-Bernoulli beam elements, neglecting all shear deformation whithin the elements

# Background
The following is a short description of the beam kinematics and cross-sectional kinetics implemented in this software.
A more detailed description can be found in [@gärtner_diss]

TODO (~400 words)

# Publications
[dynLattice]{.sc} has already been used in a series of publications [@gärtner24; @gärtner25; @gärtner_TNO] and laid the foundation for several student theses [@niessen22; @ijzendoorn24; @smit24].
During these efforts it was used both on big computational clusters [@DHPC2024] and on various local machines using the provided `apptainer`. 

# Acknowledgements
The research of Til Gärtner is financed by TNO through the PhD program of the Dutch Ministry of Defence.

# References
