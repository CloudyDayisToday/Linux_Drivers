cmd_/home/christina/Linux_Drivers/scull/modules.order := {   echo /home/christina/Linux_Drivers/scull/scull.ko; :; } | awk '!x[$$0]++' - > /home/christina/Linux_Drivers/scull/modules.order
