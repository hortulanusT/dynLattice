DefineConstant [ 
  x = {0, Name "X"}
  y = {0, Name "Y"}
  z = {0, Name "Z"}
];

Point(1) = {0,0,0, 0.01};
Point(2) = {x,y,z, 0.01};

Line(1) = { 1,2 };
