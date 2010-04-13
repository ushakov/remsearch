#!/usr/bin/python

import httplib
import pickle
import re
import urllib

import tiles

class DownloadException(Exception):
  def __init__(self, msg):
    self.msg = msg

def ParseArgs(uri):
  if not uri:
    return {}
  kvpairs = uri.split('&')
  result = {}
  for kvpair in kvpairs:
    k, v = kvpair.split('=')
    result[k] = urllib.unquote(v)
  return result

class Cache(object):
  def __init__(self, filename):
    self.filename = filename

  def Load(self):
    print 'Loading cache'
    try:
      infile = open(self.filename, 'r')
      self._cache = pickle.load(infile)
      infile.close()
    except IOError:
      print 'No cache found'
      self._cache = {}

  def Save(self):
    print 'Saving cache'
    outfile = open(self.filename, 'w')
    pickle.dump(self._cache, outfile)
    outfile.close()

  def GetValue(self, key):
    return self._cache.get(key)

  def Add(self, key, value):
    self._cache[key] = value
    self.Save()


class ProxyHandler(object):
  def __init__(self, cache, tile_storage):
    self.cache = cache
    self.tile_storage = tile_storage

  def Register(self, server):
    server.RegisterProxyHandler(self._HandleRequest)

  def _SplitUrl(self, url):
    m = re.match('^http://([^/]+)(/?.*)$', url)
    if m == None:
      raise Exception('Invalid URL ' + url)
    host = m.group(1)
    uri = m.group(2)
    return (host, uri)

  def _HandleTileRequest(self, path):
    print 'Got tile request', path
    m = re.match('http://[a-z0-9]*.google.com/vt/(.*)', path)
    args = ParseArgs(m.group(1))
    if 'x' in args and 'y' in args and 'z' in args:
      x = int(args['x'])
      y = int(args['y'])
      zoom = int(args['z'])
      tile = self.tile_storage.GetTile(x, y, zoom)
      if not tile:
        raise Exception('Request for non-existing tile %d,%d@%d' % (x, y, zoom))
      return tile, 'image/png'

  def _HandleRequest(self, path):
    if re.match('http://[a-z0-9]*.google.com/vt/lyrs=m@[0-9]+', path):
      return self._HandleTileRequest(path)
    cached = self.cache.GetValue(path)
    if cached:
      print 'Returning cached copy of ', path
      return cached
    data = self._FetchUrl(path)
    # HACK HACK HACK
    mime_type = 'text/javascript'
    result = (data, mime_type)
    self.cache.Add(path, result)
    return result

  def _FetchUrl(self, url):
    (host, uri) = self._SplitUrl(url)
    conn = httplib.HTTPConnection(host)
    conn.connect()
    conn.request("GET", uri)
    r = conn.getresponse()
    data = r.read()
    conn.close()
    # handle redirection
    if r.status in [301, 302, 303]:
      new_loc = r.getheader('location')
      new_url = 'http://' + host + '/' + new_loc
      return self._FetchUrl(new_url)
    if r.status != 200:
      print 'Got HTTP status %d on %s' % (r.status, url)
      print 'Server response: %s' % data
      os.abort()
      raise DownloadException('Failed HTTP request ' + url + ' reason ' + r.reason)
    return data
