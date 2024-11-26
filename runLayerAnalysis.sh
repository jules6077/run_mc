#!/bin/bash

# Check if at least one argument is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <parameter>"
    exit 1
fi

# Accessing the first argument passed to the script
rootFile="$1"
tdr="$2"

root -l "layer_analysis.C(\"$rootFile\", \"[72,108]\", \"Rand\", \"10 GeV\", \"Muon\")"
