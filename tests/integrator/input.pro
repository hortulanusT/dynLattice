// INPUT
Input.modules = [ "input", "groupInput" ];

Input.input.type = "GMSHInput";
Input.input.file = "$(CASE_NAME).geo";

Input.groupInput.type = "GroupInput";
Input.groupInput.nodeGroups = [ "fixed", "free" ];
Input.groupInput.fixed.ytype = "min";
Input.groupInput.free.ytype = "max";
