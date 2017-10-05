#!/usr/bin/env python3

#
#  Copyright (C) 2011-2013 EOS di Manlio Morini.
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this file,
#  You can obtain one at http://mozilla.org/MPL/2.0/
#

import argparse
import os
import subprocess
import time


verbose = False

# Column meaning / position for dynamic log file.
col_run = 1;
col_gen = 2;
col_fitness = 3;
col_mean_fit = 4;
col_fit_sd = 5;
col_entropy = 6;
col_min_fit = 7;
col_mean_len = 8;
col_len_sd = 9;
col_max_len = 10;
col_mutations = 11;
col_crossovers = 12;
col_functions = 13;
col_terminals = 14;
col_active_functions = 15;
col_active_terminals = 16;

# Column meaning / position for ages log file.
col_p_run = 1;
col_p_gen = 2;
col_p_fit = 3;
col_p_frq = 4;



def plot1(pipe, args):
    pipe.write(b"set xlabel 'GENERATION'\n")
    pipe.write(b"set ylabel 'FITNESS'\n")

    cmd = "plot [{from_gen}:{to_gen}] '{data}' index {from_run}:{to_run} using {col_gen}:{col_fitness} title 'Best' with fsteps linestyle 2, '{data}' index {from_run}:{to_run} using {col_gen}:{col_mean_fit}:{col_fit_sd} title 'Population' with yerrorbars linestyle 1\n".format(
        from_gen = "" if args.from_gen is None else args.from_gen,
        to_gen = "" if args.to_gen is None else args.to_gen,
        data = args.logfile,
        from_run = args.from_run,
        to_run = args.to_run,
        col_gen = col_gen,
        col_fitness = col_fitness,
        col_mean_fit = col_mean_fit,
        col_fit_sd = col_fit_sd)

    if verbose:
        print(cmd)
    pipe.write(str.encode(cmd))


def plot2(pipe, args):
    pipe.write(b"set xlabel 'GENERATION'\n")
    pipe.write(b"set ylabel 'ENTROPY'\n")

    cmd = "plot [{from_gen}:{to_gen}] '{data}' index {from_run}:{to_run} using {col_gen}:{col_entropy} title 'Entropy' with lines\n".format(
        from_gen = "" if args.from_gen is None else args.from_gen,
        to_gen = "" if args.to_gen is None else args.to_gen,
        data = args.logfile,
        from_run = args.from_run,
        to_run = args.to_run,
        col_gen = col_gen,
        col_entropy = col_entropy)

    if verbose:
        print(cmd)
    pipe.write(str.encode(cmd))


def plot3(pipe, args):
    pipe.write(b"set xlabel 'GENERATION'\n")
    pipe.write(b"set ylabel 'EFFECTIVE SIZE'\n")

    cmd = "plot [{from_gen}:{to_gen}] '{data}' index {from_run}:{to_run} using {col_gen}:{col_mean_len}:{col_len_sd} title 'Population' with yerrorbars linestyle 1\n".format(
        from_gen = "" if args.from_gen is None else args.from_gen,
        to_gen = "" if args.to_gen is None else args.to_gen,
        data = args.logfile,
        from_run = args.from_run,
        to_run = args.to_run,
        col_gen = col_gen,
        col_mean_len = col_mean_len,
        col_len_sd = col_len_sd)

    if verbose:
        print(cmd)
    pipe.write(str.encode(cmd))


def plot4(pipe, args):
    pipe.write(b"set xlabel 'GENERATION'\n")
    pipe.write(b"set ylabel 'NR. OF SYMBOLS'\n")

    cmd = "plot [{from_gen}:{to_gen}] '{data}' index {from_run}:{to_run} " \
          "using {col_gen}:{col_functions} title 'Functions' with lines, " \
          "'{data}' index {from_run}:{to_run} " \
          "using {col_gen}:{col_terminals} title 'Terminals' with lines, " \
          "'{data}' index {from_run}:{to_run} " \
          "using {col_gen}:{col_active_functions} title 'Active functions' " \
          "with lines, '{data}' index {from_run}:{to_run} " \
          "using {col_gen}:{col_active_terminals} title 'Active terminals' " \
          "with lines\n".format(
        from_gen = "" if args.from_gen is None else args.from_gen,
        to_gen = "" if args.to_gen is None else args.to_gen,
        data = args.logfile,
        from_run = args.from_run,
        to_run = args.to_run,
        col_gen = col_gen,
        col_functions = col_functions,
        col_terminals = col_terminals,
        col_active_functions = col_active_functions,
        col_active_terminals = col_active_terminals
        )

    if verbose:
        print(cmd)
    pipe.write(str.encode(cmd))


