function bool = isValidOrdersType(ordersType)
%ISVALIDORDERSTYPE Summary of this function goes here
%   Detailed explanation goes here
if (ordersType == "offset" || ordersType == "percentage")
    bool = true;
    return
else
    bool = false;
    return
end
end

