radius = 0.254;
size = Pi * radius / 40;

// Center and arc points
Point(1) = { radius, 0, 0, size };
Point(2) = { 0, 0, 0, size };
Point(3) = { -radius, 0, 0, size };

// create a line
Circle(1) = { 1, 2, 3 };