-include config
-include project
BACKENDS?=easyeditor
MOUNTPOINT?=/var/www/html/fopprojekt
OUT?=out
FOPC_PARAMETERS?=
FOP_FOLDER?=../fop
PROJECT_SRC?=../src

all:
	(cd $(FOP_FOLDER) && git pull)
	(git rev-parse HEAD; cd $(FOP_FOLDER); git rev-parse HEAD) > $(OUT)/revision
	(cd $(OUT) && find . -maxdepth 1 ! -name conf.json ! -name fop-files ! -name vendor ! -name composer.phar ! -name node_modules ! -name . -exec rm -rf {} +)
	$(FOP_FOLDER)/fopc -i data -i $(FOP_FOLDER)/data src $(FOP_FOLDER)/lib $(foreach backend,$(BACKENDS),$(FOP_FOLDER)/backends/$(backend)) -o $(OUT) $(FOPC_PARAMETERS)
	(cd $(OUT) && bash build.sh)

windows: stop all start

fast:
	$(FOP_FOLDER)/fopc -i data -i $(FOP_FOLDER)/data src $(FOP_FOLDER)/lib $(foreach backend,$(BACKENDS),$(FOP_FOLDER)/backends/$(backend)) -o $(OUT) $(FOPC_PARAMETERS) -repl

debug.fop:
	$(FOP_FOLDER)/fopc -i data -i $(FOP_FOLDER)/data src $(FOP_FOLDER)/lib $(foreach backend,$(BACKENDS),$(FOP_FOLDER)/backends/$(backend)) -d debug.fop $(FOPC_PARAMETERS)

PORT?=8008
ide:
	# mit Spezialcode zum Allokieren eines Ports falls der Proxy aktiv ist
	$(eval PORT2 := $(shell curl http://localhost:8611/register))
	$(eval PORT := $(or $(PORT2),$(PORT)))
	(while ! nc -z localhost $(PORT); do sleep 0.1; done; if [ "$(PORT2)" = "" ]; then xdg-open http://localhost:$(PORT); else echo http://srv.launix.de/p/p$(PORT2)/; fi)&
	(cd $(FOP_FOLDER); ./fopide --ide-dir $(FOP_FOLDER)/ide "$(shell pwd)/src" --data "$(shell pwd)/data" --data ./data $(foreach backend,$(BACKENDS),backends/$(backend)) ./lib -p$(PORT))

docs:
	(cd $(FOP_FOLDER) && git pull)
	rm $(OUT)/docs -rf
	$(FOP_FOLDER)/fopc -i data -i $(FOP_FOLDER)/data src $(FOP_FOLDER)/lib $(FOP_FOLDER)/backends/doku $(foreach backend,$(BACKENDS),$(FOP_FOLDER)/backends/$(backend)) -c 'Doku<>.' -o $(OUT) $(FOPC_PARAMETERS)
	(cd $(OUT) && mkdocs serve)

release.tar.xz: all 
	(cd $(OUT) && tar cavf ../release.tar.xz --exclude=conf.json --exclude='fop-files/[0-9]*' . && if [ -f "version.sh" ] ; then bash version.sh $(PROJECT_SRC) ; fi)

mount:
	mkdir $(MOUNTPOINT) || true
	mount --bind $(OUT) $(MOUNTPOINT)

init: update
	mkdir $(OUT) || true
	mkdir src || true
	mkdir data || true
	touch config || true
	if [ ! -f .gitignore ]; then echo 'config*' > .gitignore; echo $(OUT) >> .gitignore; echo '.*.swp' >> .gitignore; echo release.tar.xz >> .gitignore; echo makeremote >> .gitignore; fi

update:
	bash -c 'if [ ! -d $(FOP_FOLDER) ] ; then  git clone git@fop.launix.de:fop $(FOP_FOLDER) ; else cd $(FOP_FOLDER) && git pull; fi'
	(cd $(FOP_FOLDER)/tools && npm install && npm update)
	make -C $(FOP_FOLDER) update
	chmod +x $(FOP_FOLDER)/fopc $(FOP_FOLDER)/fopide
	cp $(FOP_FOLDER)/project_Makefile Makefile
	cp $(FOP_FOLDER)/makeremote makeremote
