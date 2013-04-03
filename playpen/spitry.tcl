

package require tclspi
package require pilights

set nLEDs 160

set lights [pilight create #auto $nLEDs 1000]

$lights open /dev/spidev0.0

$lights write 0

$lights setrow 1 "255 255 255"
$lights setrow 2 "128 0 0"
$lights setrow 3 "0 128 0"
$lights setrow 4 "0 0 128"

$lights write 1

after 500

$lights write 0

$lights fillrows 10 1 0 0 10
$lights setpixels 10 50 10 10 0 0
