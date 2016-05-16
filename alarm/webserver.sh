#!/bin/bash

# web server written in bash
read line
echo HTTP/1.1 200 OK
echo Content-Type: text/html
echo Connection: Closed
echo
# front page (only one HTML file allowed
if echo "$line" | grep -q 'GET / '
	then cat alarm.html
	exit;
fi
# 5 switch mode events
if echo "$line" | grep -q 'GET /mode-1 '
	then echo 1 > mode
	exit;
fi
if echo "$line" | grep -q 'GET /mode-2 '
	then echo 2 > mode
	exit;
fi
if echo "$line" | grep -q 'GET /mode-3 '
	then echo 3 > mode
	exit;
fi
if echo "$line" | grep -q 'GET /mode-4 '
	then echo 4 > mode
	exit;
fi
if echo "$line" | grep -q 'GET /mode-5 '
	then echo 5 > mode
	exit;
fi

echo 400
echo $line
