package com.example.climate.model;

import javax.persistence.*;
import java.time.LocalDateTime;

@Entity
@Table(name = "climate_records")
public class ClimateRecord {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(name="record_time", nullable = false)
    private LocalDateTime timestamp;

    @Column(nullable = false)
    private Double temperature;

    @Column(nullable = false)
    private Double humidity;

    @Column(nullable = false)
    private Double gas;

    public ClimateRecord() {}

    public ClimateRecord(LocalDateTime timestamp, Double temperature, Double humidity, Double gas) {
        this.timestamp = timestamp;
        this.temperature = temperature;
        this.humidity = humidity;
        this.gas = gas;
    }

    public Long getId() { return id; }
    public LocalDateTime getTimestamp() { return timestamp; }
    public void setTimestamp(LocalDateTime timestamp) { this.timestamp = timestamp; }
    public Double getTemperature() { return temperature; }
    public void setTemperature(Double temperature) { this.temperature = temperature; }
    public Double getHumidity() { return humidity; }
    public void setHumidity(Double humidity) { this.humidity = humidity; }
    public Double getGas() { return gas; }
    public void setGas(Double gas) { this.gas = gas; }
}
