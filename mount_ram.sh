#!/bin/sh
mount ramfs -t ramfs ram
#mkdir ram/cache
cp t2.dat ram
cp t2deep.dat ram
#cp t2q ram
chmod o+rw ram/t2.dat
chmod o+rw ram/t2deep.dat
#chmod o+rw ram/cache
#chmod o+rx ram/t2q