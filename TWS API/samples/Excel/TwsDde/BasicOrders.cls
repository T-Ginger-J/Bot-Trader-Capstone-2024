VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "BasicOrders"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = True
Option Explicit

' constants
Const STR_SHEET_NAME = "BasicOrders"
Const STR_PLACEORDER = "placeOrder"
Const STR_CLEARORDER = "clearOrder"
Const STR_CANCELORDER = "cancelOrder"
Const STR_ORDERSTATUS = "orderStatus"
Const STR_GLOBAL_CANCEL = "globalCancel"
Const STR_WHAT_IF = "whatIf"
Const STR_WHAT_IF_REQUEST = "whatIfRequest"

' cells
Const CELL_SERVER_NAME = "B5" ' cell with server name
Const CELL_GLOBAL_CANCEL_EXT_OPERATOR = "M5" ' cell with global cancel ext operator value
Const CELL_GLOBAL_CANCEL_MANUAL_ORDER_INDICATOR = "N5" ' cell with global cancel manual order indicator

' columns
Const startOfContractColumns = 1 ' contract first column index (symbol)
Const orderBaseColumnsStart = 17 ' index of first column in Order Description
Const orderBaseColumnsEnd = 21 ' index of last column in Order Description
Const errorColumnIndex = 22 ' index of error column
Const idColumnIndex = 23 ' index of id column
Const idCommissionIndex = 34 ' index of commission column
Const idManualOrderCancelTime = 160 ' index of manual order cancel time column
Const idExtOperator = 145 ' index of ext operator column
Const idManualOrderIndicator = 169 ' index of manual order indicator column
Const orderExtColumnsStart = 46 ' index of first column in Extended Order Attributes
Const orderExtColumnsEnd = 169 ' index of last column in Extended Order Attributes

' rows
Const dataStartRowIndex = 10 ' starting row of data

' variables
Dim orderStatusColumnsArray() As Variant
Dim contractColumnsArray() As Variant
Dim whatIfColumnsArray() As Variant

' ========================================================
' contract columns
' ========================================================
Private Function getContractColumns() As Variant()

    If (Not Not contractColumnsArray) <> 0 Then
        ' do not re-initialize array
        GoTo getContractColumnsEnd
    End If

    contractColumnsArray = Array("SYMBOL", "SECTYPE", "LASTTRADEDATE", "STRIKE", "RIGHT", "MULTIPLIER", "TRADINGCLASS", "EXCH", "PRIMEXCH", "CURRENCY", "LOCALSYMBOL", "CONID", _
        "COMBOLEGS", "DELTANEUTRAL", "SECIDTYPE", "SECID")
    
getContractColumnsEnd:
    getContractColumns = contractColumnsArray
End Function

' ========================================================
' order status columns
' ========================================================
Private Function getOrderStatusColumns() As Variant()
    Dim size As Integer

    If (Not Not orderStatusColumnsArray) <> 0 Then
        ' do not re-initialize array
        GoTo getOrderStatusColumnsEnd
    End If
    
    orderStatusColumnsArray = Array("status", "filled", "remaining", "price", "lastFillPrice", "whyHeld", "mktCapPrice", "parentId", "clientId", "permId")

getOrderStatusColumnsEnd:
    getOrderStatusColumns = orderStatusColumnsArray
End Function

' ========================================================
' what-if columns
' ========================================================
Private Function getWhatIfColumns() As Variant()
    Dim size As Integer

    If (Not Not whatIfColumnsArray) <> 0 Then
        ' do not re-initialize array
        GoTo getWhatIfColumnsEnd
    End If
    
    whatIfColumnsArray = Array("commission", "commissionCurrency", "currentEquityWithLoan", "currentInitMargin", "currentMaintMargin", _
        "changeEquityWithLoan", "changeInitMargin", "changeMaintMargin", "postEquityWithLoan", "postInitMargin", "postMaintMargin", "bondAccruedInterest")

getWhatIfColumnsEnd:
    getWhatIfColumns = whatIfColumnsArray
End Function

' ========================================================
' copies extended order attributes to basic orders page
' ========================================================
Sub applyTemplate()
    Call OrderFunctions.applyTemplate(Selection.rows, orderExtColumnsStart)
End Sub

' ========================================================
' clear order for active row
' ========================================================
Sub clearOrder()
    Dim server As String, id As String, i As Integer, row As Range, lastRowIndex As Integer
    
    server = util.getServerVal(STR_SHEET_NAME, CELL_SERVER_NAME)
    If server = util.STR_EMPTY Then Exit Sub

    With Worksheets(STR_SHEET_NAME)
        For Each row In Selection.rows
            lastRowIndex = row.row
            If .Cells(row.row, idColumnIndex).value = STR_EMPTY Then GoTo Continue
            If Not util.hasContractData(Worksheets(STR_SHEET_NAME), dataStartRowIndex, row, startOfContractColumns, getContractColumns()) Then GoTo Continue
            
            id = .Cells(row.row, idColumnIndex).value
            
            clearOrderStatusColumns row
            clearWhatIfColumns row
            
            util.sendRequest server, STR_CLEARORDER, id
            
Continue:
        Next row
    
        .Cells(lastRowIndex, 1).offset(1, 0).Activate
    End With
End Sub

' ========================================================
' clears order status columns
' ========================================================
Sub clearOrderStatusColumns(cell As Range)
    Dim i As Integer
    With Worksheets(STR_SHEET_NAME)
        .Cells(cell.row, idColumnIndex).value = util.STR_EMPTY
        .Cells(cell.row, errorColumnIndex).value = util.STR_EMPTY
        
        ' order status columns
        orderStatusColumnsArray = getOrderStatusColumns()
        For i = 0 To UBound(orderStatusColumnsArray) - LBound(orderStatusColumnsArray)
            .Cells(cell.row, idColumnIndex + 1 + i).ClearContents
        Next i
    End With
