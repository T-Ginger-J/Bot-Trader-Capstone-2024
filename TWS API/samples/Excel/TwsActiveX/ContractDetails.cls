VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "ContractDetails"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = True
Option Explicit

'=================
' local constants
'=================

Private Enum QueryColumns
    Col_INCLUDEEXPIRED = 1
    Col_STATUS
End Enum

Private Enum NewPageOptionsColumns
    Col_SHEETNAME = 1
    Col_SHEETACTIVATE
End Enum

Private contractTable As Range
Private queryTable As Range
Private newPageTable As Range

'=================
' methods
'=================
' request contract details
Public Sub RequestContractDetails_Click()
    If Not CheckConnected Then Exit Sub
    
    Dim id As Integer
    
    ' create contract structure
    Dim lContractInfo As TWSLib.IContract
    Set lContractInfo = Api.Tws.createContract()
    
    id = ActiveCell.row - contractTable.Rows(1).row + 1
    With lContractInfo
        .Symbol = UCase(contractTable(id, Col_SYMBOL).value)
        .SecType = UCase(contractTable(id, Col_SECTYPE).value)
        .lastTradeDateOrContractMonth = contractTable(id, Col_LASTTRADEDATE).value
        .Strike = contractTable(id, Col_STRIKE).value
        .Right = UCase(contractTable(id, Col_RIGHT).value)
        .multiplier = UCase(contractTable(id, Col_MULTIPLIER).value)
        .Exchange = UCase(contractTable(id, Col_EXCH).value)
        .primaryExchange = UCase(contractTable(id, Col_PRIMEXCH).value)
        .currency = UCase(contractTable(id, Col_CURRENCY).value)
        .localSymbol = UCase(contractTable(id, Col_LOCALSYMBOL).value)
        .conId = contractTable(id, Col_CONID).value
        
        .includeExpired = queryTable(id, Col_INCLUDEEXPIRED).value
    End With
    
    Dim sheetName As String
    sheetName = newPageTable(id, Col_SHEETNAME).value
    If (sheetName = STR_EMPTY And newPageTable(id, Col_SHEETACTIVATE).value) Or _
        IsNumeric(sheetName) Then
        newPageTable(id, Col_SHEETNAME).value = GenerateSheetName(lContractInfo)
    End If
    
    ' set request status
    queryTable(id, Col_STATUS).value = STR_PROCESSING
    
    Api.Tws.ReqContractDetailsEx id + ID_CONTRACTDETAILS, lContractInfo
    
    ActiveCell.Offset(1, 0).Activate
End Sub

