#!/usr/bin/python

import ioutils

class TileStorage(object):
  def __init__(self, path):
    self._ParseIndex(ioutils.ReadFile(path + '/index.new'))
    self.datafile = open(path + '/map.data', 'r')
    print 'Loaded %d tiles' % self.entries

  def _ParseIndex(self, data):
    self.entries = ioutils.StrToInt(data[0:4])
    data = data[4:]

    self.x = ioutils.ReadIntArray(data[0:4*self.entries])
    data = data[4*self.entries:]
    self.y = ioutils.ReadIntArray(data[0:4*self.entries])
    data = data[4*self.entries:]
    self.zoom = ioutils.ReadByteArray(data[0:self.entries])
    data = data[self.entries:]
    self.start = ioutils.ReadIntArray(data[0:4 * (self.entries + 1)])

  def _Compare(self, tz, tx, ty, index):
    if tz != self.zoom[index]:
      return tz - self.zoom[index]
    if tx != self.x[index]:
      return tx - self.x[index]
    return ty - self.y[index]

  def _Search(self, zoom, x, y):
    low = 0
    high = self.entries - 1
    # print "searching for %d,%d@%d" % (x, y, zoom)
    while high - low > 1:
      middle = (low + high) / 2
      d = self._Compare(zoom, x, y, middle)
      if d == 0:
        return middle
      if d < 0:
        high = middle
      else:
        low = middle + 1
    if self._Compare(zoom, x, y, low) == 0:
      return low
    return -1

  def GetTile(self, x, y, zoom):
    index = self._Search(zoom, x, y)
    if index < 0:
      return None
    offset = self.start[index]
    length = self.start[index + 1] - self.start[index]
    assert offset >= 0
    assert length > 0
    self.datafile.seek(offset, 0)
    return self.datafile.read(length)


class TileRequestHandler(object):
  def __init__(self, tile_storage):
    self.tile_storage = tile_storage

  def Register(self, server):
    server.RegisterHandler('/gettile', self._HandleTileRequest)

  def _HandleTileRequest(self, args):
    x = int(args['x'])
    y = int(args['y'])
    zoom = int(args['z'])
    tile = self.tile_storage.GetTile(x, y, zoom)
    if not tile:
      raise Exception('Request for non-existing tile %d,%d@%d' % (x, y, zoom))
    return tile, 'image/png'
