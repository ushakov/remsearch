import ioutils
import random
import re
import traceback

_JAVASCRIPT = """
<style type="text/css">
div.maintext {
  font-weight: normal;
  font-size: 10pt;
  font-family: Arial;
}
h1 {
  font-size: 12pt;
}
</style>
<script type="text/javascript">
function onImageClick(elt) {
  var zoom_factor = 4;
  if (!elt.zoomed) {
    elt.width = elt.width * zoom_factor;
    elt.zoomed = true;
  } else {
    elt.width = elt.width / zoom_factor;
    elt.zoomed = false;
  }
}
</script>
"""

_IMAGE_TAG=('<img onclick="javascript:onImageClick(this)" '
            'width="20%%" '
            'src="wmcache?path=%s"/>')

class WMCacheHandler(object):
  def __init__(self, storage,):
    self.storage = storage

  def Register(self, server):
    server.RegisterHandler('/wmcache', self._HandleGetCached)

  def _GetType(self, local_url):
    if local_url.endswith('.html'):
      return 'text/html'
    if local_url.endswith('.jpg'):
      return 'image/jpeg'
    return 'text/plain'

  def _HandleGetCached(self, args):
    local_url = args['path']
    if re.match('[^0-9a-z._]', local_url):
      raise Exception('Bad file requested')
    if not self.storage.Exists(local_url):
      raise Exception('Doesn\'t exist in cache: %s' % local_url)
    if local_url.endswith('.jpg'):
      return self.storage.Read(local_url), 'image/jpeg'
    if local_url.endswith('.html'):
      content = self.storage.Read(local_url)
      return self._FixupHTML(content), 'text/html'
    raise Exception('unknown local_url %s' % local_url)

  def _FixupHTML(self, html):
    # Inject CSS/Javascript.
    html = html.replace('</head>', _JAVASCRIPT + '</head>')
    images = re.findall('<img src="(wmimage[^"]*)"/>', html)
    html = re.sub('<img src="(wmimage[^"]*)"/>', '', html)
    html = html.replace('font-size:20px;', '')
    all_images = '\n'.join([_IMAGE_TAG % img for img in images])
    html = html.replace('<div style=\'margin:0 0 20px 0\'>', all_images + '<div class=\'maintext\'>')
    html = html.replace('<html>', '')
    html = html.replace('</html>', '')
    return html

