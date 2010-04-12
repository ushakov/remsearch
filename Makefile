FAST_CGI_PATH = /opt/fastcgi
CXXFLAGS=-O2 -g `pkg-config --cflags glib-2.0` -I$(FAST_CGI_PATH)/include
LDFLAGS = -lboost_thread-mt -lboost_filesystem-mt -lboost_regex-mt \
	-lboost_iostreams-mt -lboost_system-mt \
	`pkg-config --libs glib-2.0` -L$(FAST_CGI_PATH)/lib -lfastcgipp -Wl,-rpath $(FAST_CGI_PATH)/lib

all: serve gather-files

serve: serve.o disk-index.o

gather-files: gather-files.o placemark-storage.o

test: test.o placemark-storage.o

clean:
	rm *.o
