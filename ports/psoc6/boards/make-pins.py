from __future__ import print_function
import argparse
import sys
import csv
import re
import os


def get_pin_addr_helper(pin_def):
    pattern = r"CYHAL_PORT_(\d+),\s*(\d+)"
    match = re.search(pattern, pin_def)
    port_number = match.group(1)
    pin_number = match.group(2)
    return (int(port_number) << 3) + int(pin_number)


def get_pin_package_path(filename):
    root_dir = "./mtb_shared/mtb-hal-cat1"
    mid_dir = "COMPONENT_CAT1A/include/pin_packages"
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for dirname in dirnames:
            if dirname.startswith("release-"):
                release_version = dirname
                file_path = os.path.join(root_dir, release_version, mid_dir, filename)
                if os.path.isfile(file_path):
                    return file_path
    return None


class NamedPin(object):
    def __init__(self, name, pin):
        self._name = name
        self._pin = pin

    def pin(self):
        return self._pin

    def name(self):
        return self._name


class Pin(object):
    def __init__(self, name, pin_addr):
        self._name = name
        self._pin_addr = pin_addr
        self._board_pin = False

    def cpu_pin_name(self):
        return self._name

    def is_board_pin(self):
        return self._board_pin

    def set_is_board_pin(self):
        self._board_pin = True

    def print(self):
        print(
            "const machine_pin_obj_t pin_{:s}_obj = PIN({:s}, {:s});".format(
                self._name,
                self._name,
                self._pin_addr,
            )
        )
        print("")

    def print_header(self, hdr_file):
        n = self.cpu_pin_name()
        hdr_file.write("extern const machine_pin_obj_t pin_{:s}_obj;\n".format(n))
        hdr_file.write("#define pin_{:s} (&pin_{:s}_obj)\n".format(n, n))

    def qstr_list(self):
        return [self._name]


class Pins(object):
    def __init__(self):
        self.cpu_pins = []  # list of NamedPin objects
        self.board_pins = []  # list of NamedPin objects
        self.board_pin_csv_path = ""

    def find_pin(self, cpu_pin_name):
        for named_pin in self.cpu_pins:
            pin = named_pin.pin()
            if pin.cpu_pin_name() == cpu_pin_name:
                return pin

    def generate_pins_csv(self, pin_package_filename, pins_csv_filename):
        file_path = get_pin_package_path(pin_package_filename)

        if file_path is None:
            sys.exit(1)

        # Read the header file
        with open(file_path, "r") as file:
            content = file.read()

        # Find the starting and ending points of the enum declaration
        enum_start = content.find("typedef enum {")
        enum_end = content.find("}")

        if enum_start != -1 and enum_end != -1:
            enum_content = content[enum_start:enum_end]

            # Extract enum values using regex
            enum_values = re.findall(r"\b(\w+)\s*=", enum_content)

            # Write enum values to a CSV file
            with open("./" + pins_csv_filename, "w", newline="") as csv_file:
                csv_writer = csv.writer(csv_file)
                csv_writer.writerows([[value, value] for value in enum_values])
                self.board_pin_csv_path = pins_csv_filename

            print("// pins.csv generated successfully")
        else:
            print("// Error: pins.csv generation failed")

    def generate_af_pins_csv(self, pin_package_filename, pins_af_csv_filename):
        file_path = get_pin_package_path(pin_package_filename)

        if file_path is None:
            sys.exit(1)
        # Read the header file
        with open(file_path, "r") as file:
            content = file.read()

        # Find the starting and ending points of the enum declaration
        enum_start = content.find("typedef enum {")
        enum_end = content.find("}")

        if enum_start != -1 and enum_end != -1:
            enum_content = content[enum_start:enum_end]

            # Extract enum values using regex
            pin_name = re.findall(r"\b(?!NC\b)(\w+)\s*=", enum_content)
            pin_def = re.findall(r"=\s*(.*?\))", enum_content)
            # Write enum values to a CSV file
            with open("./" + pins_af_csv_filename, "w", newline="") as csv_file:
                csv_writer = csv.writer(csv_file)
                for pname, pdef in zip(pin_name, pin_def):
                    val = get_pin_addr_helper(pdef)
                    csv_writer.writerow([pname, val])

            print("// pins_af.csv generated successfully")
        else:
            print("Error: pins_af.csv generation failed")

    # ToDo: Complete for alternate functions
    def parse_af_file(self, filename):
        with open("./" + filename, "r") as csvfile:
            rows = csv.reader(csvfile)
            for row in rows:
                try:
                    pin_name = row[0]
                    pin_addr = row[1]
                except:
                    continue
                pin = Pin(pin_name, pin_addr)
                self.cpu_pins.append(NamedPin(pin_name, pin))

    def parse_board_file(self):
        with open(self.board_pin_csv_path, "r") as csvfile:
            rows = csv.reader(csvfile)
            for row in rows:
                try:
                    board_pin_name = row[0]
                    cpu_pin_name = row[1]
                except:
                    continue
                pin = self.find_pin(cpu_pin_name)
                if pin:
                    pin.set_is_board_pin()
                    self.board_pins.append(NamedPin(board_pin_name, pin))

    def print_named(self, label, named_pins):
        print(
            "STATIC const mp_rom_map_elem_t pin_{:s}_pins_locals_dict_table[] = {{".format(label)
        )
        for named_pin in named_pins:
            pin = named_pin.pin()
            if pin.is_board_pin():
                print(
                    "  {{ MP_ROM_QSTR(MP_QSTR_{:s}), MP_ROM_PTR(&pin_{:s}_obj) }},".format(
                        named_pin.name(), pin.cpu_pin_name()
                    )
                )
        print("};")
        print(
            "MP_DEFINE_CONST_DICT(pin_{:s}_pins_locals_dict, pin_{:s}_pins_locals_dict_table);".format(
                label, label
            )
        )

    def print(self):
        for named_pin in self.cpu_pins:
            pin = named_pin.pin()
            if pin.is_board_pin():
                pin.print()
        self.print_named("cpu", self.cpu_pins)
        print("")

    def print_header(self, hdr_filename):
        with open(hdr_filename, "wt") as hdr_file:
            for named_pin in self.cpu_pins:
                pin = named_pin.pin()
                if pin.is_board_pin():
                    pin.print_header(hdr_file)

    def print_qstr(self, qstr_filename):
        with open(qstr_filename, "wt") as qstr_file:
            qstr_set = set([])
            for named_pin in self.cpu_pins:
                pin = named_pin.pin()
                if pin.is_board_pin():
                    qstr_set |= set(pin.qstr_list())
                    qstr_set |= set([named_pin.name()])
            for named_pin in self.board_pins:
                qstr_set |= set([named_pin.name()])
            for qstr in sorted(qstr_set):
                # cond_var = None

                # ToDO: For next iteration
                """if qstr.startswith("AF"):
                af_words = qstr.split("_")
                cond_var = conditional_var(af_words[1])
                print_conditional_if(cond_var, file=qstr_file)"""
                print("Q({})".format(qstr), file=qstr_file)


