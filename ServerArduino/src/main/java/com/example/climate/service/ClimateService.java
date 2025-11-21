package com.example.climate.service;

import com.example.climate.model.ClimateRecord;
import com.example.climate.repository.ClimateRecordRepository;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;

@Service
public class ClimateService {

    private final ClimateRecordRepository repo;

    public ClimateService(ClimateRecordRepository repo) {
        this.repo = repo;
    }

    public ClimateRecord save(ClimateRecord record) {
        return repo.save(record);
    }

    public List<ClimateRecord> findAll() {
        return repo.findAll();
    }

    public List<ClimateRecord> findBetween(LocalDateTime start, LocalDateTime end) {
        return repo.findByTimestampBetweenOrderByTimestampAsc(start, end);
    }
}
