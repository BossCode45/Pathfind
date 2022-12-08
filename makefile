.PHONY: install i remove r

DESTDIR:=/usr/local/bin

pathfind: pathfind.cpp
	g++ pathfind.cpp -o pathfind

install: pathfind
	sudo mv pathfind $(DESTDIR)

i: install

remove:
	sudo rm $(DESTDIR)/pathfind

r: remove
