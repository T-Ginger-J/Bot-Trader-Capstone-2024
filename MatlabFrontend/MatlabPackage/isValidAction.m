function bool = isValidAction(action)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
if (action == "buy" || action == "sell")
    bool = true;
    return
else
    bool = false;
    return
end
end

