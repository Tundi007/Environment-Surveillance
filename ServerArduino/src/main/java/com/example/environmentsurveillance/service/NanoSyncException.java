package com.example.environmentsurveillance.service;

public class NanoSyncException extends RuntimeException {
    public NanoSyncException(String message) {
        super(message);
    }

    public NanoSyncException(String message, Throwable cause) {
        super(message, cause);
    }
}
