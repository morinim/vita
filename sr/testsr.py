#!/usr/bin/env python3

#
#  Copyright (C) 2011-2013 EOS di Manlio Morini.
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this file,
#  You can obtain one at http://mozilla.org/MPL/2.0/
#
#  This is just a command line utility to simplify debugging and profiling.
#

import argparse
import configparser
import os
import random
from string import Template


arl_filename = "arl"
data_set_dir = "dataset/"
dynamic_filename = "dynamic"
stat_dir = "stat/"
summary_filename = "summary"
symbol_set_dir = "symbolset/"
ttable_bit = 20

# Key: [dataset file, generations, individuals, code_length, runs, symbolset]
testcases = {
    "adult":      [     "adult.csv", 25, 100, 100,  10,       "class.xml"],
    "fibonacci":  [ "fibonacci.csv", 60, 180, 200,  80,  "arithmetic.xml"],
    "iris":       [     "iris.xrff", 50, 180, 100,  80,       "class.xml"],
    "mep":        [       "mep.csv", 50,  75, 500, 100],
    "petalrose":  ["petalrose.xrff", 50, 180, 200,  80, "iarithmetic.xml"],
    "petalrose3": ["petalrose3.csv", 50, 180, 200,  80,  "arithmetic.xml"],
    "petalrose2": ["petalrose2.csv", 50, 180, 200,  80,  "arithmetic.xml"],
    "petalrosec": ["petalrosec.csv", 50, 180, 200,  30,      "class3.xml"],
    "spambase":   [  "spambase.csv", 50, 260, 100,  50,       "class.xml"],
    "wine":       [      "wine.csv", 60, 170, 100,  80,       "class.xml"],
    "x2y2z2":     [   "x2y2_z2.csv", 50,  75, 500,  80,        "math.xml"],
    "x2y2z2bias": [   "x2y2_z2.csv", 50,  75, 100,  80,  "arithmetic.xml"]
    }
    # "even3": ["even3.dat", 80, 200, 500,  80, "logic"]
    # "even4": ["even4.dat", 80, 200, 500,  80, "logic"]

settings = {
    "debug": {},
    "profile": {"deeptest": True, "randomize": True}
    }



def sr(args, data_set, generations, individuals, code_length, rounds,
       symbol_set):
    sr = "sr_test" if os.path.exists("sr_test") else "sr"

    mode = args.mode;

    arl = ""
    if "arl" in settings[mode]:
        arl = "--arl --stat-arl" if settings[mode]["arl"] else "",

    elitism = ""
    if "elitism" in settings[mode]:
        elitism = "--elitism " + str(settings[mode]["elitism"])

    # Default values are for a fast evaluation. Profiling requires bigger
    # numbers...
    if "deeptest" in settings[mode] and settings[mode]["deeptest"]:
        rounds *= 3
        generations = (generations * 3) // 2

    dss = ""
    if "dss" in settings[mode]:
        dss = "--dss " + str(settings[mode]["dss"])

    # When randomize is off, tests are reproducible (good for debugging).
    randomize = ""
    if "randomize" in settings[mode]:
        random.seed()
        randomize = "--random-seed " + str(random.randint(0, 1000000000))

    cmd = Template("$sr --verbose $elitism_switch --stat-dir $sd "\
                   "--stat-dynamic --stat-summary --ttable $tt -g $gen "\
                   "--layers 4 -P $nind -l $cl -r $rs $rnd_switch "\
                   "$arl_switch $dss_switch $ss $ds")
    s = cmd.substitute(
        sr = sr,
        elitism_switch = elitism,
        sd = stat_dir,
        tt = ttable_bit,
        gen = generations,
        nind = individuals,
        cl = code_length,
        rs = rounds,
        rnd_switch = randomize,
        arl_switch = arl,
        dss_switch = dss,
        ss = "-s " + os.path.join(symbol_set_dir, symbol_set)
             if symbol_set != "" else "",
        ds = os.path.join(data_set_dir, data_set))

    return os.system(s) == 0


def save_results(args, name, data_set):
    """Renames output files so they wouldn't conflict with the output of a new
       test

    """

    # SR can produce up to three output files...
    files = {
        arl_filename: ".arl",
        dynamic_filename: ".dyn",
        summary_filename: ".sum"}

    for f, ext in files.items():
        before = os.path.join(stat_dir, f)
        if os.path.exists(before):
            after = os.path.join(stat_dir, name + "_" + args.mode + ext)
            os.rename(before, after)


def test_dataset(args, name, data_set, generations, individuals, code_length,
                 rounds, symbol_set = ""):
    print("Testing " + name + " [" + str(args) + "]")
    sr(args, data_set, generations, individuals, code_length, rounds,
       symbol_set)
    save_results(args, name, data_set)


def start_testing(args):
    for k, a in testcases.items():
        if (args.test == []) or (k in args.test):
            test_dataset(args, k, *a)


def get_cmd_line_options():
    """Get argument flags and command options"""

    parser = argparse.ArgumentParser(
        fromfile_prefix_chars = "@",
        description = "A simple command line utility to simplify debugging "\
                      "and profiling")

    parser.add_argument("mode", choices = settings)
    parser.add_argument("test", nargs = "*")

    return parser.parse_args()


def main():
    args = get_cmd_line_options()

    start_testing(args)


if __name__ == "__main__":
    main()