End Sub

' ========================================================
' clears what-if columns
' ========================================================
Sub clearWhatIfColumns(cell As Range)
    Dim i As Integer
    With Worksheets(STR_SHEET_NAME)
        ' what-if columns
        whatIfColumnsArray = getWhatIfColumns()
        For i = 0 To UBound(whatIfColumnsArray) - LBound(whatIfColumnsArray)
            .Cells(cell.row, idCommissionIndex + i).ClearContents
        Next i
    End With
End Sub

' ========================================================
' global cancel
' ========================================================
Sub globalCancel()
    Dim server As String
    Dim extOperator As String, manualOrderIndicator As String
    
    server = util.getServerVal(STR_SHEET_NAME, CELL_SERVER_NAME)
    If server = util.STR_EMPTY Then Exit Sub
    
    With Worksheets(STR_SHEET_NAME)
        extOperator = .Range(CELL_GLOBAL_CANCEL_EXT_OPERATOR).value
        manualOrderIndicator = .Range(CELL_GLOBAL_CANCEL_MANUAL_ORDER_INDICATOR).value
    End With
    
    util.sendRequest server, STR_GLOBAL_CANCEL, IDENTIFIER_ZERO & util.QMARK & STR_EMPTY & util.UNDERSCORE & extOperator & util.UNDERSCORE & manualOrderIndicator
End Sub

' ========================================================
' cancel order for active row
' ========================================================
Sub cancelOrder()
    Dim server As String, id As String, manualOrderCancelTime As String, i As Integer, row As Range, lastRowIndex As Integer
    Dim extOperator As String, manualOrderIndicator As String
    
    server = util.getServerVal(STR_SHEET_NAME, CELL_SERVER_NAME)
    If server = util.STR_EMPTY Then Exit Sub

    With Worksheets(STR_SHEET_NAME)
        For Each row In Selection.rows
            lastRowIndex = row.row
            If .Cells(row.row, idColumnIndex).value = STR_EMPTY Then GoTo Continue
            If Not util.hasContractData(Worksheets(STR_SHEET_NAME), dataStartRowIndex, row, startOfContractColumns, getContractColumns()) Then GoTo Continue
            
            id = .Cells(row.row, idColumnIndex).value
            manualOrderCancelTime = .Cells(row.row, idManualOrderCancelTime).value
            extOperator = .Cells(row.row, idExtOperator).value
            manualOrderIndicator = .Cells(row.row, idManualOrderIndicator).value
            
            util.sendRequest server, STR_CANCELORDER, id & util.QMARK & manualOrderCancelTime & util.UNDERSCORE & extOperator & util.UNDERSCORE & manualOrderIndicator
            
Continue:
        Next row
        
        .Cells(lastRowIndex, 1).offset(1, 0).Activate
        
    End With
    
End Sub
' ========================================================
' place order for active row
' ========================================================
Sub placeOrder()
    Dim row As Range, server As String, lastRowIndex As Integer
    
    server = util.getServerVal(STR_SHEET_NAME, CELL_SERVER_NAME)
    If server = util.STR_EMPTY Then Exit Sub

    For Each row In Selection.rows
        lastRowIndex = row.row
        If Not util.hasContractData(Worksheets(STR_SHEET_NAME), dataStartRowIndex, row, startOfContractColumns, getContractColumns()) Then GoTo Continue
        sendPlaceOrder server, row
        
Continue:
    Next row
    
    Worksheets(STR_SHEET_NAME).Cells(lastRowIndex, 1).offset(1, 0).Activate

End Sub

' ========================================================
' sends place order for row
' ========================================================
Sub sendPlaceOrder(server As String, cell As Range)

    ' get id
    Dim id As String
    Dim placeOrderStr As String
    Dim requestStr As String
    Dim i As Integer
    Dim columnsArray() As Variant
    Dim columnIndex As Integer

    With Worksheets(STR_SHEET_NAME)
        id = .Cells(cell.row, errorColumnIndex + 1).value
        If id = util.STR_EMPTY Then ' none exists yet -- is original placement as opposed to modify
            id = OrderFunctions.makeId(server)
            If id = STR_EMPTY Then
                Exit Sub
            End If
            ' fill "id" column with id
            .Cells(cell.row, errorColumnIndex + 1).value = id
        End If
        
        ' what-if
        If cbWhatIf.value = True Then
            placeOrderStr = STR_WHAT_IF
            requestStr = STR_WHAT_IF_REQUEST
            columnsArray = getWhatIfColumns()
            columnIndex = idCommissionIndex
        Else
            placeOrderStr = STR_PLACEORDER
            requestStr = STR_ORDERSTATUS
            columnsArray = getOrderStatusColumns()
            columnIndex = idColumnIndex + 1
        End If
        
        ' fill "error" column with formula
        .Cells(cell.row, errorColumnIndex).Formula = util.composeLink(server, requestStr, id, util.STR_ERROR)
        If util.cleanOnError(.Cells(cell.row, errorColumnIndex)) Then
            clearOrderStatusColumns cell
            clearWhatIfColumns cell
            Exit Sub
        End If
        
        ' send request
        util.sendPoke Worksheets(STR_SHEET_NAME), server, placeOrderStr, id, cell, startOfContractColumns, getContractColumns(), 0, idColumnIndex, orderBaseColumnsStart, orderBaseColumnsEnd, orderExtColumnsStart, orderExtColumnsEnd
        
        ' fill columns with formulas
        For i = 0 To UBound(columnsArray) - LBound(columnsArray)
            .Cells(cell.row, columnIndex + i).Formula = util.composeLink(server, requestStr, id, columnsArray(i))
        Next i
    End With

End Sub
