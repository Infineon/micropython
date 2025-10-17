import argparse
from dataclasses import dataclass
from abc import ABC, abstractmethod
from enum import Enum
import os
import sys
import subprocess
import yaml
import time

from get_devs import get_devices_port


class TestRunner:
    """
    This class takes care of running the different MicroPython test types.
    It supports the following:
    - single: single device tests
    - single_post_delay: single device tests with a delay between tests
    - multi: multi device tests
    - multi_stub: multi device tests with a stub device
    - custom: custom test scripts
    """

    class DeviceRole(Enum):
        DUT = "dut"
        STUB = "stub"

    def __init__(
        self,
        name: str,
        test_script_list: list[str],
        test_exclude_list: list[str] = [],
        post_test_delay_ms: int = 0,
        stub_script: str = None,
        supported_dut_dev_list: list[dict] = [],
        supported_stub_dev_list: list[dict] = [],
        post_stub_delay_ms: int = 0,
        test_type: str = None,
        custom_args: list[str] = [],
        myp_test_dir: str = None,
    ):
        """
        Initializes the TestRunner instance.
        At least for a single test the test name and the test script list must be provided.
        The test script paths need to be relative to the MicroPython test directory (/tests).
        The rest of the parameters are required depending on the test type.
        """
        self.name = name
        self.test_script_list = test_script_list
        self.test_exclude_list = test_exclude_list
        self.post_test_delay_ms = post_test_delay_ms
        self.stub_script = stub_script
        self.supported_dut_dev_list = supported_dut_dev_list
        self.supported_stub_dev_list = supported_stub_dev_list
        self.post_stub_delay_ms = post_stub_delay_ms
        self.custom_args = custom_args
        self.type = (
            test_type if test_type is not None else TestRunner.__determine_implicit_type(self)
        )
        self.runner_func = TestRunner.__get_runner_func(self, self.type)
        self.myp_test_dir = (
            os.path.join(TestRunner.__set_default_mpy_dir(), "tests")
            if myp_test_dir is None
            else myp_test_dir
        )

    def run(self, dut_port: str, stub_port: str = None) -> int:
        """
        Run the test using the appropriate runner function.
        Before running the test, change to the MicroPython test directory.
        All test scripts are relative to that directory.
        """
        os.chdir(self.myp_test_dir)

        if "multi" in self.type:
            return self.runner_func(dut_port, stub_port)
        else:
            return self.runner_func(dut_port)

    def get_supported_dev_list(
        self, dev_role: DeviceRole, board: str, version: str = None
    ) -> list[dict]:
        """
        Get the list of supported devices for the given role (dut or stub),
        board and version.
        If version is None, all versions for the given board are returned.

        Additionally, the "multi" type uses the dut supported device list
        for the stub as well.
        """
        if dev_role == TestRunner.DeviceRole.DUT:
            device_list = self.supported_dut_dev_list
        elif dev_role == TestRunner.DeviceRole.STUB:
            device_list = self.supported_stub_dev_list
            # For multi, the supported stub list is the same as dut
            if "multi" == self.type:
                device_list = self.supported_dut_dev_list

        supported_device_list = []
        for device in device_list:
            if board == device.get("board"):
                if version is None or device.get("version") == version:
                    supported_device_list.append(device)

        return supported_device_list

    def requires_multiple_devs(self) -> bool:
        """
        Returns True if the test type requires multiple devices (multi or multi_stub).
        """
        return "multi" in self.type

    def are_supported_devs_available(self, dut_port: str, stub_port: str) -> bool:
        """
        Check if the required devices are available based on the test type.
        For single tests, only the dut_port is required.
        For multi tests, both dut_port and stub_port are required.
        """
        if dut_port is None:
            return False

        if self.requires_multiple_devs() and stub_port is None:
            return False

        return True

    @classmethod
    def load_list_from_yaml(
        cls, test_plan_yaml: str, myp_test_dir: str = None
    ) -> list["TestRunner"]:
        """
        Load a list of TestRunner instances from a YAML test plan file.
        Each test in the YAML file is used to create a TestRunner instance.
        The following keys are available in the YAML file for each test:

        name: <Name of the test>
        type: <Type of the test: single, single_post_delay, multi, multi_stub, custom> # Required for custom and multi. Optional otherwise.
        test:
          script: <List of test scripts or directories> # it can be a scalar or a list. For multi-stub it can be a single script.
          exclude: <List of test scripts to exclude> # it can be a scalar or a list
          device: <List of supported dut devices>
            - board: <Board name>
              version: <Version string> # Optional if specific device required

          post_test_delay_ms: <Delay between tests in milliseconds> # Optional
          args: <List of custom arguments> # Only for custom test type
        stub: # If a stub device running a script is required
          script: <Stub script to run> # A single script is supported in this case
          device: <List of supported stub devices>
            - board: <Board name>
              version: <Version string> # Optional if specific device required
          post_stub_delay_ms: <Delay after stub is started in milliseconds> # Optional

        """
        if not os.path.exists(test_plan_yaml):
            print(f'error: test plan file "{test_plan_yaml}" does not exist')
            sys.exit(1)

        try:
            with open(test_plan_yaml, "r") as f:
                test_plan = yaml.safe_load(f)
        except:
            print(f'error: unable to open YAML file "{test_plan_yaml}"')
            sys.exit(1)

        # TODO: we can add schema validation, which involves
        # defining a schema and using a non built-in library like
        # https://github.com/pyeve/cerberus (check hil-makers schema validation)

        test_list = []
        for test in test_plan:
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
                supported_dut_dev_list=test.get("test", {}).get("device", []),
                supported_stub_dev_list=test.get("stub", {}).get("device", []),
                post_stub_delay_ms=test.get("test", {}).get("post_stub_delay_ms", 0),
                test_type=test.get("type", None),
                custom_args=test.get("test", {}).get("args", []),
                myp_test_dir=myp_test_dir,
            )
            test_list.append(test_runner)

        return test_list

    """
    Private methods 
    """

    def __determine_implicit_type(self) -> str:
        """
        Determine the test type based on the provided parameters.
        If the test type is not explicitly provided, it is inferred
        from the parameters.
        """
        if self.stub_script is not None:
            return "multi_stub"
        else:
            if self.post_test_delay_ms > 0:
                return "single_post_delay"
            else:
                return "single"

    def __get_runner_func(self, type: str) -> callable:
        """
        Get the appropriate runner function based on the test type.
        """
        runner_func = {
            "single": self.__run_single_test,
            "single_post_delay": self.__run_single_post_delay_test,
            "multi_stub": self.__run_multi_stub_test,
            "multi": self.__run_multi_test,
            "custom": self.__custom_test,
        }

        return runner_func.get(type, None)

    def __run_single_test_cmd(
        self, dut_port: str, test_args: list[str], exclude_args: list[str]
    ) -> int:
        """
        Run a single test command with the given dut_port, test arguments and exclude arguments.
        It prints the failures and cleans them up if the test fails.
        """
        run_test_cmd = ["python", "run-tests.py", "-t", f"port:{dut_port}"]
        run_test_cmd.extend(test_args)
        run_test_cmd.extend(exclude_args)

        run_test_proc = subprocess.run(run_test_cmd)

        if run_test_proc.returncode != 0:
            run_test_print_fail_cmd = ["python", "run-tests.py", "--print-failures"]
            subprocess.run(run_test_print_fail_cmd)

            run_test_clean_fail_cmd = ["python", "run-tests.py", "--clean-failures"]
            subprocess.run(run_test_clean_fail_cmd)

        return run_test_proc.returncode

    def __run_single_test(self, dut_port: str) -> int:
        """
        Run a single test with the given dut_port.
        It constructs the test arguments and exclude arguments
        """

        def get_test_list_args():
            """
            If a test is a directory, append -d before it.
            """
            test_list_args = []
            for test in self.test_script_list:
                if os.path.isdir(test):
                    test_list_args.append("-d")

                test_list_args.append(test)
            return test_list_args

        def get_test_list_exclude_args():
            """
            Construct the exclude arguments list.
            It appends -e before each excluded test.
            """
            test_list_exclude_args = []
            for excluded_test in self.test_exclude_list:
                test_list_exclude_args.append("-e")
                test_list_exclude_args.append(excluded_test)
            return test_list_exclude_args

        test_list_args = get_test_list_args()
        test_list_exclude_args = get_test_list_exclude_args()

        return self.__run_single_test_cmd(dut_port, test_list_args, test_list_exclude_args)

    def __run_single_post_delay_test(self, dut_port: str) -> int:
        """
        Run single tests with a delay between each test.

        """

        def get_test_list_args():
            """
            Expand directories in the test script list to individual test files.
            If a test is a directory, find all .py files in it and add them to the list.
            """
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

        def remove_excluded_tests(test_list_args: list[str]) -> None:
            """
            Remove excluded tests from the test list arguments.
            """
            if self.test_exclude_list:
                for excluded_test in self.test_exclude_list:
                    if excluded_test in test_list_args:
                        test_list_args.remove(excluded_test)

        test_list_args = get_test_list_args()
        remove_excluded_tests(test_list_args)

        for test in test_list_args:
            return_code = self.__run_single_test_cmd(dut_port, [test], [])
            if return_code != 0:
                return return_code

            if self.post_test_delay_ms > 0:
                time.sleep(self.post_test_delay_ms / 1000.0)

        return 0

    def __run_stub(self, stub_port: str) -> int:
        """
        Run the stub script on the stub device.
        It uses the mpremote tool to connect to the stub device and run the script.
        """
        mpremote_py = os.path.join(self.myp_test_dir, "..", "tools", "mpremote", "mpremote.py")
        stub_run_cmd = [mpremote_py, "connect", stub_port, "run", "--no-follow", self.stub_script]
        stub_run_proc = subprocess.run(stub_run_cmd)
        return stub_run_proc.returncode

    def __run_multi_stub_test(self, dut_port: str, stub_port: str) -> int:
        """
        Run multi device tests with a stub device.
        It first runs the stub script on the stub device, then runs the single test on the dut device.
        If there is a post stub delay, it waits for the specified time before running the dut test.
        """
        return_code = self.__run_stub(stub_port)
        if return_code != 0:
            return return_code

        if self.post_stub_delay_ms > 0:
            time.sleep(self.post_stub_delay_ms / 1000.0)

        return self.__run_single_test(dut_port)

    def __run_multi_test(self, dut_a_port: str, dut_b_port: str) -> int:
        """
        Run a multi device test.
        These are special tests that require two devices to run the test.
        And the test scripts are designed to run in that way.
        """

        def get_test_list():
            """
            Expand directories in the test script list to individual test files.
            If a test is a directory, find all .py files in it and add them to the list.
            """
            test_list = []
            for test in self.test_script_list:
                if os.path.isdir(test):
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

        multi_test_proc = subprocess.run(multi_test_cmd)

        return multi_test_proc.returncode

    # TODO: Add vfs mode to avoid repl tests
    # def vfs_mode_test(self, dut_port):
    # https://github.com/mattytrentini/micropython-test-port

    def __custom_test(self, dut_port: str) -> int:
        """
        Run custom test scripts with the given dut_port.
        These tests will be python scripts and there is no define way
        regarding how they interact with the micropython serial device.
        Usually they will use mpremote to interact with the device.
        """
        result = 0
        for test in self.test_script_list:
            custom_test_cmd = ["python", test, dut_port]

            if self.custom_args:
                custom_test_cmd.extend(self.custom_args)

            custom_test_proc = subprocess.run(custom_test_cmd)

            if custom_test_proc.returncode != 0:
                result = 1

        return result

    @staticmethod
    def __set_default_mpy_dir() -> str:
        """
        Set the default MicroPython root directory based on the script location.
        The root dir is two levels up from the script path.
        Returns the absolute path to the MicroPython root directory.
        """
        run_test_plan_script_dir = os.path.abspath(os.path.dirname(__file__))
        return os.path.abspath(os.path.join(run_test_plan_script_dir, "..", ".."))


