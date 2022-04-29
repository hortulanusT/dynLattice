// INPUT
Input.modules = [ "input", "groupInput" ];
Input.input.type = "GMSHInput";
Input.input.file = "$(CASE_NAME).geo";
Input.input.save_mesh = false;
Input.input.store_tangents = true;

Input.groupInput.type = "GroupInput";
Input.groupInput.nodeGroups = [ "fixed_left", "fixed_right", "free" ];
Input.groupInput.fixed_left.ytype = "min";
Input.groupInput.fixed_left.xtype = "min";
Input.groupInput.fixed_right.ytype = "min";
Input.groupInput.fixed_right.xtype = "max";
Input.groupInput.free.ytype = "max";