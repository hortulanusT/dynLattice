// INPUT
Input.modules = [ "input", "groupInput" ];

Input.input.type = "GMSHInput";
Input.input.file = "$(CASE_NAME).geo";
Input.input.store_tangents = true;

Input.groupInput.type = "GroupInput";
Input.groupInput.nodeGroups = [ "fixed_left", "fixed_right" ];
Input.groupInput.fixed_left.ytype = "min";
Input.groupInput.fixed_right.ytype = "max";
