// straight cantilever along y; the number of elements is set from the input with Input.input.onelab.Elements
DefineConstant [ n = {16, Name "Elements"} ];
Point(1) = { 0, 0, 0, 1. / n };
Point(2) = { 0, 1, 0, 1. / n };
Line(1) = { 1, 2 };