class TestPlanResults:
    """
    This class will help to keep track of the test during the
    test plan execution.
    The class will keep track of the passed, failed and skipped tests.
    And also it will keep track of the tests that need to be retried.
    """

    @dataclass
    class TestRetries:
        test_name: str = ""
        retries: int = 0

    def __init__(self, max_retries: int):
        """
        Initializes the TestPlanResults instance.
        """
        self.pass_test_name_list = []
        self.skip_test_name_list = []
        self.fail_test_name_list = []
        self.max_retries = max_retries
        self.retry_test_list: list[TestPlanResults.TestRetries] = []

    def register_skip(self, test_name: str) -> None:
        """
        Register a skipped test.
        """
        self.skip_test_name_list.append(test_name)

    def register_fail(self, test_name: str) -> None:
        """
        Register a failed test.
        If the test is not already in the fail list, add it and
        add it to the retry list with the max retries.
        If the test is already in the fail list, decrease the retries count.
        """
        if test_name not in self.fail_test_name_list:
            self.fail_test_name_list.append(test_name)
            test_retry = self.TestRetries(test_name=test_name, retries=self.max_retries)
            self.retry_test_list.append(test_retry)
        else:
            test_retry_index = self.__get_test_retry_index(test_name)
            if test_retry_index is not None:
                self.retry_test_list[test_retry_index].retries -= 1

    def register_pass(self, test_name: str) -> None:
        """
        Register a passed test.
        If the test is in the fail list, remove it from there and
        also remove it from the retry list.
        Then add it to the pass list if not already there.
        """
        if test_name in self.fail_test_name_list:
            self.fail_test_name_list.remove(test_name)
            test_retry_index = self.__get_test_retry_index(test_name)
            if test_retry_index is not None:
                test_retry_obj = self.retry_test_list[test_retry_index]
                self.retry_test_list.remove(test_retry_obj)

        if test_name not in self.pass_test_name_list:
            self.pass_test_name_list.append(test_name)

    def filter_retries(self, test_list: list[TestRunner]) -> list[TestRunner]:
        """
        Given a list of test runners, return a list of test runners
        that need to be retried based on the retry test list.
        """
        retry_test_runner_list = []
        for test in test_list:
            for retry in self.retry_test_list:
                if test.name == retry.test_name and retry.retries > 0:
                    retry_test_runner_list.append(test)

        return retry_test_runner_list

    """
    Private methods
    """

    def __get_test_retry_index(self, test_name) -> int | None:
        """
        Return the index of the test in the retry test list.
        If the test is not found, return None.
        """
        for index, test_retry in enumerate(self.retry_test_list):
            if test_retry.test_name == test_name:
                return index
        return None


