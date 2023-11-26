all: bot

bot: source/Makefile Makefile
	cd source; make
	
plugins: source/Makefile Makefile
	cd source; make flash3
	cd source; make flash5
	cd source; make mflash
	cd source; make tspoof

clean:
	rm -f EnergyMech *core *.o *.a sendmsg flash3 flash5 mflash tspoof
	cd source; rm -f EnergyMech *core *.o *.a sendmsg flash3 flash5 mflash tspoof
