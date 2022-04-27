cmd_/home/christina/Linux_Drivers/modules.order := {   echo /home/christina/Linux_Drivers/hello.ko; :; } | awk '!x[$$0]++' - > /home/christina/Linux_Drivers/modules.order
