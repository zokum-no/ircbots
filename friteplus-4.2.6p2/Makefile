
RM = rm -f
MV = mv
VERSION_LABEL = 4.2__BETA__
INSTALL_NAME = SucKeR
DISTRIB_DIR = SucKeR

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
          else if [ -x /bin/bash ]; then echo /bin/bash; \
          else echo sh; fi ; fi)


all:
	@echo Now compiling your bot.  You can also just type "Compile" to make your bot.
	@echo You can also used "make compress" to compress the source when finished.
	@Compile
	@${MV} src/bot ./${INSTALL_NAME}

config:
	@${CONFIG_SHELL} Configure -d src/config.in

compress:
	@echo Now compiling your bot, and compressing source.
	@Compile
	@${MV} src/bot ./${INSTALL_NAME}
	@Compress

compressed:
	@echo Now compiling your bot from compressed source code.
	@echo Compressing source when done.
	@Uncompress
	@Compile
	@${MV} src/bot ./${INSTALL_NAME}
	@Compress

distribution:
	@echo This will DELETE the files ${INSTALL_NAME} \*.o \*.bak core and \*~.
	@echo If you are sure, use make Distribution instead

Distribution:
	@${RM} ${INSTALL_NAME} *~ *.bak
	@cd src
	@${RM} *.o
	@cd ..
	@cd ..
	@tar -pzcvf fRitE+_v${VERSION_LABEL}.tgz ${DISTRIB_DIR}

clean:
	${RM} bot ${INSTALL_NAME} helpread *.o *.bak core 
