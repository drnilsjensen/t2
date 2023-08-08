#!/bin/sh

python3 -I -m http.server -b 127.0.0.1 -d testfolder 8000 --cgi &
python3 -I -m http.server -b 127.0.0.1 -d . 8001 --cgi &

ps | grep 'python3' | awk '{print $1}' > .server
