#!/bin/sh

set -e


# AFS directory containing additional data files for the PEC-tuples package
DISTRIBUTION_DIR=/afs/cern.ch/work/a/aapopov/public/distribution/PECTuples/

if [ ! -d "$DISTRIBUTION_DIR" ]; then
    echo "Do not have access to directory \"$DISTRIBUTION_DIR\". Aborted."
    exit 1
fi


# Nothing to copy in the current set-up
