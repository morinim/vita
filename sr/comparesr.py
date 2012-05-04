#!/usr/bin/env python3

#
#  Copyright (C) 2011 EOS di Manlio Morini.
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


format_string_head = "\n{:<32} {:>8} {:>9} {:>12} {:>11}"
format_string_row  = "{:<32} {:>+8.2%} {:>+9.2f} {:>+12.2f} {:>+11.2f}"
format_string_row2  = "{:<32} {:>+8.2%} {:>+9.2f} {:>+12.6g} {:>+11.5g}"

verbose = False


def compare_file(files, scores):
    avg_depth_found = dict()
    mean_fitness = dict()
    standard_deviation = dict()
    success_rate = dict()

    for f in files:
        tree = ElementTree()
        tree.parse(f)
        summary = tree.find("summary")
        if summary.find("success_rate") is None:
            print("Missing success rate in file {0}.".format(f))
        else:
            success_rate[f] = float(summary.find("success_rate").text)

        best = summary.find("best")
        if best.find("avg_depth_found") is None:
            print("Missing solution average depth in file {0}.".format(f))
        else:
            avg_depth_found[f] = int(best.find("avg_depth_found").text)
        if best.find("mean_fitness") is None:
            print("Missing mean fitness in file {0}.".format(f))
        else:
            mean_fitness[f] = float(best.find("mean_fitness").text)
        if best.find("standard_deviation") is None:
            print("Missing standard deviation in file {0}.".format(f))
        else:
            standard_deviation[f] = float(best.find("standard_deviation").text)

        fn = f if len(f) <= 32 else "..."+f[-29:]
        if mean_fitness[f] < -1000000 or standard_deviation[f] > 1000000:
            format_str = format_string_row2
        else:
            format_str = format_string_row

        print(format_str.format(fn, success_rate[f],
                                avg_depth_found[f], mean_fitness[f],
                                standard_deviation[f]))

    l = len(files)
    for f in files:
        opt = decode_opt(f, files)
        if scores[opt] is None:
            scores[opt] = Decimal("0.00")

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
            mff = round(mean_fitness[f], 4)
            mfg = round(mean_fitness[good[0]], 4)
            if mff > mfg:
                good = [f]
            elif mff == mfg:
                good.append(f)
        for f in good:
            scores[decode_opt(f, files)] += Decimal(".01")


def decode_opt(f, files):
    if len(files) > 2:
        (fn, ext) = os.path.splitext(os.path.basename(f))
        (l, s, r) = fn.rpartition("_")

        values = {
            "00": "(00) DEBUG elitism",
            "01": "(01) DEBUG",
            "02": "(02) elitism",
            "03": "(03)",
            "04": "(04) DEBUG arl elitism",
            "05": "(05) DEBUG arl",
            "06": "(06) arl elitism",
            "07": "(07) arl",
            "08": "(08) arl elitism force_input"
        }
        return values.get(r,"UNKNOWN")
    else:
        return files.index(f)


def start_comparison(args):
    scores = defaultdict(Decimal)

    print(format_string_head.format("FILE", "SUCCESS", "AVG.DEPTH",
                                    "AVG.FIT.", "FIT.ST.DEV."))

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
    elif os.path.isdir(args.filepath[0]) and os.path.isdir(args.filepath[1]):
        dir_list = os.listdir(args.filepath[0])
        for f in dir_list:
            if os.path.splitext(f)[1] == ".sum":
                fn1 = os.path.join(args.filepath[0], os.path.basename(f))
                fn2 = os.path.join(args.filepath[1], os.path.basename(f))
                if os.path.isfile(fn2):
                    compare_file([fn1, fn2], scores)
                    print("-"*79)
                else:
                    print("Missing {0} file".format(fn2))
    elif os.path.isfile(args.filepath[0]) and os.path.isfile(args.filepath[1]):
        compare_file([args.filepath[0], args.filepath[1]], scores)

    print("\nOverall testsets comparison")
    for f in sorted(scores.keys()):
        print("{:<32}".format(f), " --> ", "{:>3.2f}".format(scores[f]))


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
