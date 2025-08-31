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

    ~SlidingWindowLimiter() 
    {
        delete[] additions;
    }

    bool addIfPossible(float quantity) 
    {
        cleanupAndRefresh(millis());

        if (totalAddedOnPeriod + quantity <= limitOverPeriod) {

            if(!add(quantity)){
                return false;
            }

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

    float addAllowed(float quantity) 
    {
        float toAdd = getAllowed(quantity);
        
        if(toAdd <= 0.0){
            ESP_LOGD("custom", "addAllowed : asked : %f, added %f, new total : %f, limit %f",
                quantity, 0.0, totalAddedOnPeriod, limitOverPeriod
            );
            return 0.0;
        }

        if(!add(quantity)){
            return 0.0;
        }

        ESP_LOGD("custom", "addAllowed : asked : %f, added %f, new total : %f, limit %f",
            quantity, toAdd, totalAddedOnPeriod, limitOverPeriod
        );


        return toAdd;
    }

    float getAllowed(float quantity)
    {
        cleanupAndRefresh(millis());

        float maxAllowed = std::fmaxf(0.0f, limitOverPeriod - totalAddedOnPeriod);

        return std::fminf(maxAllowed, quantity);
    }

    void setLimit(float newLimit) 
    {
        ESP_LOGD("custom", "setlimit : limit: %f -> %f total: %f", limitOverPeriod, newLimit, totalAddedOnPeriod);
        limitOverPeriod = newLimit;
    }

    void setDuration(unsigned long newDurationHours) 
    {
        duration = newDurationHours * 60UL * 60UL * 1000UL;
    }

    float getTotalAddedOnPeriod()
    {
        return totalAddedOnPeriod;
    }

    void reset()
    {
        totalAddedOnPeriod = 0.0;
        queueStart = 0;
        queueEnd = 0;
    }

private:
    Addition* additions;
    size_t maxQueueSize;
    float totalAddedOnPeriod;
    int queueStart;
    int queueEnd;
    float limitOverPeriod;
    unsigned long duration;

    void cleanupAndRefresh(unsigned long currentTime) 
    {
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

    /**
     * @return false if queue is full
     */
    bool add(float quantity) 
    {
        
        if ((queueEnd + 1) % maxQueueSize == queueStart) {
            // Queue is full. So returning false.
            ESP_LOGD("custom", "add : queue is full");
            return false;
        }

        additions[queueEnd] = Addition(quantity, millis());
        queueEnd = (queueEnd + 1) % maxQueueSize;
        totalAddedOnPeriod += quantity;

        return true;
    }
};

#endif /* SLIDING_WINDOW_LIMITER_H */