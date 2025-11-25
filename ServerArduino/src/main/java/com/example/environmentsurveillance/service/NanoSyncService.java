package com.example.environmentsurveillance.service;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import com.example.environmentsurveillance.config.NanoClientProperties;
import com.example.environmentsurveillance.model.Reading;
import com.example.environmentsurveillance.repository.ReadingRepository;

@Service
public class NanoSyncService {

    private static final Logger log = LoggerFactory.getLogger(NanoSyncService.class);
    private static final String COMMAND_FETCH = "FETCH";

    private final NanoClientProperties props;
    private final ReadingRepository repository;

    public NanoSyncService(NanoClientProperties props, ReadingRepository repository) {
        this.props = props;
        this.repository = repository;
    }

    @Transactional
    public SyncReport syncFromNano() {
        List<Integer> values = fetchValues();
        if (values.isEmpty()) {
            return new SyncReport(Collections.emptyList());
        }
        LocalDateTime now = LocalDateTime.now();
        List<Reading> readings = values.stream()
                .map(v -> new Reading("MQ135", v, now))
                .toList();
        List<Reading> saved = repository.saveAll(readings);
        return new SyncReport(saved);
    }

    private List<Integer> fetchValues() {
        try (Socket socket = new Socket()) {
            socket.connect(new InetSocketAddress(props.getHost(), props.getPort()), props.getConnectTimeoutMs());
            socket.setSoTimeout(props.getReadTimeoutMs());

            try (PrintWriter writer = new PrintWriter(
                    new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8), true);
                 BufferedReader reader = new BufferedReader(
                         new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8))) {

                writer.println(COMMAND_FETCH);
                return readLines(reader);
            }
        } catch (IOException e) {
            throw new NanoSyncException("Failed to reach Nano gateway at "
                    + props.getHost() + ":" + props.getPort(), e);
        }
    }

    private List<Integer> readLines(BufferedReader reader) throws IOException {
        List<Integer> values = new ArrayList<>();
        boolean seenBegin = false;
        String line;
        while ((line = reader.readLine()) != null) {
            line = line.trim();
            if (line.isEmpty()) {
                continue;
            }
            if (line.startsWith("ERR:")) {
                throw new NanoSyncException("Nano reported error: " + line.substring(4));
            }
            if (line.toUpperCase().startsWith("BEGIN")) {
                seenBegin = true;
                continue;
            }
            if ("END".equalsIgnoreCase(line)) {
                break;
            }
            if (!seenBegin) {
                // Ignore stray text until the frame begins.
                continue;
            }
            if (line.startsWith("DATA:")) {
                try {
                    int raw = Integer.parseInt(line.substring(5).trim());
                    values.add(raw);
                } catch (NumberFormatException nfe) {
                    log.warn("Skipping unparsable line from Nano: {}", line);
                }
            }
        }
        return values;
    }

    public record SyncReport(List<Reading> imported) {
        public int importedCount() {
            return imported.size();
        }
    }
}
