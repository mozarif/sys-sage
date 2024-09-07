#!/bin/bash

cd docs
./show_undocumented.py doc-coverage.info
RESULT=$?
echo "Script exit status: $RESULT"
if [ $RESULT -ne 0 ]; then
  echo "Documentation is incomplete. The build will now fail."
  exit 1
fi
echo "Documentation check passed successfully."