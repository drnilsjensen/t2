all:
	gcc t2.c tools.c -I/usr/local/include -L/usr/local/lib -lcurl -o t2
	gcc t2q.c tools.c -o t2q
	gcc t2sc.c tools.c -I/usr/local/include -L/usr/local/lib -lcurl -o t2sc
	gcc t2res.c -o t2res

clean:
	rm -f *.dat
	rm -f t2
	rm -f t2sc
	rm -f t2res
	rm -f t2q

