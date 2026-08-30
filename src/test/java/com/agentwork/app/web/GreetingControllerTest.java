package com.agentwork.app.web;

import static org.assertj.core.api.Assertions.assertThat;

import java.time.Clock;
import java.time.Instant;
import java.time.ZoneOffset;

import org.junit.jupiter.api.Test;

/**
 * Unit tests for {@link GreetingController}.
 *
 * <p>The controller is exercised directly with a fixed {@link Clock} so the assertions are
 * deterministic and do not require a running web server.</p>
 */
class GreetingControllerTest {

    /** Fixed instant (epoch millis) used to make timestamp assertions deterministic. */
    private static final long FIXED_EPOCH_MILLIS = 1_700_000_000_000L;

    private final Clock fixedClock =
            Clock.fixed(Instant.ofEpochMilli(FIXED_EPOCH_MILLIS), ZoneOffset.UTC);

    private final GreetingController controller = new GreetingController(fixedClock);

    /**
     * A supplied non-blank name is echoed back and used to build the greeting message.
     */
    @Test
    void greetsSuppliedName() {
        GreetingResponse response = controller.hello("Cursor");

        assertThat(response.name()).isEqualTo("Cursor");
        assertThat(response.message()).isEqualTo("Hello, Cursor!");
        assertThat(response.timestamp()).isEqualTo(FIXED_EPOCH_MILLIS);
    }

    /**
     * A missing or blank name falls back to the default greeting target.
     */
    @Test
    void fallsBackToDefaultNameWhenBlank() {
        GreetingResponse response = controller.hello("   ");

        assertThat(response.name()).isEqualTo("World");
        assertThat(response.message()).isEqualTo("Hello, World!");
    }
}
