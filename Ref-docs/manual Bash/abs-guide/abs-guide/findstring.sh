#!/bin/bash
# findstring.sh:
# Find a particular string in binaries in a specified directory.

directory=/usr/bin/
fstring="Free Software Foundation"  # See which files come from the FSF.

for file in $( find $directory -type f -name '*' | sort )
do
  strings -f $file | grep "$fstring" | sed -e "s%$directory%%"
  #  In the "sed" expression,
  #+ it is necessary to substitute for the normal "/" delimiter
  #+ because "/" happens to be one of the characters filtered out.
  #  Failure to do so gives an error message (try it).
done  

exit 0

#  Exercise (easy):
#  ---------------
#  Convert this script to taking command-line parameters
#+ for $directory and $fstring.
