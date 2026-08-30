package com.agentwork.app.web;

import java.time.Clock;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

/**
 * REST controller exposing the greeting endpoints of the agentWork reference service.
 *
 * <p>These endpoints provide a real, exercisable request/response flow so the development
 * environment can be verified end to end (build, run, and answer an HTTP request).</p>
 */
@RestController
public class GreetingController {

    /** Default name used when the caller does not supply a {@code name} query parameter. */
    private static final String DEFAULT_NAME = "World";

    /** Clock used to timestamp responses; injectable to keep the controller unit-testable. */
    private final Clock clock;

    /**
     * Creates a controller backed by the system UTC clock.
     */
    public GreetingController() {
        this(Clock.systemUTC());
    }

    /**
     * Creates a controller backed by the supplied clock.
     *
     * @param clock clock used to stamp responses; must not be {@code null}
     */
    GreetingController(Clock clock) {
        this.clock = clock;
    }

    /**
     * Returns a friendly greeting as JSON.
     *
     * @param name optional name to greet; falls back to {@value #DEFAULT_NAME} when blank or absent
     * @return a {@link GreetingResponse} containing the greeting, the resolved name and a timestamp
     */
    @GetMapping("/api/hello")
    public GreetingResponse hello(@RequestParam(name = "name", required = false) String name) {
        String resolvedName = (name == null || name.isBlank()) ? DEFAULT_NAME : name.strip();
        String message = "Hello, " + resolvedName + "!";
        return new GreetingResponse(message, resolvedName, clock.millis());
    }
}
