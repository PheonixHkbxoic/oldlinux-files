#!/bin/bash

# This is a simple script that removes blank lines from a file.
# No argument checking.

# Same as
#    sed -e '/^$/d' filename
# invoked from the command line.

sed -e /^$/d "$1"
# The '-e' means an "editing" command follows (optional here).
# '^' is the beginning of line, '$' is the end.
# This match lines with nothing between the beginning and the end,
# blank lines.
# The 'd' is the delete command.

# Quoting the command-line arg permits
# whitespace and special characters in the filename.

exit 0
