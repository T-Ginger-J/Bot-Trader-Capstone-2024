/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

using System;
using System.Windows.Forms;

using IBApi;

namespace IBSampleApp.ui
{
    partial class MarginDialog : Form
    {
        delegate void UpdateMarginInformationDelegate(OrderState state);

        public MarginDialog()
        {
            InitializeComponent();
        }

        public void FillAndDisplay(OrderState state, Order order)
        {
            equityWithLoanBefore.Text = Util.formatDoubleString(state.EquityWithLoanBefore);
            initialMarginBefore.Text = Util.formatDoubleString(state.InitMarginBefore);
            maintenanceMarginBefore.Text = Util.formatDoubleString(state.MaintMarginBefore);
            equityWithLoanChange.Text = Util.formatDoubleString(state.EquityWithLoanChange);
            initialMarginChange.Text = Util.formatDoubleString(state.InitMarginChange);
            maintenanceMarginChange.Text = Util.formatDoubleString(state.MaintMarginChange);
            equityWithLoanAfter.Text = Util.formatDoubleString(state.EquityWithLoanAfter);
            initialMarginAfter.Text = Util.formatDoubleString(state.InitMarginAfter);
            maintenanceMarginAfter.Text = Util.formatDoubleString(state.MaintMarginAfter);
            bondAccruedInterest.Text = order.BondAccruedInterest;
        }

        public void UpdateMarginInformation(OrderState state, Order order)
        {
            FillAndDisplay(state, order);
            ShowDialog();  
        }

        private void acceptMarginButton_Click(object sender, EventArgs e)
        {
            equityWithLoanBefore.Text = "";
            initialMarginBefore.Text = "";
            maintenanceMarginBefore.Text = "";
            equityWithLoanChange.Text = "";
            initialMarginChange.Text = "";
            maintenanceMarginChange.Text = "";
            equityWithLoanAfter.Text = "";
            initialMarginAfter.Text = "";
            maintenanceMarginAfter.Text = "";
            bondAccruedInterest.Text = "";
            Visible = false;
        }

    }
}
