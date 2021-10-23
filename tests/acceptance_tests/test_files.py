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


def get_file_content(path):
    full_path = get_full_path(path)
    try:
        with open(full_path, 'r') as file:
            data = file.readlines()
            data = ''.join(data)
            return data
    except FileNotFoundError:
        pass
    return None


def validate_file(path, contents):
    actual_contents = get_file_content(path)
    return contents == actual_contents


def get_files_in_dir(path):
    full_path = get_full_path(path)
    if not full_path.is_dir():
        return None
    files = os.listdir(full_path)
    files = [pathlib.Path(path, file) for file in files]
    return files


def is_dir_with_n_files(path, n):
    files = get_files_in_dir(path)
    if files is None:
        return False
    else:
        return n == len(files)


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


def are_files_equal(path1, path2):
    if pathlib.Path(path1).name != pathlib.Path(path2).name:
        return False

    content1 = get_file_content(path1)
    content2 = get_file_content(path2)
    return content1 == content2 and content1 is not None
