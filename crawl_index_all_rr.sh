#!/bin/bash
# crawl sequence of site bulk sets

./umount_ram.sh;
rm -f all_tld.dat;
rm -f all_robots.dat;
rm -f all_sites.dat;
touch all_tld.dat;
touch all_robots.dat;
touch all_sites.dat;

dom="local:8000";
crwldb="t2.dat";
deep="t2deep.dat";

if [ -n "$1" ]; then
    dom="$1";
fi

for i in $(seq 0 2);
do
    offset="$(($i*1000))";
    # crawl each charge
    ./t2res $offset;
    ./t2 $dom $offset;
    if [ ! -e "$deep" ]; then
	cp "$crwldb" "$deep";
    fi
    # must fit to previous call to t2, updates crwldb and deep
    ./t2sc $dom $offset;
    cat tld.dat >> all_tld.dat;
    cat robots.dat >> all_robots.dat;
    cat sites.dat >> all_sites.dat;

    outfolder="_""$dom""_""$i";
    mkdir "$outfolder";
    mv tld.dat "$outfolder";
    mv robots.dat "$outfolder";
    mv sites.dat "$outfolder";
    echo `date` > "$outfolder""/DATE.txt";
    zip -r "$outfolder.zip" "$outfolder";
    rm -r "$outfolder";
done

mv all_tld.dat tld.dat;
mv all_robots.dat robots.dat;
mv all_sites.dat sites.dat;

zip www_data.zip tld.dat robots.dat sites.dat;

rm tld.dat robots.dat sites.dat;

ln -fs $HOME/.avfs$PWD/www_data.zip# www_data;
./mount_ram.sh;

cp "$crwldb" ram/;
cp "$deep" ram/;
