Cells.Select
Selection.Copy

Dim wb As Workbook
Set wb = Workbooks.Add(1)

wb.Sheets("����1").Select
wb.Sheets("����1").Name = "����_����������"
wb.Sheets("����_����������").Paste

Dim n As Name
For Each n In ThisWorkbook.Names
wb.Names.Add Name:=n.Name, RefersTo:=Replace(n.RefersTo, ThisWorkbook.Sheets(1).Name, wb.Sheets(1).Name)