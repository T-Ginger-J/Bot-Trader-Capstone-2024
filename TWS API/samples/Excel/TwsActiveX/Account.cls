VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "Account"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = True
Option Explicit

'=================
' local constants
'=================
' table constants

Const CELL_ACCOUNT_NAME = "A6"
Const CELL_FA_DATATYPE = "C6"
Const CELL_TIMESTAMP = "B9"
Const CELL_MANAGEDACCOUNTS = "D9"
Const CELL_USERINFO = "F9"
Const CELL_SUBSCRIPTION_STATUS = "A11"
Const RANGE_ACCOUNT_DATA = "A15:D300"

Private Enum AccountColumns
    Col_ACCT_KEY = 1
    Col_ACCT_VALUE
    Col_ACCT_CURRENCY
    Col_ACCT_NAME
End Enum

' other constants
Const FA_DATATYPE_GROUPS = 1
Const FA_DATATYPE_ACCOUNT_ALIASES = 3

Const STR_FA_CONFIG = "Financial Advisor configuration"

Private faDataTypeValue As Long

Private accountsTable As Range

'=================
' private methods
'=================
' request FA
Private Sub RequestFA_Click()
    If Not CheckConnected Then Exit Sub
    
    Api.Tws.RequestFA Range(CELL_FA_DATATYPE).value
End Sub


' Request Managed Account button was clicked
Private Sub RequestManagedAccounts_Click()
    If Not CheckConnected Then Exit Sub
    
    Api.Tws.ReqManagedAccts
End Sub

'unsubsribe to stop receiving account updates
Private Sub CancelAccountUpdates_Click()
    CancelAccountDataSubscription
End Sub

' clear account data table
Private Sub ClearAccountData_Click()
    If IsConnected Then CancelAccountDataSubscription
    
    ' account values
    accountsTable.ClearContents
    ' timestamp
    Range(CELL_TIMESTAMP).value = STR_EMPTY
    ' managed accounts
    Range(CELL_MANAGEDACCOUNTS).value = STR_EMPTY
End Sub

Private Sub CancelAccountDataSubscription()
    If Not CheckConnected Then Exit Sub
    
    Dim accountName As String
    accountName = Range(CELL_ACCOUNT_NAME).value
    
    ' update subscription status
    Range(CELL_SUBSCRIPTION_STATUS).value = STR_EMPTY
    
    Api.Tws.ReqAccountUpdates False, accountName
End Sub

' subscribe to account updates
Private Sub RequestAccountUpdates_Click()
    If Not CheckConnected Then Exit Sub
    
    Dim accountName As String
    accountName = Range(CELL_ACCOUNT_NAME).value
    
    ' update subscription status
    Range(CELL_SUBSCRIPTION_STATUS).value = STR_SUBSCRIBED
    ' clear account values
    accountsTable.ClearContents
    ' clear timestamp
    Range(CELL_TIMESTAMP).value = STR_EMPTY
    
    ' clear Portfolio sheet
    Portfolio.ClearPortfolioData_Click
    
    Api.Tws.ReqAccountUpdates True, accountName
End Sub

' find row in account table by key and currency
Private Function FindRowByKeyAndCurrency(accountKey As String, accountCurrency As String)
    Dim i As Long
    i = 1
    Do While (Not (accountsTable(i, Col_ACCT_KEY).value Like accountKey And _
            accountsTable(i, Col_ACCT_CURRENCY).value Like accountCurrency) And _
            accountsTable(i, Col_ACCT_KEY).value <> STR_EMPTY And _
            i <= accountsTable.Rows.Count)
        i = i + 1
    Loop
    FindRowByKeyAndCurrency = i
End Function

'=================
' public methods
'=================
' update timestamp value
Public Sub UpdateTimeStamp(timeStamp As String)
    Range(CELL_TIMESTAMP).value = timeStamp
End Sub

' update account values
Public Sub UpdateAccountValue(accountKey As String, accountValue As String, accountCurrency As String, accountName As String)
    Dim rowId As Long
    rowId = FindRowByKeyAndCurrency(accountKey, accountCurrency)
    
    accountsTable(rowId, Col_ACCT_KEY).value = accountKey
    accountsTable(rowId, Col_ACCT_VALUE).value = accountValue
    accountsTable(rowId, Col_ACCT_CURRENCY).value = accountCurrency
    accountsTable(rowId, Col_ACCT_NAME).value = accountName
End Sub

' update managed accounts string
Public Sub UpdateManagedAccounts(accountsList As String)
    Range(CELL_MANAGEDACCOUNTS).value = accountsList
End Sub

' show / modify FA
Public Sub UpdateFA(ByVal faDataType As Long, ByVal cxml As String)
    Dim captionStr As String
    captionStr = STR_FA_CONFIG
    faDataTypeValue = faDataType
    
    Select Case faDataTypeValue
        Case FA_DATATYPE_GROUPS
            captionStr = captionStr & " - Groups"
        Case FA_DATATYPE_ACCOUNT_ALIASES
            captionStr = captionStr & " - Account Aliases"
    
    End Select
    
    ScrollableMsgBox.ShowText cxml, captionStr, True, Account.name
End Sub

' replace FA
Public Sub ReplaceText(text As String)
    If Not CheckConnected Then Exit Sub
    
    Api.Tws.replaceFA 0, faDataTypeValue, text
End Sub

Public Sub Initialise()
    Set accountsTable = Me.Range("$A$14:$D$300")
End Sub

Private Sub ReqUserInfo_Click()
    If Not CheckConnected Then Exit Sub
    
    Api.Tws.ReqUserInfo 0
End Sub

Private Sub Worksheet_Activate()
    Main.Initialise
End Sub

Public Sub UserInfo(ByVal reqId As Long, ByVal whiteBrandingId As String)
    ' user info
    Range(CELL_USERINFO).value = whiteBrandingId
End Sub

