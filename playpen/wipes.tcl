

package require tclspi
package require pilights

set nLEDs 160

pilight create lights $nLEDs 1

lights open /dev/spidev0.0

proc low_fill {startR startG startB endR endG endB delay} {
    lights fillrows 0 1 $startR $startG $startB
    lights write 0 1 $delay
    for {set i 0} {$i < $::nLEDs} {incr i} {
	lights setpixels 0 $i 1 $endR $endG $endB
	lights write 0 1 $delay
    }
}

proc high_fill {startR startG startB endR endG endB delay} {
    lights fillrows 0 1 $startR $startG $startB
    lights write 0 1 $delay
    for {set i [expr {$::nLEDS - 1}]} {$i >= 0} {incr i -1} {
	lights setpixels 0 $i 1 $endR $endG $endB
	lights write 0 1 $delay
    }
}

proc fill_to_center {startR startG startB endR endG endB delay} {
    lights fillrows 0 1 $startR $startG $startB
    lights write 0 1 $delay
    set n [expr {$nLEDs / 2}]
    for {set i 0} {$i < $n} {incr i} {
	lights setpixels 0 $i 1 $endR $endG $endB
	lights setpixels 0 [expr {$::nLEDs - $i}] 1 $endR $endG $endB
	lights write 0 1 $delay
    }
}

proc fill_from_center {startR startG startB endR endG endB delay} {
    lights fillrows 0 1 $startR $startG $startB
    lights write 0 1 $delay
    set n [expr {($nLEDs / 2) - 1}]
    for {set i [expr {($::nLEDs / 2) - 1}]} {$i > 0} {incr i -1} {
	lights setpixels 0 $i 1 $endR $endG $endB
	lights setpixels 0 [expr {$::nLEDs - $i - 1}] 1 $endR $endG $endB
	lights write 0 1 $delay
    }
}

