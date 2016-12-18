

source lib.tcl

package require spi
package require pilights

set nLEDs 160

pilight create lights $nLEDs 36000

lights open /dev/spidev0.0

proc scatter_sins {} {
    for {set i 0} {$i < 50} {incr i} {
        set row [expr {int(rand() * [lights nRows])}]
	set nRows [expr {int(200 + rand() * 500)}]
	set startRadians [expr {rand() * 3.14159}]
	set increment [expr {rand() / 100}]
	set centerPixel [expr {int(rand() * [lights nLights])}]
	set pixelWidth [expr {int(20 + rand() * 50)}]
	set r 0
	set g 0
	set b 128
	sinwave $row $nRows $startRadians $increment $centerPixel $pixelWidth $r $g $b
    }
}

proc sprinkle_around {} {
    for {set i 0} {$i < 20} {incr i} {
        set row [expr {int(rand() * [lights nRows])}]
	sprinkle $row 1000 1 255 255 255
    }
}

proc setup {} {
    rampart 20 500
    scatter_sins
    sprinkle_around
}

setup
play
