__author__ = 'corey'

# contributors:
# ustcltx -- #https://github.com/ustcltx

# This code is only for demonstration currently,
# please use OIT plugin for practice.


from win32com.client import DispatchEx

w = ''


def doc2pdf(input, output):
    global w
    w = DispatchEx("Word.Application")
    w.Visible = False
    w.DisplayAlerts = False
    doc = w.Documents.Open(input, ReadOnly=1)
    doc.ExportAsFixedFormat(output, 17, False, 1)


def excel2pdf(input, output):
    global w
    w = DispatchEx("Excel.Application")
    w.Visible = False
    w.DisplayAlerts = False
    doc = w.Workbooks.Open(input, ReadOnly=1)
    doc.ExportAsFixedFormat(0, output)


def ppt2pdf(input, output):
    global w
    w = DispatchEx("PowerPoint.Application")
    w.DisplayAlerts = False
    doc = w.Presentations.Open(input, False, False, False)
    doc.ExportAsFixedFormat(output, 2, 1, PrintRange=None)


def vsd2pdf(input, output):
    global w
    w = DispatchEx("Visio.Application")
    w.Visible = False
    doc = w.Documents.Open(input)
    doc.ExportAsFixedFormat(1, output, 1, 0)


def DispatchFun(path, out):
    p = path.lower()
    if p.endswith('doc') or p.endswith('docx') or p.endswith('rtf'):
        doc2pdf(path, out)
    elif p.endswith('xls') or p.endswith('xlsx'):
        excel2pdf(path, out)
    elif p.endswith('ppt') or p.endswith('pptx'):
        ppt2pdf(path, out)
    elif p.endswith('vsd') or p.endswith('vsdx'):
        vsd2pdf(path, out)
    else:
        print('Unknown format.')


def main(path_in, path_out):
    import os

    if not os.path.exists(path_in):
        print(path_in + ' not found.')
        return

    if not os.path.isabs(path_in):
        path_in = os.path.abspath(path_in)
    if not os.path.isabs(path_out):
        path_out = os.path.abspath(path_out)

    DispatchFun(path_in, path_out)


def usage(execname):
    print()
    print("MS_Office2PDF -- part of the Seer app: http://1218.io")
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

    if path_output.endswith('.pdf') is False:
        path_output = sys.argv[2] + '.pdf'

    rc = main(path_input, path_output)

    w.Quit()
    sys.exit(0)
