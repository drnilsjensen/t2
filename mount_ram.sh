#!/bin/sh
mount ramfs -t ramfs ram
cp t2.dat ram/t2.dat
cp t2deep.dat ram/t2deep.dat
chmod o+rw ram/t2.dat
chmod o+rw ram/t2deep.dat


