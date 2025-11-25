package com.example.environmentsurveillance;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.context.properties.EnableConfigurationProperties;

import com.example.environmentsurveillance.config.NanoClientProperties;

@SpringBootApplication
@EnableConfigurationProperties(NanoClientProperties.class)
public class EnvironmentSurveillanceApplication {

    public static void main(String[] args) {
        SpringApplication.run(EnvironmentSurveillanceApplication.class, args);
    }
}