def main():
    parser = argparse.ArgumentParser(
        prog="make-pins.py",
        usage="%(prog)s [options] [command]",
        description="Generate board specific pin file",
    )

    parser.add_argument(
        "-g",
        "--gen-pin-for",
        dest="pin_package_filename",
        help="Specifies the pin package file from mtb assets to generate pins.csv",
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

    parser.add_argument(
        "-pr",
        "--prefix",
        dest="prefix_filename",
        help="Specifies beginning portion of generated pins file",
        default="psoc6_prefix.c",
    )

    parser.add_argument(
        "-q",
        "--qstr",
        dest="qstr_filename",
        help="Specifies name of generated qstr header file",
    )

    parser.add_argument(
        "-r",
        "--hdr",
        dest="hdr_filename",
        help="Specifies name of generated pin header file",
    )

    args = parser.parse_args(sys.argv[1:])

    pins = Pins()

    if args.pin_package_filename and args.pins_csv:
        print("// Generating pins csv file with following parsed arguments")
        print("// - --gen-pin-for {:s}".format(args.pin_package_filename))
        print("// - --save-pins-csv-at {:s}".format(args.pins_csv))
        pins.generate_pins_csv(args.pin_package_filename, args.pins_csv)

    if args.pin_package_filename and args.pins_af_csv:
        print("// Generating alternate functions pins csv file with following parsed arguments")
        print("// - --save-pins-csv-at {:s}".format(args.pins_af_csv))
        pins.generate_af_pins_csv(args.pin_package_filename, args.pins_af_csv)

    if args.pins_af_csv:
        print("// --save-pins-af-csv-at {:s}".format(args.pins_af_csv))
        pins.parse_af_file(args.pins_af_csv)

    if args.prefix_filename:
        print("// --prefix {:s}".format(args.prefix_filename))
        print("")
        with open(args.prefix_filename, "r") as prefix_file:
            print(prefix_file.read())

    pins.parse_board_file()

    pins.print()
    pins.print_header(args.hdr_filename)
    pins.print_qstr(args.qstr_filename)


if __name__ == "__main__":
    main()
