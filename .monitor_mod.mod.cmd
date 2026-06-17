savedcmd_monitor_mod.mod := printf '%s\n'   monitor_mod.o | awk '!x[$$0]++ { print("./"$$0) }' > monitor_mod.mod
