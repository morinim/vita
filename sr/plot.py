#!/usr/bin/env python

import os
import subprocess
import time
from optparse import OptionParser


verbose = False


def plot1(pipe, opts, args):
    print >>pipe, "set xlabel 'GENERATION'"
    print >>pipe, "set ylabel 'FITNESS'"

    cmd = "plot [{from_gen}:{to_gen}] '{data}' index {from_run}:{to_run} using 2:4:6 title 'Population' with yerrorbars linestyle 1, '{data}' index {from_run}:{to_run} using 2:3 title 'Best' with lines linestyle 2".format(
        from_gen = "" if opts.from_gen is None else opts.from_gen,
        to_gen = "" if opts.to_gen is None else opts.to_gen,
        data = args[0],
        from_run = opts.from_run,
        to_run = opts.to_run)

    if verbose:
        print(cmd)
    print >>pipe, cmd


def plot2(pipe, opts, args):
    print >>pipe, "set xlabel 'GENERATION'"
    print >>pipe, "set ylabel 'EFFECTIVE SIZE'"

    cmd = "plot [{from_gen}:{to_gen}] '{data}' index {from_run}:{to_run} using 2:7:8 title 'Population' with yerrorbars linestyle 1".format(
        from_gen = "" if opts.from_gen is None else opts.from_gen,
        to_gen = "" if opts.to_gen is None else opts.to_gen,
        data = args[0],
        from_run = opts.from_run,
        to_run = opts.to_run)

    if verbose:
        print(cmd)
    print >>pipe, cmd

 
def plot3(pipe, opts, args):
    print >>pipe, "set xlabel 'GENERATION'"
    print >>pipe, "set ylabel 'NR. OF SYMBOLS'"

    cmd = "plot [{from_gen}:{to_gen}] '{data}' index {from_run}:{to_run} using 2:12 title 'Functions' with lines, '{data}' index {from_run}:{to_run} using 2:13 title 'Terminals' with lines, '{data}' index {from_run}:{to_run} using 2:14 title 'Active functions' with lines, '{data}' index {from_run}:{to_run} using 2:15 title 'Active terminals' with lines".format(
        from_gen = "" if opts.from_gen is None else opts.from_gen,
        to_gen = "" if opts.to_gen is None else opts.to_gen,
        data = args[0],
        from_run = opts.from_run,
        to_run = opts.to_run)

    if verbose:
        print(cmd)
    print >>pipe, cmd


def plot4(pipe, opts, args):
    pass


def plot(opts, args):
    pipe = subprocess.Popen("gnuplot -persist", shell=True, 
                            stdin=subprocess.PIPE).stdin

    loop = True
    while loop:
        reparse_args(opts,args)

        if (opts.image is not None):
            print >>pipe, "set terminal png"
            print >>pipe, "set output '"+opts.image+"'"

        print >>pipe, "set grid"
        print >>pipe, "set key bottom right"
        print >>pipe, "set style line 1 lt 1 pt 13"
        print >>pipe, "set style line 2 lt 1 lw 2 lc 2"

        if (opts.graph is None):
            # Uncomment the following to line up the axes
            print >>pipe, "set lmargin 11"
            # Gnuplot recommends setting the size and origin before going to
            # multiplot mode.
            # This sets up bounding boxes and may be required on some terminals
            print >>pipe, "set size 1,1"
            print >>pipe, "set origin 0,0"
            print >>pipe, "set multiplot"

        # Plot the first graph so that it takes a quarter of the screen
        if (opts.graph is None):
            print >>pipe, "set size 0.5,0.5"
            print >>pipe, "set origin 0,0.5"
        if (opts.graph is None or opts.graph==1):
            plot1(pipe,opts,args)

        # Plot the second graph so that it takes a quarter of the screen
        if (opts.graph is None):
            print >>pipe, "set size 0.5,0.5"
            print >>pipe, "set origin 0,0" 
        if (opts.graph is None or opts.graph==2):
            plot2(pipe,opts,args) 

        # Plot the third graph so that it takes a quarter of the screen
        if (opts.graph is None):
            print >>pipe, "set size 0.5,0.5"
            print >>pipe, "set origin 0.5,0.5" 
        if (opts.graph is None or opts.graph==3):
            plot3(pipe,opts,args) 
    
        # Plot the fourth graph so that it takes a quarter of the screen
        if (opts.graph is None):
            print >>pipe, "set size 0.5,0.5"
            print >>pipe, "set origin 0.5,0" 
        if (opts.graph is None or opts.graph==4):
            plot4(pipe,opts,args) 

        # On some terminals, nothing gets plotted until this command is issued
        if (opts.graph is None):
            print >>pipe, "unset multiplot"

        # Remove all customizations
        print >>pipe, "reset"

        if (opts.loop is None):
            loop = False
        else:
            time.sleep(opts.loop)


def get_max_dataset(filename):
    count = 4
    while count > 0:
        try:
            with open(filename, "rb") as file:
                return int(list(file)[-1].split(' ')[0])
        except IOError:
            time.sleep(1)
            --count       


def get_cmd_line_options():
    usage = "Usage: %prog [options] datafile"
    version = "%prog 1.0"
    parser = OptionParser(usage=usage, version=version)

    parser.add_option("-g", "--graph", dest="graph", type="int",
                      help="Plot only one graph")
    parser.add_option("-l", "--loop", dest="loop", type="int",
                      help="Refresh the plot reloading data every x seconds.")
    parser.add_option("", "--image", dest="image",
                      help="Saves the plot to a png file")
    parser.add_option("", "--from_gen", dest="from_gen", type="int",
                      help="Plots statistic starting from a minimum generation")
    parser.add_option("", "--to_gen", dest="to_gen", type="int",
                      help="Plots statistics up to a maximum generation")
    parser.add_option("-r", "--run", dest="run", type="int",
                      help="Plots statistics regarding one run")
    parser.add_option("", "--from_run", dest="from_run", default=0, type="int",
                      help="Plots statistics starting from a minimum run")
    parser.add_option("", "--to_run", dest="to_run", type="int",
                      help="Plots statistics up up to a maximum run")
    parser.add_option("-v", "--verbose", action="store_true", default=False, 
                      dest="verbose",
                      help="Turns on verbose mode")

    return parser


def reparse_args(options, args):
    max_dataset = get_max_dataset(args[0])

    if options.run is not None:
        options.from_run = options.run
        options.to_run   = options.run

    if options.to_run is None or options.to_run > max_dataset:
        options.to_run = max_dataset
    elif options.to_run <= 0:
        options.to_run = max_dataset + options.to_run + 1

    if options.from_run < 0:
        options.from_run = max_dataset + options.from_run + 1

    if options.from_run > options.to_run:
        options.from_run = options.to_run


def main():
    # Get argument flags and command options
    parser = get_cmd_line_options()
    (options,args) = parser.parse_args()

    global verbose
    verbose = options.verbose
   
    # Print out usage if no arguments are present
    if len(args) < 1:
        parser.error("\n\tPlease specify a datafile.")

    plot(options,args)


if __name__ == "__main__":
    main()
