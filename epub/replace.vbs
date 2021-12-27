Const ForReading = 1
Const ForWriting = 2

strFileName = Wscript.Arguments(0)
strOldText = Wscript.Arguments(1)
strNewText = Wscript.Arguments(2)

Set objFSO = CreateObject("Scripting.FileSystemObject")
Set objFile = objFSO.OpenTextFile(strFileName, ForReading)

strText = objFile.ReadAll
objFile.Close
strNewText = Replace(strText, strOldText, strNewText)

' Set objFile = objFSO.OpenTextFile(strFileName, ForWriting, true, -2)
' objFile.Write strNewText
' objFile.Close
Set fsT = CreateObject("ADODB.Stream")
fsT.Type = 2
fsT.Charset = "utf-8"
fsT.Open
fsT.WriteText strNewText
fsT.SaveToFile strFileName, 2