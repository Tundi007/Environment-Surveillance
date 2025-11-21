package com.example.climate.dto;

import javax.validation.constraints.NotNull;

public class ClimateReportRequest {

    @NotNull
    private String deviceId;

    @NotNull
    private String timestamp; // expected MM/dd/yyyy

    @NotNull
    private Double temperature;

    @NotNull
    private Double humidity;

    @NotNull
    private Double gas;

    public ClimateReportRequest() {}

    public String getDeviceId() { return deviceId; }
    public void setDeviceId(String deviceId) { this.deviceId = deviceId; }

    public String getTimestamp() { return timestamp; }
    public void setTimestamp(String timestamp) { this.timestamp = timestamp; }

    public Double getTemperature() { return temperature; }
    public void setTemperature(Double temperature) { this.temperature = temperature; }

    public Double getHumidity() { return humidity; }
    public void setHumidity(Double humidity) { this.humidity = humidity; }

    public Double getGas() { return gas; }
    public void setGas(Double gas) { this.gas = gas; }
}
