# coding=utf-8


def main(path_in, path_out):
    from pygments import highlight
    from pygments.formatters import HtmlFormatter
    from pygments.lexers import TexLexer
    with open(path_in, 'r') as f:
        code = f.read()
        with open(path_out, 'w') as f_out:
            f_out.write(highlight(code, TexLexer(), HtmlFormatter(full=True, style='friendly')))


def usage(execname):
    print()
    print("LaTex2HTML -- part of the Seer app: http://1218.io")
    print("----------------------------------------------------------")
    print("Usage: " + execname + " input output")
    print()
    sys.exit()


if __name__ == '__main__':
    import sys

    if len(sys.argv) != 3:
        usage(sys.argv[0])

    path_input = sys.argv[1]
    path_output = sys.argv[2].lower()

    if path_output.endswith('.html') is False:
        path_output = sys.argv[2] + '.html'

    main(path_input, path_output)

    sys.exit(0)
