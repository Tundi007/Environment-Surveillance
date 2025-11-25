package com.example.environmentsurveillance.repository;

import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import com.example.environmentsurveillance.model.Reading;

@Repository
public interface ReadingRepository extends JpaRepository<Reading, Long> {
    List<Reading> findTop100ByOrderByRecordedAtDesc();
}
