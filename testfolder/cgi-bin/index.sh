#!/bin/sh
echo "Content-type: text/html"
echo "Cache-Control: private,max-age=1800"
echo ""
echo "<!DOCTYPE html>"
echo "<html><head>"
echo "<title>Home</title></head><body>"
cat "testfolder/index.htmlf"
echo "</body></html>"
