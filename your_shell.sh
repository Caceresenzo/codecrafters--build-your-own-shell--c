#!/bin/sh
#
# DON'T EDIT THIS!
#
# CodeCrafters uses this file to test your code. Don't make any changes here!
#
# DON'T EDIT THIS!
set -e
tmpFile=$(mktemp)
gcc app/*.c -o $tmpFile -g3 -Wfatal-errors
# exec valgrind --leak-check=full --show-leak-kinds=all $tmpFile "$@"
exec $tmpFile "$@"