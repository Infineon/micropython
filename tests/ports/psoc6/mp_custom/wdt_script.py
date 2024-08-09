"""import subprocess
import sys
import os
import time

device = sys.argv[1]

remote_directory_path = "/"

wdt = "ports/psoc6/mp_custom/wdt.py"
wdt_reset_check = "ports/psoc6/mp_custom/wdt_reset_check.py"

# out and exp file paths
wdt_op_fp = "./ports/psoc6/mp_custom/wdt.py.out"
wdt_reset_check_op_fp = "./ports/psoc6/mp_custom/wdt_reset_check.py.out"
exp_wdt = "./ports/psoc6/mp_custom/wdt.py.exp"
exp_wdt_reset_check = "./ports/psoc6/mp_custom/wdt_reset_check.py.exp"

# List of mpremote commands
mpr_run_wdt = f"../tools/mpremote/mpremote.py connect {device} run {wdt}"
mpr_run_resume = f"../tools/mpremote/mpremote.py resume exec {wdt_reset_check}"
# mpr_run_resume = f"../tools/mpremote/mpremote.py resume exec \"import machine; print(machine.reset_cause)\""
# mpr_run_resume = f'../tools/mpremote/mpremote.py resume exec "print_state_info()"'


def exec(cmd, output_file):
    try:
        with open(output_file, "w") as f:
            process = subprocess.Popen(cmd, shell=True, stdout=f, stderr=subprocess.PIPE, bufsize=0, universal_newlines=True)
            output, error = process.communicate()
        if process.returncode != 0:
            print(f"Command execution failed with error: {error.decode('utf-8')}")
            sys.exit(1)
        else:
            print(f"Command executed successfully: {cmd}")
    except Exception as e:
        print(f"An error occurred while executing the command: {cmd}\nError: {e}")


def validate_test(op, exp_op):
    with open(op, "r") as output_file:
        output = [line.strip() for line in output_file]

    with open(exp_op, "r") as exp_output_file:
        exp_output = [line.strip() for line in exp_output_file]

    if output != exp_output:
        print("Test Failed!")
        for line in output:
            print(line)
        sys.exit(1)
    else:
        print("Test successful!")


def wdt_test():
    print("Running wdt test")
    exec(mpr_run_wdt, wdt_op_fp)
    validate_test(wdt_op_fp, exp_wdt)
    os.remove(wdt_op_fp)


def wdt_reset_check():
    print("Running wdt reset test")
    exec(mpr_run_resume, wdt_reset_check_op_fp)
    validate_test(wdt_reset_check_op_fp, exp_wdt_reset_check)
    os.remove(wdt_reset_check_op_fp)


wdt_test()
# time.sleep(1)
# wdt_reset_check()"""

import time
import subprocess
import sys
import os


device = sys.argv[1]
wdt = "ports/psoc6/wdt.py"
mpr_cmd = f"../tools/mpremote/mpremote.py connect {device} run {wdt}"
mpr_run_resume = f"../tools/mpremote/mpremote.py resume exec \"import psoc6; print(psoc6.system_reset_cause() if psoc6.system_reset_cause() != 1 else ' ')\""

wdt_op_fp = "./ports/psoc6/test_scripts/wdt.py.out"
wdt_reset_check_op_fp = "./ports/psoc6/test_scripts/wdt_reset_check.py.out"
mpr_connect_cmd_out = "./ports/psoc6/test_scripts/connect.py.out"
exp_wdt = "./ports/psoc6/test_scripts/wdt.py.exp"
exp_wdt_reset_check = "./ports/psoc6/test_scripts/wdt_reset_check.py.exp"


def exec(cmd, output_file):
    try:
        with open(output_file, "w") as f:
            process = subprocess.Popen(
                cmd,
                shell=True,
                stdout=f,
                stderr=subprocess.PIPE,
                bufsize=0,
                universal_newlines=True,
            )
            output, error = process.communicate()
        if process.returncode != 0:
            print(f"Command execution failed with error: {error}")
            sys.exit(1)
        else:
            print(f"Command executed successfully: {cmd}")
    except Exception as e:
        print(f"An error occurred while executing the command: {cmd}\nError: {e}")


def validate_test(op, exp_op):
    with open(op, "r") as output_file:
        output = [line.strip() for line in output_file]

    with open(exp_op, "r") as exp_output_file:
        exp_output = [line.strip() for line in exp_output_file]

    if output != exp_output:
        print("Test Failed!")
        for line in output:
            print(line)
        sys.exit(1)
    else:
        print("Test successful!")


def wdt_test():
    print("Running wdt test")
    exec(mpr_cmd, wdt_op_fp)
    validate_test(wdt_op_fp, exp_wdt)
    os.remove(wdt_op_fp)


def wdt_reset_check():
    print("Running wdt reset test")
    exec(mpr_run_resume, wdt_reset_check_op_fp)
    validate_test(wdt_reset_check_op_fp, exp_wdt_reset_check)
    os.remove(wdt_reset_check_op_fp)


wdt_test()
time.sleep(2)
wdt_reset_check()
