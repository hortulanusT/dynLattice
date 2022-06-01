DefineConstant [ 
  s = {0.1, Name "size"}
];

// 2 points
Point(1) = { 0, 0, 0, s };
Point(2) = { 1, 0, 0, s };

// create a line
Line(1) = { 1, 2 };