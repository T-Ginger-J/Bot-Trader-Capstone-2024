function func_status = executeBot(TWShost, TWSport, connectionOptionsString, botPath)
%EXECUTEBOT
%   Return Values:
%       func_status
%   Parameters:
%       host
%       port
%       connectionOptions
%       botPath

% Validate Inputs
p = inputParser;
addRequired(p, "TWShost", @isstring);
addRequired(p, "TWSport", @isstring);
addRequired(p, "connectionOpstionsString", @isstring);
addRequired(p, "botPath", @isstring);
parse(p, TWShost, TWSport, connectionOptionsString, botPath)

executablePath = 'MatlabInterface.exe';

% Construct the command.
% Quotation marks are added around the paths to handle spaces properly.
% "1" indicates execute function
commandStr = sprintf('"%s" "%s" "%s" "%s" "%s" "%s"', executablePath, "1", TWShost, TWSport, connectionOptionsString, botPath);

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