__author__ = 'corey'

from win32com.client import DispatchEx


def doc2pdf(input, output):
    w = DispatchEx("Word.Application")
    w.Visible = False
    try:
        doc = w.Documents.Open(input, ReadOnly=1)
        doc.ExportAsFixedFormat(output, 17, False, 1)
        return 0
    except:
        return 1
    finally:
        w.Quit()


def excel2pdf(input, output):
    w = DispatchEx("Excel.Application")
    w.Visible = False
    try:
        doc = w.Workbooks.Open(input, ReadOnly=1)
        doc.ExportAsFixedFormat(0, output)
        return 0
    except:
        return 1
    finally:
        w.Quit()


def ppt2pdf(input, output):
    w = DispatchEx("PowerPoint.Application")
    try:
        doc = w.Presentations.Open(input, False, False, False)
        doc.ExportAsFixedFormat(output,
                                2,
                                1,
                                PrintRange=None)
        return 0
    except:
        return 1
    finally:
        w.Quit()


def vsd2pdf(input, output):
    w = DispatchEx("Visio.Application")
    w.Visible = False
    try:
        doc = w.Documents.Open(input)
        doc.ExportAsFixedFormat(1, output, 1, 0)
        return 0
    except Exception as inst:
        print(type(inst), inst.args, inst)
        return 1
    finally:
        w.Quit()


def DispatchFun(path, out):
    p = path.lower()
    if p.endswith('doc') or p.endswith('docx') or p.endswith('rtf'):
        return doc2pdf(path, out)
    elif p.endswith('xls') or p.endswith('xlsx'):
        return excel2pdf(path, out)
    elif p.endswith('ppt') or p.endswith('pptx'):
        return ppt2pdf(path, out)
    elif p.endswith('vsd') or p.endswith('vsdx'):
        return vsd2pdf(path, out)
    else:
        print('Unknown format.')
        return -1


def main(path_in, path_out):
    import os

    if not os.path.exists(path_in):
        print(path_in + ' not found.')
        return -1

    if not os.path.isabs(path_in):
        path_in = os.path.abspath(path_in)
    if not os.path.isabs(path_out):
        path_out = os.path.abspath(path_out)
    try:
        rc = DispatchFun(path_in, path_out)
        return rc
    except:
        return -1


def usage(execname):
    print()
    print("Office2PDF -- part of the Seer app: http://sourceforge.net/projects/ccseer")
    print("----------------------------------------------------------")
    print("Usage: " + execname + " input output")
    print()
    sys.exit()


if __name__ == '__main__':
    import sys

    execname = sys.argv[0]

    if len(sys.argv) != 3:
        usage(execname)

    path_input = sys.argv[1]
    path_output = sys.argv[2].lower()

    if path_output.endswith('.pdf') is False:
        path_output = sys.argv[2] + '.pdf'

    rc = main(path_input, path_output)
    if rc:
        sys.exit(rc)
    sys.exit(0)
