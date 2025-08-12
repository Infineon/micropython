import argparse
import os
# import subprocess
# import sys
# import time

# TOOLS_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../tools'))
# TESTS_PSOC6_DIR = os.path.abspath(os.path.dirname(__file__))

# def run_cmd(cmd, check=True):
#     print(f"Running: {' '.join(str(x) for x in cmd)}")
#     result = subprocess.run(cmd, check=check)
#     return result.returncode

# def start_test_info(tests_name, tests_dev=None, stub_dev=None):
#     print("\n------------------------------------------")
#     print(f"running tests  : {tests_name}")
#     if tests_dev:
#         print(f"test dev       : {tests_dev}")
#     if stub_dev:
#         print(f"stub dev       : {stub_dev}")
#     print()

# def run_tests(tests_name, test_dev, tests, excluded_tests=None, stub_name=None, stub_dev=None, stub_script=None):
#     start_test_info(tests_name, test_dev, stub_dev)
#     if stub_name and stub_script and stub_dev:
#         print(f"executing stub : {stub_name}")
#         run_cmd([os.path.join(TOOLS_DIR, "mpremote", "mpremote.py"), "connect", stub_dev, "run", "--no-follow", stub_script])
#         print()
#     test_dir_flag = "-d" if not tests.endswith(".py") else ""
#     cmd = ["./run-tests.py", "-t", f"port:{test_dev}"]
#     if test_dir_flag:
#         cmd += [test_dir_flag, tests]
#     else:
#         cmd += [tests]
#     if excluded_tests:
#         cmd += excluded_tests.split()
#     exit_code = run_cmd(cmd, check=False)
#     if exit_code != 0:
#         run_cmd(["./run-tests.py", "--print-failures"], check=False)
#         run_cmd(["./run-tests.py", "--clean-failures"], check=False)
#     return exit_code

# def mpremote_vfs_large_file_tests(dev_test, afs, storage_device):
#     print("\nrunning tests : vfs large files\n")
#     fs_py = os.path.join(TESTS_PSOC6_DIR, "mp_custom", "fs.py")
#     os.chmod(fs_py, 0o777)
#     enable_adv_tests = "adv" if afs else "basic"
#     return run_cmd(["python3", fs_py, dev_test, enable_adv_tests, storage_device], check=False)

# def vfs_flash_tests(dev_test, afs):
#     exit_code = run_tests("file system flash", dev_test,
#         "extmod/vfs_basic.py extmod/vfs_lfs_superblock.py extmod/vfs_userfs.py")
#     storage_device = "flash"
#     exit_code |= mpremote_vfs_large_file_tests(dev_test, afs, storage_device)
#     return exit_code

# def vfs_sdcard_tests(dev_test, afs):
#     exit_code = run_tests("file system sdcard", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/single/sdcard.py"))
#     storage_device = "sd"
#     exit_code |= mpremote_vfs_large_file_tests(dev_test, afs, storage_device)
#     return exit_code

# def no_ext_hw_tests(dev_test):
#     excluded = f"-e {os.path.join(TESTS_PSOC6_DIR, 'board_only_hw/single/wdt.py')} -e {os.path.join(TESTS_PSOC6_DIR, 'board_only_hw/single/wdt_reset_check.py')}"
#     return run_tests("no extended hardware", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_only_hw/single"), excluded)

# def adc_tests(dev_test):
#     return run_tests("adc", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/single/adc.py"))

# def pwm_tests(dev_test):
#     return run_tests("pwm", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/single/pwm.py"))

# def pin_tests(dev_test):
#     return run_tests("pin", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/single/pin.py"))

# def signal_tests(dev_test):
#     return run_tests("signal", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/single/signal.py"))

# def i2c_tests(dev_test):
#     return run_tests("i2c", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/single/i2c.py"))

# def uart_tests(dev_test):
#     return run_tests("uart", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/single/uart.py"))

# def bitstream_tests(dev_test, dev_stub):
#     return run_tests("bitstream", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/multi/bitstream_rx.py"),
#                      None, "bitstream_tx", dev_stub, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/multi/bitstream_tx.py"))

# def spi_tests(dev_test, dev_stub):
#     return run_tests("spi", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/multi/spi_master.py"),
#                      None, "spi_slave", dev_stub, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/multi/spi_slave.py"))

# def i2s_tests(dev_test, dev_stub):
#     return run_tests("i2s", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/multi/i2s_rx.py"),
#                      None, "i2s_tx", dev_stub, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/multi/i2s_tx.py"))

# def time_pulse_tests(dev_test, dev_stub):
#     return run_tests("time_pulse", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/multi/time_pulse_us.py"),
#                      None, "time_pulse_sig_gen", dev_stub, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/multi/time_pulse_sig_gen.py"))

