/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using IBApi;
using System.Collections;

namespace TWSLib
{
    /**
     * @class OrderCancel
     */
    [ComVisible(true), ClassInterface(ClassInterfaceType.None)]
    public class ComOrderCancel : ComWrapper<OrderCancel>, TWSLib.IOrderCancel
    {        

        public override bool Equals(Object p_other)
        {
            if (!(p_other is ComOrderCancel))
                return false;

            return data.Equals((p_other as ComOrderCancel).data);
        }

        string TWSLib.IOrderCancel.manualOrderCancelTime { get { return data.ManualOrderCancelTime; } set { data.ManualOrderCancelTime = value; } }
        string TWSLib.IOrderCancel.extOperator { get { return data.ExtOperator; } set { data.ExtOperator = value; } }
        int TWSLib.IOrderCancel.manualOrderIndicator { get { return data.ManualOrderIndicator; } set { data.ManualOrderIndicator = value; } }

        public static explicit operator ComOrderCancel(IBApi.OrderCancel oc)
        {
            return new ComOrderCancel().ConvertFrom(oc) as ComOrderCancel;
        }

        public static explicit operator IBApi.OrderCancel(ComOrderCancel coc)
        {
            return coc.ConvertTo();
        }
    }
}
