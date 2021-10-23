import sys


class Config:
    def __init__(self) -> None:
        self.is_valid = True
        try:
            self.test_directory = sys.argv[1]
            self.charon_exe = sys.argv[2]

            self.terminate_timeout = 1.0
            self.wait_for_charon_timeout = 0.1
        except LookupError:
            self.is_valid = False


charonTestsConfig = Config()
