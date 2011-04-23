#!/usr/bin/env python3

import argparse
import os
from collections import defaultdict
from xml.etree.ElementTree import ElementTree


format_string_head = "{0:<32} {1:>12} {2:>9} {3:>11} {4:>11}"
format_string_row  = "{0:<32} {1:>+11.2f}% {2:>+8.2f} {3:>+12.2f} {4:>+11.2f}"

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
        print(format_string_row.format(fn, success_rate[f]*100, 
                                       avg_depth_found[f], mean_fitness[f],
                                       standard_deviation[f]))

    best = [files[0]]
    for f in files[1:]:
        if success_rate[f] > success_rate[best[0]]:
            best = [f]
        elif success_rate[f] == success_rate[best[0]]:
            best.append(f)

    if success_rate[best[0]] > 0:
        for f in best:
            scores[files.index(f)] += 1
    else:
        good = [best[0]]
        for f in best[1:]:
            if mean_fitness[f] > mean_fitness[good[0]]:
                good = [f]
            elif mean_fitness[f] == mean_fitness[good[0]]:
                good.append(f)
        for f in good:
            scores[files.index(f)] += .01
    

def start_comparison(args):
    scores = defaultdict(float)

    print(format_string_head.format("FILE", "SUCCESS RATE", "AVG.DEPTH",
                                    "AVG.FIT.", "FIT.ST.DEV."))

    if len(args.filepath) == 1 and os.path.isdir(args.filepath[0]):
        groups = dict()
        dir_list = os.listdir(args.filepath[0])
        dir_list.sort()
        for f in dir_list:
            if os.path.splitext(f)[1] == ".sum":
                dataset = os.path.splitext(f.split("_")[0])[0]
                fn = os.path.join(args.filepath[0],os.path.basename(f))
                if groups.get(dataset) is None:
                    groups[dataset] = [fn]
                else:
                    groups[dataset].append(fn)
        for k in groups.keys():
            compare_file(groups[k],scores)
            print("-"*79)               
    elif os.path.isdir(args.filepath[0]) and os.path.isdir(args.filepath[1]):
        dir_list = os.listdir(args.filepath[0])
        for f in dir_list:
            if os.path.splitext(f)[1] == ".sum":
                fn1 = os.path.join(args.filepath[0],os.path.basename(f))
                fn2 = os.path.join(args.filepath[1],os.path.basename(f))
                if os.path.isfile(fn2):
                    compare_file([f1n,fn2],scores)
                    print("-"*79)
                else:
                    print("Missing {0} file".format(fn2))
    elif os.path.isfile(args.filepath[0]) and os.path.isfile(args.filepath[1]):
        compare_file([args.filepath[0],args.filepath[1]],scores)

    print("Overall testsets comparison")
    for idx, score in scores.items():
        print(idx, " --> ", score)


def get_cmd_line_options():
    description = "Compare execution summaries of datasets"
    parser = argparse.ArgumentParser(description = description)

    parser.add_argument("-v","--verbose", action="store_true",
                        help="Turn on verbose mode")
    parser.add_argument("filepath", nargs="*")

    return parser


def main():
    # Get argument flags and command options
    parser = get_cmd_line_options()
    args = parser.parse_args()

    verbose = args.verbose

    start_comparison(args)


if __name__ == "__main__":
    main()
