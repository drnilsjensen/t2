#!/bin/sh
cp /etc/hosts /etc/hosts~
echo "\n# experiments only" >> /etc/hosts
echo "127.0.0.1       www.0.local" >> /etc/hosts
echo "127.0.0.1       www.b.local" >> /etc/hosts
echo "127.0.0.1       www.c.local" >> /etc/hosts
echo "127.0.0.1       www.q-.local" >> /etc/hosts
