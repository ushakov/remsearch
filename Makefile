CXXFLAGS=-O2 -g
LDFLAGS = -lboost_thread-mt -lboost_filesystem-mt -lboost_regex-mt -lboost_iostreams-mt -lfastcgipp

serve: serve.o disk-index.o

gather-files: gather-files.o

clean:
	rm *.o