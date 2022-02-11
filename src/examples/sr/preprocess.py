#!/usr/bin/env python3

#
#  Copyright (C) 2013-2022 EOS di Manlio Morini.
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this file,
#  You can obtain one at http://mozilla.org/MPL/2.0/
#
#  A command line utility to simplify xrff preprocessing.
#

import argparse
import xml.etree.ElementTree as ET


def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False


def numbers_only(args):
    string_type = ["string", "nominal"]

    # Read the xrff skeleton.
    xrff = ET.parse(args.input_file)
    dataset = xrff.getroot();

    attributes = dataset.findall("./header/attributes/attribute")

    # Default attribute for class is the last one...
    skip_attr = set()
    for i, a in enumerate(attributes):
        # ... but it is a limitation that can be removed.
        if a.get("class") == "yes":
            skip_attr.add(i)
        if a.get("type") not in string_type:
            skip_attr.add(i)

    # reference[attribute] = [attr_val_1, attr_val_2, ..., attr_val_n]
    reference = dict()
    for i in range(0, len(attributes)):
        reference[i] = []

    # Compile reference dictionary and perform the transformation.
    for instance in dataset.findall("./body/instances/instance"):
        if len(instance) == len(attributes):
            for idx, value in enumerate(instance.iter("value")):
                if idx in skip_attr:
                    continue

                if value.text not in reference[idx]:
                    reference[idx].append(value.text)
                value.text = str(reference[idx].index(value.text))

    for i, a in enumerate(attributes):
        if i not in skip_attr:
            a.set("type", "numeric")

    # Just some pretty print.
    reference = {"Attribute " + str(k_attr) :
                 [k_val + " => " + str(reference[k_attr].index(k_val))
                  for k_val in reference[k_attr]]
                 for k_attr in reference if reference[k_attr]}

    # Add a comment in the XRFF file to document the transformation performed.
    dataset.insert(0, ET.Comment("Numbers only. " + str(reference)))

    xrff.write(args.output_file)


def get_cmd_line_options():
    description = "Common preprocessing for XRFF dataset files."
    parser = argparse.ArgumentParser(description = description)

    parser.add_argument("-n", "--numbers_only", action = "store_true",
                        default = False,
                        help = "Transforms nominal (string) attributes into " \
                               "numbers")

    parser.add_argument("input_file")
    parser.add_argument("output_file")

    return parser


def main():
    # Get argument flags and command options
    parser = get_cmd_line_options()
    args = parser.parse_args()

    if args.numbers_only:
        numbers_only(args)


if __name__ == "__main__":
    main()
