cmd_/home/christina/Linux_Drivers/Module.symvers := sed 's/\.ko$$/\.o/' /home/christina/Linux_Drivers/modules.order | scripts/mod/modpost -m -a  -o /home/christina/Linux_Drivers/Module.symvers -e -i Module.symvers   -T -
