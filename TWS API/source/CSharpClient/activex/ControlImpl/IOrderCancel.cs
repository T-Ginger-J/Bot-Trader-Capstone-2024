/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

using System;
using System.Runtime.InteropServices;

namespace TWSLib
{
    [ComVisible(true), Guid("8A434142-C7C8-43EE-9870-DB72B897CBE7")]
    public interface IOrderCancel
    {
        [DispId(1)]
        string manualOrderCancelTime { get; set; }

        [DispId(2)]
        string extOperator { get; set; }

        [DispId(3)]
        int manualOrderIndicator { get; set; }

    }
}
