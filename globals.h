#include "sliding_window_limiter.h"

/*
 * globals used in lambdas
 * as there is not possible to create them from esphome globals directive
 * at this time.
 */

// nutrient_limiter
SlidingWindowLimiter* nutrient_limiter = new SlidingWindowLimiter(0.0, 24);

// nutrient_limiter
SlidingWindowLimiter* ph_minus_limiter = new SlidingWindowLimiter(0.0, 24);