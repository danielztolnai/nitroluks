SRC_DIR = nitroluks.orig
VERSION = 0.1

init:
	mkdir -p build

clean:
	rm -rf build/
	rm -rf DEBUILD/

binary: init
	g++ src/nitro_luks.cpp -o build/nitro_luks -Lbuild/ -l:libnitrokey.so.3 -Wall

debianize: init
	rm -fr DEBUILD
	mkdir -p DEBUILD/${SRC_DIR}
	cp -r * DEBUILD/${SRC_DIR} || true
	(cd DEBUILD; tar -zcf nitroluks_${VERSION}.orig.tar.gz --exclude=${SRC_DIR}/debian ${SRC_DIR})
	(cd DEBUILD/${SRC_DIR}; debuild -us -uc)

install: binary
	install -D -m 0755 build/nitro_luks $(DESTDIR)/usr/bin/nitro_luks
	install -D -m 0755 keyscript.sh $(DESTDIR)/usr/bin/keyscript.sh
	install -D -m 0755 nitroluks_crypttab $(DESTDIR)/etc/nitroluks/nitroluks_crypttab.conf
	install -D -m 0755 initramfs-hook $(DESTDIR)/etc/initramfs-tools/hooks/nitroluks
