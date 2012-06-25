#!/usr/bin/env python3

#
#  Copyright (C) 2011, 2012 EOS di Manlio Morini.
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this file,
#  You can obtain one at http://mozilla.org/MPL/2.0/
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
verbose = False

def sr(data_set, generations, individuals, code_length, rounds, symbol_set,
       args):
    if args.debug:
        rnd = "";
    else:
        random.seed()
        rnd = "--random-seed " + str(random.randint(0,1000000000))
        rounds *= 2
        generations = (generations * 3) // 2

    if args.arl:
        rounds //= 2
        generations //= 2

    if args.dss:
        individuals *= 2

    cmd = Template("sr --verbose $elitism_switch --stat-dir $sd "\
                   "--stat-dynamic --stat-summary --ttable $tt -g $gen "\
                   "-P $nind -l $cl -r $rs $rnd_switch $arl_switch "\
                   "$dss_switch $ss $ds")
    s = cmd.substitute(elitism_switch = "--elitism " + str(args.elitism),
                       sd = stat_dir,
                       tt = ttable_bit,
                       gen = generations,
                       nind = individuals,
                       cl = code_length,
                       rs = rounds,
                       rnd_switch = rnd,
                       arl_switch = "--arl --stat-arl" if args.arl else "",
                       dss_switch = "--dss 1" if args.dss else "--dss 0",
                       ss = "-s " + os.path.join(symbol_set_dir, symbol_set)
                                    if symbol_set != "" else "",
                       ds = os.path.join(data_set_dir, data_set))

    if verbose:
        print(s)

    return os.system(s) == 0


def save_results(name, data_set, args):
    files = {arl_filename: ".arl",
             dynamic_filename: ".dyn",
             summary_filename: ".sum"}

    for f, ext in files.items():
        before = os.path.join(stat_dir,f)
        if os.path.exists(before):
            after = os.path.join(stat_dir, name + "_" +
                                 os.path.basename(args.config) +
                                 ext)
            os.rename(before, after)


def test_dataset(name, args, data_set, generations, individuals, code_length,
                 rounds, symbol_set = ""):
    print("Testing " + name + " [" + str(args) + "]")
    sr(data_set, generations, individuals, code_length, rounds, symbol_set,
       args)
    save_results(name, data_set, args)


def start_testing(args):
    testcases = {
        "adult":      [     "adult.csv",  50, 300, 100,  10,      "class.xml"],
        "fibonacci":  [ "fibonacci.csv", 120, 500, 200,  80, "arithmetic.xml"],
        "iris":       [     "iris.xrff", 100, 500, 100,  80,      "class.xml"],
        "mep":        [       "mep.csv", 100, 200, 500, 100],
        "petalrose":  [ "petalrose.csv", 100, 500, 200,  80, "arithmetic.xml"],
        "petalrose3": ["petalrose3.csv", 100, 500, 200,  80, "arithmetic.xml"],
        "petalrose2": ["petalrose2.csv", 100, 500, 200,  80, "arithmetic.xml"],
        "petalrosec": ["petalrosec.csv", 100, 500, 200,  30,     "class3.xml"],
        "spambase":   [  "spambase.csv", 100, 999, 100,  50,      "class.xml"],
        "wine":       [      "wine.csv", 120, 500, 100,  80,      "class.xml"],
        "x2y2z2":     [   "x2y2_z2.csv", 100, 200, 500,  80,       "math.xml"],
        "x2y2z2bias": [   "x2y2_z2.csv", 100, 200, 100,  80, "arithmetic.xml"]
        }
    # "even3": ["even3.dat", 80, 200, 500,  80, "logic"]
    # "even4": ["even4.dat", 80, 200, 500,  80, "logic"]

    for k, a in testcases.items():
        if (args.test == []) or (k in args.test):
            test_dataset(k, args, *a)


def get_cmd_line_options(defaults):
    parser = argparse.ArgumentParser(fromfile_prefix_chars="@")

    parser.set_defaults(**defaults)

    parser.add_argument("-c", "--config",
                        help="Load configuration from config file")

    parser.add_argument("-r","--random", dest="debug", action="store_false",
                        help="Randomize the test")
    parser.add_argument("-d","--debug", action="store_true",
                        help="Make the test reproducible")

    parser.add_argument("--arl", action="store_true",
                        help="Turn on ARL")
    parser.add_argument("--dss", action="store_true",
                        help="Turn on Dynamic Subset Training")

    parser.add_argument("--elitism", action="store_true",
                        help="Enable elitism")
    parser.add_argument("--no_elitism", action="store_false", dest="elitism",
                        help="Disable elitism")
    parser.add_argument("--force_input", action="store_true",
                        help="Input variables in every individual")
    parser.add_argument("--free_input", action="store_false",
                        dest="force_input",
                        help="Input variables used randomly")

    parser.add_argument("-q", "--quiet", action="store_false", dest="verbose",
                        help="Turn off verbose mode")
    parser.add_argument("-v", "--verbose", action="store_true", dest="verbose",
                        help="Turn on verbose mode")

    parser.add_argument("test", nargs="*")

    return parser.parse_args()


def load_defaults(filename, defaults):
    config = configparser.ConfigParser()
    config.read(filename)

    opt = config["Options"]
    defaults["arl"]         = opt.getboolean("arl", defaults["arl"])
    defaults["dss"]         = opt.getboolean("dss", defaults["dss"])
    defaults["debug"]       = opt.getboolean("debug", defaults["debug"])
    defaults["elitism"]     = opt.getboolean("elitism", defaults["elitism"])
    defaults["force_input"] = opt.getboolean("force_input",
                                             defaults["force_input"])
    defaults["verbose"]     = opt.getboolean("verbose", defaults["verbose"])


def main():
    defaults = {"arl": False,
                "dss": False,
                "debug": True,
                "elitism": True,
                "force_input": False,
                "verbose": False}

    # Get argument flags and command options
    args = get_cmd_line_options(defaults)

    if args.config is not None:
        load_defaults(args.config, defaults)
        args = get_cmd_line_options(defaults)
    else:
        args.config = "default"

    verbose = args.verbose

    start_testing(args)


if __name__ == "__main__":
    main()
