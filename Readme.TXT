1.hex2bin (convert a hex file to a bin file)
command example: hex2bin xx.hex -s00000000 -l100
a xx.bin should be generated in the same folder.
the hex file name should have a .hex suffix.
-s set the start position in the hex
-l length

2.bin2hex (convert a bin file to a hex file)
command example: bin2hex xx.bin -s00000000 -l100 -bffff0000
a xx.hex should be generated in the same folder.
the bin file name should have a .bin suffix.
-s set the start position in the bin file (default: 0x00000000)
-l length (default: the length of bin file)
-b the linear position in the hex file (default: 0x00000000)
Note: the hex file does not contain the segment address

3.combineBin (merger 2 bin files into a file)
command example: combineBin aa.bin ab.bin bb.bin
