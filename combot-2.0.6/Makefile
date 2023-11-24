all: bot

bot: source/Makefile Makefile
	cd source; make
	mv source/ComBot .
	mv source/sendmsg .

clean:
	rm -rf source/*.o
	rm -rf source/ComBot
	rm -rf source/sendmsg
	rm -rf ComBot sendmsg
