Climate Spring Boot Project
===========================

What is included
- Maven-based Spring Boot project (packaging WAR) with JSP frontend.
- Endpoints:
  - POST /api/report  - receives JSON report from microcontrollers.
  - GET  /api/climate - returns JSP page with three charts (temperature, humidity, gas).
  - POST /api/summary - placeholder AI summary endpoint.

How timestamps are handled
- The POST /api/report expects `timestamp` in `MM/dd/yyyy` format (time is stored as start of day).
- The backend will also attempt ISO date-time parse as fallback.

Database
- Configured for PostgreSQL in src/main/resources/application.properties.
- JPA auto-creates/updates the table `climate_records`.
- Change datasource settings as required.

Build and run
- Requires Java 17 and Maven.
- To build: `mvn clean package`
- To run locally (embedded Tomcat): `mvn spring-boot:run`
- Deploy WAR to an external Tomcat if desired.

Notes
- JSP uses Chart.js via CDN. The Generate Summary button calls a placeholder endpoint.
- This project is a scaffold. Extend validation, security, and AI integration as needed.
