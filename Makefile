CXXFLAGS=-O2 -g `pkg-config --cflags glib-2.0`
LDFLAGS = -lboost_thread-mt -lboost_filesystem-mt -lboost_regex-mt \
	-lboost_iostreams-mt -lfastcgipp \
	`pkg-config --libs glib-2.0`

serve: serve.o disk-index.o

gather-files: gather-files.o placemark-storage.o

test: test.o placemark-storage.o

clean:
	rm *.o