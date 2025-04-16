function func_status = createBot(file_path, positionPaths, times, numbers)
    % Validate inputs
    if ~ischar(file_path)
        error('file_path must be a string.');
    end
    if ~iscellstr(positionPaths) || ~iscellstr(times)
        error('positionPaths and times must be cell arrays of strings.');
    end
    if ~isnumeric(numbers) || ~isvector(numbers)
        error('numbers must be an array of integers.');
    end
    if numel(positionPaths) ~= numel(times) || numel(times) ~= numel(numbers)
        error('positionPaths, times, and numbers must all have the same number of elements.');
    end
    
    % Construct the command string
    command_str = sprintf('MatlabInterface.exe 2 %s', file_path);
    for i = 1:numel(positionPaths)
        command_str = sprintf('%s %s %s %d', command_str, positionPaths{i}, times{i}, numbers(i));
    end
    
    % Execute the command
    [status, cmdOutput] = system(command_str);
    
    % Check and display the results
    if status == 0
        fprintf('Execution successful:\n%s\n', cmdOutput);
    else
        fprintf('Execution failed with status %d:\n%s\n', status, cmdOutput);
    end
    
    func_status = status;
end