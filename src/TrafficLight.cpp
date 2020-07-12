#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this]{return !_queue.empty();});
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    T message = std::move(_queue.front());
    // The received object should then be returned by the receive function.
    _queue.pop_front();

    return message;

}


template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.emplace_back(msg);
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    _condition.notify_one();

}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    while(true){
      auto currentphase = _messagequeue.receive();  
      // Once it receives TrafficLightPhase::green, the method returns.
        if(currentphase == green){
            return;
        }
    }
    
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}



void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

//virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(4, 6);

    const auto cycle_duration = std::chrono::seconds(dis(gen));

    // Init stop watch
    auto lastupdate = std::chrono::system_clock::now();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // Compute time difference to stop watch
        auto time_difference = std::chrono::system_clock::now()-lastupdate;
        if(time_difference >= cycle_duration){

            // Toggle between red and green
            if (_currentPhase == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;
            }
            else
            {
                _currentPhase = TrafficLightPhase::red;
            }

            // Send update message to queue using move semantics
              _messagequeue.send(std::move(_currentPhase));

            // Reset stop watch
            lastupdate = std::chrono::system_clock::now();
        }
    }
}

