#!/bin/bash

# Delete filenames in current directory containing bad characters.

for filename in *
do
badname=`echo "$filename" | sed -n /[\+\{\;\"\\\=\?~\(\)\<\>\&\*\|\$]/p`
# Files containing those nasties:     + { ; " \ = ? ~ ( ) < > & * | $
rm $badname 2>/dev/null    # So error messages deep-sixed.
done

# Now, take care of files containing all manner of whitespace.
find . -name "* *" -exec rm -f {} \;
# The path name of the file that "find" finds replaces the "{}".
# The '\' ensures that the ';' is interpreted literally, as end of command.

exit 0

#---------------------------------------------------------------------
# Commands below this line will not execute because of "exit" command.

# An alternative to the above script:
find . -name '*[+{;"\\=?~()&lt;&gt;&*|$ ]*' -exec rm -f '{}' \;
exit 0
# (Thanks, S.C.)
