r = DefineNumber[0.05];
Point(1) = {0, 0, 0, r};
Point(2) = {1, 0, 0, r};
Point(3) = {1, 1, 0, r};
Point(4) = {0, 1, 0, r};
Line(1) = {4, 3};
Line(2) = {2, 3};
Line(3) = {4, 1};
Line(4) = {1, 2};
Line Loop(6) = {3, 4, 2, -1};
Plane Surface(6) = {6};
Recombine Surface {6};
Physical Line("Inflow",2) = {1,2,3,4};
Physical Surface("Domain",9) = {6};