class TestPlanLogger:
    """
    This class will handle the logging of the test plan execution.
    It takes cares of the output formatting and coloring.
    """

    blue_on = "\033[94m"
    yellow_on = "\033[33m"
    magenta_on = "\033[35m"
    green_on = "\033[92m"
    red_on = "\033[91m"
    grey_on = "\033[90m"
    color_off = "\033[0m"

    decorator_line_len = 41

    def test_plan_info(
        self, test_plan_file: str, hil_devs_file: str = None, board: str = None
    ) -> None:
        print(
            f"{TestPlanLogger.blue_on}"
            + "#" * TestPlanLogger.decorator_line_len
            + f"{TestPlanLogger.color_off}"
        )
        if board:
            print(f"{TestPlanLogger.blue_on}> board        : {board}{TestPlanLogger.color_off}")
        print(f"test plan file : {os.path.relpath(test_plan_file)}")
        if hil_devs_file:
            print(f"hil devs file  : {os.path.relpath(hil_devs_file)}")
        print(
            f"{TestPlanLogger.blue_on}"
            + "#" * TestPlanLogger.decorator_line_len
            + f"{TestPlanLogger.color_off}"
        )

    def test_info(self, test_name: str, dut_port: str, stub_port: str = None) -> None:
        print("-" * TestPlanLogger.decorator_line_len)
        print(f"{TestPlanLogger.blue_on}> running test : {test_name}{TestPlanLogger.color_off}")
        print(f"dut port       : {dut_port}")
        if stub_port:
            print(f"stub port      : {stub_port}")
        print("- " * (int(TestPlanLogger.decorator_line_len / 2)) + "-")

    def test_info_footer(self):
        print("-" * TestPlanLogger.decorator_line_len)

    def test_fail_info(self, test_name: str) -> None:
        print("- " * (int(TestPlanLogger.decorator_line_len / 2)) + "-")
        print(f"{TestPlanLogger.red_on}> failed test  : {test_name} {TestPlanLogger.color_off}")

    def test_pass_info(self, test_name: str) -> None:
        print("- " * (int(TestPlanLogger.decorator_line_len / 2)) + "-")
        print(f"{TestPlanLogger.green_on}> passed test  : {test_name}{TestPlanLogger.color_off}")

    def test_skip_info(self, test_name: str) -> None:
        print("-" * TestPlanLogger.decorator_line_len)
        print(f"{TestPlanLogger.yellow_on}> skipped test : {test_name}{TestPlanLogger.color_off}")

    def test_retries_info(self, test_retry_list: list[TestRunner]) -> None:
        if test_retry_list:
            print("#" * TestPlanLogger.decorator_line_len)
            print(f"{TestPlanLogger.yellow_on}> retry tests  : ", end="")
            for test_retry in test_retry_list:
                print(f"{test_retry.name} ", end="")
            print(f"{TestPlanLogger.color_off}")
            print("#" * TestPlanLogger.decorator_line_len)

    def test_summary_info(
        self,
        pass_test_name_list: list[str],
        fail_test_name_list: list[str],
        skip_test_name_list: list[str],
    ) -> None:
        print(
            f"{TestPlanLogger.blue_on}"
            + "#" * TestPlanLogger.decorator_line_len
            + f"{TestPlanLogger.color_off}"
        )
        print("> test summary : ", end="")

        fail_test_num = len(fail_test_name_list)
        pass_test_num = len(pass_test_name_list)
        skip_test_num = len(skip_test_name_list)
        total_test_num = fail_test_num + pass_test_num + skip_test_num

        if fail_test_num == 0 and skip_test_num == 0:
            print(
                f"all {self.green_on}{pass_test_num}{self.color_off} tests {self.green_on}passed{self.color_off}"
            )
        else:
            if pass_test_num > 0:
                print(
                    f"only {self.green_on}{pass_test_num}{self.color_off} out of {self.blue_on}{total_test_num}{self.color_off} test passed"
                )
            elif skip_test_num == 0:
                print(
                    f"all {self.red_on}{fail_test_num}{self.color_off} tests {self.red_on}failed{self.color_off}"
                )
            else:
                print("")  # Just a new line

            if pass_test_num > 0:
                print(f"{self.green_on} - passed      : ", end="")
                for test_name in pass_test_name_list:
                    print(f"{test_name} ", end="")
                print(self.color_off)

            if skip_test_num > 0:
                print(f"{self.yellow_on} - skipped     : ", end="")
                for test_name in skip_test_name_list:
                    print(f"{test_name} ", end="")
                print(self.color_off)

            if fail_test_num > 0:
                print(f"{self.red_on} - failed      : ", end="")
                for test_name in fail_test_name_list:
                    print(f"{test_name} ", end="")
                print(self.color_off)

        print(
            f"{TestPlanLogger.blue_on}"
            + "#" * TestPlanLogger.decorator_line_len
            + f"{TestPlanLogger.color_off}"
        )


