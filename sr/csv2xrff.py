#!/usr/bin/env python3

#
#  Copyright 2011 EOS di Manlio Morini.
#
#  This file is part of VITA.
#
#  VITA is free software: you can redistribute it and/or modify it under the
#  terms of the GNU General Public License as published by the Free Software
#  Foundation, either version 3 of the License, or (at your option) any later
#  version.
#
#  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
#  details.
#
#  You should have received a copy of the GNU General Public License along
#  with VITA. If not, see <http://www.gnu.org/licenses/>.
#

import argparse
import csv
import os
from xml.etree.ElementTree import Element, ElementTree, SubElement


verbose = False


def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

def csv_to_xrff(args):
    # Many tips for Element / SubElement manipulation in:
    # <http://effbot.org/zone/element.htm#reading-and-writing-xml-files>

    # Build the xrff skeleton.
    xrff = Element("dataset", name=os.path.splitext(args.xrff_file)[0])
    header = SubElement(xrff, "header")
    body = SubElement(xrff, "body")
    attributes = SubElement(header, "attributes")
    instances = SubElement(body, "instances")

    # Sniff the csv file dialect.
    csv_file = open(args.csv_file, newline='')
    dialect = csv.Sniffer().sniff(csv_file.read(1024))
    csv_file.seek(0)
    csv_reader = csv.reader(csv_file, dialect)

    number = []
    for i, row in enumerate(csv_reader):
        instance = SubElement(instances, "instance")
        for j, field in enumerate(row):
            if i == 0:
                if is_number(field):
                    arg_type = "numeric"
                    number.append(True)
                else:
                    arg_type = "string"
                    number.append(False)

                attribute = SubElement(attributes, "attribute", type=arg_type)

                if j == args.class_column:
                    attribute.set("class", "yes")

            value = SubElement(instance, "value")
            if (number[j]):
                value.text = field
            else:
                value.text = '"' + field + '"'

    # Note that the standard element writer creates a compact output. There is
    # no built-in support for pretty printing or user-defined namespace
    # prefixes in the current version, so the output may not always be suitable
    # for human consumption (to the extent XML is suitable for human
    # consumption, that is).
    # SOLUTION 1 (Unix)
    #   xmllint --format ugly.xml > pretty.xml
    # SOLUTION 2
    #   from xml.etree.ElementTree import tostring
    #   def prettify(elem):
    #      rough_string = tostring(elem)
    #      reparsed = minidom.parseString(rough_string)
    #      return reparsed.toprettyxml(indent="  ")
    #   print prettify(xrff)
    ElementTree(xrff).write(args.xrff_file)


def get_cmd_line_options():
    description = "Convert a file from CSV data format to XRFF."
    parser = argparse.ArgumentParser(description=description)

    parser.add_argument("-v", "--verbose", action="store_true",
                        help="Turn on verbose mode")
    parser.add_argument("-c", "--class_column", type=int, default=0,
                        help="Which attribute should act as class attribute?")

    parser.add_argument("csv_file")
    parser.add_argument("xrff_file")

    return parser


def main():
    # Get argument flags and command options
    parser = get_cmd_line_options()
    args = parser.parse_args()

    verbose = args.verbose

    csv_to_xrff(args)


if __name__ == "__main__":
    main()
