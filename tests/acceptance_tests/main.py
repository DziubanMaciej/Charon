import json
import os
import subprocess
import sys
import tempfile
import time
import unittest

import test_files
from config import charonTestsConfig


class CharonAcceptanceTest(unittest.TestCase):
    def setUp(self):
        test_files.remove_test_directory()
        test_files.create_test_directory()

    def tearDown(self):
        test_files.remove_test_directory()

    @staticmethod
    def _write_to_tmp_file(content):
        fd, path = tempfile.mkstemp()
        with os.fdopen(fd, 'w') as file:
            file.write(content)
        return path

    def enable_charon(self, charon_config):
        charon_config = json.dumps(charon_config)
        test_files.create_file("charon_config.json", charon_config)

        args = [
            charonTestsConfig.charon_exe,
            '--config',
            test_files.get_full_path_str("charon_config.json"),
            '--log',
            test_files.get_full_path_str("charon_log.txt"),
            '--verbose',
        ]
        self.charon_process = subprocess.Popen(args,
                                               stdin=subprocess.PIPE,
                                               stdout=subprocess.PIPE,
                                               stderr=subprocess.DEVNULL)
        self._wait_for_charon_output_line("Charon started", 1)

    def disable_charon(self, operations_count):
        self._wait_for_charon_output_line("Operation succeeded", operations_count)
        try:
            self.charon_process.stdin.write(b'q')
            # noinspection PyUnusedLocal
            (stdout, stderr) = self.charon_process.communicate(timeout=charonTestsConfig.terminate_timeout)
            return_value = self.charon_process.wait(timeout=charonTestsConfig.terminate_timeout)
            assert return_value == 0
        except subprocess.TimeoutExpired as e:
            raise AssertionError(e)
        finally:
            self.charon_process.terminate()
            self.charon_process = None

    def _wait_for_charon_output_line(self, required_line, required_count):
        if required_count == 0:
            return

        # TODO: use timeout 'charonTestsConfig.terminate_timeout'
        current_count = 0
        while True:
            out = self.charon_process.stdout.readline().decode()
            if required_line in out:
                current_count += 1
                if current_count == required_count:
                    break


