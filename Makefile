all:traffic iftrafficd

traffic:traffic.c
	gcc -o traffic traffic.c -lgd
	cp traffic /var/www/cgi-bin

iftrafficd:iftrafficd.c
	gcc -o iftrafficd iftrafficd.c
