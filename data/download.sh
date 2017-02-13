#!/bin/sh

set -e


# Files with preliminary JER
wget https://github.com/cms-jet/JRDatabase/raw/master/SQLiteFiles/Spring16_25nsV10_MC.db


# AFS directory containing additional data files for the PEC-tuples package
DISTRIBUTION_DIR=/afs/cern.ch/work/a/aapopov/public/distribution/PECTuples/

if [ ! -d "$DISTRIBUTION_DIR" ]; then
    echo "Do not have access to directory \"$DISTRIBUTION_DIR\". Aborted."
    exit 1
fi


# Nothing to copy in the current set-up
