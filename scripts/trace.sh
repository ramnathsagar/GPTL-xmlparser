#!/bin/sh

#GPTL_HOME refers to the root directory of GPTL installation
export GPTL_HOME=/home/sshuser/gptl-2.005
#Name of the config file ( under config/ dir.)
export GPTL_CONFIG_FILE=gptl.xml


#While building app, its was linked with libgptl.so

./app -x 0 -T 63 -s 8G -m 4k -n 1M

