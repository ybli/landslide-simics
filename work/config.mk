
# -*- makefile -*-
# Do not edit this file.
# This file will be overwritten by the workspace setup script.

SIMICS_MODEL_BUILDER=/afs/cs.cmu.edu/academic/class/15410-f13/simics/simics-4.0.60/../simics-model-builder-4.0.16
SIMICS_BASE=/afs/cs.cmu.edu/academic/class/15410-f13/simics/simics-4.0.60
SIMICS_WORKSPACE=/afs/andrew.cmu.edu/usr12/bblum/fp/landslide/work
PYTHON=$(SIMICS_WORKSPACE)/bin/mini-python

INCLUDE_PATHS = /afs/cs.cmu.edu/academic/class/15410-f13/simics/simics-4.0.60/src/include:/afs/cs.cmu.edu/academic/class/15410-f13/simics/simics-4.0.60/../simics-model-builder-4.0.16/src/include

# Put user definitions in config-user.mk
-include config-user.mk

# allow user to override HOST_TYPE
ifeq (,$(HOST_TYPE))
 HOST_TYPE:=$(shell $(PYTHON) $(SIMICS_BASE)/scripts/host_type.py)
endif

include compiler.mk

include $(SIMICS_MODEL_BUILDER)/config/workspace/config.mk

