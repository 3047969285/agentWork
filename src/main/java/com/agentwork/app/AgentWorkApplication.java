package com.agentwork.app;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

/**
 * Application entry point for the agentWork reference service.
 *
 * <p>This service exists to validate that the Cloud Agent development environment can build,
 * test and run a real Spring Boot web application end to end.</p>
 */
@SpringBootApplication
public class AgentWorkApplication {

    /**
     * Boots the Spring application context and starts the embedded web server.
     *
     * @param args standard Java process arguments forwarded to Spring Boot
     */
    public static void main(String[] args) {
        SpringApplication.run(AgentWorkApplication.class, args);
    }
}