class TestPlanRunner(ABC):
    """
    This class takes care of running a test plan.
    It loads the test plan from a YAML file and runs the tests
    using the appropriate test device ports.
    The test device ports are obtained from the derived classes.
    """

    def __init__(self, test_plan_file: str) -> None:
        """
        Initializes the TestPlanRunner instance.
        """
        self.test_plan_file = test_plan_file
        self.logger = TestPlanLogger()

    def run(self, test_name_list: list[str] = [], max_retries: int = 0) -> int:
        """
        Run the test plan with the given test names and max retries.
        If no test names are provided, all tests in the test plan are run.
        The test results are logged and a summary is printed at the end.
        If there are failed tests after all retries, the script exits with code 1.
        """
        test_list = self.__get_test_list(test_name_list)
        test_results = TestPlanResults(max_retries)
        pending_retries = True

        while pending_retries:
            for test in test_list:
                dut_port, stub_port = self.get_test_device_ports(test)

                if not test.are_supported_devs_available(dut_port, stub_port):
                    test_results.register_skip(test.name)
                    self.logger.test_skip_info(test.name)
                    continue

                # TODO: Add device.switch management.

                self.logger.test_info(test.name, dut_port, stub_port)
                ret_code = test.run(dut_port, stub_port)

                if ret_code != 0:
                    test_results.register_fail(test.name)
                    self.logger.test_fail_info(test.name)
                else:
                    test_results.register_pass(test.name)
                    self.logger.test_pass_info(test.name)

            self.logger.test_info_footer()

            test_list = test_results.filter_retries(test_list)
            if test_list:
                pending_retries = True
                self.logger.test_retries_info(test_list)
            else:
                pending_retries = False

        self.logger.test_summary_info(
            test_results.pass_test_name_list,
            test_results.fail_test_name_list,
            test_results.skip_test_name_list,
        )

        if test_results.fail_test_name_list:
            sys.exit(1)

    """
    Private methods
    """

    def __get_test_list(self, test_name_list: list[str] = []):
        """
        Get the list of tests to run from the test plan file.
        If no test names are provided, all tests in the test plan are returned.
        Otherwise, only the tests with the given names are returned.
        """
        test_plan_list = TestRunner.load_list_from_yaml(self.test_plan_file)

        if test_name_list == []:
            return test_plan_list

        test_list = []
        for test_name in test_name_list:
            for test in test_plan_list:
                if test.name == test_name:
                    test_list.append(test)

        return test_list

    @abstractmethod
    def get_test_device_ports(self, test: TestRunner) -> tuple[str, str]:
        """
        Abstract method to get the test device ports for the given test.
        This method must be implemented by the derived classes.
        It should return a tuple of (dut_port, stub_port).
        """
        return None, None