# def pdm_pcm_tests(dev_test, dev_stub):
#     return run_tests("pdm_pcm", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/multi/pdm_pcm_rx.py"),
#                      None, "pdm_pcm_tx", dev_stub, os.path.join(TESTS_PSOC6_DIR, "board_ext_hw/multi/pdm_pcm_tx.py"))

# def wdt_tests(dev_test):
#     exit_code = run_tests("wdt", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_only_hw/single/wdt.py"))
#     time.sleep(2)
#     exit_code |= run_tests("wdt reset check", dev_test, os.path.join(TESTS_PSOC6_DIR, "board_only_hw/single/wdt_reset_check.py"))
#     return exit_code

# def multi_tests(dev_test, dev_stub):
#     start_test_info("multiple boards instances", dev_test, dev_stub)
#     multi_tests_dir = os.path.join(TESTS_PSOC6_DIR, "board_only_hw/multi/")
#     multi_tests = [os.path.join(multi_tests_dir, f) for f in os.listdir(multi_tests_dir) if f.endswith('.py')]
#     cmd = ["./run-multitests.py", "-i", f"pyb:{dev_test}", "-i", f"pyb:{dev_stub}"] + multi_tests
#     exit_code = run_cmd(cmd, check=False)
#     return exit_code


def run_test_suite(test_suite, test_plan, hil, board, dev_test, dev_stub):
    print("#" * 40)
    print(f"test suite  : {test_suite}")
    print(f"test plan   : {test_plan}")

    if hil is not None:
        print(f"hil         : {hil}")
        if args.board is not None:
            print(f"board       : {board}")

        # if hil is provided test and stub devices are ignored
        dev_test = None
        dev_stub = None

    else:
        print(f"test dev    : {dev_test}")
        print(f"stub dev    : {dev_stub}")

        # if board is provided, it will be ignored
        board = None

    # Parse test plan file
    # find the matching test suite in the test plan
    # if test_suite == "all": it will take the complete list
    # if it is a list, find the matching elements
    # exception if not found

    # if hil, get device list a, b, c

    # For each test_suite
    # Run the test according to the type
    #   if type == "single":
    #     get_devices_from_hil_and_test(())
    #     run_single_test(test, dev_test, except)
    #   elif type == "multi_stub":
    #     run_multi_stub_test(test, dev_test, dev_stub)
    #   elif type == "multi":
    #     run_multi_test(test, dev_test, dev_stub)
    #   elif type == "storage":
    #     run_storage_test()


def run_single_test():
    pass


def run_multi_stub_test():
    pass


def run_multi_test():
    pass


def run_storage_test():
    pass


def tests_parser_func(args):
    run_test_suite(
        args.test_suite, args.test_plan, args.hil, args.board, args.dev_test, args.dev_stub
    )


def tests_list_subparser_func(args):
    print("#" * 40)
    print(f"test plan   : {args.test_plan}")

    # Ignored arguments:
    # --test-suite --hil, --board, --dev-test, --dev-stub


tests_parser = argparse.ArgumentParser(description="Run PSOC6 test suites.")
tests_parser.set_defaults(func=tests_parser_func)

tests_parser.add_argument(
    "--test-suite",
    "-t",
    nargs="+",
    type=str,
    default="all",
    help="Test suite to run. Default is 'all'.",
)

tests_parser.add_argument(
    "--test-plan",
    type=str,
    default=os.path.join(os.getcwd(), "test-plan.json"),
    help="Path to the test plan file. Default is 'test-plan.json' located in 'tests/ports/psoc6'.",
)

tests_parser.add_argument("--hil", type=str, default=None, help="Path to the HIL server file.")

tests_parser.add_argument(
    "--board",
    type=str,
    default=None,
    help="Board filter if HIL file is provided. Applicable tests will only run on the specified board.",
)

tests_parser.add_argument(
    "--dev-test", type=str, default="/dev/ttyACM0", help="Test device. Default is '/dev/ttyACM0'."
)

tests_parser.add_argument(
    "--dev-stub", type=str, default="/dev/ttyACM1", help="Stub device. Default is '/dev/ttyACM1'."
)

tests_subparser = tests_parser.add_subparsers()
test_list_subparser = tests_subparser.add_parser(
    "list", help="List available tests from the test plan file."
)
test_list_subparser.set_defaults(func=tests_list_subparser_func)

# run_psoc6_tests.py <test-suite> --board <board> --hil <hil-file> --test-conf <test-plan-file>
# run_psoc6_tests.py <test-suite> --dev-test /dev/ttyACM0 --dev-stub /dev/ttyACM1 --test-conf <test-plan-file>
# Mandatory arguments:
#  tests-suite
#  test-plan
#  direct serial device or hil file list
# Optional:
#  board (if hil file is provided) Otherwise not relevant, the user decides.
# run_psoc6_test.py list --test-plan <test-plan-file> -> {json}


if __name__ == "__main__":
    # Parse the command line arguments
    args = tests_parser.parse_args()
    args.func(args)
