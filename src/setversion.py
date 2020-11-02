#!/usr/bin/env python3
#
#  Copyright (C) 2017-2020 EOS di Manlio Morini.
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this file,
#  You can obtain one at http://mozilla.org/MPL/2.0/
#
#  A python program that helps to set up a new version of Vita.
#

import argparse
import datetime
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


def changelog_rule(data, args):
    new_version = version_str(args)

    regex = r"## \[Unreleased\]"
    subst = r"## [Unreleased]\n\n## [" + new_version + r"] - " + datetime.date.today().isoformat()

    result = re.subn(regex, subst, data)
    if result[1] != 1:
        return None

    regex = r"(\[Unreleased)(\]: https://github.com/morinim/vita/compare/v)(.+)(\.\.\.HEAD)"
    subst = r"\g<1>\g<2>" + new_version + r"\g<4>\n[" + new_version + r"\g<2>\g<3>...v" + new_version

    result = re.subn(regex, subst, result[0])

    return result[0] if result[1] == 1 else None


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

    file_process("../NEWS.md", changelog_rule, args)
    file_process("../doc/doxygen/doxygen.h", doxygen_rule, args)

    print("\n\nRELEASE NOTE\n")
    print("1. Build.  cmake -DCMAKE_BUILD_TYPE=Release -B build/ src/ ; cmake --build build/")
    print("2. Check.  cd build/ ; ./tests")
    print('3. Commit. git commit -am "[DOC] Changed revision number to v'
          + version_str(args) + '"')
    print("4. Tag.    git tag -a v" + version_str(args) + " -m \"tag message\"")
    print("\nRemember to 'git push' both code and tag. For the tag:\n")
    print("   git push origin [tagname]\n")


if __name__ == "__main__":
    main()
