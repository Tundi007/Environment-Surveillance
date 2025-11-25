package com.example.climate.controller;

import com.example.climate.model.ClimateRecord;
import com.example.climate.service.ClimateService;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

import java.time.format.DateTimeFormatter;
import java.util.*;
import java.util.stream.Collectors;

@Controller
public class ClimateController {

    private final ClimateService service;
    private final DateTimeFormatter outFmt = DateTimeFormatter.ofPattern("MM/dd/yyyy HH:mm");

    public ClimateController(ClimateService service) {
        this.service = service;
    }

    @GetMapping("/api/climate")
    public String showClimate(Model model) {
        List<ClimateRecord> all = service.findAll().stream()
                .sorted(Comparator.comparing(ClimateRecord::getTimestamp))
                .collect(Collectors.toList());

        List<String> labels = all.stream()
                .map(r -> r.getTimestamp().format(outFmt))
                .collect(Collectors.toList());

        List<Double> temps = all.stream().map(ClimateRecord::getTemperature).collect(Collectors.toList());
        List<Double> hums = all.stream().map(ClimateRecord::getHumidity).collect(Collectors.toList());
        List<Double> gases = all.stream().map(ClimateRecord::getGas).collect(Collectors.toList());

        model.addAttribute("labels", labels);
        model.addAttribute("temps", temps);
        model.addAttribute("hums", hums);
        model.addAttribute("gases", gases);
        return "climate";
    }
}
