

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

pilight create lights $nLEDs 1000

proc sinwave {} {
    set half [expr {$::nLEDs / 2}]
    for {set i 0.0} {$i < 300} {set i [expr {$i + 0.01}]} {
        lights fillrows 10 1 0 0 0
	set pixel [expr {$half + int(sin($i) * $half)}]
	#puts "$pixel"
	lights setpixels 10 $pixel 1 0 0 128
	lights write $::spi 10 1 5000
    }
}

sinwave

