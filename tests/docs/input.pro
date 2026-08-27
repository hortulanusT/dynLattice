// INPUT
Input.modules = [ "input", "nodeInput" ];

Input.input.type = "GMSHInput";
Input.input.verbose = false;
Input.input.store_tangents = true;
Input.input.onelab.Rotation = "180";
Input.input.onelab.BaseWidth='0.0083';
Input.input.onelab.LengthRatio='0.5883253012048192';
Input.input.onelab.Angle='55';
Input.input.onelab.Thickness='0.00075';
Input.input.onelab.RepX='6.0';
Input.input.onelab.RepY='4';
Input.input.onelab.Scale='1.0';
Input.input.file='tests/docs/re-entrant.geo';

Input.nodeInput.type = "PBCGroupInput";
Input.nodeInput.groupSettings.restrictToGroup = "points";
Input.nodeInput.ymax.xbounds=['ORIGIN.X+SIZE.X/2-SIZE.X/6','ORIGIN.X+SIZE.X/2+SIZE.X/6'];
Input.nodeInput.ymax.restrictToGroup = "beams";