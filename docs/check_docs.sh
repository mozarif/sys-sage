#!/bin/bash

./show_undocumented.py
RESULT=$?
echo "Script exit status: $RESULT"
if [ $RESULT -ne 0 ]; then
  echo "Documentation is incomplete. The build will now fail."
  exit 1
fi
echo "Documentation check passed successfully."