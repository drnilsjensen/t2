#!/bin/sh

sudo ./umount_ram.sh
rm t2deep.dat
./t2 --clear-all
./t2res 0
# crawl first charge, next would be 1000, 2000 and so on.
./t2 local:8000 0
cp t2.dat t2deep.dat
# must fit to previous call
./t2sc local:8000 0
sudo ./mount_ram.sh
# simple query
./t2q .local:8000 "p=sand+surfen&l=38162"
