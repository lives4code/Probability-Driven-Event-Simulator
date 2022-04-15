USAGE:
to compile run make


to use you can either run ./simulator and be prompted to enter the variables
or you can run with a file argument the variables are assigned on each other new line 
as follows, see input.txt for an example. 

Cache policy 1: oldest, 2: largest
Number of Files
File per second Poisson Value
Pareto File Size Request Alpha
pareto File Request Beta
Cache Size
numSeconds
Pareto File Size Alpha
Pareto File Size Beta

a text file is given for the command ./simulator input.txt

It is of course recommended that you use 10000 for number of files and that your 
file size alpha be large than zero. Addiditionally file size Alpha* File Size Beta should 
multilply to equal one. 

