all: cu

clean:
	rm -rf bin
	rm -rf debug
	rm -rf ./src/parser.c
	rm -rf ./tests/*.runner.c

cu:
	cd src && $(MAKE)

Unity:
	cd Unity && meson setup builddir
	cd Unity/builddir && meson compile
	cp Unity/builddir/libunity.a bin

test_utils: Unity
	cd test_utils && $(MAKE)

build_tests: cu test_utils
	cd tests && $(MAKE)

tests: build_tests
	clear
	bin/tests/scanner.test