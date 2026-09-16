% POMPEIU_HAUSDORFF Compute lower and upper bounds on the Pompeiu-Hausdorff
% distance between two triangle soups A and B.
%
% [lower,upper_max,dA,time_taken_bvh,time_taken_bounds] = ...
%   pompeiu_hausdorff(VA,FA,VB,FB,tol,max_factor,normalize)
%
% Inputs:
%   VA  #VA by 3 list of vertex positions of mesh A
%   FA  #FA by 3 list of triangle indices into VA
%   VB  #VB by 3 list of vertex positions of mesh B
%   FB  #FB by 3 list of triangle indices into VB
%   Optional:
%     tol  tolerance for the difference between upper and lower bounds
%       {1e-8}
%     max_factor  factor to define the maximum allowed number of faces and
%       vertices in the subdivided mesh A with respect to the number of
%       faces and vertices of the initial mesh A {1000000}
%     normalize  whether to normalize tolerance by the length of the
%       diagonal of A's bounding box {true}
% Outputs:
%   lower  lower bound on the Pompeiu-Hausdorff distance
%   upper_max  upper bound on the Pompeiu-Hausdorff distance
%   dA  length of the diagonal of mesh A's bounding box
%   time_taken_bvh  time taken to build the BVH for mesh B
%   time_taken_bounds  time taken to compute the bounds
%
% Example:
%   [VA,FA] = readOBJ('meshes/107100.obj');
%   [VB,FB] = readOBJ('meshes/107100_sf.obj');
%   [lower,upper_max,dA] = pompeiu_hausdorff(VA,FA,VB,FB,1e-8,1000000,true);
