#!/bin/bash

available_modules=""
for dir in *; do
	if [ -f $dir/spec ]; then
		available_modules="$available_modules $dir"
	fi
done

if test "$1" == "ON"; then
	for file in *.web; do
		dir=`basename $file .web`
		if [ ! -f $dir/spec ]; then
			available_modules="$available_modules $dir"
		fi
	done
fi

echo $available_modules
