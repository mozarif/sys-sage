#!/usr/bin/env python3

# Shows undocumented symbols from the sys-sage project.
# Requires https://github.com/psycofdj/coverxygen, install via: pip3 install coverxygen
# Generate coverage file: python3 -m coverxygen --xml-dir xml --src-dir ../src/ --output doc-coverage.info --format json-v3

import sys
import json

if len(sys.argv) != 2:
    print("Missing input file argument")
    exit(-1)

cov = json.load(open(sys.argv[1]))

with open(sys.argv[1], 'r') as f:
    print(f.read())
    
print(f"Loading coverage file: {sys.argv[1]}")

missing = 0
for file in cov['files']:
    for sym in cov['files'][file]:
        if not sym['documented'] and sym['kind'] not in ['file', 'namespace']:
            print("[{}:{}]  \033[0;33m{}\033[0m".format(
                sym['file'], sym['line'], sym['symbol']
            ))
            missing += 1

print(f"Undocumented symbols: {missing}")

exit(missing)
