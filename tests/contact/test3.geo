// center + arc points
Point(1) = { 0, -1, 0, .1 }; // center
Point(2) = { -Sin(Pi/4), -1+Sin(Pi/4), 0, .1 }; // start
Point(3) = { +Sin(Pi/4), -1+Sin(Pi/4), 0, .1 }; // end

Point(4) = { -1., 0.1, 0, .1 };
Point(5) = { +1., 0.1, 0, .1 };

// create a circle
Circle(1) = { 2, 1, 3 };
Line(2) = { 4, 5 };