function [func_status] = executeBot(botPath)
    %EXECUTEORDER Summary of this function goes here
    %   Detailed explanation goes here
    
    % Define the path to the executable
    executablePath = 'MatlabInterface.exe';
    
    % Construct the command.
    % Quotation marks are added around the paths to handle spaces properly.
    commandStr = sprintf('"%s" "%s"', executablePath, botPath);
    
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

