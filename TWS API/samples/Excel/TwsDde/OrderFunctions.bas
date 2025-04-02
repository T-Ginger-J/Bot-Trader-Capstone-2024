Attribute VB_Name = "OrderFunctions"

' constants
Const STR_GET_NEXT_VALID_ID = "getNextValidId"
Public Const EXTENDED_WORKSHEET = "ExtendedOrderAttributes"
Public Const EXTENDED_ATTRIB_COL = "d"
Public Const FIRST_EXTENDED_ROW = 7
Public Const LAST_EXTENDED_ROW = 131
Const DAYS_PRIOR_TO_DDE_API = 43151
Const orderMult = 1000000

Sub applyTemplate(ByRef orderRange As Range, ByVal extendedAttribColumn As Integer)
    Dim orderRowCtr As Integer, extRowCtr As Integer, orderCol As Integer
    Dim orderRow As Range
    Dim orderSheet As Worksheet
    For Each orderRow In orderRange.rows
        orderRowCtr = orderRow.row
        For extRowCtr = OrderFunctions.FIRST_EXTENDED_ROW To OrderFunctions.LAST_EXTENDED_ROW
            orderCol = extendedAttribColumn + extRowCtr - OrderFunctions.FIRST_EXTENDED_ROW
            If Worksheets(EXTENDED_WORKSHEET).Range(EXTENDED_ATTRIB_COL & CStr(extRowCtr)).value <> orderRow.Worksheet.Cells(orderRowCtr, orderCol).value Then
                orderRow.Worksheet.Cells(orderRowCtr, orderCol).value = Worksheets(EXTENDED_WORKSHEET).Range(EXTENDED_ATTRIB_COL & CStr(extRowCtr)).value
            End If
        Next extRowCtr
    Next orderRow
End Sub

Function makeId(server As String) As String
    If server <> STR_EMPTY Then
        Dim result() As Variant
        Dim dimension As Integer
        result = util.sendRequest(server, STR_GET_NEXT_VALID_ID, util.IDENTIFIER_ZERO & util.QMARK)
        dimension = util.getDimension(result)
        If dimension = 1 Then
            Dim i As Integer
            For i = 1 To UBound(result) - LBound(result) + 1
                makeId = util.IDENTIFIER_PREFIX & result(i)
            Next i
        Else
            makeId = STR_EMPTY
        End If
    Else
        makeId = STR_EMPTY
    End If
End Function
