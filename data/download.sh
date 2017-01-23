#!/bin/sh

set -e


# Files with preliminary JEC
wget https://github.com/cms-jet/JECDatabase/raw/master/SQLiteFiles/Summer16_23Sep2016AllV2_DATA.db
wget https://github.com/cms-jet/JECDatabase/raw/master/SQLiteFiles/Summer16_23Sep2016V2_MC.db


# AFS directory containing additional data files for the PEC-tuples package
DISTRIBUTION_DIR=/afs/cern.ch/work/a/aapopov/public/distribution/PECTuples/

if [ ! -d "$DISTRIBUTION_DIR" ]; then
    echo "Do not have access to directory \"$DISTRIBUTION_DIR\". Aborted."
    exit 1
fi


# Nothing to copy in the current set-up
