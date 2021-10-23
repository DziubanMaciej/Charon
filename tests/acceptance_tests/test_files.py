import hashlib
import os
import pathlib
import shutil

from config import charonTestsConfig


def remove_test_directory():
    shutil.rmtree(charonTestsConfig.test_directory, True)


def create_test_directory():
    pathlib.Path(charonTestsConfig.test_directory).mkdir(parents=True, exist_ok=True)


def create_directory(path):
    get_full_path(path).mkdir(parents=True, exist_ok=True)


def get_full_path(path):
    return pathlib.Path(charonTestsConfig.test_directory, path)


def get_full_path_str(path):
    return str(pathlib.Path(charonTestsConfig.test_directory, path))


def create_file(path, contents):
    full_path = get_full_path(path)
    full_path.parent.mkdir(parents=True, exist_ok=True)
    with open(full_path, 'w') as file:
        file.write(contents)


def validate_file(path, contents):
    full_path = get_full_path(path)
    try:
        with open(full_path, 'r') as file:
            data = file.readlines()
            data = ''.join(data)
        return contents == data
    except FileNotFoundError:
        pass
    return False


def is_dir_with_n_files(path, n):
    full_path = get_full_path(path)
    if not full_path.is_dir():
        return False
    count = len(os.listdir(full_path))
    return count == n


def is_empty_dir(path):
    return is_dir_with_n_files(path, 0)


def generate_content_for_file(seed):
    def int_to_hash(arg):
        string = str(arg)
        result = hashlib.md5(string.encode())
        return result.hexdigest()

    line_count = 4
    lines = [int_to_hash(seed + i) for i in range(line_count)]
    return '\n'.join(lines)


def generate_name_for_file(seed):
    def int_to_hash(arg):
        string = str(arg)
        result = hashlib.md5(string.encode())
        return result.hexdigest()

    return int_to_hash(seed)
