DefineConstant [ 
  x = {0, Name "x"},  // x end
  y = {0, Name "y"},  // y end
  z = {0, Name "z"},  // z end
  nel = { 1, Name "nel"} // number of elements
];

// calc mesh size
size = Sqrt(x*x + y*y + z*z) / nel;
// construct the points
Point(1) = { 0, 0, 0, size };
Point(2) = { x, y, z, size };
// construct the line
Line(1) = { 1, 2 };
// get rid of uneccessary elements
Coherence;