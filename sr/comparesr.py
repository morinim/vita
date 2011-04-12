#!/usr/bin/env python3

import argparse
import os
from xml.etree.ElementTree import ElementTree


format_string_head = "{0:<32} {1:>12} {2:>9} {3:>11} {4:>11}"
format_string_row  = "{0:<32} {1:>+11.2f}% {2:>+8.2f} {3:>+12.2f} {4:>+11.2f}"

verbose = False


def generate_html_table(headers, rows):
    html = []

    if headers:
        html.append("<tr>")
        for header in headers:
            html.append("<th>{0}</th>".format(header))
        html.append("</tr>")

    if rows:
        for row in rows:
            html.append("<tr>")
            for cell in row:
                html.append("<td>{0}</td>".format(cell))
            html.append("</tr>")

    if html:
        html = ["<table>"] + html + ["</table>"]

    return "\n".join(html)


def compare_file(file1, file2, html):
    result = {file1:[], file2:[]}
    files = [file1, file2]

    for f in files:
        tree = ElementTree()
        tree.parse(f)
        summary = tree.find("summary")
        if summary.find("success_rate") is None:
            print("Missing success rate in file {0}.".format(f))
        else:
            success_rate = float(summary.find("success_rate").text)
            result[f].append(["{0:.2f}%".format(success_rate*100)])
            if f is file1:
                 success_rate1 = success_rate
            else:
                 success_rate2 = success_rate

        best = summary.find("best")
        if best.find("avg_depth_found") is None:
            print("Missing solution average depth in file {0}.".format(f))
        else:
            avg_depth_found = int(best.find("avg_depth_found").text)
            result[f].append([str(avg_depth_found)])
        if best.find("mean_fitness") is None:
            print("Missing mean fitness in file {0}.".format(f))
        else:
            mean_fitness = float(best.find("mean_fitness").text)
            result[f].append(["{0:.2f}".format(mean_fitness)])
            if f is file1:
                mean_fitness1 = mean_fitness
            else:
                mean_fitness2 = mean_fitness
        if best.find("standard_deviation") is None:
            print("Missing standard deviation in file {0}.".format(f))
        else:
            standard_deviation = float(best.find("standard_deviation").text)
            result[f].append(["{0:.2f}".format(standard_deviation)])
        print(format_string_row.format(f, success_rate*100, avg_depth_found,
                                       mean_fitness, standard_deviation))

    results = []
    for f, a in result.items():
        results.append([f] + a)

    html.append(generate_html_table(["FILE", "SUCCESS RATE", "AVG. DEPTH", 
                                     "AVG. FIT.", "FIT. ST.DEV"], results))

    if success_rate1 > success_rate2:
        return 1
    elif success_rate1 < success_rate2:
        return -1
    elif mean_fitness1 > mean_fitness2:
        return .1
    elif mean_fitness1 < mean_fitness2:
        return -.1
    return 0


def start_comparison(args):
    html = []
    diff = 0

    print(format_string_head.format("FILE", "SUCCESS RATE", "AVG.DEPTH",
                                    "AVG.FIT.", "FIT.ST.DEV."))

    if os.path.isdir(args.filepath[0]) and os.path.isdir(args.filepath[1]):
        dir_list = os.listdir(args.filepath[0])
        for f1 in dir_list:
            if os.path.splitext(f1)[1] == ".sum":
                f1 = os.path.join(args.filepath[0],os.path.basename(f1))
                f2 = os.path.join(args.filepath[1],os.path.basename(f1))
                if os.path.isfile(f2):
                    diff += compare_file(f1,f2,html)
                    html += "\n<br>\n"
                    print("-"*79)
                else:
                    print("Missing {0} file".format(f2))
    elif os.path.isfile(args.filepath[0]) and os.path.isfile(args.filepath[1]):
        diff = compare_file(args.filepath[0],args.filepath[1],html)

    print(format_string_head.format("Overall testsets comparison:", diff, 
                                    "", "", ""))

    if args.html is not None:
        with open(args.html,"w") as out:
            out.write("".join(html))


def get_cmd_line_options():
    description = "Compare execution summaries of datasets"
    parser = argparse.ArgumentParser(description = description)

    parser.add_argument("--html", help="Generate a html report")
    parser.add_argument("-v","--verbose", action="store_true",
                        help="Turn on verbose mode")
    parser.add_argument("filepath", nargs=2)

    return parser


def main():
    # Get argument flags and command options
    parser = get_cmd_line_options()
    args = parser.parse_args()

    verbose = args.verbose

    start_comparison(args)


if __name__ == "__main__":
    main()
