#!/usr/bin/env python3
# Raspi flight simulator


#in /boot/config.txt:
#start_x=1
#gpu_mem=128
#dtparam=i2c1=on
# sudo pip3 install adafruit-circuitpython-pca9685

from __future__ import division
import time
 
import io
import picamera
import logging
import socketserver
from threading import Condition
from http import server

# Import the PCA9685 module.
import adafruit_pca9685
from board import SCL, SDA
import busio

# set up servos

i2c_bus = busio.I2C(SCL, SDA)
pwm = adafruit_pca9685.PCA9685(i2c_bus)
pwm.frequency = 50
def updatePos(x, y, u, v):
    # -255..255
    print("neue pos: " + str(x) + ", " + str(y) + ", " + str(u) + ", " + str(v))
    pwm.channels[0].duty_cycle = (int)(0xFFFF / 20 * (1.5 + x / 512))
    pwm.channels[1].duty_cycle = (int)(0xFFFF / 20 * (1.5 + y / 512))
    pwm.channels[2].duty_cycle = (int)(0xFFFF / 20 * (1.5 + u / 512))
    pwm.channels[3].duty_cycle = (int)(0xFFFF / 20 * (1.5 + v / 512))
 
# set up web server with camera and touch requests
PAGE="""\
<html>
<head>
<title>Raspberry Pi - Flight Simulator</title>
</head>
<body style="margin: 0px; padding: 0px; touch-action: none;">
<img src="stream.mjpg" width="800" height="480" style="width: 100vw; height: 56.25vw;">
<div id='info'></div>
<script type="text/javascript">
    var touches = {};
    var posX = 0, posY = 0;
    var posU = 0, posV = 0;

    var isDirty = false;
    var lastReq = null;
    function updatePos() {
        if (posX < -255) posX = -255;
        if (posX > 255) posX = 255;
        if (posY < -255) posY = -255;
        if (posY > 255) posY = 255;
        if (posU < -255) posU = -255;
        if (posU > 255) posU = 255;
        if (posV < -255) posV = -255;
        if (posV > 255) posV = 255;
        isDirty = true;
        if (!lastReq) sendPos();
    }
    function sendPos() {
        var req = new XMLHttpRequest();
        req.open('GET', 'pos/' + posX + '/' + posY + '/' + posU + '/' + posV);
        req.send();
        req.onreadystatechange = function () {
            if (req.readyState == 4) {
                isDirty = false;
                lastReq = null;
                if (isDirty) {
                    sendPos();
                }
            }
        }
        lastReq = req;
    }

    document.body.ontouchstart = function (e) {
        for (var i = 0; i < e.changedTouches.length; i++) {
            touches[e.changedTouches[i].identifier] = {
                x: e.changedTouches[i].pageX,
                y: e.changedTouches[i].pageY,
                isRight: e.changedTouches[i].pageX > document.body.clientWidth / 2
            };
        }
    };
    document.body.ontouchmove = function (e) {
        for (var i = 0; i < e.changedTouches.length; i++) {
            var relX = e.changedTouches[i].pageX - touches[e.changedTouches[i].identifier].x;
            var relY = e.changedTouches[i].pageY - touches[e.changedTouches[i].identifier].y;
            touches[e.changedTouches[i].identifier].x = e.changedTouches[i].pageX;
            touches[e.changedTouches[i].identifier].y = e.changedTouches[i].pageY;

            if (touches[e.changedTouches[i].identifier].isRight) {
                posU = posU + relX;
                posV = posV + relY;
            } else {
                posX = posX + relX;
                posY = posY + relY;
            }
            updatePos();
        }
        e.preventDefault();
    };
    document.body.ontouchend = function (e) {
        for (var i = 0; i < e.changedTouches.length; i++) {
            delete touches[e.changedTouches[i].identifier];
        }
    };
    body.onclick = function () {
        posX = 0;
        posY = 0;
        updatePos();
    }
</script>
</body>
</html>
"""
 
class StreamingOutput(object):
    def __init__(self):
        self.frame = None
        self.buffer = io.BytesIO()
        self.condition = Condition()
 
    def write(self, buf):
        if buf.startswith(b'\xff\xd8'):
            # New frame, copy the existing buffer's content and notify all
            # clients it's available
            self.buffer.truncate()
            with self.condition:
                self.frame = self.buffer.getvalue()
                self.condition.notify_all()
            self.buffer.seek(0)
        return self.buffer.write(buf)
 
class StreamingHandler(server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(301)
            self.send_header('Location', '/index.html')
            self.end_headers()
        elif self.path == '/index.html':
            content = PAGE.encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Content-Length', len(content))
            self.end_headers()
            self.wfile.write(content)
        elif self.path == '/stream.mjpg':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            try:
                while True:
                    with output.condition:
                        output.condition.wait()
                        frame = output.frame
                    self.wfile.write(b'--FRAME\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', len(frame))
                    self.end_headers()
                    self.wfile.write(frame)
                    self.wfile.write(b'\r\n')
            except Exception as e:
                logging.warning(
                    'Removed streaming client %s: %s',
                    self.client_address, str(e))
        elif self.path.startswith('/pos/'):
            self.send_response(200)
            self.end_headers()
            parts = self.path.split('/')
            v = float(parts.pop())
            u = float(parts.pop())
            y = float(parts.pop())
            x = float(parts.pop())
            updatePos(x, y, u, v)
        else:
            self.send_error(404)
            self.end_headers()
 
class StreamingServer(socketserver.ThreadingMixIn, server.HTTPServer):
    allow_reuse_address = True
    daemon_threads = True
 
with picamera.PiCamera(resolution='800x480', framerate=24) as camera:
    output = StreamingOutput()
    #Uncomment the next line to change your Pi's Camera rotation (in degrees)
    #camera.rotation = 90
    camera.start_recording(output, format='mjpeg')
    try:
        address = ('', 8000)
        server = StreamingServer(address, StreamingHandler)
        server.serve_forever()
    finally:
        camera.stop_recording()
