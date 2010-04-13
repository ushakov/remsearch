#!/usr/bin/python

import sys

import http_server
import proxy
import storage
import tiles
import wm_cache

_DEFAULT_PORT = 15000

def RunServer():
  tiles_path = sys.argv[1]
  hash_dir_root = sys.argv[2]

  server = http_server.MoreBaseHttpServer(_DEFAULT_PORT)

  proxy_cache = proxy.Cache('localcache')
  proxy_cache.Load()

  wmcache = storage.HashDirStorage(hash_dir_root)
  wmcache_handler = wm_cache.WMCacheHandler(wmcache)
  wmcache_handler.Register(server)

  tile_storage = tiles.TileStorage(tiles_path)

  host_handler = proxy.ProxyHandler(proxy_cache, tile_storage)
  host_handler.Register(server)

  print 'OK. Serving.'

  server.Serve()


def main():
  RunServer()


if __name__ == '__main__':
    main()
