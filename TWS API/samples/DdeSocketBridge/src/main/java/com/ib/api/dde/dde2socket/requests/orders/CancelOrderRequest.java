/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

package com.ib.api.dde.dde2socket.requests.orders;

import com.ib.api.dde.dde2socket.requests.DdeRequest;
import com.ib.api.dde.dde2socket.requests.DdeRequestType;
import com.ib.client.OrderCancel;

/** Class represents DDE cancel order request (cancelOrder) */
public class CancelOrderRequest extends DdeRequest {

    private final OrderCancel m_orderCancel;

    // gets
    public OrderCancel orderCancel() { return m_orderCancel; }

    public CancelOrderRequest(int requestId, OrderCancel orderCancel, String ddeRequestString) {
        super(requestId, DdeRequestType.CANCEL_ORDER, ddeRequestString);
        m_orderCancel = orderCancel;
    }
}
