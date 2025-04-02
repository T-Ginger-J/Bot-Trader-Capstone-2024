/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

namespace IBSampleApp.messages
{
    class OrderBoundMessage
    {
        public long PermId { get; private set; }
        public int ClientId { get; private set; }
        public int OrderId { get; private set; }

        public OrderBoundMessage(long permId, int clientId, int orderId)
        {
            PermId = permId;
            ClientId = clientId;
            OrderId = orderId;
        }
    }
}
