#include "esphome.h"

#ifndef SLIDING_WINDOW_LIMITER_H
#define SLIDING_WINDOW_LIMITER_H

class Addition {
public:
    float quantity;
    unsigned long timeAdded;

    Addition(float q = 0.0, unsigned long t = 0) : quantity(q), timeAdded(t) {}
};

class SlidingWindowLimiter {
public:
    SlidingWindowLimiter(float limit, unsigned long durationHours, size_t maxSize = 100) : 
        maxQueueSize(maxSize), 
        totalAddedOnPeriod(0.0), 
        queueStart(0), 
        queueEnd(0), 
        limitOverPeriod(limit), 
        duration(durationHours * 60UL * 60UL * 1000UL) {
        additions = new Addition[maxQueueSize];
    }

    ~SlidingWindowLimiter() {
        delete[] additions;
    }

    bool addIfPossible(float quantity) {
        unsigned long currentTime = millis();
        cleanupAndRefresh(currentTime);

        if (totalAddedOnPeriod + quantity <= limitOverPeriod) {
            if ((queueEnd + 1) % maxQueueSize == queueStart) {
                // Queue is full. So returning false.
                ESP_LOGD("custom", "addIfPossible : queue is full");
                return false;
            }

            additions[queueEnd] = Addition(quantity, currentTime);
            queueEnd = (queueEnd + 1) % maxQueueSize;
            totalAddedOnPeriod += quantity;
            ESP_LOGD("custom", "addIfPossible : added. new total : %f, limit %f", 
                totalAddedOnPeriod, limitOverPeriod
            );
            // ok added.
            return true;
        } else {
            // refused
            ESP_LOGD("custom", "addIfPossible : refused. total: %f , try to add: %f, limit: %f",
                totalAddedOnPeriod, quantity, limitOverPeriod
            );
            return false;
        }
    }

    void setLimit(float newLimit) {
        ESP_LOGD("custom", "setlimit : limit: %f -> %f total: %f", limitOverPeriod, newLimit, totalAddedOnPeriod);
        limitOverPeriod = newLimit;
    }

    void setDuration(unsigned long newDurationHours) {
        duration = newDurationHours * 60UL * 60UL * 1000UL;
    }

    float getTotalAddedOnPeriod(){
        return totalAddedOnPeriod;
    }

private:
    Addition* additions;
    size_t maxQueueSize;
    float totalAddedOnPeriod;
    int queueStart;
    int queueEnd;
    float limitOverPeriod;
    unsigned long duration;

    void cleanupAndRefresh(unsigned long currentTime) {
        while (queueStart != queueEnd) {
            Addition &front = additions[queueStart];
            if (currentTime - front.timeAdded >= duration) {
                totalAddedOnPeriod -= front.quantity;
                queueStart = (queueStart + 1) % maxQueueSize;
            } else {
                break;
            }
        }
    }
};

#endif /* SLIDING_WINDOW_LIMITER_H */