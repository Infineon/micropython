import subprocess
import sys
import os
import logging
import argparse

logger = logging.getLogger("fs")
logging.basicConfig(format="%(levelname)s: %(message)s", encoding="utf-8", level=logging.DEBUG)


test_input_dir = "./ports/psoc6/inputs"
test_script_dir = "./ports/psoc6/mp_custom"
args = ""

mpremote = "./tools/mpremote/mpremote.py"


def set_mpr_run_script(mem_type):
    if mem_type == "sd":
        return f"run {test_script_dir}/fs_mount_sd.py"
    return ""


def set_remote_dir_path(mem_type):
    return "/sd/" if mem_type == "sd" else "/"


def get_test_input_files(test_type):
    cp_input_files = [
        "test_fs_small_file.txt",
        "test_fs_medium_file.txt",
        "test_fs_large_file.txt",
    ]
    input_files_sizes = ["10240", "511876", "1047584"]

    if test_type == "basic":
        return [cp_input_files[0]], [input_files_sizes[0]]
    elif test_type == "adv":
        return cp_input_files, input_files_sizes


def get_test_name(test_type, mem_type):
    mem_type_suffix = "_sd" if mem_type == "sd" else ""
    return f"fs_{test_type}{mem_type_suffix}.py"


def run_subprocess(command):
    try:
        result = subprocess.run(command, shell=True, capture_output=True, check=True)
        return result.stdout.decode().split("\n")
    except subprocess.CalledProcessError as e:
        logger.error(f"Command '{command}' failed with error: {e}")
        sys.exit(1)


def ls_files(files, mpr_connect, mpr_run_script, remote_directory_path):
    mpr_ls = f"{mpr_connect} {mpr_run_script} fs ls {remote_directory_path}"
    output_lines = run_subprocess(mpr_ls)
    files_result = []

    for file in files:
        file_size = -1
        for line in output_lines:
            line = line.split()
            if file in line:
                file_size = line[0]
        files_result.append(file_size)

    return files_result


def rm_files(files, mpr_connect, mpr_run_script, remote_directory_path):
    rm_sub_cmd = " + ".join([f"fs rm {remote_directory_path}{file}" for file in files])
    mpr_rm_cmd = f"{mpr_connect} {mpr_run_script} {rm_sub_cmd}"
    run_subprocess(mpr_rm_cmd)


def rm_files_if_exist(files, mpr_connect, mpr_run_script, remote_directory_path):
    matches = ls_files(files, mpr_connect, mpr_run_script, remote_directory_path)
    existing_files = [files[i] for i in range(len(matches)) if matches[i] != -1]

    if existing_files:
        print(f"Removing existing files...")
        rm_files(existing_files, mpr_connect, mpr_run_script, remote_directory_path)
        matches = ls_files(files, mpr_connect, mpr_run_script, remote_directory_path)
        if matches == [-1 for _ in range(len(files))]:
            print(f"Existing files removed.")


def copy_files(input_cp_files, mpr_connect, remote_directory_path):
    cp_sub_cmd = " + ".join(
        [f"cp {test_input_dir}/{file} :{remote_directory_path}" for file in input_cp_files]
    )
    cp_cmd = f"{mpr_connect} {cp_sub_cmd}"
    run_subprocess(cp_cmd)


def validate_test(files, file_sizes, mpr_connect, mpr_run_script, remote_directory_path):
    found_sizes = ls_files(files, mpr_connect, mpr_run_script, remote_directory_path)
    if found_sizes != file_sizes:
        print(f"\nfail  {get_test_name(test_type, mem_type)}")
        sys.exit(1)
    else:
        print(f"\npass  {get_test_name(test_type, mem_type)}")
        sys.exit(0)


def cp_files_test(
    input_files, input_files_size, mpr_connect, mpr_run_script, remote_directory_path
):
    rm_files_if_exist(input_files, mpr_connect, mpr_run_script, remote_directory_path)
    copy_files(input_files, mpr_connect, remote_directory_path)
    validate_test(
        input_files, input_files_size, mpr_connect, mpr_run_script, remote_directory_path
    )


def large_file_tests(device, test_type, mem_type):
    mpr_connect = f"{mpremote} connect {device}"
    mpr_run_script = set_mpr_run_script(mem_type)
    remote_directory_path = set_remote_dir_path(mem_type)

    input_files, input_files_size = get_test_input_files(test_type)
    cp_files_test(
        input_files, input_files_size, mpr_connect, mpr_run_script, remote_directory_path
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="fs_test.py",
        usage="%(prog)s [options] [command]",
        description="Filesystem test script",
    )
    parser.add_argument(
        '--device', required=False, default="/dev/ttyACM4", help="Device to connect to"
    )
    parser.add_argument(
        '--test_type', choices=['basic', 'adv'], default='basic', help="Type of test to run"
    )
    parser.add_argument(
        '--mem_type', choices=['flash', 'sd'], default='flash', help="Type of memory to test"
    )

    args = parser.parse_args(sys.argv[1:])
    device = args.device
    test_type = args.test_type
    mem_type = args.mem_type

    large_file_tests(device, test_type, mem_type)
