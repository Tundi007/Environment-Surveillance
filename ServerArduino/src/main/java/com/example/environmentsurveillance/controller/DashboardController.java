package com.example.environmentsurveillance.controller;

import java.util.List;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;

import com.example.environmentsurveillance.model.Reading;
import com.example.environmentsurveillance.repository.ReadingRepository;
import com.example.environmentsurveillance.service.NanoSyncException;
import com.example.environmentsurveillance.service.NanoSyncService;

@Controller
public class DashboardController {

    private final ReadingRepository repository;
    private final NanoSyncService syncService;

    public DashboardController(ReadingRepository repository, NanoSyncService syncService) {
        this.repository = repository;
        this.syncService = syncService;
    }

    @GetMapping("/login")
    public String login() {
        return "login";
    }

    @GetMapping("/")
    public String dashboard(Model model) {
        List<Reading> readings = repository.findTop100ByOrderByRecordedAtDesc();
        model.addAttribute("readings", readings);
        return "dashboard";
    }

    @PostMapping("/sync")
    public String triggerSync(Model model) {
        try {
            var report = syncService.syncFromNano();
            model.addAttribute("flashMessage",
                    "Fetched " + report.importedCount() + " new readings from the Nano.");
        } catch (NanoSyncException ex) {
            model.addAttribute("flashError", ex.getMessage());
        }
        return dashboard(model);
    }
}
