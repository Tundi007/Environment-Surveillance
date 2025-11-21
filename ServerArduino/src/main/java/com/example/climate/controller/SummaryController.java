package com.example.climate.controller;

import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api")
public class SummaryController {

    @PostMapping("/summary")
    public ResponseEntity<?> generateSummary() {
        // Placeholder. Integrate AI summarization later.
        return ResponseEntity.ok(java.util.Map.of(
                "summary", "AI summarization not implemented on this server."
        ));
    }
}
