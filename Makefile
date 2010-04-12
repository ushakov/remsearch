FAST_CGI_PATH = /usr/local
CXXFLAGS=-O0 -g `pkg-config --cflags glib-2.0 fastcgi++`
LDFLAGS = -lboost_thread-mt -lboost_filesystem-mt -lboost_regex-mt \
	-lboost_iostreams-mt \
	`pkg-config --libs glib-2.0 fastcgi++` 

all: serve gather-files

serve: serve.o disk-index.o

gather-files: gather-files.o placemark-storage.o

test: test.o placemark-storage.o

clean:
	rm *.o
