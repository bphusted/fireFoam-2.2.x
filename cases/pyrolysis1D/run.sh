#!/bin/sh
cd ${0%/*} || exit 1    # run from this directory

# Source tutorial run functions
. $WM_PROJECT_DIR/bin/tools/RunFunctions

./clean.sh
./mesh.sh

# Set application name
application="fireFoam"

$application > log.$application 2> log.error

# -----------------------------------------------------------------------------