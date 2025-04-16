function func_status = createPosition(savePath, frontLegOptionType, frontLegExpiryDate, frontLegStrikeOffset, frontLegAction, backLegOptionType, backLegExpiryDate, backLegStrikeOffset, backLegAction, ordersType, takeProfit, stopLoss)
%CREATEPOSITION
%   Return Values:
%       func_status
%   Parameters:
%       path
%       front leg - tuple(option{call,put}, expiry_date, strike_offset, action{buy,sell})
%       back leg - tuple(option{call,put}, expiry_date, strike_offset, action{buy,sell})
%       orders type {percentage, offset}
%       take_profit
%       stop_loss

% Validate Inputs

    isNumericScalar = @(value) isnumeric(value) && isscalar(value);
    isIntegerScalar = @(value) isinteger(value) && isscalar(value);
    refisValidOptionType = @(value) isstring(value) && isValidOptionType(value);
    refisValidAction = @(value) isstring(value) && isValidAction(value);
    refisValidOrdersType = @(value) isstring(value) && isValidOrdersType(value);

p = inputParser;
addRequired(p, "savePath", @isstring);
addRequired(p, "frontLegOptionType", refisValidOptionType);
addRequired(p, "frontLegExpiryDate", isIntegerScalar);
addRequired(p, "frontLegStrikeOffset", isNumericScalar);
addRequired(p, "frontLegAction", refisValidAction);
addRequired(p, "backLegOptionType", refisValidOptionType);
addRequired(p, "backLegExpiryDate", isIntegerScalar);
addRequired(p, "backLegStrikeOffset", isNumericScalar);
addRequired(p, "backLegAction", refisValidAction);
addRequired(p, "ordersType", refisValidOrdersType);
addRequired(p, "takeProfit", isNumericScalar);
addRequired(p, "stopLoss", isNumericScalar);
parse(p, savePath, frontLegOptionType, frontLegExpiryDate, frontLegStrikeOffset, frontLegAction, backLegOptionType, backLegExpiryDate, backLegStrikeOffset, backLegAction, ordersType, takeProfit, stopLoss)

executablePath = 'MatlabInterface.exe';

% Construct the command.
% Quotation marks are added around the paths to handle spaces properly.
commandStr = sprintf('"%s" "%d" "%s" "%s" "%d" "%0.9f" "%s" "%s" "%d" "%0.9f" "%s" "%s" "%0.9f" "%0.9f"', executablePath, 3, savePath, frontLegOptionType, frontLegExpiryDate, frontLegStrikeOffset, frontLegAction, backLegOptionType, backLegExpiryDate, backLegStrikeOffset, backLegAction, ordersType, takeProfit, stopLoss);

% Execute the command
[status, cmdOutput] = system(commandStr);

% Check and display the results
if status == 0
    fprintf('Execution successful:\n%s\n', cmdOutput);
else
    fprintf('Execution failed with status %d:\n%s\n', status, cmdOutput);
end

func_status = status;

end