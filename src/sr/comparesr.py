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
from collections import defaultdict
from decimal import *
from xml.etree.ElementTree import ElementTree


max_fn_l = 30
fn_format = "{:<" + str(max_fn_l) + "} "
fmt_str_head = "\n" + fn_format + "{:>8} {:>9} {:>14} {:>14}"
fmt_str_row = fn_format + "{:>+8.2%} {:>+9.2f} {:>14s} {:>14s}"

verbose = False


def to_tuple(s):
    """Returns a tuple constructed converting string s"""
    if s[0] == "(" and s[-1] == ")":
        s1 = s[1:-1]
    else:
        s1 = s

    return tuple([float(x) for x in s1.split(",")])

def tuple_pp(t):
    """Pretty print for numerical tuples"""
    if min(t) < -1000000 or max(t) > 1000000:
        fs = "{:>+2.0f}"
    else:
        fs = "{:>+2.2f}"

    return "(" + ", ".join([fs.format(x) for x in t]) + ")"


def compare_file(files, scores):
    avg_depth_found = dict()
    f_mean = dict()
    f_deviation = dict()
    success_rate = dict()

    for f in files:
        tree = ElementTree()
        tree.parse(f)
        summary = tree.find("summary")
        if summary.find("success_rate") is None:
            print("Missing success rate in file {0}.".format(f))
        else:
            success_rate[f] = float(summary.find("success_rate").text)
        if summary.find("mean_fitness") is None:
            print("Missing mean fitness in file {0}.".format(f))
        else:
            f_mean[f] = to_tuple(summary.find("mean_fitness").text)
        if summary.find("standard_deviation") is None:
            print("Missing standard deviation in file {0}.".format(f))
        else:
            f_deviation[f] = to_tuple(summary.find("standard_deviation").text)

        solutions = summary.find("solutions")
        if solutions.find("avg_depth") is None:
            print("Missing solution average depth in file {0}.".format(f))
        else:
            avg_depth_found[f] = int(solutions.find("avg_depth").text)

        fn = f if len(f) <= max_fn_l else "..." + f[-(max_fn_l - 3):]

        print(fmt_str_row.format(fn, success_rate[f],
                                 avg_depth_found[f], tuple_pp(f_mean[f]),
                                 tuple_pp(f_deviation[f])))

    best = [files[0]]
    for f in files[1:]:
        if success_rate[f] > success_rate[best[0]]:
            best = [f]
        elif success_rate[f] == success_rate[best[0]]:
            best.append(f)

    if success_rate[best[0]] > 0:
        for f in best:
            scores[decode_opt(f, files)] += Decimal("1.00")
    else:
        good = [best[0]]
        for f in best[1:]:
            mff = tuple([round(x, 4) for x in f_mean[f]])
            mfg = tuple([round(x, 4) for x in f_mean[good[0]]])
            if mff > mfg:
                good = [f]
            elif mff == mfg:
                good.append(f)
        for f in good:
            scores[decode_opt(f, files)] += Decimal(".01")


def decode_opt(f, files):
    if (len(files) == 1 or
        os.path.dirname(files[0]) != os.path.dirname(files[1])):
        return os.path.dirname(f)
    else:
        return f


def start_comparison(args):
    scores = defaultdict(Decimal)

    print(fmt_str_head.format("FILE", "SUCCESS", "AVG.DEPTH",
                              "AVG.FIT.", "FIT.ST.DEV."))

    # Case 1. Just list the results contained in a directory.
    if len(args.filepath) == 1 and os.path.isdir(args.filepath[0]):
        groups = dict()
        dir_list = os.listdir(args.filepath[0])
        dir_list.sort()
        for f in dir_list:
            if os.path.splitext(f)[1] == ".sum":
                dataset = os.path.splitext(f.split("_")[0])[0]
                fn = os.path.join(args.filepath[0], os.path.basename(f))
                if groups.get(dataset) is None:
                    groups[dataset] = [fn]
                else:
                    groups[dataset].append(fn)
        for k in groups.keys():
            compare_file(groups[k], scores)
            print("-" * 79)
    # Case 2. Comparison between two directories.
    elif os.path.isdir(args.filepath[0]) and os.path.isdir(args.filepath[1]):
        dir_list = os.listdir(args.filepath[0])
        for f in dir_list:
            if os.path.splitext(f)[1] == ".sum":
                fn1 = os.path.join(args.filepath[0], os.path.basename(f))
                fn2 = os.path.join(args.filepath[1], os.path.basename(f))
                if os.path.isfile(fn2):
                    compare_file([fn1, fn2], scores)
                    print("-" * 79)
                else:
                    print("Missing {0} file".format(fn2))
    # Case 3. Comparison between two files.
    elif os.path.isfile(args.filepath[0]) and os.path.isfile(args.filepath[1]):
        compare_file([args.filepath[0], args.filepath[1]], scores)

    print("\nOverall testsets comparison")
    for f in sorted(scores.keys()):
        print(fn_format.format(f), " --> ", "{:>3.2f}".format(scores[f]))


def get_cmd_line_options():
    description = "Compare execution summaries of datasets"
    parser = argparse.ArgumentParser(description = description)

    parser.add_argument("-v", "--verbose", action = "store_true",
                        help = "Turn on verbose mode")
    parser.add_argument("filepath", nargs = "*")

    return parser


def main():
    # Get argument flags and command options
    parser = get_cmd_line_options()
    args = parser.parse_args()

    verbose = args.verbose

    start_comparison(args)


if __name__ == "__main__":
    main()
