/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

package com.ib.api.dde.dde2socket.requests.orders;

import com.ib.client.Contract;
import com.ib.api.dde.dde2socket.requests.DdeRequest;
import com.ib.api.dde.dde2socket.requests.DdeRequestType;

/** Class represents DDE exercise options request (exerciseOptions) */
public class ExerciseOptionsRequest extends DdeRequest {

    private final Contract m_contract;
    private final String m_account;
    private final int m_exerciseAction;
    private final int m_exerciseQuantity;
    private final int m_override;
    private final String m_manualOrderTime;
    private final String m_customerAccount;
    private final boolean m_professionalCustomer;
    
    // gets
    public Contract contract()    { return m_contract; }
    public String account()       { return m_account; }
    public int exerciseAction()   { return m_exerciseAction; }
    public int exerciseQuantity() { return m_exerciseQuantity; }
    public int override()         { return m_override; }
    public String manualOrderTime() { return m_manualOrderTime; }
    public String customerAccount() { return m_customerAccount; }
    public boolean professionalCustomer() { return m_professionalCustomer; }

    public ExerciseOptionsRequest(int requestId, Contract contract, String account, int exerciseAction, int exerciseQuantity, int override,
            String manualOrderTime, String customerAccount, boolean professionalCustomer, String ddeRequestString) {
        super(requestId, DdeRequestType.EXERCISE_OPTIONS, ddeRequestString);
        m_contract = contract;
        m_account = account;
        m_exerciseAction = exerciseAction;
        m_exerciseQuantity = exerciseQuantity;
        m_override = override;
        m_manualOrderTime = manualOrderTime;
        m_customerAccount = customerAccount;
        m_professionalCustomer = professionalCustomer;
    }
}