class TestPlanRunnerHIL(TestPlanRunner):
    """
    This class takes care of running a test plan using HIL devices.
    """

    def __init__(self, test_plan_file: str, hil_devs_file: str, board: str = None):
        """
        Initializes the TestPlanRunnerHIL instance.
        """
        super().__init__(test_plan_file)
        self.hil_devs_file = hil_devs_file
        self.board = board

    def set_board(self, board: str) -> None:
        """
        Set the test board name.
        This can be used to change the board after the instance is created.
        """
        self.board = board

    def run(self, test_name_list: list[str] = [], max_retries: int = 0) -> int:
        """
        Run the test plan with the given test names and max retries.
        It logs the test plan information before running the tests."""
        self.logger.test_plan_info(self.test_plan_file, self.hil_devs_file, self.board)
        return super().run(test_name_list, max_retries)

    """ 
    Private methods
    """

    def get_test_device_ports(self, test: TestRunner) -> tuple[str, str]:
        """
        Get the test device ports for the given test.
        It uses the HIL devices file and the board name to find the appropriate ports.
        Returns a tuple of (dut_port, stub_port).

        If multiple test devices are available for the given role, it takes the first one for DUT
        and any other for the STUB.
        """
        dut_port = None
        stub_port = None

        dut_port_list = self.__get_ports_for_role(test, self.board, TestRunner.DeviceRole.DUT)

        if not dut_port_list:
            return dut_port, stub_port

        dut_port = dut_port_list[0]  # Take the first

        if test.requires_multiple_devs():
            stub_port_list = self.__get_ports_for_role(
                test, self.board, TestRunner.DeviceRole.STUB
            )

            for port in stub_port_list:
                # Take any element from stub_port_list that is not dut_port
                if port != dut_port:
                    stub_port = port
                    break

        return dut_port, stub_port

    def __get_ports_for_role(
        self, test: TestRunner, board: str, device_role: TestRunner.DeviceRole
    ) -> list[str]:
        """
        Get the list of ports for the given device role (dut or stub) and board.
        It uses the HIL devices file to find the available ports.
        """
        supported_dev_list = test.get_supported_dev_list(device_role, board)

        port_list = []
        for device in supported_dev_list:
            available_ports = get_devices_port(
                device.get("board"), self.hil_devs_file, device.get("version", None)
            )
            port_list.extend(available_ports)

        return port_list