' update contract details
Public Sub UpdateContractDetails(ByVal reqId As Long, ByVal ContractDetails As TWSLib.IContractDetails)
        
    reqId = reqId - ID_CONTRACTDETAILS
    
    Dim sheetName As String
    sheetName = newPageTable(reqId, Col_SHEETNAME).value

    If sheetName = STR_EMPTY Or IsNumeric(sheetName) Then Exit Sub
    
    Dim sheet As Worksheet
    Dim needsInitialising As Boolean
    Set sheet = FindOrAddSheet(sheetName, needsInitialising)
    If needsInitialising Then InitialiseSheet sheet
    
    Dim rowId As Long
    rowId = GetSheetRowId(sheet)
    
    Dim contract As TWSLib.IContract
    Set contract = ContractDetails.contract
    Dim i As Integer
    Dim ineligibilityReason As TWSLib.ComIneligibilityReason
    Dim ineligibilityReasons As String
    
    With sheet.Rows(rowId)
        .Cells(1).value = contract.conId
        .Cells(2).value = contract.Symbol
        .Cells(3).value = contract.SecType
        .Cells(4).value = contract.lastTradeDateOrContractMonth
        .Cells(5).value = contract.lastTradeDate
        .Cells(6).value = contract.Strike
        .Cells(7).value = contract.Right
        .Cells(8).value = contract.multiplier
        .Cells(9).value = contract.Exchange
        .Cells(10).value = contract.primaryExchange
        .Cells(11).value = contract.currency
        .Cells(12).value = contract.localSymbol
        .Cells(13).value = contract.tradingClass
        .Cells(14).value = ContractDetails.marketName
        .Cells(15).value = ContractDetails.minTick
        .Cells(16).value = ContractDetails.priceMagnifier
        .Cells(17).value = ContractDetails.orderTypes
        .Cells(18).value = ContractDetails.validExchanges
        .Cells(19).value = ContractDetails.underConId
        .Cells(20).value = ContractDetails.longName
        .Cells(21).value = ContractDetails.contractMonth
        .Cells(22).value = ContractDetails.industry
        .Cells(23).value = ContractDetails.Category
        .Cells(24).value = ContractDetails.subcategory
        .Cells(25).value = ContractDetails.timeZoneId
        .Cells(26).value = ContractDetails.tradingHours
        .Cells(27).value = ContractDetails.liquidHours
        .Cells(28).value = ContractDetails.evRule
        .Cells(29).value = ContractDetails.evMultiplier
        .Cells(30).value = ContractDetails.aggGroup
        .Cells(31).value = ContractDetails.underSymbol
        .Cells(32).value = ContractDetails.underSecType
        .Cells(33).value = ContractDetails.marketRuleIds
        .Cells(34).value = ContractDetails.realExpirationDate
        .Cells(35).value = ContractDetails.stockType
        .Cells(36).value = Util.DecimalToString(ContractDetails.minSize)
        .Cells(37).value = Util.DecimalToString(ContractDetails.sizeIncrement)
        .Cells(38).value = Util.DecimalToString(ContractDetails.suggestedSizeIncrement)
        .Cells(39).value = ContractDetails.fundName
        .Cells(40).value = ContractDetails.fundFamily
        .Cells(41).value = ContractDetails.fundType
        .Cells(42).value = ContractDetails.fundFrontLoad
        .Cells(43).value = ContractDetails.fundBackLoad
        .Cells(44).value = ContractDetails.fundBackLoadTimeInterval
        .Cells(45).value = ContractDetails.fundManagementFee
        .Cells(46).value = ContractDetails.fundClosed
        .Cells(47).value = ContractDetails.fundClosedForNewInvestors
        .Cells(48).value = ContractDetails.fundClosedForNewMoney
        .Cells(49).value = ContractDetails.fundNotifyAmount
        .Cells(50).value = ContractDetails.fundMinimumInitialPurchase
        .Cells(51).value = ContractDetails.fundSubsequentMinimumPurchase
        .Cells(52).value = ContractDetails.fundBlueSkyStates
        .Cells(53).value = ContractDetails.fundBlueSkyTerritories
        .Cells(54).value = ContractDetails.FundDistributionPolicyIndicator
        .Cells(55).value = ContractDetails.FundAssetType
        For i = 1 To ContractDetails.ineligibilityReasonList.Count
            Set ineligibilityReason = ContractDetails.ineligibilityReasonList.Item(i - 1)
            ineligibilityReasons = ineligibilityReasons & "[id: " & ineligibilityReason.id & ", description: " & ineligibilityReason.Description & "];"
        Next
        .Cells(56).value = ineligibilityReasons
        
    End With
    
    IncrementSheetRowId sheet
End Sub

