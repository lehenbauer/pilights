#
# this uses tclspi to create an SPI device and configures it through the
#  tcl interface to doing that, then attaches the tclspi object to
#  the pilights object.
#
#  the easier way is to just use "pilights open", but this approach more
#  fine-grained control of the SPI bus configuration.
#

package require spi
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

lights attach $spi

