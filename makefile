.PHONY: install i remove r

pathfind: pathfind.cpp
	g++ pathfind.cpp -o pathfind

install: pathfind
	sudo mv pathfind /usr/local/bin

i: install

remove:
	sudo rm /usr/local/bin/pathfind

r: remove
