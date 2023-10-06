from __future__ import print_function
import argparse
import sys
import csv
import re
import os


mtb_root_dir = "./mtb_shared"
mtb_device_header = "./mtb_shared/mtb-pdl-cat1/release-v3.6.0/devices/COMPONENT_CAT1A/include/"
mtb_lib_log_file = "./mtb-libs/deps/locking_commit.log"


def get_mtb_hal_release_version():
    with open(mtb_lib_log_file, "r") as f:
        log_file = f.read()
        mtb_hal_cat1_version = log_file.split("mtb-hal-cat1 ")[1].split("\n")[0]
        return mtb_hal_cat1_version


def get_mtb_pdl_release_version():
    with open(mtb_lib_log_file, "r") as f:
        log_file = f.read()
        mtb_pdl_cat1_version = log_file.split("mtb-pdl-cat1 ")[1].split("\n")[0]
        return mtb_pdl_cat1_version


def get_pin_addr_helper(pin_def):
    pattern = r"CYHAL_PORT_(\d+),\s*(\d+)"
    match = re.search(pattern, pin_def)
    port_number = match.group(1)
    pin_number = match.group(2)
    return (int(port_number) << 3) + int(pin_number)


def get_gpio_hdr_path(filename):
    file_path = (
        f"./mtb_shared/mtb-pdl-cat1/{get_mtb_pdl_release_version()}/devices/COMPONENT_CAT1A/include/{filename}"
        if get_mtb_pdl_release_version()
        else None
    )
    if os.path.isfile(file_path):
        return file_path
    return None


def get_pin_package_path(filename):
    file_path = (
        f"./mtb_shared/mtb-hal-cat1/{get_mtb_hal_release_version()}/COMPONENT_CAT1A/include/pin_packages/{filename}"
        if get_mtb_hal_release_version()
        else None
    )
    if os.path.isfile(file_path):
        return file_path
    return None


def _get_pin_def(file_path):
    with open(file_path, "r") as file:
        content = file.read()

    # Find the starting and ending points of the enum declaration
    enum_start = content.find("typedef enum {")
    enum_end = content.find("}")

    if enum_start != -1 and enum_end != -1:
        enum_content = content[enum_start:enum_end]

        # Extract enum values using regex
        pin_name = re.findall(r"\b(?!NC\b)(P\w+)\s*=", enum_content)
        pin_def = re.findall(r"=\s*(.*?\))", enum_content)
        pin_addr = [get_pin_addr_helper(pin) for pin in pin_def]

        return pin_name, pin_addr, pin_def
    else:
        return []


def _get_pin_af_details(gpio_hdr_file_path):
    with open(gpio_hdr_file_path, "r") as file:
        content = file.read()

    enum_start = content.index("/* HSIOM Connections */\ntypedef enum\n{\n")
    enum_end = content.index("} en_hsiom_sel_t;") + len("} en_hsiom_sel_t;")
    enum_content = content[enum_start:enum_end]

    # extract the enum values
    enum_values = re.findall(r"(\w+)\s*=\s*([\-\d]+)", enum_content)

    ## create dictionary from enum values
    enum_content1 = {name: int(value) for name, value in enum_values}

    hsiom_name = [name for name in enum_content1.keys() if name.startswith("HSIOM_SEL_ACT_")]

    ## Create a list of tuples storing the name and number for each "HSIOM_SEL_ACT_" value
    hsiom_defs = [(name, enum_content1[name]) for name in hsiom_name]

    pins = {}
    for key in enum_content1:
        value = enum_content1[key]
        for i in range(len(hsiom_defs)):
            if hsiom_defs[i][1] == value:
                pin = key.split("_")[0] + "_" + key.split("_")[1]
                if pin not in pins:
                    pins[pin] = [" "] * len(hsiom_defs)
                if i < len(pins[pin]):
                    pins[pin][i] = key
                break
    pins = {key: value for key, value in pins.items() if key.startswith("P")}

    return pins


def _write_to_csv(data, csv_filename):
    with open("./" + csv_filename, "w", newline="") as csv_file:
        csv_writer = csv.writer(csv_file)
        csv_writer.writerows(data)


def generate_pins_csv(pin_package_filename, pins_csv_filename):
    file_path = get_pin_package_path(pin_package_filename)
    if file_path is None:
        sys.exit(1)

    pin_details = _get_pin_def(file_path)

    csv_data = [[pin_name, pin_name] for pin_name in pin_details[0]]

    _write_to_csv(csv_data, pins_csv_filename)


def generate_af_pins_csv(pin_package_filename, gpio_mtb_header_filename, pins_af_csv_filename):
    pin_package_file_path = get_pin_package_path(pin_package_filename)
    gpio_hdr_file_path = get_gpio_hdr_path(gpio_mtb_header_filename)

    if pin_package_file_path is None or pin_package_file_path is None:
        sys.exit(1)

    pin_details = _get_pin_def(pin_package_file_path)

    pin_af_details = _get_pin_af_details(gpio_hdr_file_path)

    csv_data = [
        (
            pin_details[0][i],
            pin_details[1][i],
            pin_details[2][i],
            pin_af_details[pin_details[0][i]],
        )
        for i in range(len(pin_af_details))
    ]

    _write_to_csv(csv_data, pins_af_csv_filename)


def main():

    parser = argparse.ArgumentParser(
        prog="make-pins-csv.py",
        usage="%(prog)s [options] [command]",
        description="Generate intermediate board specific pin csv files to be used by make-pins script",
    )

    parser.add_argument(
        "-g",
        "--gen-pin-for",
        dest="pin_package_filename",
        help="Specifies the pin package file from mtb assets to generate pins.csv",
    )

    parser.add_argument(
        "-gp",
        "--gpio-file",
        dest="gpio_mtb_header_filename",
        help="Specifies the gpio header file from mtb assets to generate pins_af.csv",
    )

    parser.add_argument(
        "-p",
        "--save-pins-csv-at",
        dest="pins_csv",
        help="Specifies name of generated pins csv file",
    )

    parser.add_argument(
        "-gaf",
        "--save-pins-af-csv-at",
        dest="pins_af_csv",
        help="Specifies name of generated alternate functions pins csv file",
    )

    args = parser.parse_args(sys.argv[1:])

    if args.pin_package_filename and args.pins_csv:
        print("// Generating pins csv file")
        print("// - --gen-pin-for {:s}".format(args.pin_package_filename))
        print("// - --save-pins-csv-at {:s}".format(args.pins_csv))
        generate_pins_csv(args.pin_package_filename, args.pins_csv)

    if args.pin_package_filename and args.gpio_mtb_header_filename and args.pins_af_csv:
        print("// Generating alternate functions pins csv file")
        print("// - --save-pins-csv-at {:s}".format(args.pins_af_csv))
        generate_af_pins_csv(
            args.pin_package_filename, args.gpio_mtb_header_filename, args.pins_af_csv
        )


if __name__ == "__main__":
    main()
