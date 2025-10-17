import argparse
from dataclasses import dataclass, field
from enum import Enum
import logging
import os
import sys
import subprocess
import yaml
import time

from get_devs import get_devices_port

logging.basicConfig(level=logging.DEBUG, format="%(levelname)s: %(message)s")


class TestRunner:
    class DeviceRole(Enum):
        DUT = "dut"
        STUB = "stub"

    def __init__(
        self,
        name,
        test_script_list,
        test_exclude_list=[],
        post_test_delay_ms=0,
        stub_script=None,
        allowed_dut_device_list=[],
        allowed_stub_device_list=[],
        post_stub_delay_ms=0,
        test_type=None,
        custom_args=[],
        myp_test_dir=None,
    ):
        self.name = name
        self.test_script_list = test_script_list
        self.test_exclude_list = test_exclude_list
        self.post_test_delay_ms = post_test_delay_ms
        self.stub_script = stub_script
        self.allowed_dut_device_list = allowed_dut_device_list
        self.allowed_stub_device_list = allowed_stub_device_list
        self.post_stub_delay_ms = post_stub_delay_ms
        self.custom_args = custom_args
        self.type = test_type if test_type is not None else TestRunner.determine_type(self)
        self.myp_test_dir = (
            os.path.join(TestRunner.set_default_mpy_dir(), "tests")
            if myp_test_dir is None
            else myp_test_dir
        )
        self.runner_func = TestRunner.get_runner_func(self, self.type)

        # TODO: Encapsulate this.
        # In case of multi test we don´t make this distinction...
        # But it could be the case.
        if self.type == "multi":
            self.allowed_stub_device_list = allowed_dut_device_list

        logging.debug(
            f"TestRunner initialized with type: {self.type} and runner function: {getattr(self.runner_func, '__name__', 'None')}"
        )

    # def __str__(self):
    #     return yaml.safe_dump(self.__dict__, sort_keys=False)

    def run(self, dut_port, stub_port=None):
        # Move to the test directory
        os.chdir(self.myp_test_dir)

        if "multi" in self.type:
            return self.runner_func(dut_port, stub_port)
        else:
            return self.runner_func(dut_port)

    def get_allowed_board_list(self, dev_role, board, version=None):
        if dev_role == TestRunner.DeviceRole.DUT:
            device_list = self.allowed_dut_device_list
        elif dev_role == TestRunner.DeviceRole.STUB:
            device_list = self.allowed_stub_device_list

        allowed_board_device_list = []
        for device in device_list:
            # if passed board is in required devices
            if board in device.get("board"):
                if version is None or device.get("version") == version:
                    allowed_board_device_list.append(device)

        return allowed_board_device_list

    def requires_multiple_devs(self):
        return "multi" in self.type

    def is_board_supported(self, board):
        # Check if the board is in the allowed device list for DUT
        allowed_boards = [device.get("board") for device in self.allowed_dut_device_list]
        for allowed_board in allowed_boards:
            if board == allowed_board:
                return True
        return False

    """
    Private methods 
    """

    def determine_type(self):
        if self.stub_script is not None:
            return "multi_stub"
        else:
            if self.post_test_delay_ms > 0:
                return "single_post_delay"
            else:
                return "single"

    def get_runner_func(self, type):
        runner_func = {
            "single": self.run_single_test,
            "single_post_delay": self.run_single_post_delay_test,
            "multi_stub": self.run_multi_stub_test,
            "multi": self.run_multi_test,
            "custom": self.custom_test,
        }

        return runner_func.get(type, None)

    def run_single_test_cmd(self, dut_port, test_args, exclude_args):
        run_test_cmd = ["python", "run-tests.py", "-t", f"port:{dut_port}"]
        run_test_cmd.extend(test_args)
        run_test_cmd.extend(exclude_args)
        logging.debug(f"Single test command args: {run_test_cmd}")

        run_test_proc = subprocess.run(run_test_cmd)

        if run_test_proc.returncode != 0:
            run_test_print_fail_cmd = ["python", "run-tests.py", "--print-failures"]
            subprocess.run(run_test_print_fail_cmd)

            run_test_clean_fail_cmd = ["python", "run-tests.py", "--clean-failures"]
            subprocess.run(run_test_clean_fail_cmd)

        return run_test_proc.returncode

    def run_single_test(self, dut_port):
        logging.debug(f"Running single test {self.name} on DUT port {dut_port}")

        def get_test_list_args():
            test_list_args = []
            for test in self.test_script_list:
                # Append -d if it is a directory
                if os.path.isdir(test):
                    test_list_args.append("-d")

                test_list_args.append(test)
            return test_list_args

        def get_test_list_exclude_args():
            test_list_exclude_args = []
            for excluded_test in self.test_exclude_list:
                # Append -e to exclude a test
                test_list_exclude_args.append("-e")
                test_list_exclude_args.append(excluded_test)
            return test_list_exclude_args

        test_list_args = get_test_list_args()
        test_list_exclude_args = get_test_list_exclude_args()

        return self.run_single_test_cmd(dut_port, test_list_args, test_list_exclude_args)

    def run_single_post_delay_test(self, dut_port):
        logging.debug(f"Running single test post delay {self.name} on DUT port {dut_port}")

        def get_test_list_args():
            test_list_args = []
            for test in self.test_script_list:
                if os.path.isdir(test):
                    for root, dirs, files in os.walk(test):
                        for file in files:
                            if file.endswith(".py"):
                                test_file = os.path.join(root, file)
                                test_list_args.append(test_file)
                else:
                    test_list_args.append(test)

            return test_list_args

        def remove_excluded_tests(test_list_args):
            if self.test_exclude_list:
                for excluded_test in self.test_exclude_list:
                    if excluded_test in test_list_args:
                        test_list_args.remove(excluded_test)

        test_list_args = get_test_list_args()
        remove_excluded_tests(test_list_args)

        for test in test_list_args:
            print(test)
            return_code = self.run_single_test_cmd(dut_port, [test], [])
            if return_code != 0:
                logging.error(
                    f"Test {test} failed on port {dut_port} with return code {return_code}"
                )
                return return_code

            if self.post_test_delay_ms > 0:
                time.sleep(self.post_test_delay_ms / 1000.0)

        return 0

    def run_stub(self, stub_port):
        logging.debug(f"Running stub script {self.stub_script} on stub port {stub_port}")
        mpremote_py = os.path.join(self.myp_test_dir, "..", "tools", "mpremote", "mpremote.py")
        stub_run_cmd = [mpremote_py, "connect", stub_port, "run", "--no-follow", self.stub_script]
        logging.debug(f"Stub run command args: {stub_run_cmd}")
        stub_run_proc = subprocess.run(stub_run_cmd)
        return stub_run_proc.returncode

    def run_multi_stub_test(self, dut_port, stub_port):
        logging.debug(
            f"Running multi stub test {self.name} on DUT port {dut_port} and stub port {stub_port}"
        )

        return_code = self.run_stub(stub_port)
        if return_code != 0:
            logging.error(
                f"Stub script {self.stub_script} failed on port {stub_port} with return code {return_code}"
            )
            return return_code

        if self.post_stub_delay_ms > 0:
            time.sleep(self.post_stub_delay_ms / 1000.0)

        return self.run_single_test(dut_port)

    def run_multi_test(self, dut_a_port, dut_b_port):
        logging.debug(
            f"Running multi test {self.name} on DUT ports {dut_a_port} and  {dut_b_port}"
        )

        def get_test_list():
            test_list = []
            # Check if it is a directory or a list of files
            for test in self.test_script_list:
                if os.path.isdir(test):
                    # Find all the python files in the directory
                    for root, dirs, files in os.walk(test):
                        for file in files:
                            if file.endswith(".py"):
                                test_list.append(os.path.join(root, file))
                else:
                    test_list.append(test)

            return test_list

        multi_test_cmd = [
            "python",
            "run-multitests.py",
            "-t",
            f"{dut_a_port}",
            "-t",
            f"{dut_b_port}",
        ]
        multi_test_list_args = get_test_list()
        multi_test_cmd.extend(multi_test_list_args)
        logging.debug(f"Multi test command args: {multi_test_cmd}")

        multi_test_proc = subprocess.run(multi_test_cmd)

        return multi_test_proc.returncode

    # TODO: Add vfs mode to avoid repl tests
    # def vfs_mode_test(self, dut_port):
    # https://github.com/mattytrentini/micropython-test-port

    def custom_test(self, dut_port):
        logging.debug(f"Running custom test {self.name} on DUT port {dut_port}")

        for test in self.test_script_list:
            custom_test_cmd = ["python", test, dut_port]

            if self.custom_args:
                custom_test_cmd.extend(self.custom_args)

            logging.debug(f"Custom test command args: {custom_test_cmd}")
            custom_test_proc = subprocess.run(custom_test_cmd)

            # TODO: Shall we keep this fail fast approach?
            if custom_test_proc.returncode != 0:
                return custom_test_proc.returncode

    @staticmethod
    def set_default_mpy_dir():
        # The root dir is two levels up from the script path
        run_test_plan_script_dir = os.path.abspath(os.path.dirname(__file__))
        return os.path.abspath(os.path.join(run_test_plan_script_dir, "..", ".."))

    @classmethod
    def load_list_from_yaml(cls, test_plan_yaml, myp_test_dir=None):
        """
        Load a list of items from a yaml file, one item per line.
        """
        if not os.path.exists(test_plan_yaml):
            logging.error(f'Test plan file "{test_plan_yaml}" does not exist')
            sys.exit(1)

        try:
            with open(test_plan_yaml, "r") as f:
                test_plan = yaml.safe_load(f)
        except:
            logging.error(f'Unable to open YAML file "{test_plan_yaml}"')
            sys.exit(1)

        # TODO: we can add schema validation, which involves
        # defining a schema and using a non built-in library like
        # https://github.com/pyeve/cerberus (check hil-makers schema validation)
        test_list = []
        for test in test_plan:
            # Create test runner instance from test

            test_file_list = test.get("test", {}).get("script", [])
            if not isinstance(test_file_list, list):
                test_file_list = [test_file_list]

            test_file_exclude_list = test.get("test", {}).get("exclude", [])
            if not isinstance(test_file_exclude_list, list):
                test_file_exclude_list = [test_file_exclude_list]

            test_runner = cls(
                test.get("name"),
                test_script_list=test_file_list,
                test_exclude_list=test_file_exclude_list,
                post_test_delay_ms=test.get("test", {}).get("post_test_delay_ms", 0),
                stub_script=test.get("stub", {}).get("script", None),
                allowed_dut_device_list=test.get("test", {}).get("device", []),
                allowed_stub_device_list=test.get("stub", {}).get("device", []),
                post_stub_delay_ms=test.get("post_stub_delay_ms", 0),
                test_type=test.get("type", None),
                custom_args=test.get("test", {}).get("args", []),
                myp_test_dir=myp_test_dir,
            )
            # logging.debug(f"\n{"#" * 40}\nLoaded test\n{"#" * 40}\n{test_runner}{"#" * 40}")
            test_list.append(test_runner)

        return test_list


