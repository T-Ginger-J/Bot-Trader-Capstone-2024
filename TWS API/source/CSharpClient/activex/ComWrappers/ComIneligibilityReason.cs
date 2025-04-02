/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

using IBApi;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace TWSLib
{
    [ComVisible(true), ClassInterface(ClassInterfaceType.None)]
    public class ComIneligibilityReason : ComWrapper<IneligibilityReason>, IIneligibilityReason
    {
        public ComIneligibilityReason() { }
        public ComIneligibilityReason(string id, string description)
        {
            data.Id = id;
            data.Description = description;
        }

        public string Id { get { return data != null ? data.Id : default(string); } set { if (data != null) data.Id = value; } }
        public string Description { get { return data != null ? data.Description : default(string); } set { if (data != null) data.Description = value; } }

        public static explicit operator IneligibilityReason(ComIneligibilityReason cir)
        {
            return cir.ConvertTo();
        }

        public static explicit operator ComIneligibilityReason(IneligibilityReason ir)
        {
            return new ComIneligibilityReason().ConvertFrom(ir) as ComIneligibilityReason;
        }

        string IIneligibilityReason.id
        {
            get
            {
                return Id;
            }
            set
            {
                Id = value;
            }
        }

        string IIneligibilityReason.description
        {
            get
            {
                return Description;
            }
            set
            {
                Description = value;
            }
        }
    }
}
