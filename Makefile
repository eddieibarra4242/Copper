all: cu

cu:
	cd src && $(MAKE)

build_tests:
	cd tests && $(MAKE)

tests: build_tests
	clear
	bin/tests/scanner.test