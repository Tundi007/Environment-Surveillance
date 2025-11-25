package com.example.climate.repository;

import com.example.climate.model.ClimateRecord;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.List;

@Repository
public interface ClimateRecordRepository extends JpaRepository<ClimateRecord, Long> {
    List<ClimateRecord> findByTimestampBetweenOrderByTimestampAsc(LocalDateTime start, LocalDateTime end);
}
