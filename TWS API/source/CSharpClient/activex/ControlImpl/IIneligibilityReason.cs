/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace TWSLib
{
    [ComVisible(true)]
    [Guid("3A0D5891-EC6B-4AD3-BB46-A3C5728DA4DF")]
    public interface IIneligibilityReason
    {
        [DispId(1)]
        string id { get; set; }
        [DispId(2)]
        string description { get; set; }
    }
}
