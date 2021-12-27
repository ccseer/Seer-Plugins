# coding=utf-8

import pyexcel as p
import sys


def convert(path_input, path_output):
    book = p.get_book(file_name=path_input)
    book.save_as(path_output, readOnly=True)


def usage(execname):
    print()
    print("excel2html -- part of the Seer app: http://1218.io")
    print("----------------------------------------------------------")
    print("Usage: " + execname + " input output")
    print()
    sys.exit()


if __name__ == "__main__":
    print(sys.version)
    if len(sys.argv) != 3:
        usage(sys.argv[0])

    path_input = sys.argv[1]
    path_output = sys.argv[2].lower()

    if path_output.endswith(".handsontable.html") is False:
        path_output = sys.argv[2] + ".handsontable.html"

    rc = convert(path_input, path_output)

    sys.exit(rc)