class TestPlanRunnerPorts(TestPlanRunner):
    """
    This class takes care of running a test plan using direct device ports.
    """

    def __init__(self, test_plan_file, dut_port: str = None, stub_port: str = None):
        """
        Initializes the TestPlanRunnerPorts instance.
        """
        super().__init__(test_plan_file)
        self.dut_port = dut_port
        self.stub_port = stub_port

    def set_ports(self, dut_port: str, stub_port: str = None) -> None:
        """
        Set the test device ports.
        This can be used to change the ports after the instance is created.
        """
        self.dut_port = dut_port
        self.stub_port = stub_port

    def run(self, test_name_list: list[str] = [], max_retries: int = 0):
        """
        Run the test plan with the given test names and max retries.
        It logs the test plan information before running the tests.
        """
        self.logger.test_plan_info(self.test_plan_file)
        return super().run(test_name_list, max_retries)

    """ 
    Private methods
    """

    def get_test_device_ports(self, test: TestRunner) -> tuple[str, str]:
        """
        Get the test device ports for the given test.
        It returns the ports set in the instance.
        """
        return self.dut_port, self.stub_port


class TestPlanRunnerCLI:
    """
    This class takes care of parsing the command line arguments
    for the test plan runner script.
    """

    def __init__(self):
        """
        Initializes the TestPlanRunnerCLI instance.
        It sets up the argument parser.
        """
        self.parser = argparse.ArgumentParser(description="MicroPython test suites runner.")
        self.parser.add_argument("test_suite", nargs="*", type=str, help="Test suite to run.")
        self.parser.add_argument(
            "--test-plan", type=str, default=None, help="Path to the test plan file."
        )
        self.parser.add_argument(
            "--hil-devs", type=str, default=None, help="Path to the HIL devices file."
        )
        self.parser.add_argument(
            "-b",
            "--board",
            type=str,
            default=None,
            help="Test board name (only used with --hil-devs).",
        )
        self.parser.add_argument(
            "-d",
            "--dut-port",
            type=str,
            default=None,
            help="Device under test port. Default is /dev/ttyACM0.",
        )
        self.parser.add_argument(
            "-s",
            "--stub-port",
            type=str,
            default=None,
            help="Stub device port. Default is /dev/ttyACM1.",
        )
        self.parser.add_argument(
            "--max-retries",
            type=int,
            default=0,
            help="Maximum number of retries for failed tests.",
        )
        self.parser.add_argument(
            "--mpy-root-dir",
            type=str,
            default=None,
            help="Path to the root of the MicroPython repository. Default is two levels up from this script.",
        )

    def parse(self) -> argparse.Namespace:
        """
        Parse the command line arguments and validate them.
        It also sets the default values for the arguments if not provided.
        """
        args = self.parser.parse_args()
        args = self.__set_validate_args(args)
        return args

    """
    Private methods
    """

    def __set_validate_args(self, args: argparse.Namespace) -> argparse.Namespace:
        """
        Validate the command line arguments and set default values if not provided.
        If hil devices file is provided, the board is required, and the
        direct port arguments are not supported.

        If hil devices file is not provided, the board argument is not relevant, and
        therefore not supported.
        In that case, the direct port arguments are used. If not provided, default values are set.

        It also consider the "test-plan.yml" as the default test plan file if not specified.
        And it sets the default MicroPython root directory if not provided. Its value is
        two levels up from this script location.
        """
        if args.hil_devs:
            args.hil_devs = os.path.abspath(args.hil_devs)
            if args.board is None:
                self.parser.error("--board is required when --hil-devs is provided")

            if args.dut_port or args.stub_port:
                self.parser.error(
                    "--dut-port and --stub-port are not supported when --hil-devs is provided"
                )
        else:
            if args.board is not None:
                self.parser.error("--hil-devs is required when --board is provided")

            # If the ports are not provide, the default values are set.
            if args.dut_port is None:
                args.dut_port = "/dev/ttyACM0"

            if args.stub_port is None:
                args.stub_port = "/dev/ttyACM1"

        if args.test_plan is None:
            args.test_plan = os.path.abspath(
                os.path.join(os.path.abspath(os.path.dirname(__file__)), "test-plan.yml")
            )
        else:
            args.test_plan = os.path.abspath(args.test_plan)

        if args.mpy_root_dir is None:
            run_test_plan_script_dir = os.path.abspath(os.path.dirname(__file__))
            args.mpy_root_path = os.path.abspath(
                os.path.join(run_test_plan_script_dir, "..", "..")
            )

        return args


def main_run_test_plan():
    """
    Parses the cli arguments, creates the appropriate TestPlanRunner instance
    and runs the test plan.

    If --hil-devs is provided, it uses TestPlanRunnerHIL, otherwise it uses
    TestPlanRunnerPorts. The parser also validates the arguments accordingly.
    """
    test_plan_runner_cli = TestPlanRunnerCLI()
    tpr_args = test_plan_runner_cli.parse()

    # HIL device file based mode
    if tpr_args.hil_devs:
        test_plan_runner = TestPlanRunnerHIL(tpr_args.test_plan, tpr_args.hil_devs, tpr_args.board)
    # Direct port passing mode
    elif tpr_args.dut_port:
        test_plan_runner = TestPlanRunnerPorts(
            tpr_args.test_plan, tpr_args.dut_port, tpr_args.stub_port
        )

    test_plan_runner.run(tpr_args.test_suite, tpr_args.max_retries)


if __name__ == "__main__":
    main_run_test_plan()
