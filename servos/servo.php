<?php

// adjustable sleep function
function my_sleep($secs) {
	// sleep for seconds: use sleep
	if ($secs > 1) sleep($secs);
	else if ($secs > 0.01) {
		// small intervalls
		time_nanosleep(0, 1000000000 * $secs);
	} else {
		// really small intervals: busy waiting
		$targettime = microtime(TRUE) + $secs;
		while (microtime(TRUE) < $targettime);
	}
}

// open pin file for permanent writing - adjust pin number to your needs
$pin = fopen("/sys/class/gpio/gpio11/value", "w");
function position($val) {
	global $pin;
	fwrite($pin, '1');
	// pin on for 1ms up to 2ms
	my_sleep(0.0015 + $val * 0.0005);
	fwrite($pin, '0');
	my_sleep(0.02);
}

$climbSteps = 100.0; // number of steps to smoothly lift the arm
$duration = 60; // duration of the wait cycle

// move arm down
for ($i = 0; $i < $climbSteps; $i++) position(1.0 - $i / $climbSteps);
// wait
for ($i = 0; $i < $duration * 1000 / 21; $i++) position(0.0);
// move arm up
for ($i = 0; $i < $climbSteps; $i++) position($i / $climbSteps);

// close pin file
fclose($pin);
