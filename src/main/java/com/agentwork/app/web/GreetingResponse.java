package com.agentwork.app.web;

/**
 * Immutable payload returned by the greeting endpoint.
 *
 * @param message   human-readable greeting text
 * @param name      the name that was greeted
 * @param timestamp epoch-millisecond instant at which the greeting was produced
 */
public record GreetingResponse(String message, String name, long timestamp) {
}