Private Sub InitialiseSheet(sheet As Worksheet)
    sheet.Cells.ClearContents

    InitialiseSheetRowId sheet, 3
    
    sheet.Cells(2, 1).value = "ConId"
    sheet.Cells(2, 2).value = "Symbol"
    sheet.Cells(2, 3).value = "SecType"
    sheet.Cells(2, 4).value = "LastTradeDateOrContractMonth"
    sheet.Cells(2, 5).value = "LastTradeDate"
    sheet.Cells(2, 6).value = "Strike"
    sheet.Cells(2, 7).value = "P/C"
    sheet.Cells(2, 8).value = "Multiplier"
    sheet.Cells(2, 9).value = "Exchange"
    sheet.Cells(2, 10).value = "PrimExchange"
    sheet.Cells(2, 11).value = "Currency"
    sheet.Cells(2, 12).value = "Local Sym"
    sheet.Cells(2, 13).value = "Trading Class"
    
    sheet.Cells(2, 14).value = "Market Name"
    sheet.Cells(2, 15).value = "Min Tick"
    sheet.Cells(2, 16).value = "Price Magnifier"
    sheet.Cells(2, 17).value = "Order Types"
    sheet.Cells(2, 18).value = "Exchanges"
    sheet.Cells(2, 19).value = "Under ConId"
    sheet.Cells(2, 20).value = "Long Name"
    sheet.Cells(2, 21).value = "Contract Month"
    sheet.Cells(2, 22).value = "Industry"
    sheet.Cells(2, 23).value = "Category"
    sheet.Cells(2, 24).value = "Subcategory"
    sheet.Cells(2, 25).value = "Time Zone Id"
    sheet.Cells(2, 26).value = "Trading Hours"
    sheet.Cells(2, 27).value = "Liquid Hours"
    sheet.Cells(2, 28).value = "EvRule"
    sheet.Cells(2, 29).value = "EvMultiplier"
    sheet.Cells(2, 30).value = "Agg Group"
    sheet.Cells(2, 31).value = "Under Symbol"
    sheet.Cells(2, 32).value = "Under SecType"
    sheet.Cells(2, 33).value = "Market Rule Ids"
    sheet.Cells(2, 34).value = "RealExpirationDate"
    sheet.Cells(2, 35).value = "Stock Type"
    sheet.Cells(2, 36).value = "Min Size"
    sheet.Cells(2, 37).value = "Size Incr"
    sheet.Cells(2, 38).value = "Sugg Size Incr"
    sheet.Cells(2, 39).value = "Fund Name"
    sheet.Cells(2, 40).value = "Fund Family"
    sheet.Cells(2, 41).value = "Fund Type"
    sheet.Cells(2, 42).value = "Fund Front Load"
    sheet.Cells(2, 43).value = "Fund Back Load"
    sheet.Cells(2, 44).value = "Fund Back Load Time Interval"
    sheet.Cells(2, 45).value = "Fund Management Fee"
    sheet.Cells(2, 46).value = "Fund Closed"
    sheet.Cells(2, 47).value = "Fund Closed For New Investors"
    sheet.Cells(2, 48).value = "Fund Closed For New Money"
    sheet.Cells(2, 49).value = "Fund Notify Amount"
    sheet.Cells(2, 50).value = "Fund Minimum Initial Purchase"
    sheet.Cells(2, 51).value = "Fund Subsequent Minimum Purchase"
    sheet.Cells(2, 52).value = "Fund Blue Sky States"
    sheet.Cells(2, 53).value = "Fund Blue Sky Territories"
    sheet.Cells(2, 54).value = "Fund Distribution Policy Indicator"
    sheet.Cells(2, 55).value = "Fund Asset Type"
    sheet.Cells(2, 56).value = "Ineligibility Reasons"
End Sub


Public Sub ContractDetailsEnd(ByVal reqId As Long)
    ' finishing
    reqId = reqId - ID_CONTRACTDETAILS
    
    queryTable(reqId, Col_STATUS).value = STR_FINISHED
    
    Dim sheetName As String
    sheetName = newPageTable(reqId, Col_SHEETNAME).value
    If sheetName <> STR_EMPTY And Not IsNumeric(sheetName) Then
        Dim sheet As Worksheet
        Set sheet = Worksheets(sheetName)
        ClearSheetRowId sheet
        
        If newPageTable(reqId, Col_SHEETACTIVATE).value Then sheet.Activate
    End If
End Sub


Public Sub ProcessError(ByVal id As Long, ByVal errorCode As Long, ByVal errorMsg As String)
    
    queryTable(id - ID_CONTRACTDETAILS, Col_STATUS).value = STR_ERROR + STR_COLON + str(errorCode) + STR_SPACE + errorMsg
        
End Sub

Private Function GenerateSheetName(contractInfo As TWSLib.IContract)
    Dim s As String
    s = "Details" & GenerateContractIdentifier(contractInfo)
    GenerateSheetName = Left(s, 31)
End Function

Public Sub Initialise()
    Set contractTable = Range("$A$7:$K$130")
    Set queryTable = Range("$L$7:$M$130")
    Set newPageTable = Range("$N$7:$O$130")
End Sub

Private Sub Worksheet_Activate()
    Main.Initialise
    
    Dim macroName As String
    macroName = "ContractDetails.RequestContractDetails_Click"
    
    Application.OnKey "+^c", macroName
End Sub

Private Sub Worksheet_Deactivate()
    Application.OnKey "+^c"
End Sub




