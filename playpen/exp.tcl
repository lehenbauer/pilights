

package require spi
package require pilights

set nLEDs 160

pilight create lights $nLEDs 1000

lights open /dev/spidev0.0

proc splatter {row centerPixel nRows r g b} {
    set increment 2.0
    set multiplier 0.99
    set width 1.0
    for {set i 0} {$i < $nRows} {incr i} {
        set dWidth [expr {int($width)}]
	lights setpixels $row [expr {$centerPixel - $dWidth / 2}] $dWidth $r $g $b
	set width [expr {$width + $increment}]
	set increment [expr {$increment * $multiplier}]
	lights write $row 1 5000
	incr row
    }
}

proc suckup {row centerPixel width nRows r g b} {
    set increment 1.0
    set multiplier 1.01
    for {set i 0} {$i < $nRows} {incr i} {
        set dWidth [expr {int($width)}]
	lights clear $row
	lights setpixels $row [expr {$centerPixel - $dWidth / 2}] $dWidth $r $g $b
	set width [expr {$width - $increment}]
	if {$width == 0} {
	    break
	}
	set increment [expr {$increment * $multiplier}]
	lights write $row 1 5000
	incr row
    }
}

if {!$tcl_interactive} {
    sinwave
} else {
    puts "'sinwave' to see something"
}
 
