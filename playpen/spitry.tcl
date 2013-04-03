

package require tclspi
package require pilights

proc setup_spi {} {
    set spi [spi #auto /dev/spidev0.0]

    $spi read_mode 0
    $spi write_mode 0
    $spi write_bits_word 8
    $spi read_bits_word 8
    $spi write_maxspeed 2000000
    $spi read_maxspeed 2000000

    return $spi
}

set spi [setup_spi]

set nLEDs 160

set lights [pilight create #auto $nLEDs 1000]

$lights write $spi 0

$lights setrow 1 "255 255 255"
$lights setrow 2 "128 0 0"
$lights setrow 3 "0 128 0"
$lights setrow 4 "0 0 128"

$lights write $spi 1

after 500

$lights write $spi 0

