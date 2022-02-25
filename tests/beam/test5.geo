angle = Pi/2;
radius = 100;
size = angle * radius / 8;

// Center and arc points
Point(1) = { -radius, 0, 0, size };
Point(2) = { 0, 0, 0, size };
Point(3) = { (Cos(angle)-1)*radius, Sin(angle)*radius, 0, size };

// create a line
Circle(1) = { 2, 1, 3 };