# N2 - Open, tiny web search engine

## Overview

N2 is for educational purposes and research. Use at your own risk!
The search engine is written in plain C under GNU/Linux (Ubuntu) and
it is composed of a crawler/indexer (t2/t2sc) and simple web front end (t2q).
t2res is for resetting portions of the index.
The project can be deployed on any standard web server that runs cgi.
For testing, python3 -m http.server is required.
The library curl from libcurl.se is used.

## Algorithms

N2 employs a linguistic-specific form of fuzzy hashing and reverse indexing that is currently
under development. Most relevant data is in main memory (<2 GB).
Some sites data is stored in BLOB files with each site consuming just the first 4K and no client-side
metadata. URLs are numbered instead of explicitly stored.
robots.txt is stored, and each 1st sitemap, if available.
The algorithms try to read the /impressum (imprint) URL to determine a city code (just DE-PLZ yet)
that is stored in the first 5 bytes in each sitemap bucket, as the information
there is always assumed to be redundant ('<?xml').
This is correlated with the 1st 3 bytes of the l-parameter in t2q.
Entries nearby the location are marked with '*' in the search output.

## Code quality (as of this pre-alpha release, work in progress)

Lots of C+HTML+JS code blunder to offer ;-) Needs lot of cleaning, code checking and documentation.
Caching may not be necessary and is turned off by default.
Sitemaps might be generated if none available in the future.
No profile is read under .t2/.profile, but flexible customization is planned for the future.
Extensions for other code formats and better Unicode support is planned.

## Getting Started

0) Make+Install libCURL from https://curl.se/download.html
e.g.:
- tar -xf curl-8.1.2.tar.gz
- cd curl-8.1.2
- make ssl
- sudo make install

Make sure python3 is installed and gcc.
Don't use other instances of python3 concurrently as we will kill blindly some python http servers later on

1) Download and install/download T2 in a folder of your choice, go to that folder with cd.
Make sure mydefs.h contains the correct configuration for either https oder http (testing).
Call 'make', that generates t2, t2sc, t2res, t2q. ./ram is by intention empty, as we'll use this as
a fast filesystem mounting point.

2) Run sudo setup_test_hosts_ONCE.sh. WARNING: this modifies /etc/hosts for some dummy setups and should be called only *once*!

3) Run run_test_environment.sh

4) Run test.sh, which populates the index with data from location ./testfolder and performs a first query on the command line;
a simple call of just t2 would be:
usage: t2 [--clear-all] <tld> <positive offset>
T2, T2SC, T2RES & T2Q (c) 2023 Nils Jensen
Licence: GNUPL3.0
This program comes with ABSOLUTELY NO WARRANTY; This is free software, and you are welcome to redistribute it under certain conditions; See COPYING in this folder.
Thanks to libcurl.se!

example: ./t2 de 0

5) Call the web client at [http://127.0.0.1:8001](http://127.0.0.1:8001);
apart from entering queries with some geo code (e.g. the Postleitzahl, looked up from client's LocalStorage)
you can evaluate Javascript by putting the query in brackets. Use (!...) to evaluate JS and to put it in Web storage for
automation/customization purposes. Push the search button to jump forward by +10 entries.
Enter a new query to reset.

6) Finish by calling stop_test_environment.sh and sudo ./umount_ram.sh

## Community

This project is public but I'm not accepting code contributions yet.
This will change in the future and be announced, together with more documentation. Thanks.

## Licence

T2 Copyright (c) 2023 Nils Jensen, njsplus(at)gmx.de, all parts if not stated otherwise
This software is under GNUPL 3.0

## Credits

Some geo data: kfz.net (with permission), simonfranzen(@)zauberware.com (in accordance to CCA 4.0; details at ./res/db/SOURCE)
web tools: libcurl.se, OpenSSL

## Contact

Mail me at njsplus(at)gmx.de with subject 'T2', or at n.jensen(at)ostfalia.de.
I might not be able to reply in every case, however, your message is much appreciated.

