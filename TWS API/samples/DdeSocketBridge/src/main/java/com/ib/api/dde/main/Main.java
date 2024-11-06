/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

package com.ib.api.dde.main;

import com.ib.api.dde.utils.DdeSocketBridgePrintStream;
import com.pretty_tools.dde.DDEException;

/** Main class with entry point */
public class Main {

    private static final String DEFAULT_DDE_SERVICE_NAME = "Stwsserver";
    private static final String DEFAULT_TWS_HOST = "127.0.0.1";
    private static final int DEFAULT_TWS_PORT = 7496;
    private static final int DEFAULT_TWS_CLIENT_ID = 0;
    private static final String DEFAULT_TWS_CONNECT_OPTIONS = "+PACEAPI";

    private static String s_ddeServiceName = DEFAULT_DDE_SERVICE_NAME;
    private static String s_host = DEFAULT_TWS_HOST;
    private static int s_port = DEFAULT_TWS_PORT;
    private static int s_clientId = DEFAULT_TWS_CLIENT_ID;
    private static String s_connectOptions = DEFAULT_TWS_CONNECT_OPTIONS;

    /** Main */
    public static void main(String[] args) throws InterruptedException, DDEException {
        try {
            System.setOut(new DdeSocketBridgePrintStream(System.out));
            parseCommandLineArguments(args);
            
            MainFrame mainFrame = new MainFrame();
            mainFrame.setVisible(true);
            mainFrame.start(s_ddeServiceName, s_host, s_port, s_clientId, s_connectOptions);
        } catch (Exception e) {
            System.out.println("Failed! " + e);
        }
    }

    /** Method parses command line arguments */
    private static boolean parseCommandLineArguments(String[] args) {
        boolean ret = true;
        for (int i = 0; i < args.length; i++) {
            
            if (args[i].length() > 2 && args[i].charAt(0) == '-') {
                switch(args[i].charAt(1)) {
                    case 'h':
                        s_host = args[i].substring(2);
                        break;
                    case 'p':
                        s_port = Integer.parseInt(args[i].substring(2));
                        break;
                    case 'c':
                        s_clientId = Integer.parseInt(args[i].substring(2));
                        break;
                    case 's':
                        s_ddeServiceName = "S" + args[i].substring(2);
                        break;
                    case 'o':
                        s_connectOptions = args[i].substring(2);
                        break;
                    default:
                        ret = false;
                }
            } else {
                ret = false;
            }
        }
        if (!ret) {
            System.out.println("Invalid command line arguments");
            showArgs();
        }
        
        return ret;
    }    

    /** Method shows command line arguments */
    private static void showArgs() {
        System.out.println("Valid command line arguments: -h<host> -p<port> -c<clientId> -s<servicename> -o<connectOptions>, e.g. -h127.0.0.1 -p7496 -c0 -stwsserver -o+PACEAPI");
    }
}
