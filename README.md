# tube-amp
Tube amp emulator written in C by Arthur Befumo

This was completed as part of Yale's CPSC 490 Senior Project class. The associated paper can be found here: https://zoo.cs.yale.edu/classes/cs490/18-19b/befumo.arthur.atb44/finalprojectreport.pdf.

To compile, run make.

Usage: 
ampsim input_file output_file [-g input_gain -s output_scale -debug]

Optional flags:
-g (input_gain): Input gain is a multiplication factor applied to the input file before processing. It defaults to 0.4. Higher values generally result in higher distortion. It takes any double value (negative values will flip the incoming sound wave).

-s (output_scale): Output scale is the absolute value bounds on the output soundfile. It takes any value in the range [-1, 1]. Smaller values will result in a quieter output, and negative values will flip the soundwave in output.

-debug: The debug flag turns on optional messages which may help with debugging.
