# agentWork

Minimal Java 21 + Spring Boot reference service. It exists to give the repository a real,
runnable application so the development environment can be verified end to end (build, test,
run, and answer an HTTP request).

## Tech stack

- Java 21
- Spring Boot 3.3.5 (`spring-boot-starter-web`, `spring-boot-starter-actuator`)
- Maven, invoked through the committed Maven Wrapper (`./mvnw`) — no system Maven required
- JUnit 5 + AssertJ for tests

## Requirements

- JDK 21 (available on the Cloud Agent default image)
- Network access to Maven Central on first build (to download the Maven distribution and dependencies)

## Common commands

```bash
# Compile + run unit tests
./mvnw -B test

# Build the runnable jar (skips tests)
./mvnw -B -DskipTests clean package

# Run the service (development)
./mvnw -B spring-boot:run

# Or run the packaged jar
java -jar target/agent-work-0.0.1-SNAPSHOT.jar
```

The service listens on port `8080`.

## Endpoints

| Method | Path | Description |
| --- | --- | --- |
| GET | `/api/hello` | Returns a JSON greeting. Optional `name` query parameter (defaults to `World`). |
| GET | `/actuator/health` | Spring Boot Actuator health probe. |

### Examples

```bash
curl http://localhost:8080/api/hello
# {"message":"Hello, World!","name":"World","timestamp":...}

curl "http://localhost:8080/api/hello?name=Cursor"
# {"message":"Hello, Cursor!","name":"Cursor","timestamp":...}

curl http://localhost:8080/actuator/health
# {"status":"UP","groups":["liveness","readiness"]}
```

## Cloud Agent environment

`.cursor/environment.json` configures the Cloud Agent development environment:

- `install`: `./mvnw -B -e -DskipTests clean package` — warms the Maven distribution and
  dependency cache and verifies the project compiles and packages.
- `terminals.app`: `./mvnw -B spring-boot:run` — runs the service on port `8080`.
- `ports`: exposes port `8080`.
