A project on developing a webserver to work with microcontrollers and electronic modules, basically IoT.

## What was added
- Spring Boot (Tomcat) webserver with JSP UI, PostgreSQL storage, simple in-memory auth.
- TCP fetch routine that asks the Nano for every stored reading, ingests them, and wipes Nano storage when done.
- Updated Arduino sketches to support the fetch protocol and full-duplex forwarding through the ESP01 endpoint.

## Spring Boot server (ServerArduino/)
- Build/run: `mvn spring-boot:run` (Java 17+). Maven is needed locally.
- Default login: `admin / changeme` (set in `app.auth.*`).
- JSP views live under `src/main/webapp/WEB-INF/jsp/` (login + dashboard).
- Data model: `Reading` (sensor, rawValue, recordedAt) persisted via Spring Data JPA.

### Configuration (`ServerArduino/src/main/resources/application.properties`)
- PostgreSQL: `spring.datasource.url=jdbc:postgresql://<host>:5432/envsurv`, plus username/password.
- Gateway to Nano (ESP01 endpoint): `nano.gateway.host` (default `192.168.4.1`), `nano.gateway.port` (default `5050`), plus timeouts.
- Auth: `app.auth.username`, `app.auth.password` (in-memory, form login).

### How sync works
1. From the dashboard, hit **Sync now**. The server opens a TCP socket to the ESP01 endpoint.
2. It sends `FETCH` and waits for the Nano to stream buffered values.
3. Nano replies:
   - `BEGIN <count>`
   - Zero or more `DATA:<value>` lines (raw MQ135 values).
   - `END` (after which the Nano wipes its EEPROM buffer).
4. The server stores every value in PostgreSQL and renders the last 100 readings on the dashboard.

## Arduino firmware changes
### ESP01 Wifi Node (`Arduino/ESP01WifiNode/ESP01WifiNode.ino`)
- Still reads MQ135 on A0 and sends `MQ135:<raw>` to the hub every 5s.
- Now closes the TCP connection after each send so the server can grab the line when it needs to fetch.

### ESP01 Wifi Endpoint (`Arduino/ESP01WifiEndpoint/ESP01WifiEndpoint.ino`)
- Acts as SoftAP + TCP bridge on port 5050.
- Forwards client lines to the Nano, and forwards Nano responses back to the active TCP client.
- Accepts a new incoming client by dropping the previous one (keeps things simple).

### Arduino Nano (`Arduino/ArduinoNano/ArduinoNano.ino`)
- Stores raw MQ135 readings in EEPROM (ring buffer of 500 values).
- Commands over the ESP serial link:
  - `MQ135:<value>` or `<value>`: store reading (0-1023).
  - `FETCH`: emit `BEGIN <count>`, each `DATA:<value>`, then `END` and wipe buffer.
  - `CLEAR`: wipe buffer immediately.
  - `PING`: respond `PONG`.

## Wiring / flow recap
- ESP01 Wifi Node reads the sensor and pushes values to the ESP01 Wifi Endpoint.
- The Endpoint forwards values to the Nano for storage.
- When requested, the Spring Boot server connects to the Endpoint, asks for `FETCH`, and ingests + wipes the Nano buffer.
