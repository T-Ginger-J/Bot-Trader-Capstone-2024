/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Linq;

namespace TWSLib
{
    [ComVisible(true), Guid("13744AAD-4D03-4B7F-A270-D1E05DE4F0E6")]
    public interface IIneligibilityReasonList
    {
        [DispId(-4)]
        object _NewEnum { [return: MarshalAs(UnmanagedType.IUnknown)] get; }
        [DispId(0)]
        object this[int index] { [return: MarshalAs(UnmanagedType.IDispatch)] get; }
        [DispId(1)]
        int Count { get; }
        [DispId(2)]
        [return: MarshalAs(UnmanagedType.IDispatch)]
        object AddEmpty();
        [DispId(3)]
        [return: MarshalAs(UnmanagedType.IDispatch)]
        object Add(string id, string description);
    }

    [ComVisible(true), ClassInterface(ClassInterfaceType.None)]
    public class ComIneligibilityReasonList : IIneligibilityReasonList
    {
        private ComList<ComIneligibilityReason, IBApi.IneligibilityReason> Irl;

        public ComIneligibilityReasonList() : this(null) { }
        public ComIneligibilityReasonList(ComList<ComIneligibilityReason, IBApi.IneligibilityReason> irl)
        {
            this.Irl = irl == null ? new ComList<ComIneligibilityReason, IBApi.IneligibilityReason>(new List<IBApi.IneligibilityReason>()) : irl;
        }

        public object _NewEnum
        {
            get { return Irl.GetEnumerator(); }
        }

        public object this[int index]
        {
            get { return Irl[index]; }
        }

        public int Count
        {
            get { return Irl.Count; }
        }

        public object AddEmpty()
        {
            var rval = new ComIneligibilityReason();
            Irl.Add(rval);
            return rval;
        }

        public object Add(string id, string description)
        {
            var rval = new ComIneligibilityReason(id, description);
            Irl.Add(rval);
            return rval;
        }

        public static implicit operator List<IBApi.IneligibilityReason>(ComIneligibilityReasonList from)
        {
            return from.Irl.ConvertTo();
        }
    }
}
