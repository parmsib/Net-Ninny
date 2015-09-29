
all:
	gcc main.c client.c server.c util.c http.c -g -o ninny -D_GNU_SOURCE
