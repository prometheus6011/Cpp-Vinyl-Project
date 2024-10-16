EXEC = cppaudiocapture
CLIB = -I/lib/portudio/include -L/lib/portaudio/lib -lrt -lasound -ljack -pthread -lportaudio

$(EXEC): main.cpp
	g++ -o $@ $^ $(CLIB)

install_deps:
	mkdir -p lib

	curl https://files.portaudio.com/archives/pa_stable_v190700_20210406.tgz | tar -zx -C lib
	cd lib/portaudio && ./configure && $(MAKE) -j
.PHONY: install_deps

uninstall_deps:
	cd lib/portaudio && $(MAKE) uninstall
	rm -rf lib/portaudio
.PHONY: uninstall_deps

clean:
	rm -f $(EXEC)
.PHONY: clean
