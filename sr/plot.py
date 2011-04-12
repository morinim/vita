#!/usr/bin/env python3

import argparse
import os
import subprocess
import time


verbose = False


def plot1(pipe, args):
    pipe.write(b"set xlabel 'GENERATION'\n")
    pipe.write(b"set ylabel 'FITNESS'\n")

    cmd = "plot [{from_gen}:{to_gen}] '{data}' index {from_run}:{to_run} using 2:4:6 title 'Population' with yerrorbars linestyle 1, '{data}' index {from_run}:{to_run} using 2:3 title 'Best' with lines linestyle 2\n".format(
        from_gen = "" if args.from_gen is None else args.from_gen,
        to_gen = "" if args.to_gen is None else args.to_gen,
        data = args.dynfile,
        from_run = args.from_run,
        to_run = args.to_run)

    if verbose:
        print(cmd)
    pipe.write(str.encode(cmd))


def plot2(pipe, args):
    pipe.write(b"set xlabel 'GENERATION'\n")
    pipe.write(b"set ylabel 'EFFECTIVE SIZE'\n")

    cmd = "plot [{from_gen}:{to_gen}] '{data}' index {from_run}:{to_run} using 2:7:8 title 'Population' with yerrorbars linestyle 1\n".format(
        from_gen = "" if args.from_gen is None else args.from_gen,
        to_gen = "" if args.to_gen is None else args.to_gen,
        data = args.dynfile,
        from_run = args.from_run,
        to_run = args.to_run)

    if verbose:
        print(cmd)
    pipe.write(str.encode(cmd))

 
def plot3(pipe, args):
    pipe.write(b"set xlabel 'GENERATION'\n")
    pipe.write(b"set ylabel 'NR. OF SYMBOLS'\n")

    cmd = "plot [{from_gen}:{to_gen}] '{data}' index {from_run}:{to_run} using 2:12 title 'Functions' with lines, '{data}' index {from_run}:{to_run} using 2:13 title 'Terminals' with lines, '{data}' index {from_run}:{to_run} using 2:14 title 'Active functions' with lines, '{data}' index {from_run}:{to_run} using 2:15 title 'Active terminals' with lines\n".format(
        from_gen = "" if args.from_gen is None else args.from_gen,
        to_gen = "" if args.to_gen is None else args.to_gen,
        data = args.dynfile,
        from_run = args.from_run,
        to_run = args.to_run)

    if verbose:
        print(cmd)
    pipe.write(str.encode(cmd))


def plot4(pipe, args):
    pass


def plot(args):
    pipe = subprocess.Popen("gnuplot -persist", shell=True,
                            stdin=subprocess.PIPE).stdin

    loop = True
    while loop:
        reparse_args(args)

        if (args.image is not None):
            pipe.write(b"set terminal png"+"\n")
            pipe.write(b"set output '"+args.image+"'\n")

        pipe.write(b"set grid\n")
        pipe.write(b"set key bottom right\n")
        pipe.write(b"set style line 1 lt 1 pt 13\n")
        pipe.write(b"set style line 2 lt 1 lw 2 lc 2\n")

        if (args.graph is None):
            # Uncomment the following to line up the axes
            pipe.write(b"set lmargin 11\n")
            # Gnuplot recommends setting the size and origin before going to
            # multiplot mode.
            # This sets up bounding boxes and may be required on some terminals
            pipe.write(b"set size 1,1\n")
            pipe.write(b"set origin 0,0\n")
            pipe.write(b"set multiplot\n")

        # Plot the first graph so that it takes a quarter of the screen
        if (args.graph is None):
            pipe.write(b"set size 0.5,0.5\n")
            pipe.write(b"set origin 0,0.5\n")
        if (args.graph is None or args.graph==1):
            plot1(pipe, args)

        # Plot the second graph so that it takes a quarter of the screen
        if (args.graph is None):
            pipe.write(b"set size 0.5,0.5\n")
            pipe.write(b"set origin 0,0\n")
        if (args.graph is None or args.graph==2):
            plot2(pipe, args) 

        # Plot the third graph so that it takes a quarter of the screen
        if (args.graph is None):
            pipe.write(b"set size 0.5,0.5\n")
            pipe.write(b"set origin 0.5,0.5\n")
        if (args.graph is None or args.graph==3):
            plot3(pipe, args) 
    
        # Plot the fourth graph so that it takes a quarter of the screen
        if (args.graph is None):
            pipe.write(b"set size 0.5,0.5\n")
            pipe.write(b"set origin 0.5,0\n")
        if (args.graph is None or args.graph==4):
            plot4(pipe, args) 

        # On some terminals, nothing gets plotted until this command is issued
        if (args.graph is None):
            pipe.write(b"unset multiplot\n")

        # Remove all customizations
        pipe.write(b"reset\n")

        if (args.loop is None):
            loop = False
        else:
            time.sleep(args.loop)


def get_max_dataset(filename):
    count = 4
    while count > 0:
        try:
            with open(filename, "r") as file:
                return int(list(file)[-1].split(' ')[0])
        except IOError:
            time.sleep(1)
            --count       


def get_cmd_line_options():
    description = "Plot a dynamic execution summary"
    parser = argparse.ArgumentParser(description = description)

    parser.add_argument("-g", "--graph", type=int, help="Plot only graph nr. GRAPH")
    parser.add_argument("-l", "--loop", type=int,
                        help="Refresh the plot reloading data every LOOP seconds.")
    parser.add_argument("--image", help="Saves the plot to IMAGE file (png format)")
    parser.add_argument("--from_gen", type=int,
                        help="Plot statistics starting from FROM_GEN generation")
    parser.add_argument("--to_gen", type=int,
                        help="Plot statistics up to a TO_GEN generation")
    parser.add_argument("-r", "--run", type=int,
                        help="Plot statistics regarding run RUN only")
    parser.add_argument("--from_run", default=0, type=int,
                        help="Plot statistics starting from FROM_RUN run (default: %(default))")
    parser.add_argument("--to_run", type=int,
                        help="Plot statistics up to TO_RUN run")
    parser.add_argument("-v", "--verbose", action="store_true", default=False, 
                        help="Turn on verbose mode")
    parser.add_argument("dynfile")

    return parser


def reparse_args(args):
    max_dataset = get_max_dataset(args.dynfile)

    if args.run is not None:
        args.from_run = args.run
        args.to_run   = args.run

    if args.to_run is None or args.to_run > max_dataset:
        args.to_run = max_dataset
    elif args.to_run <= 0:
        args.to_run = max_dataset + args.to_run + 1

    if args.from_run < 0:
        args.from_run = max_dataset + args.from_run + 1

    if args.from_run > args.to_run:
        args.from_run = args.to_run


def main():
    # Get argument flags and command options
    parser = get_cmd_line_options()
    args = parser.parse_args()

    global verbose
    verbose = args.verbose
   
    plot(args)


if __name__ == "__main__":
    main()
