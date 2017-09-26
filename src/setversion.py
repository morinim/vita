#!/usr/bin/env python3
#
#  Copyright (C) 2017 EOS di Manlio Morini.
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this file,
#  You can obtain one at http://mozilla.org/MPL/2.0/
#
#  A python program that helps to set up a new version of Vita.
#

import argparse
import os
import re


def version_str(args):
    return str(args.major) + "." + str(args.minor) + "." + str(args.maintenance)


def file_process(name, rule, args):
    print("--- Processing " + os.path.basename(name))
    with open(name) as source:
        data = rule(source.read(), args)

    if not data:
        return

    print("Writing " + name)
    with open(name) as dest:
        dest = open(name, "w")
        dest.write(data)


def doxygen_rule(data, args):
    regex = r"([\s]+)(\*[\s]+\\mainpage VITA v)([\d]+)\.([\d]+)\.([\d]+)([\s]*)"
    subst = r"\g<1>\g<2>" + version_str(args) + r"\g<6>"

    result = re.subn(regex, subst, data)

    return result[0] if result[1] > 0 else None


def get_cmd_line_options():
    description = "Helps to set up a new version of Vita"
    parser = argparse.ArgumentParser(description = description)

    parser.add_argument("-v", "--verbose", action = "store_true",
                        help = "Turn on verbose mode")

    # Now the positional arguments.
    parser.add_argument("major", type=int)
    parser.add_argument("minor", type=int)
    parser.add_argument("maintenance", type=int)

    return parser


def main():
    args = get_cmd_line_options().parse_args()

    print("Setting version to v" + str(args.major)
          + "." + str(args.minor)
          + "." + str(args.maintenance))

    file_process("../doc/doxygen/doxygen.h", doxygen_rule, args)

    print("\n\nRELEASE NOTE\n")
    print("1. Build and check.  make TYPE=debug")
    print('2. Commit.           git commit -am"[DOC] Changed revision number to v'
          + version_str(args) + '"')
    print("3. Tag.              git tag -a v" + version_str(args)
          + " -m [tag message]")
    print("\nRemember to 'git push' both code and tag. For the tag:\n")
    print("    git push [tagname]\n")


if __name__ == "__main__":
    main()