def plot5(pipe, args):
    pipe.write(b"set zrange [0:*]\n")
    pipe.write(b"set yrange [0:*]\n")
    pipe.write(b"set xlabel 'FITNESS'\n")
    pipe.write(b"set ylabel 'GENERATION'\n")
    pipe.write(b"set zlabel 'NR.'\n")

    pipe.write(b"set dgrid3d exp\n")
    #pipe.write(b"set dgrid3d exp\n")
    #pipe.write(b"set dgrid3d cauchy\n")
    pipe.write(b"set hidden3d\n")

    #pipe.write(b"set xyplane at -1\n");
    #pipe.write(b"set pm3d at b\n")
    pipe.write(b"set pm3d at s hidden3d 1\n")

    pipe.write(b"set view ,150\n")

    cmd = "splot [] [{from_gen}:{to_gen}] '{data}' index {from_run}:{to_run} using {col_p_fit}:{col_p_gen}:{col_p_frq} title '' with lines\n".format(
        from_gen = "" if args.from_gen is None else args.from_gen,
        to_gen = "" if args.to_gen is None else args.to_gen,
        data = args.logfile,
        from_run = args.from_run,
        to_run = args.to_run,
        col_p_gen = col_p_gen,
        col_p_fit = col_p_fit,
        col_p_frq = col_p_frq)

    if verbose:
        print(cmd)
    pipe.write(str.encode(cmd))


def plot(args):
    pipe = subprocess.Popen("gnuplot -persist -noraise", shell = True,
                            bufsize = 0, stdin = subprocess.PIPE).stdin

    loop = True
    while loop:
        reparse_args(args)

        if (args.image is not None):
            pipe.write(b"set terminal png\n")
            pipe.write(b"set output '" + args.image + "'\n")

        pipe.write(b"set grid\n")
        pipe.write(b"set key bottom right\n")
        # pipe.write(b"set autoscale fix\n")
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
        if (args.graph is None or args.graph == 1):
            plot1(pipe, args)

        # Plot the second graph so that it takes a quarter of the screen
        if (args.graph is None):
            pipe.write(b"set size 0.5,0.5\n")
            pipe.write(b"set origin 0,0\n")
        if (args.graph is None or args.graph == 2):
            plot2(pipe, args)

        # Plot the third graph so that it takes a quarter of the screen
        if (args.graph is None):
            pipe.write(b"set size 0.5,0.5\n")
            pipe.write(b"set origin 0.5,0.5\n")
        if (args.graph is None or args.graph == 3):
            plot3(pipe, args)

        # Plot the fourth graph so that it takes a quarter of the screen
        if (args.graph is None):
            pipe.write(b"set size 0.5,0.5\n")
            pipe.write(b"set origin 0.5,0\n")
        if (args.graph is None or args.graph == 4):
            plot4(pipe, args)

        # Plot the fifth graph (always full screen)
        if (args.graph == 5):
            plot5(pipe, args)

        # On some terminals, nothing gets plotted until this command is issued
        if (args.graph is None):
            pipe.write(b"unset multiplot\n")

        # Remove all customizations
        pipe.write(b"reset\n")

        if (args.loop is None):
            loop = False
        else:
            time.sleep(args.loop)
            pipe.flush()


def get_columns(filename):
    count = 4
    while count > 0:
        try:
            with open(filename, "r") as file:
                return len(list(file)[0].split(' '))
        except IOError:
            time.sleep(1)
            --count

def get_max_run(filename):
    count = 4
    while count > 0:
        try:
            with open(filename, "r") as file:
                return int(list(file)[-1].split(' ')[0])
        except IOError:
            time.sleep(1)
            --count


def get_cmd_line_options():
    description = "Plot various graphs based on evolution log files"
    parser = argparse.ArgumentParser(description = description)

    parser.add_argument("-g", "--graph", type = int, choices = range(1,5),
                        help = "plot only graph nr. GRAPH")
    parser.add_argument("-l", "--loop", type = int,
                        help = "refresh the plot reloading data every LOOP " \
                               "seconds.")
    parser.add_argument("--image",
                        help = "save the plot to IMAGE file (png format)")
    parser.add_argument("--from_gen", type = int,
                        help = "plot statistics starting from FROM_GEN " \
                               "generation")
    parser.add_argument("--to_gen", type = int,
                        help = "plot statistics up to a TO_GEN generation")
    parser.add_argument("-r", "--run", type = int,
                        help = "plot statistics regarding run RUN only")
    parser.add_argument("--from_run", default = 0, type = int,
                        help = "plot statistics starting from FROM_RUN run")
    parser.add_argument("--to_run", type = int,
                        help = "plot statistics up to TO_RUN run")
    parser.add_argument("-v", "--verbose", action = "store_true",
                        default  = False, help = "Turn on verbose mode")
    parser.add_argument("logfile")

    return parser


def reparse_args(args):
    max_run = get_max_run(args.logfile)

    if args.run is not None:
        args.from_run = args.run
        args.to_run   = args.run

    if args.to_run is None or args.to_run > max_run:
        args.to_run = max_run
    elif args.to_run <= 0:
        args.to_run = max_run + args.to_run + 1

    if args.from_run < 0:
        args.from_run = max_run + args.from_run + 1

    if args.from_run > args.to_run:
        args.from_run = args.to_run


def main():
    # Get argument flags and command options
    parser = get_cmd_line_options()
    args = parser.parse_args()

    global verbose
    verbose = args.verbose

    if os.path.isdir(args.logfile):
        filenames = ["dynamic", "population"]

        for f in filenames:
            if os.path.exists(os.path.join(args.logfile, f)):
                args.logfile = os.path.join(args.logfile, f)
                break

    if get_columns(args.logfile) == 4:  # 4 columns => population file
        args.graph = 5;                 # so plot only fifth graph

    plot(args)


if __name__ == "__main__":
    main()
