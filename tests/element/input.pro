// INPUT
Input.modules = [ "input", "groupInput" ];

Input.input.type = "GMSHInput";
Input.input.file = "tests/element/test.geo";

Input.groupInput.type = "GroupInput";
Input.groupInput.nodeGroups = [ "fixed", "free" ];
Input.groupInput.fixed.xtype = "min";
Input.groupInput.free.xtype = "max";
Input.groupInput.fixed.ytype = "min";
Input.groupInput.free.ytype = "max";
Input.groupInput.fixed.ztype = "min";
Input.groupInput.free.ztype = "max";