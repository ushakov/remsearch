#!/usr/bin/python

def StrToInt(data):
  return ((ord(data[0]) << 24) +
          (ord(data[1]) << 16) +
          (ord(data[2]) << 8) +
          (ord(data[3])))


def IntToStr(num):
  result = ''
  for _ in range(4):
    char = chr(num & 255)
    result = char + result
    num >>= 8
  return result


def ReadByteArray(data):
  return map(ord, data)


def ReadIntArray(data):
  result = []
  length = len(data)
  assert length % 4 == 0
  for k in range(length / 4):
    result.append(StrToInt(data[k*4:k*4 + 4]))
  return result


def ReadFile(filename):
  in_file = open(filename, 'r')
  content = in_file.read()
  in_file.close()
  return content


def Int32ToStr(num):
  result = ''
  assert num < 2**32
  assert num >= 0
  for _ in range(4):
    char = chr(num & 255)
    result = char + result
    num >>= 8
  return result


def Int16ToStr(num):
  assert num >= 0
  assert num < 2**16
  result = ''
  for _ in range(2):
    char = chr(num & 255)
    result = char + result
    num >>= 8
  return result


def EncodeString(string):
  assert type(string) == unicode
  return Int32ToStr(len(string)) + ''.join(map(Int16ToStr, map(ord, string)))


def EncodeJsonStr(string):
  result = ''
  for ch in string:
    if ch in ['"']:
      result += '\\' + ch
    else:
      result += ch
  return '"' + result + '"'


def EncodeJson(value):
  if type(value) == int:
    return str(value)
  if type(value) == long:
    return str(value)
  if type(value) == float:
    return str(value)
  if type(value) == str:
    return EncodeJsonStr(value)
  if type(value) == unicode:
    return EncodeJsonStr(value.encode('utf8'))
  if type(value) == list:
    return EncodeJson(dict(enumerate(value)))
  if type(value) == dict:
    result = []
    for k, v in value.iteritems():
      result.append(EncodeJson(k) + ':' + EncodeJson(v))
    return '{' + ', '.join(result) + '}'
  raise Exception('Value %s has unsupported type %s' % (value, type(value)))

