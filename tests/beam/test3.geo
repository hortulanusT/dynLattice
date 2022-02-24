radius = 0.254;
overshoot_angle = 5;
overshoot_angle = overshoot_angle/180 * Pi;
size = (Pi+2*overshoot_angle) * radius / 40;

// Center and arc points
Point(0) = { 0, 0, 0, size };
Point(1) = {  Cos(overshoot_angle)*radius, -Sin(overshoot_angle)*radius, 0, size };
Point(2) = { 0, radius, 0, size };
Point(3) = { -Cos(overshoot_angle)*radius, -Sin(overshoot_angle)*radius, 0, size };

// create a line
Circle(1) = { 1, 0, 2 };
Circle(2) = { 2, 0, 3 };