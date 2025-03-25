function dataTable = readTradeRecords()
    % Read the CSV file into a table
    dataTable = readtable('TradeRecords.csv');
    % Assign in MATLAB environment
    assignin('base', 'TradeRecords', dataTable);
end
