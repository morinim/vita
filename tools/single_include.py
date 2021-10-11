#!/usr/bin/env python3

# Copyright 2019-2020 Rene Ferdinand Rivera Morell
# Distributed under the Boost Software License, Version 1.0.
# (see https://www.boost.org/LICENSE_1_0.txt)
#
# Adapted by Manlio Morini for use in Vita.

import argparse
import os.path
import re

# Requires 3 command-line arguments:
# * `--src-include` the include file to convert, i.e. to merge its include
#   directives into its body
# * `--dst-include` the output file to write
# * `--src-include-dir` the directory relative to which include files are
#   specified (i.e. an "include search path" of one directory; the script
#   doesn't support the complex mechanism of multiple include paths and search
#   priorities which the C++ compiler offers)
class GenSingleInclude(object):

    def __init__(self):
        self.pp_re = re.compile(r'''#\s*include\s+["<]([^">]+)[">]\s*''')
        parser = argparse.ArgumentParser()
        parser.add_argument('--src-include-dir')
        parser.add_argument('--src-include')
        parser.add_argument('--dst-include')
        self.args = parser.parse_args()
        if not os.path.isabs(self.args.src_include_dir):
            self.args.src_include_dir = os.path.abspath(
                os.path.join(os.curdir, self.args.src_include_dir))
        self.parsed = set()
        self.run()

    def run(self):
        with open(self.args.dst_include, "w", encoding="UTF8") as dst_file:
            dst_file.write('''/**
 *  Automatically generated global include file for VITA
 *  (https://github.com/morinim/vita).
 *  This is for convenience only and should not be edited.
 *
 *  Copyright (C) 2021 EOS di Manlio Morini.
 *
 *  License
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */
''')
            self.cpp(dst_file, None, self.args.src_include)

    def resolve_include(self, cur_dir, include_name):
        if os.path.isabs(include_name):
            return include_name
        elif cur_dir and os.path.exists(os.path.join(cur_dir, include_name)):
            return os.path.abspath(os.path.join(cur_dir, include_name))
        elif os.path.exists(os.path.join(self.args.src_include_dir, include_name)):
            return os.path.abspath(os.path.join(self.args.src_include_dir, include_name))
        else:
            return None

    # See https://stackoverflow.com/a/241506/3235496
    def comment_remover(self, text):
        """ Remove comments.
        text: text with comments (can include newlines)
        returns: text with comments removed
        """

        def replacer(match):
            s = match.group(0)
            if s.startswith('/'):
                # Note: a space and NOT an empty string (otherwise the legal
                # expression `int/**/x=5;` would become `intx=5;` which doesn't
                # compile.
                return " "
            else:
                return s

        pattern = re.compile(
            r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
            re.DOTALL | re.MULTILINE
        )
        return re.sub(pattern, replacer, text)

    def cpp(self, dst_file, cur_include_dir, src_include):
        src_i = self.resolve_include(cur_include_dir, src_include)
        if src_i and src_i not in self.parsed:
            self.parsed.add(src_i)
            with open(src_i, "r", encoding="UTF8") as raw_src_file:
                src_file = self.comment_remover(raw_src_file.read())

                for line in src_file.splitlines():
                    pp_match = self.pp_re.fullmatch(line)
                    src_n = None
                    if pp_match:
                        src_n = self.cpp(dst_file, os.path.dirname(
                            src_i), pp_match.group(1))
                    if not src_n:
                        dst_file.write(line + os.linesep)
        return src_i


if __name__ == "__main__":
    GenSingleInclude()
