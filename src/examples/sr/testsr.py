#!/usr/bin/env python3

#
#  Copyright (C) 2011-2022 EOS di Manlio Morini.
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this file,
#  You can obtain one at http://mozilla.org/MPL/2.0/
#
#  A command line utility to simplify debugging and profiling.
#

import argparse
import configparser
import os
import random
import subprocess


dynamic_filename = "dynamic"
stat_dir = "stat/"
summary_filename = "summary"


class TestParameters:
    def __init__(self, filename, generations = 100, individuals = 180,
                 code_length = 100, runs = 80, layers = 4, symbol_set = "",
                 evaluator = "", dss = 0):
        if not filename:
            raise Exception("Unspecified test filename")

        self.filename = filename
        self.generations = generations
        self.individuals = individuals
        self.code_length = code_length
        self.runs = runs
        self.layers = layers
        self.symbol_set = symbol_set
        self.evaluator = evaluator
        self.dss = dss

test_collection = {
    #"adult": TestParameters("adult.csv", generations = 50, individuals = 100,
    #                        runs = 10, symbol_set = "class.xml"),

    "fibonacci": TestParameters("fibonacci.csv", generations = 120,
                                code_length = 200,
                                symbol_set = "arithmetic.xml"),

    "ionosphere": TestParameters("ionosphere.csv", symbol_set = "class.xml",
                                 evaluator = "binary"),

    "iris": TestParameters("iris.xrff", symbol_set = "class.xml"),

    "mep": TestParameters("mep.csv", individuals = 75, code_length = 500,
                          runs = 100),

    "mepdss": TestParameters("mep.csv", individuals = 75, code_length = 500,
                             runs = 100, dss = 1),

    "petalrose": TestParameters("petalrose.xrff", code_length = 200,
                                runs = 10, symbol_set = "iarithmetic.xml"),

    "petalrose3": TestParameters("petalrose3.csv", code_length = 200,
                                 symbol_set = "arithmetic.xml"),

    "petalrose2": TestParameters("petalrose2.csv", code_length = 200,
                                 symbol_set = "arithmetic.xml"),

    "petalrosec": TestParameters("petalrosec.csv", code_length = 200,
                                 runs = 10, symbol_set = "class3.xml"),

    #"spambase": TestParameters("spambase.csv", individuals = 260, runs = 10,
    #                           symbol_set = "class.xml"),

    "wine": TestParameters("wine.csv", generations = 120, individuals = 170,
                           symbol_set = "class.xml"),

    "x2y2z2": TestParameters("x2y2_z2.csv", individuals = 75,
                             code_length = 500, symbol_set = "math.xml"),

    "x2y2z2bias": TestParameters("x2y2_z2.csv", individuals = 75,
                                 symbol_set = "arithmetic.xml")
}


mode_settings = {
    "debug": [],
    "profile": ["deeptest", "randomize"],
    "stats": ["deeptest"]
}


def sr(args, dataset):
    cache_bit = 20
    dataset_dir = "./dataset/"
    sr_dir = "../../../build/examples/sr/"
    symbol_set_dir = "./symbolset/"

    sr = os.path.join(sr_dir, "sr_test")
    if not os.path.exists(sr):
        sr = os.path.join(sr_dir, "sr")

    mode = args.mode;

    generations = test_collection[dataset].generations
    individuals = test_collection[dataset].individuals
    code_length = test_collection[dataset].code_length
    runs = test_collection[dataset].runs
    layers = test_collection[dataset].layers
    symbol_set = test_collection[dataset].symbol_set
    evaluator = test_collection[dataset].evaluator
    dss = test_collection[dataset].dss

    # Default values are for a fast/debug evaluation. Gathering statistics
    # requires bigger numbers...
    if "deeptest" in mode_settings[mode]:
        runs *= 15
        generations = (generations * 3) // 2

    cmd = [sr, "--verbose",
           "--stat-dir", stat_dir,
           "--stat-dynamic", "--stat-summary",
           "--cache", str(cache_bit),
           "--generations", str(generations),
           "--layers", str(layers),
           "--population", str(individuals),
           "--length", str(code_length),
           "--runs", str(runs)]

    if evaluator:
        cmd += ["--evaluator", evaluator]

    if symbol_set:
        cmd += ["--symbols", os.path.join(symbol_set_dir, symbol_set)]

    if dss > 0:
        cmd += ["--dss", str(dss)]

    # When randomize is off, tests are reproducible (good for debugging).
    if "randomize" in mode_settings[mode]:
        random.seed()
        cmd += ["--random-seed", str(random.randint(0, 1000000000))]

    cmd.append(os.path.join(dataset_dir, test_collection[dataset].filename))

    return subprocess.run(cmd).returncode == 0


def save_results(args, dataset):
    """Renames output files so they wouldn't conflict with the output of a new
       test

    """

    # SR can produce up to three output files...
    files = {
        dynamic_filename: ".dyn",
        summary_filename: ".sum"}

    for f, ext in files.items():
        before = os.path.join(stat_dir, f)
        if os.path.exists(before):
            after = os.path.join(stat_dir, dataset + "_" + args.mode + ext)
            os.rename(before, after)


def test_dataset(args, dataset):
    print("Testing " + dataset + " [" + str(args) + "]")
    sr(args, dataset)
    save_results(args, dataset)


def start_testing(args):
    for test_case in test_collection:
        if (args.test == []) or (test_case in args.test):
            test_dataset(args, test_case)


def get_cmd_line_options():
    """Get argument flags and command options"""

    parser = argparse.ArgumentParser(
        fromfile_prefix_chars = "@",
        description = "A simple command line utility to simplify debugging "
                      "and profiling")

    parser.add_argument("mode", choices = mode_settings)
    parser.add_argument("test", nargs = "*")

    return parser.parse_args()


def main():
    args = get_cmd_line_options()

    start_testing(args)


if __name__ == "__main__":
    main()
