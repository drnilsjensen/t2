#!/bin/sh
mountavfs
sudo mount ramfs -t ramfs ram
sudo cp t2.dat ram/t2.dat
sudo cp t2deep.dat ram/t2deep.dat
sudo chmod o+rw ram/t2.dat
sudo chmod o+rw ram/t2deep.dat


