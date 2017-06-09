all:
	checkmodule -M -m -o traffic.mod traffic.te
	semodule_package -o traffic.pp -m traffic.mod
	semodule -i traffic.pp
