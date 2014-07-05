

source wipes.tcl

proc show {} {
    low_fill 0 0 0 0 255 0 30000
    after 5000
    low_fill 0 255 0 0 255 255 30000
    after 2000
}

proc ranshow {} {
    set r1 0
    set g1 0
    set b1 0
    while true {
    	set r2 [expr {int(rand() * 256)}]
    	set g2 [expr {int(rand() * 256)}]
    	set b2 [expr {int(rand() * 256)}]

	low_fill $r1 $g1 $b1 $r2 $g2 $b2 30000
	after 5000
	set r1 $r2
	set g1 $g2
	set b1 $b2
    }
}
ranshow
