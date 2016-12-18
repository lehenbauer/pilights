

package require spi
package require pilights

set nLEDs 160

pilight create lights $nLEDs 1000

lights open /dev/spidev0.0

proc sinwave {} {
    set half [expr {$::nLEDs / 2}]
    for {set i 0.0} {$i < 300} {set i [expr {$i + 0.01}]} {
        lights fillrows 10 1 0 0 0
	set pixel [expr {$half + int(sin($i) * $half)}]
	#puts "$pixel"
	lights setpixels 10 $pixel 1 0 0 128

	set pixel2 [expr {int(abs(sin($i * 1.5)) * $::nLEDs)}]
	lights setpixels 10 $pixel2 1 0 128 0

	lights write 10 1 5000
    }
}

if {!$tcl_interactive} {
    sinwave
} else {
    puts "'sinwave' to see something"
}

