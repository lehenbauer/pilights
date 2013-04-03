

source lib.tcl

package require tclspi
package require pilights

set nLEDs 160

pilight create lights $nLEDs 36000

lights open /dev/spidev0.0

proc setup {} {
    rampart 20 500
    sprinkle 1000 10000 4 255 255 255
}

setup
play
