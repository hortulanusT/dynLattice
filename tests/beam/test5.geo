size = Pi/4*100/8;

// Center and arc points
Point(1) = { -100, 0, 0, size };
Point(2) = { 0, 0, 0, size };
Point(3) = { (Sin(Pi/4)-1)*100, Cos(Pi/4)*100, 0, size };

// create a line
Circle(1) = { 2, 1, 3 };