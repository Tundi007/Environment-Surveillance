package com.example.climate.controller;

import com.example.climate.dto.ClimateReportRequest;
import com.example.climate.model.ClimateRecord;
import com.example.climate.service.ClimateService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.format.annotation.DateTimeFormat;
import org.springframework.http.ResponseEntity;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;

import javax.validation.Valid;
import java.time.*;
import java.time.format.DateTimeFormatter;

@RestController
@RequestMapping("/api")
@Validated
public class ReportController {

    private final ClimateService service;
    private final Logger log = LoggerFactory.getLogger(ReportController.class);
    private final DateTimeFormatter INPUT_FMT = DateTimeFormatter.ofPattern("MM/dd/yyyy");

    public ReportController(ClimateService service) {
        this.service = service;
    }

    @PostMapping("/report")
    public ResponseEntity<?> receiveReport(@Valid @RequestBody ClimateReportRequest req) {
        log.info("Received report from device {}", req.getDeviceId());
        // Parse timestamp in MM/dd/yyyy. store at start of day (00:00)
        LocalDateTime timestamp;
        try {
            LocalDate date = LocalDate.parse(req.getTimestamp(), INPUT_FMT);
            timestamp = date.atStartOfDay();
        } catch (Exception ex) {
            // Try ISO parse as fallback
            try {
                timestamp = LocalDateTime.parse(req.getTimestamp());
            } catch (Exception ex2) {
                log.warn("Failed to parse timestamp: {}", req.getTimestamp());
                return ResponseEntity.badRequest().body(java.util.Map.of(
                        "status", "error",
                        "message", "timestamp parse error, expected MM/dd/yyyy or ISO date-time"
                ));
            }
        }

        ClimateRecord rec = new ClimateRecord(timestamp, req.getTemperature(), req.getHumidity(), req.getGas());
        service.save(rec);
        return ResponseEntity.ok(java.util.Map.of("status","ok","message","recorded"));
    }
}