class TestPlanRunner:
    def __init__(self, test_plan_file, hil_devs_file):
        self.test_plan_file = test_plan_file
        self.hil_devs_file = hil_devs_file

    def run(self, board, test_name_list=None):
        # test_plan_list = TestRunner.load_list_from_yaml(self.test_plan_file)

        # # Get test_list from the list_plan according to the test arguments
        # test_list = test_plan_list
        test_list = self.get_test_list(test_name_list)

        # while TestRetriesPending():
        # Inform if retries, which number letf.
        for test in test_list:
            if not test.is_board_supported(board):
                logging.debug(f'Board "{board}" is not supported for test "{test.name}"')
                continue

            dut_port, stub_port = self.get_test_device_ports(test, board)
            # if not required devices found we skip

            # TODO: add here device.switch capabilities to reset. ?? It should be doable as per device, if we don´t
            # want to reset all the devices in case there would be other recurrent services using them.

            print(
                f">>> Running test: {test.name}, will use DUT port: {dut_port}, STUB port: {stub_port}\n"
            )

            # if test.name == "pin":
            # if test.name == "vfs-flash":
            # if test.name == "time_pulse":
            # if test.name == "wifi":
            # if test.name == "watchdog":
            # if test.name == "vfs-flash-large":
            # ret_code = test.run(dut_port, stub_port)

            # if ret_code != 0:
            ## add test to retries

        # test_list = test_list_with_failures_and_retries

    def get_test_device_ports(self, test, board):
        def get_ports_for_role(test, board, device_role):
            allowed_board_list = test.get_allowed_board_list(device_role, board)

            logging.debug(
                f'ALLOWED {device_role} board list for test "{test.name}": {allowed_board_list}'
            )

            port_list = []
            for device in allowed_board_list:
                available_ports = get_devices_port(
                    device.get("board"), self.hil_devs_file, device.get("version", None)
                )
                port_list.extend(available_ports)

            logging.debug(f'AVAILABLE {device_role} ports for test "{test.name}": {port_list}')

            return port_list

        dut_port = None
        stub_port = None

        dut_port_list = get_ports_for_role(test, board, TestRunner.DeviceRole.DUT)

        if not dut_port_list:
            # Test should skip
            logging.error(f'No DUT devices found for test "{test.name}" and board "{board}"')
            return dut_port, stub_port

        # Take the first
        dut_port = dut_port_list[0]

        if test.requires_multiple_devs():
            stub_port_list = get_ports_for_role(test, board, TestRunner.DeviceRole.STUB)

            # Take any element from stub_port_list that is not dut_port
            for port in stub_port_list:
                if port != dut_port:
                    stub_port = port
                    break

            if not stub_port:
                # Test should skip
                logging.error(f'No STUB devices found for test "{test.name}" and board "{board}"')
                return dut_port, stub_port

        return dut_port, stub_port

    def get_test_list(self, test_name_list=None):
        test_plan_list = TestRunner.load_list_from_yaml(self.test_plan_file)

        if test_name_list is None:
            return test_plan_list

        test_list = []
        for test_name in test_name_list:
            for test in test_plan_list:
                if test.name == test_name:
                    test_list.append(test)

        return test_list


if __name__ == "__main__":
    test_plan_file = "test-plan.yml"
    hil_devs_file = "local-devs.yml"

    board = "CY8CKIT-062S2-AI"
    # board = "CY8CPROTO-062-4343W"
    # board = "CY8CPROTO-063-BLE"

    test_plan_runner = TestPlanRunner(test_plan_file, hil_devs_file)
    test_name_list = None
    # test_name_list = ["wifi", "vfs-flash"]
    test_plan_runner.run(board, test_name_list)