class SimpleTestCase(CharonAcceptanceTest):
    def test_do_nothing(self):
        config = []
        self.enable_charon(config)
        self.disable_charon(0)

    def test_copy_one_file(self):
        # Prepare test data
        src_dir = "Src"
        dst_dir = "Dst"
        filename = f'myFile'
        contents = "a\nb\nc"
        test_files.create_directory(src_dir)

        # Run charon
        config = [
            {
                "watchedFolder": test_files.get_full_path_str(src_dir),
                "actions": [
                    {
                        "type": "copy",
                        "destinationDir": test_files.get_full_path_str(dst_dir),
                        "destinationName": "${name}"
                    }
                ]
            }
        ]
        self.enable_charon(config)

        # Perform file operations
        test_files.create_file(f'{src_dir}/{filename}', contents)

        # Check results
        self.disable_charon(1)
        assert test_files.validate_file(f'{dst_dir}/{filename}', contents)
        assert test_files.validate_file(f'{src_dir}/{filename}', contents)
        assert test_files.is_dir_with_n_files(src_dir, 1)
        assert test_files.is_dir_with_n_files(dst_dir, 1)

    def test_move_one_file(self):
        # Prepare test data
        src_dir = "Src"
        dst_dir = "Dst"
        filename = f'myFile'
        contents = "a\nb\nc"
        test_files.create_directory(src_dir)

        # Run charon
        config = [
            {
                "watchedFolder": test_files.get_full_path_str(src_dir),
                "actions": [
                    {
                        "type": "move",
                        "destinationDir": test_files.get_full_path_str(dst_dir),
                        "destinationName": "${name}"
                    }
                ]
            }
        ]
        self.enable_charon(config)

        # Perform file operations
        test_files.create_file(f'{src_dir}/{filename}', contents)

        # Check results
        self.disable_charon(1)
        assert test_files.validate_file(f'{dst_dir}/{filename}', contents)
        assert test_files.is_dir_with_n_files(src_dir, 0)
        assert test_files.is_dir_with_n_files(dst_dir, 1)

    def test_multiple_files_and_actions(self):
        # Prepare test data
        src_dir = "Src"
        dst_dir = "Dst"
        file_count = 20
        files_for_copy = [(f'file{i}.forCopy', test_files.generate_content_for_file(i)) for i in range(file_count)]
        files_for_move = [(f'file{i}.forMove', test_files.generate_content_for_file(i)) for i in range(file_count)]
        files_for_remove = [(f'file{i}.forRemove', test_files.generate_content_for_file(i)) for i in range(file_count)]
        test_files.create_directory(src_dir)

        # Run charon
        config = [
            {
                "watchedFolder": test_files.get_full_path_str(src_dir),
                "extensions": ["forCopy"],
                "actions": [
                    {
                        "type": "copy",
                        "destinationDir": test_files.get_full_path_str(dst_dir),
                        "destinationName": "${name}"
                    }
                ]
            },
            {
                "watchedFolder": test_files.get_full_path_str(src_dir),
                "extensions": ["forMove"],
                "actions": [
                    {
                        "type": "move",
                        "destinationDir": test_files.get_full_path_str(dst_dir),
                        "destinationName": "${name}"
                    }
                ]
            },
            {
                "watchedFolder": test_files.get_full_path_str(src_dir),
                "extensions": ["forRemove"],
                "actions": [
                    {
                        "type": "remove",
                    }
                ]
            }
        ]
        self.enable_charon(config)

        # Perform file operations
        for file in (files_for_move + files_for_copy + files_for_remove):
            test_files.create_file(f'{src_dir}/{file[0]}', file[1])

        # Check results
        self.disable_charon(file_count * 3)
        for file in files_for_copy:
            assert test_files.validate_file(f'{src_dir}/{file[0]}', file[1])
            assert test_files.validate_file(f'{dst_dir}/{file[0]}', file[1])
        for file in files_for_move:
            assert test_files.validate_file(f'{dst_dir}/{file[0]}', file[1])
        assert test_files.is_dir_with_n_files(src_dir, file_count)  # Only files for copy
        assert test_files.is_dir_with_n_files(dst_dir, file_count * 2)  # Files for copy and for move

    def test_multiple_matchers(self):
        # Prepare test data
        src_dir1 = "Src1"
        src_dir2 = "Src2"
        dst_dir = "Dst"
        test_files.create_directory(src_dir1)
        test_files.create_directory(src_dir2)
        file_count = 20
        files1 = [(f'file{i}.from1', test_files.generate_content_for_file(i)) for i in range(file_count)]
        files2 = [(f'file{i}.from2', test_files.generate_content_for_file(i + file_count)) for i in range(file_count)]

        # Run charon
        config = [
            {
                "watchedFolder": test_files.get_full_path_str(src_dir1),
                "actions": [
                    {
                        "type": "move",
                        "destinationDir": test_files.get_full_path_str(dst_dir),
                        "destinationName": "${name}"
                    }
                ]
            },
            {
                "watchedFolder": test_files.get_full_path_str(src_dir2),
                "actions": [
                    {
                        "type": "move",
                        "destinationDir": test_files.get_full_path_str(dst_dir),
                        "destinationName": "${name}"
                    }
                ]
            }
        ]
        self.enable_charon(config)

        # Perform file operations
        for file in files1:
            test_files.create_file(f'{src_dir1}/{file[0]}', file[1])
        for file in files2:
            test_files.create_file(f'{src_dir2}/{file[0]}', file[1])

        # Check results
        self.disable_charon(file_count * 2)
        for file in files1:
            assert test_files.validate_file(f'{dst_dir}/{file[0]}', file[1])
        for file in files2:
            assert test_files.validate_file(f'{dst_dir}/{file[0]}', file[1])
        assert test_files.is_dir_with_n_files(src_dir1, 0)
        assert test_files.is_dir_with_n_files(src_dir2, 0)
        assert test_files.is_dir_with_n_files(dst_dir, file_count * 2)

    def test_name_counters(self):
        # Prepare test data
        src_dir1 = "Src1"
        src_dir2 = "Src2"
        dst_dir = "Dst"
        test_files.create_directory(src_dir1)
        test_files.create_directory(src_dir2)
        file_count = 20
        files1 = [f'{test_files.generate_name_for_file(i)}.png' for i in range(file_count)]
        files2 = [f'{test_files.generate_name_for_file(i + file_count)}.jpg' for i in range(file_count)]

        # Run charon
        config = [
            {
                "watchedFolder": test_files.get_full_path_str(src_dir1),
                "actions": [
                    {
                        "type": "move",
                        "destinationDir": test_files.get_full_path_str(dst_dir),
                        "destinationName": "file_###"
                    }
                ]
            },
            {
                "watchedFolder": test_files.get_full_path_str(src_dir2),
                "actions": [
                    {
                        "type": "move",
                        "destinationDir": test_files.get_full_path_str(dst_dir),
                        "destinationName": "file_###"
                    }
                ]
            }
        ]
        self.enable_charon(config)

        # Perform file operations
        for file in files1:
            test_files.create_file(f'{src_dir1}/{file}', '')
        for file in files2:
            test_files.create_file(f'{src_dir2}/{file}', '')

        # Check results
        self.disable_charon(file_count * 2)
        for i in range(file_count * 2):
            base_name = f'{dst_dir}/file_{i:03d}'
            assert test_files.validate_file(f'{base_name}.jpg', '') or test_files.validate_file(f'{base_name}.png', '')
        assert test_files.is_dir_with_n_files(src_dir1, 0)
        assert test_files.is_dir_with_n_files(src_dir2, 0)
        assert test_files.is_dir_with_n_files(dst_dir, file_count * 2)

    def test_move_and_backup(self):
        # Prepare test data
        src_dir = "Src1"
        dst_dir1 = "Dst1"
        dst_dir2 = "Dst2"
        dst_dir3 = "Dst3"
        test_files.create_directory(src_dir)
        file_count = 20
        files = [(f'{test_files.generate_name_for_file(i)}.csv', test_files.generate_content_for_file(i)) for i in
                 range(file_count)]

        # Run charon
        config = [
            {
                "watchedFolder": test_files.get_full_path_str(src_dir),
                "actions": [
                    {
                        "type": "copy",
                        "destinationDir": test_files.get_full_path_str(dst_dir1),
                        "destinationName": "file_###"
                    },
                    {
                        "type": "copy",
                        "destinationDir": test_files.get_full_path_str(dst_dir2),
                        "destinationName": "${previousName}"
                    },
                    {
                        "type": "move",
                        "destinationDir": test_files.get_full_path_str(dst_dir3),
                        "destinationName": "${previousName}"
                    }
                ]
            }
        ]
        self.enable_charon(config)

        # Create unexpected file in backup directories. It should be overwritten be the application
        test_files.create_file(f'{dst_dir2}/file_003.csv', test_files.generate_content_for_file(999))
        test_files.create_file(f'{dst_dir3}/file_004.csv', test_files.generate_content_for_file(999))

        # Perform file operations
        for file in files:
            test_files.create_file(f'{src_dir}/{file[0]}', file[1])

        # Check results
        self.disable_charon(file_count * 3)
        assert test_files.is_dir_with_n_files(src_dir, 0)
        assert test_files.is_dir_with_n_files(dst_dir1, file_count)
        assert test_files.is_dir_with_n_files(dst_dir2, file_count)
        assert test_files.is_dir_with_n_files(dst_dir3, file_count)
        for file1, file2, file3 in zip(test_files.get_files_in_dir(dst_dir1),
                                       test_files.get_files_in_dir(dst_dir2),
                                       test_files.get_files_in_dir(dst_dir3)):
            assert test_files.are_files_equal(file1, file2)
            assert test_files.are_files_equal(file1, file3)
            assert test_files.are_files_equal(file2, file3)

    def test_name_counters_polish_source_files(self):
        # Prepare test data
        src_dir = "Src"
        dst_dir = "Dst"
        filenames = [ 'file1.ź', 'file2_ą.ź', 'file3_ę.ź', 'file4.ź']
        contents = "a\nb\nc"
        test_files.create_directory(src_dir)

        # Run charon
        config = [
            {
                "watchedFolder": test_files.get_full_path_str(src_dir),
                "actions": [
                    {
                        "type": "move",
                        "destinationDir": test_files.get_full_path_str(dst_dir),
                        "destinationName": "file_###"
                    }
                ]
            }
        ]
        self.enable_charon(config)

        # Perform file operations
        for filename in filenames:
            test_files.create_file(f'{src_dir}/{filename}', contents)

        # Check results
        self.disable_charon(4)
        for i in range(len(filenames)):
            assert test_files.validate_file(f'{dst_dir}/file_{i:03d}.ź', contents)
        assert test_files.is_dir_with_n_files(src_dir, 0)
        assert test_files.is_dir_with_n_files(dst_dir, len(filenames))

if __name__ == "__main__":
    unittest.main(argv=[sys.argv[0], '-v'])  # run all tests
