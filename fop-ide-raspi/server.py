#!/usr/bin/env python3

from http.server import SimpleHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
import os
import subprocess
import threading
import urllib.parse

PORT = 12840
FOP_FOLDER = '~/projekte/fop'

os.chdir('public') # web directory to serve from

subprocess.run('mkdir ../out', shell = True)

building = threading.Semaphore(1)
def compileFile(inp, filename):
	building.acquire()
	# write input to input.fop
	f = open('../input.fop', 'w')
	f.write(inp)
	f.close()
	# check validity of code
	p = subprocess.run(FOP_FOLDER + '/fopc ../input.fop definitions.fop', stderr = subprocess.PIPE, shell = True)
	if p.returncode != 0:
		result = (False, p.stderr)
	else:
		p = subprocess.run(FOP_FOLDER + "/fopc " + FOP_FOLDER + '/lib ' + FOP_FOLDER + '/backends/raspberry ../input.fop definitions.fop -o ../out', stderr = subprocess.PIPE, shell = True)
		if p.returncode != 0:
			result = (False, p.stderr)
		else:
			subprocess.run('bash build.sh', cwd = '../out', shell = True)
			f = open('../out/' + filename, 'rb')
			result = (True, f.read())
			f.close()
	building.release()
	return result


class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    """Handle requests in a separate thread."""

class FOPIdeRaspi(SimpleHTTPRequestHandler):
	def do_POST(self):
		p = urllib.parse.urlparse(self.path)
		params = urllib.parse.parse_qs(p.query)
		if p.path == '/compile':
			length = int(self.headers.get('content-length', 0))
			field_data = self.rfile.read(length).decode("utf-8")
			fields = urllib.parse.parse_qs(field_data)
			print(fields)
			result = compileFile(fields['code'][0], 'raspi/raspi/main.py' if fields['mode'][0] == 'main' else 'raspi/raspi/setup.sh')
			self.send_response(200 if result[0] else 422);
			self.send_header("Content-Type", "text/plain") # TODO: attachment
			self.end_headers()
			self.wfile.write(result[1])
		else:
			SimpleHTTPRequestHandler.do_GET(self)
	def do_GET(self):
		SimpleHTTPRequestHandler.do_GET(self)

webServer = ThreadedHTTPServer(("0.0.0.0", PORT), FOPIdeRaspi)
print("Server startet at http://localhost:%s" % (PORT))
try:
	webServer.serve_forever()
except KeyboardInterrupt:
	pass

webServer.server_close()
print("Server stopped")



