#!/bin/sh

#  setup_xcode.sh
#  g4gP2
#
#  Created by Eric Ameres on 9/5/21.
#
DIRECTORY=`dirname "$0"`
cd "$DIRECTORY"

mkdir ./g4gbuild
cd g4gbuild
cmake -GXcode ../../src/Project2
