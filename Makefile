
compile:
	gcc -Wall -Wextra -g -o project final_version_project.c -lSDL2 -lSDL2_ttf -lSDL2_image -lm

run:
	./project

clean:
	rm -f project

.PHONY: clean run compile