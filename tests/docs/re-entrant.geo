// Geometry parameters and mesh size
DefineConstant [ 
  scale = {1, Name "Scale"},      // scaling factor of the size
  b = {1e-3, Name "BaseWidth"},   // width of the base bar
  ratio = {1, Name "LengthRatio"},// ratio between both bar lengths
  alpha = {65, Name "Angle"},     // angle between those two [deg]  

  nx = {1, Name "RepX"},          // number of repetitions in x-dir
  ny = {1, Name "RepY"},          // number of repetitions in y-dir
  n_el = {8, Name "Elems"}        // number of elements in the smaller bar
  rot = {0, Name "Rotation"}      // rotation of the structure [deg]
];

b = b * scale;

// calculated values
l = ratio * b;
If (l<b)
  size = l / n_el;
Else
  size = b / n_el;
EndIf

alpha = alpha * Pi / 180;
rot = rot * Pi / 180;

bx = b/2;
lx = Cos(alpha) * l;
ly = Sin(alpha) * l;

dx = 2*b - 2*lx;
dy = 2*ly;

//////////////////////////////////
// construct an RUC //////////////
//////////////////////////////////
// get the points around the honeycomb
p1 = newp + 0;
p2 = newp + 1;
p3 = newp + 2;
p4 = newp + 3;
p5 = newp + 4;
p6 = newp + 5;

p7 = newp + 6;
p8 = newp + 7;

// define the points
Point(p1) = {-bx     , -ly, 0, size};
Point(p2) = { bx     , -ly, 0, size};
Point(p3) = { bx - lx,   0, 0, size};
Point(p4) = { bx     ,  ly, 0, size};
Point(p5) = {-bx     ,  ly, 0, size};
Point(p6) = {-bx + lx,   0, 0, size};

Point(p7) = { 2*bx - lx - dx, 0, 0, size};
Point(p8) = {-2*bx + lx + dx, 0, 0, size};

// get the lines around the honeycomb
l1 = newl + 0;
l2 = newl + 1;
l3 = newl + 2;
l5 = newl + 4;
l6 = newl + 5;

l7 = newl + 6;
l8 = newl + 7;


// define the lines
Line(l1) = {p1, p2};
Line(l2) = {p2, p3};
Line(l3) = {p3, p4};
Line(l5) = {p5, p6};
Line(l6) = {p6, p1};

Line(l7) = {p6, p7};
Line(l8) = {p3, p8};

///////////////////////////////////////
// Multiply according to the Repetition
///////////////////////////////////////
For ix In {1:nx}
  For iy In {1:ny}
    Translate {(ix-1)*dx,(iy-1)*dy,0} { Duplicata { Line { l1, l2, l3, l5, l6, l7, l8 }; } }
  EndFor
EndFor

// Rotate everything
allPoints[] = Point "*" ;
Rotate {{0,0,1},{0,0,0}, rot} { Point{ allPoints[] } ; }

// get rid of uneccessary elements
Coherence;
