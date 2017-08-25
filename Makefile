all:traffic iftrafficd

traffic:traffic.c
	gcc -o traffic traffic.c -lgd
	cp traffic /var/www/cgi-bin

iftrafficd:iftrafficd.c
	gcc -o iftrafficd iftrafficd.c

indent:
	indent traffic.c iftrafficd.c -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4  \
-cli0 -d0 -di1 -nfc1 -i8 -ip0 -l160 -lp -npcs -nprs -npsl -sai \
-saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1
