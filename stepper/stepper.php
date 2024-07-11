<?php

function my_sleep($secs) {
	if ($secs >= 1) sleep($secs);
	else if ($secs > 0.01) {
		time_nanosleep(0, 1000000000 * $secs);
	} else {
		$targettime = microtime(TRUE) + $secs;
		while (microtime(TRUE) < $targettime) sleep(0);
	}
}

$pins = array( // 11
	fopen("/sys/class/gpio/gpio25/value", "w"),
	fopen("/sys/class/gpio/gpio22/value", "w"),
	fopen("/sys/class/gpio/gpio23/value", "w"),
	fopen("/sys/class/gpio/gpio24/value", "w")
);
$pos = 0;
fwrite($pins[$pos], '1');
function move($forwd = true) {
	global $pins;
	global $pos;
	$t = 0.1;
	$opos = $pos;
	if ($forwd) {
		$pos++;
		if ($pos > 3) $pos = 0;
	} else {
		$pos--;
		if ($pos < 0) $pos = 3;
	}
	fwrite($pins[$pos], '1');
	my_sleep($t);
	fwrite($pins[$opos], '0');
	my_sleep($t);
}

for (;;) {
	$w = 100;
	for ($i = 0; $i < $w; $i++) {
		move(true);
	}
	for ($i = 0; $i < $w; $i++) {
		move(false);
	}
}

foreach($pins as $pin) fclose($pin);
