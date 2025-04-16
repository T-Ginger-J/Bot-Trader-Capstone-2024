function bool = isValidOptionType(optionType)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
if (optionType == "call" || optionType == "put")
    bool = true;
    return
else
    bool = false;
    return
end
end

