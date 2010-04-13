#!/usr/bin/python

import os

class HashDirStorage(object):
  def __init__(self, basedir):
    self.basedir = basedir
    
  def Exists(self, name):
    return os.path.exists(self._GetPath(name))
    pass
    
  def Read(self, name):
    in_file = open(self._GetPath(name), 'r')
    content = in_file.read()
    in_file.close()
    return content

  def Write(self, name, content):
    dirname = self._GetDir(name)
    if not os.path.exists(dirname):
      os.mkdir(dirname)
    out_file = open(dirname + '/' + name, 'w')
    out_file.write(content)
    out_file.close()

  def _HashFunc(self, name):
    value = 0
    for ch in name:
      value = int(value * 33 + ord(ch))
    value = value & 0xff
    return value

  def _GetDir(self, name):
    return self.basedir + '/' + ('%02x' % self._HashFunc(name))

  def _GetPath(self, name):
    return self._GetDir(name) + '/' + name
    

